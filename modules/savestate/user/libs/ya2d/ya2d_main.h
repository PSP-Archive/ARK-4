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

#ifndef _YA2D_MAIN_H_
#define _YA2D_MAIN_H_

#define BUF_WIDTH  (512)
#define SCR_WIDTH  (480)
#define SCR_HEIGHT (272)

#define uS_PER_SEC (1000000)

#define YA2D_GU_LIST_SIZE 262144


int ya2d_init();
int ya2d_shutdown();

void ya2d_start_drawing();
void ya2d_finish_drawing();
void ya2d_clear_screen(unsigned int color);
void ya2d_swapbuffers();
void ya2d_calc_fps();
void ya2d_set_vsync(int enabled);

void *ya2d_get_drawbuffer();
float ya2d_get_fps();



#endif
