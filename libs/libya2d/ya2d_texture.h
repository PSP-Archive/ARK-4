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

#ifndef _YA2D_TEXTURE_H_
#define _YA2D_TEXTURE_H_


#define YA2D_TEXTURE_SLICE (32)

enum ya2d_texture_place {
    YA2D_PLACE_RAM,
    YA2D_PLACE_VRAM,
    YA2D_PLACE_UNK
};

struct ya2d_texture {
    int place;               /** RAM or VRAM **/
    int swizzled;            /** 1: swizzled, 0: not swizzled **/
    int has_alpha;
    int pixel_format;        /** Pixel Storage Format **/
    int width, height;       /** Texture width and height (non base 2) **/
    int pow2_w, pow2_h;      /** Texture width and height (base 2) **/
    int stride, data_size;
    void *data;
};

struct ya2d_texture *ya2d_create_texture(int width, int height, int pixel_format, int place);
struct ya2d_texture *ya2d_create_empty_texture(int width, int height, int pixel_format);
void ya2d_free_texture(struct ya2d_texture *texture);
void ya2d_set_texture(struct ya2d_texture *texture);

void ya2d_draw_texture(struct ya2d_texture *texture, int x, int y);
void ya2d_draw_texture_blend(struct ya2d_texture *texture, int x, int y, unsigned int color);
void ya2d_draw_texture_centered(struct ya2d_texture *texture, int x, int y);
void ya2d_draw_texture_hotspot(struct ya2d_texture *texture, int x, int y, int center_x, int center_y);

void ya2d_draw_texture_scale(struct ya2d_texture *texture, int x, int y, float scale_x, float scale_y);

void ya2d_draw_texture_rotate(struct ya2d_texture *texture, int x, int y, float angle);
void ya2d_draw_texture_rotate_hotspot(struct ya2d_texture *texture, int x, int y, float angle, int center_x, int center_y);

void ya2d_flush_texture(struct ya2d_texture *texture);
void ya2d_swizzle_texture(struct ya2d_texture *texture);

#endif
