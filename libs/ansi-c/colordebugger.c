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

#include "ansi_c_functions.h"

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
	}
}

