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

#ifndef _YA2D_DRAW_H_
#define _YA2D_DRAW_H_

void ya2d_draw_pixel(int x, int y, unsigned int color);
void ya2d_draw_line(int x0, int y0, int x1, int y1, unsigned int color);

void ya2d_draw_rect(int x, int y, int w, int h, unsigned int color, int filled);
void ya2d_draw_rect_rotate(int x, int y, int w, int h, unsigned int color, int filled, float angle);
void ya2d_draw_rect_rotate_hotspot(int x, int y, int w, int h, unsigned int color, int filled, float angle, int center_x, int center_y);


#endif
