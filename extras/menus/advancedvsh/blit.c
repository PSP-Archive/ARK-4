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

/*
	PSP VSH 24bpp text bliter
*/
#include "common.h"

#include "vsh.h"

//#define ALPHA_BLEND 1

extern unsigned char msx[];
static unsigned char *g_cur_font = msx;

// extern ARKConfig* ark_config;

extern SceOff findPkgOffset(const char* filename, unsigned* size, const char* pkgpath);

static SceUID g_memid = -1;


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
int pwidth;
int pheight, bufferwidth, pixelformat;
unsigned int* vram32;

u32 fcolor = 0x00ffffff;
u32 bcolor = 0xff000000;

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static u32 adjust_alpha(u32 col)
{
	u32 alpha = col>>24;
	u8 mul;
	u32 c1,c2;

	if(alpha==0)	return col;
	if(alpha==0xff) return col;

	c1 = col & 0x00ff00ff;
	c2 = col & 0x0000ff00;
	mul = (u8)(255-alpha);
	c1 = ((c1*mul)>>8)&0x00ff00ff;
	c2 = ((c2*mul)>>8)&0x0000ff00;
	return (alpha<<24)|c1|c2;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//int blit_setup(int sx,int sy,const char *msg,int fg_col,int bg_col)
int blit_setup(void)
{
	int unk;
	sceDisplayGetMode(&unk, &pwidth, &pheight);
	sceDisplayGetFrameBuf((void*)&vram32, &bufferwidth, &pixelformat, PSP_DISPLAY_SETBUF_NEXTFRAME);
	if( (bufferwidth==0) || (pixelformat!=3)) 
		return -1;

	fcolor = 0x00ffffff;
	bcolor = 0xff000000;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// blit text
/////////////////////////////////////////////////////////////////////////////
void blit_set_color(int fg_col,int bg_col)
{
	fcolor = fg_col;
	bcolor = bg_col;
}

/////////////////////////////////////////////////////////////////////////////
// blit text
/////////////////////////////////////////////////////////////////////////////
int blit_string(int sx,int sy,const char *msg)
{
	int x,y,p;
	int offset;
	u8 code, font;
	u32 fg_col,bg_col;

	u32 col,c1,c2;
	u32 alpha;

	fg_col = adjust_alpha(fcolor);
	bg_col = adjust_alpha(bcolor);


//Kprintf("MODE %d WIDTH %d\n",pixelformat,bufferwidth);
	if ((bufferwidth==0) || (pixelformat!=3))
		return -1;

	for (x = 0; msg[x] && x < (pwidth / 8); x++) {
		code = (u8)msg[x]; // no truncate now

		for (y = 0; y < 8; y++) {
			offset = (sy + y) * bufferwidth + sx + x * 8;
			font = y>=7 ? 0x00 : g_cur_font[code * 8 + y];
			for (p = 0; p < 8; p++){
				col = (font & 0x80) ? fg_col : bg_col;
				alpha = col>>24;
				if (alpha == 0) 
					vram32[offset] = col;
				else if (alpha != 0xff) {
					c2 = vram32[offset];
					c1 = c2 & 0x00ff00ff;
					c2 = c2 & 0x0000ff00;
					c1 = ((c1*alpha)>>8)&0x00ff00ff;
					c2 = ((c2*alpha)>>8)&0x0000ff00;
					vram32[offset] = (col&0xffffff) + c1 + c2;
				}

				font <<= 1;
				offset++;
			}
		}
	}
	return sx + x * 8;
}

int blit_string_ctr(int sy,const char *msg) {
	int sx;
	sx = (pwidth - scePaf_strlen(msg)*8)/2;
	return blit_string(sx, sy, msg);
}


void blit_rect_fill(int sx, int sy, int w, int h){
	int x, y;
	u32 col, c1, c2;
	u32 bg_col;
	u32 offset, alpha;
	bg_col = adjust_alpha(bcolor);
	
	for (y = 0; y < h; y++){
		for (x = 0; x < w; x++){
			col = bg_col;
			alpha = col >> 24;
			offset = (sy + y) * bufferwidth + (sx + x);
			if(alpha == 0)
				vram32[offset] = col;
			else if (alpha != 0xff) {
				c2 = vram32[offset];
				c1 = c2 & 0x00ff00ff;
				c2 = c2 & 0x0000ff00;
				c1 = ((c1 * alpha) >> 8) & 0x00ff00ff;
				c2 = ((c2 * alpha) >> 8) & 0x0000ff00;
				vram32[offset] = (col & 0xffffff) + c1 + c2;
			}
		}
	}
}


int load_external_font(const char *file) {
	SceUID fd;
	int ret;
	void *buf;
	
	vsh_Menu *vsh = vsh_menu_pointer();

	if (file == NULL || file[0] == 0) return -1;

	static char pkgpath[ARK_PATH_SIZE];
	scePaf_strcpy(pkgpath, vsh->config.p_ark->arkpath);
	strcat(pkgpath, "LANG.ARK");

	SceOff offset = findPkgOffset(file, NULL, pkgpath);

	if (offset == 0) return -1;

	fd = sceIoOpen(pkgpath, PSP_O_RDONLY, 0777);

	if(fd < 0) {
		return fd;
	}

	g_memid = sceKernelAllocPartitionMemory(2, "proDebugScreenFontBuffer", PSP_SMEM_High, 2048, NULL);

	if(g_memid < 0) {
		sceIoClose(fd);
		return g_memid;
	}

	buf = sceKernelGetBlockHeadAddr(g_memid);

	if(buf == NULL) {
		sceKernelFreePartitionMemory(g_memid);
		sceIoClose(fd);
		return -2;
	}

	sceIoLseek(fd, offset, PSP_SEEK_SET);
	ret = sceIoRead(fd, buf, 2048);

	if(ret != 2048) {
		sceKernelFreePartitionMemory(g_memid);
		sceIoClose(fd);
		return -3;
	}

	sceIoClose(fd);
	g_cur_font = buf;

	return 0;
}

void release_font(void)
{
	if(g_memid >= 0) {
		sceKernelFreePartitionMemory(g_memid);
		g_memid = -1;
	}

	g_cur_font = msx;
}

// Returns size of string in pixels
int blit_get_string_width(char *msg) {
	#define _FONT_WIDTH 8
	return scePaf_strlen(msg) * _FONT_WIDTH;
}