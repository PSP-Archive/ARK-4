/*
    libya2d
    Copyright (C) 2013  Sergi (xerpi) Granell (xerpi.g.12@gmail.com)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "ya2d_texture.h"
#include "ya2d_utils.h"
#include <pspgu.h>
#include <pspgum.h>
#include <psputils.h>
#include <pspmath.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <vram.h>

static struct ya2d_texture *ya2d_create_texture_common(int width, int height, int pixel_format)
{
    struct ya2d_texture *texture = (struct ya2d_texture *)malloc(sizeof(struct ya2d_texture));
    if (texture == NULL) return NULL;
    
    texture->width  = width;
    texture->height = height;
    texture->pow2_w = next_pow2(width);
    texture->pow2_h = next_pow2(height);
    texture->pixel_format = pixel_format;
    texture->swizzled = GU_FALSE;
    texture->has_alpha = GU_TRUE;
    
    switch (pixel_format) {
    case GU_PSM_5650:
    case GU_PSM_5551:
    case GU_PSM_4444:
        texture->stride = texture->pow2_w * 2;
        break;
    case GU_PSM_8888:
    default:
        texture->stride = texture->pow2_w * 4;
        break;
    }
    
    texture->data_size = texture->stride * texture->pow2_h;
    return texture;
}


struct ya2d_texture *ya2d_create_texture(int width, int height, int pixel_format, int place)
{
    struct ya2d_texture *texture = ya2d_create_texture_common(width, height, pixel_format);
    if (texture != NULL) {
        //If there's not enough space in the VRAM, then allocate it in the RAM
        if ((place == YA2D_PLACE_RAM) || (texture->data_size > vlargestblock())) {
            texture->data = memalign(16, texture->data_size);
            texture->place = YA2D_PLACE_RAM;
        } else if (place == YA2D_PLACE_VRAM){
            texture->data = valloc(texture->data_size);
            texture->place = YA2D_PLACE_VRAM;
        }
        memset(texture->data, 0, texture->data_size);
        ya2d_flush_texture(texture);
        return texture;
    }
    return NULL;
}

struct ya2d_texture *ya2d_create_empty_texture(int width, int height, int pixel_format)
{
    struct ya2d_texture *texture = ya2d_create_texture_common(width, height, pixel_format);
    if (texture) {
        texture->place = YA2D_PLACE_UNK;
        return texture;
    }
    return NULL;
}


void ya2d_free_texture(struct ya2d_texture *texture)
{
    if (texture->place == YA2D_PLACE_RAM) {
        free(texture->data);
    } else if (texture->place == YA2D_PLACE_VRAM){
        vfree(texture->data);
    }
    free(texture);
}

void ya2d_set_texture(struct ya2d_texture *texture)
{
    sceGuEnable(GU_TEXTURE_2D);
    sceGuTexMode(texture->pixel_format, 0, 0, texture->swizzled);
    sceGuTexImage(0, texture->pow2_w, texture->pow2_h,
                  texture->pow2_w, texture->data);
    sceGuTexFunc(GU_TFX_REPLACE, texture->has_alpha ? GU_TCC_RGBA : GU_TCC_RGB);
    sceGuTexFilter(GU_NEAREST, GU_NEAREST);   
}

static inline void _ya2d_draw_texture_slow(struct ya2d_texture *texture, int x, int y, int center_x, int center_y)
{    
    struct ya2d_vertex_2s3s *vertices = sceGuGetMemory(2 * sizeof(struct ya2d_vertex_2s3s));
    
    vertices[0].u = 0;
    vertices[0].v = 0;
    vertices[0].x = x - center_x;
    vertices[0].y = y - center_y;
    vertices[0].z = 0;
    
    vertices[1].u = texture->width;
    vertices[1].v = texture->height;
    vertices[1].x = x + (texture->width  - center_x);
    vertices[1].y = y + (texture->height - center_y);
    vertices[1].z = 0;
    
    sceGumDrawArray(GU_SPRITES, GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D, 2, 0, vertices);
}

static void _ya2d_draw_texture_fast(struct ya2d_texture *texture, int x, int y, int center_x, int center_y)
{    
    int i, k, slice, n_slices = texture->width/YA2D_TEXTURE_SLICE;
    if (texture->width%YA2D_TEXTURE_SLICE != 0) ++n_slices;
    struct ya2d_vertex_2s3s *vertices = sceGuGetMemory(2*n_slices * sizeof(struct ya2d_vertex_2s3s));
    for (k = 0, i = 0, slice = 0; i < n_slices; k += 2, ++i, slice += YA2D_TEXTURE_SLICE) {
        vertices[k].u = slice;
        vertices[k].v = 0;
        vertices[k].x = x+slice - center_x;
        vertices[k].y = y - center_y;
        vertices[k].z = 0;
        
        register int j = k+1;
        vertices[j].u = slice+YA2D_TEXTURE_SLICE;
        vertices[j].v = texture->height;
        vertices[j].x = x+slice+YA2D_TEXTURE_SLICE - center_x;
        vertices[j].y = y + texture->height - center_y;
        vertices[j].z = 0;
    }

    sceGumDrawArray(GU_SPRITES, GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D, 2*n_slices, 0, vertices);
}

void ya2d_draw_texture(struct ya2d_texture *texture, int x, int y)
{
    ya2d_draw_texture_hotspot(texture, x, y, 0, 0); 
}

void ya2d_draw_texture_blend(struct ya2d_texture *texture, int x, int y, unsigned int color)
{
    ya2d_set_texture(texture);
    
    struct ya2d_vertex_1ui2s3s *vertices = sceGuGetMemory(2 * sizeof(struct ya2d_vertex_1ui2s3s));
    
    vertices[0].color = color;
    vertices[0].u = 0;
    vertices[0].v = 0;
    vertices[0].x = x;
    vertices[0].y = y;
    vertices[0].z = 0;
    
    vertices[1].color = color;
    vertices[1].u = texture->width;
    vertices[1].v = texture->height;
    vertices[1].x = x + texture->width;
    vertices[1].y = y + texture->height;
    vertices[1].z = 0;
    
    sceGumDrawArray(GU_SPRITES, GU_COLOR_8888|GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D, 2, 0, vertices);
}

void ya2d_draw_texture_centered(struct ya2d_texture *texture, int x, int y)
{
    ya2d_draw_texture_hotspot(texture, x, y, texture->width/2, texture->height/2);
}

void ya2d_draw_texture_hotspot(struct ya2d_texture *texture, int x, int y, int center_x, int center_y)
{
    ya2d_set_texture(texture);
    
    //There's no need to use the fast algorithm with small textures
    if (texture->width > YA2D_TEXTURE_SLICE) {
        _ya2d_draw_texture_fast(texture, x, y, center_x, center_y);
    } else {
        _ya2d_draw_texture_slow(texture, x, y, center_x, center_y);
    }    
}

void ya2d_draw_texture_scale(struct ya2d_texture *texture, int x, int y, float scale_x, float scale_y)
{
    ya2d_set_texture(texture);

    struct ya2d_vertex_2s3s *vertices = sceGuGetMemory(2 * sizeof(struct ya2d_vertex_2s3s));
    
    vertices[0].u = 0;
    vertices[0].v = 0;
    vertices[0].x = x;
    vertices[0].y = y;
    vertices[0].z = 0;
    
    vertices[1].u = texture->width;
    vertices[1].v = texture->height;
    vertices[1].x = x + (texture->width)*scale_x;
    vertices[1].y = y + (texture->height)*scale_y;
    vertices[1].z = 0;
    
    sceGumDrawArray(GU_SPRITES, GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D, 2, 0, vertices);
}

void ya2d_draw_texture_rotate(struct ya2d_texture *texture, int x, int y, float angle)
{
    ya2d_draw_texture_rotate_hotspot(texture, x, y, angle, texture->width/2, texture->height/2);
}

void ya2d_draw_texture_rotate_hotspot(struct ya2d_texture *texture, int x, int y, float angle, int center_x, int center_y)
{
    ya2d_set_texture(texture);

    struct ya2d_vertex_2s3s *vertices = sceGuGetMemory(4 * sizeof(struct ya2d_vertex_2s3s));
    
    vertices[0].u = 0;
    vertices[0].v = 0;
    vertices[0].x = -center_x;
    vertices[0].y = -center_y;
    vertices[0].z = 0;
    
    vertices[1].u = texture->width;
    vertices[1].v = 0;
    vertices[1].x = texture->width-center_x;
    vertices[1].y = -center_y;
    vertices[1].z = 0;    

    vertices[2].u = 0;
    vertices[2].v = texture->height;
    vertices[2].x = -center_x;
    vertices[2].y = texture->height-center_y;
    vertices[2].z = 0;
        
    vertices[3].u = texture->width;
    vertices[3].v = texture->height;
    vertices[3].x = vertices[1].x;
    vertices[3].y = vertices[2].y;
    vertices[3].z = 0;
    
    float c = vfpu_cosf(angle);
    float s = vfpu_sinf(angle);
    int i;    
    for (i = 0; i < 4; ++i) {  //Rotate and translate
        int _x = vertices[i].x;
        int _y = vertices[i].y;
        vertices[i].x = _x*c - _y*s + x;
        vertices[i].y = _x*s + _y*c + y;
    }
    
    sceGumDrawArray(GU_TRIANGLE_STRIP, GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D, 4, 0, vertices);
}


void ya2d_flush_texture(struct ya2d_texture *texture)
{
    if (texture)
        sceKernelDcacheWritebackRange(texture->data, texture->data_size);
}

void ya2d_swizzle_texture(struct ya2d_texture *texture)
{
    //There's no need to use swizzle with small textures
    if(texture->swizzled || texture->width < YA2D_TEXTURE_SLICE) return;
    void *tmp = malloc(texture->data_size);
    swizzle_fast(tmp, texture->data, texture->stride, texture->pow2_h);
    memcpy(texture->data, tmp, texture->data_size);
    free(tmp);
    texture->swizzled = 1;
}
