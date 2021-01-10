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

#include "colordebugger.h"
#include "ansi_c_functions.h"
#include <pspgu.h>

static int is_vita_pops = 0;
u32* g_vram_base = (u32*)0x44000000;
u16* ps1_vram = (u16*)0x490C0000;
POPSVramConfigVLA* vram_config = (POPSVramConfigVLA*)0x49FE0000;

void (*_psxVramHandler)(u32* psp_vram, u16* ps1_vram) = (void*)NULL;
void* registerPSXVramHandler(void (*handler)(u32* psp_vram, u16* ps1_vram)){
	void* prev = _psxVramHandler;
	_psxVramHandler = handler;
	return prev;
}

u16 RGBA8888_to_RGBA5551(u32 color){
	int r, g, b, a;

	a = (color >> 24) ? 0x8000 : 0;
	b = (color >> 19) & 0x1F;
	g = (color >> 11) & 0x1F;
	r = (color >> 3) & 0x1F;

	return a | r | (g << 5) | (b << 10);
}

void initVitaPopsVram(){
	memset((void *)ps1_vram, 0, 0x3C0000);
	vram_config->counter = 0;
	vram_config->configs[0].x = 0x1F6;
	vram_config->configs[0].y = 0;
	vram_config->configs[0].width = 640*4;
	vram_config->configs[0].height = 240;
	vram_config->configs[0].color_width = sizeof(u16);
	vram_config->configs[0].cur_buffer = 0;
}

static u16 convert_8888_to_5551(u32 color)
{
	int r, g, b, a;

	a = (color >> 24) ? 0x8000 : 0;
	b = (color >> 19) & 0x1F;
	g = (color >> 11) & 0x1F;
	r = (color >> 3) & 0x1F;

	return a | r | (g << 5) | (b << 10);
}

u32 GetPopsVramAddr(int x, int y)
{
	return 0x490C0000 + x * 2 + y * 640 * 4;
}

u32 GetPspVramAddr(u32 framebuffer, int x, int y)
{
	return framebuffer + x * 4 + y * 512 * 4;
}

void RelocateVram(u32 framebuffer)
{
	if(framebuffer)
	{
		int y;
		for(y = 0; y < 272; y++)
		{
			int x;
			for(x = 0; x < 480; x++)
			{
				u32 color = *(u32 *)GetPspVramAddr(framebuffer, x, y);
				*(u16 *)GetPopsVramAddr(x, y) = convert_8888_to_5551(color);
			}
		}
	}
}

void copyPSPVram(u32* psp_vram){
	
	//if (_psxVramHandler != NULL)
	//	_psxVramHandler((psp_vram==NULL)?g_vram_base:psp_vram, ps1_vram);
	//else
		RelocateVram((psp_vram==NULL)?g_vram_base:psp_vram);
	
	//return;
	/*
	if (psp_vram == NULL)
		return;
	int x,y;
	for (y=0; y<272; y++){
		for (x=0; x<480; x++){
			u32 src_vram = (psp_vram + x*4 + y*512*4);
			u32 dst_vram = (((u32)ps1_vram) + x*2 + y*640*4);
			*(u16*)dst_vram = RGBA8888_to_RGBA5551(*(u32*)src_vram);
		}
	}
	*/
}

int isVitaPops(){
	return is_vita_pops;
}

void setIsVitaPops(int is){
	is_vita_pops = is;
	if (is_vita_pops)
		initVitaPopsVram();
}

// Framebuffer Painter (for debugging)
void colorDebug(unsigned int color)
{
	// Framebuffer
	unsigned int * framebuffer = (unsigned int *)0x04000000;
	
	// Paint Framebuffer
	unsigned int i = 0; for(; i < 0x100000; i++)
	{
		// Set Pixel Color
		framebuffer[i] = color;
	}
	if (is_vita_pops)
		copyPSPVram(NULL);
}

// Framebuffer Color Freeze Loop (for debugging)
void colorLoop(void)
{
	// Screen Color
	unsigned char color = 0;
	
	// Endless Loop
	while(1)
	{
		// Paint Screen in changing colors...
		memset((void *)0x04000000, color++, 0x400000);
		if (is_vita_pops)
			copyPSPVram(NULL);
	}
}
