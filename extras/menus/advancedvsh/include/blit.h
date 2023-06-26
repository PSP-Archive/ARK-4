/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

#ifndef __BLIT_H__
#define __BLIT_H__


#include <psptypes.h>


#define COLOR_CYAN	0x00ffff00
#define COLOR_MAGENDA 0x00ff00ff
#define COLOR_YELLOW  0x0000ffff

#define RGB(R,G,B)	(((B)<<16)|((G)<<8)|(R))
#define RGBT(R,G,B,T) (((T)<<24)|((B)<<16)|((G)<<8)|(R))


typedef struct _blit_Gfx{
	u32 *vram32;
	u32 fg_color, bg_color;
	
	int width, height;
	
	int bufferwidth;
	int pixelformat;
}blit_Gfx;



blit_Gfx* blit_gfx_pointer(void);
int blit_setup(void);
void blit_set_color(int fg_col, int bg_col);
int blit_string(int sx, int sy, const char *msg);
int blit_string_ctr(int sy, const char *msg);
void blit_rect_fill(int sx, int sy, int w, int h);

int load_external_font(const char *file);
void release_font(void);


// Returns size of string in pixels
int blit_get_string_width(char *msg);

#endif
