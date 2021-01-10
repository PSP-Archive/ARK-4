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

#include <pspgu.h>
#include <pspgum.h>
#include <pspmath.h>
#include "ya2d_utils.h"


/*
    Filled: 0, 1, 2, 3
    Line border : 0, 1, 3, 2, 0

   0         1
    +-------+
    |       |
    |       |
    +-------+
   2         3
*/


void ya2d_draw_pixel(int x, int y, unsigned int color)
{
    sceGuDisable(GU_TEXTURE_2D);
    struct ya2d_vertex_1ui3s *vertices = sceGuGetMemory(sizeof(struct ya2d_vertex_1ui3s)); 
    vertices->color = color;
    vertices->x = x;
    vertices->y = y;
    vertices->z = 0;
    
    sceGumDrawArray(GU_POINTS, GU_COLOR_8888|GU_VERTEX_16BIT|GU_TRANSFORM_2D, 1, 0, vertices);
}

void ya2d_draw_line(int x0, int y0, int x1, int y1, unsigned int color)
{
    sceGuDisable(GU_TEXTURE_2D);
    struct ya2d_vertex_1ui3s *vertices = sceGuGetMemory(2 * sizeof(struct ya2d_vertex_1ui3s)); 
    vertices[0].color = color;
    vertices[0].x = x0;
    vertices[0].y = y0;
    vertices[0].z = 0;
    
    vertices[1].color = color;
    vertices[1].x = x1;
    vertices[1].y = y1;
    vertices[1].z = 0;
    sceGumDrawArray(GU_LINES, GU_COLOR_8888|GU_VERTEX_16BIT|GU_TRANSFORM_2D, 2, 0, vertices);    
}

inline void _init_vertices_color(struct ya2d_vertex_1ui3s *vertices, unsigned int color)
{
    vertices[0].color = color;
    vertices[0].z = 0;
    vertices[1].color = color;
    vertices[1].z = 0;
    vertices[2].color = color;
    vertices[2].z = 0;
    vertices[3].color = color;
    vertices[3].z = 0; 
}


void ya2d_draw_rect(int x, int y, int w, int h, unsigned int color, int filled)
{
    sceGuDisable(GU_TEXTURE_2D);
    
    #define _set_common_vert(_0,_1,_2,_3) do {  \
                _init_vertices_color(vertices, color); \
                vertices[_0].x = x;  \
                vertices[_0].y = y;  \
                vertices[_1].x = x+w;  \
                vertices[_1].y = y;  \
                vertices[_2].x = x;  \
                vertices[_2].y = y+h;  \
                vertices[_3].x = x+w;  \
                vertices[_3].y = y+h;  \
            } while (0)
    
    if (filled) {
        struct ya2d_vertex_1ui3s *vertices = sceGuGetMemory(4 * sizeof(struct ya2d_vertex_1ui3s)); 
        _set_common_vert(0,1,2,3);
        sceGumDrawArray(GU_TRIANGLE_STRIP, GU_COLOR_8888|GU_VERTEX_16BIT|GU_TRANSFORM_2D, 4, 0, vertices);
    } else {
        struct ya2d_vertex_1ui3s *vertices = sceGuGetMemory(5 * sizeof(struct ya2d_vertex_1ui3s)); 
        _set_common_vert(0,1,3,2);
        
        /* First vertex again to complete line loop */
        vertices[4].color = color;
        vertices[4].x = x;
        vertices[4].y = y;
        vertices[4].z = 0;
            
        sceGumDrawArray(GU_LINE_STRIP, GU_COLOR_8888|GU_VERTEX_16BIT|GU_TRANSFORM_2D, 5, 0, vertices);
    }    
}


inline void _rot_trans_vertices(struct ya2d_vertex_1ui3s *vertices, int x, int y, float angle)
{
    float c = vfpu_cosf(angle);
    float s = vfpu_sinf(angle);
    int i;
    for (i = 0; i < 4; ++i) {  //Rotate and translate
        int _x = vertices[i].x;
        int _y = vertices[i].y;
        vertices[i].x = _x*c - _y*s + x;
        vertices[i].y = _x*s + _y*c + y;
    }
}

void ya2d_draw_rect_rotate_hotspot(int x, int y, int w, int h, unsigned int color, int filled, float angle, int center_x, int center_y)
{
    sceGuDisable(GU_TEXTURE_2D);
    
    #define _set_common_vert_rot(_0,_1,_2,_3) do {  \
                _init_vertices_color(vertices, color); \
                vertices[_0].x = -center_x;  \
                vertices[_0].y = -center_y;  \
                vertices[_1].x = w-center_x;  \
                vertices[_1].y = -center_y;  \
                vertices[_2].x = -center_x;  \
                vertices[_2].y = h-center_y;  \
                vertices[_3].x = w-center_x;  \
                vertices[_3].y = h-center_y;  \
                _rot_trans_vertices(vertices, x, y, angle);  \
            } while (0)
            
    
    if (filled) {
        struct ya2d_vertex_1ui3s *vertices = sceGuGetMemory(4 * sizeof(struct ya2d_vertex_1ui3s)); 
        _set_common_vert_rot(0,1,2,3);
        sceGumDrawArray(GU_TRIANGLE_STRIP, GU_COLOR_8888|GU_VERTEX_16BIT|GU_TRANSFORM_2D, 4, 0, vertices);
        
    } else {
        struct ya2d_vertex_1ui3s *vertices = sceGuGetMemory(5 * sizeof(struct ya2d_vertex_1ui3s)); 
        _set_common_vert_rot(0,1,3,2);

        /* First vertex again to complete line loop */
        vertices[4].color = color;
        vertices[4].x = vertices[0].x;
        vertices[4].y = vertices[0].y;
        vertices[4].z = 0;
            
        sceGumDrawArray(GU_LINE_STRIP, GU_COLOR_8888|GU_VERTEX_16BIT|GU_TRANSFORM_2D, 5, 0, vertices);
    } 
}

void ya2d_draw_rect_rotate(int x, int y, int w, int h, unsigned int color, int filled, float angle)
{
    ya2d_draw_rect_rotate_hotspot(x, y, w, h, color, filled, angle, w/2, h/2);
}

