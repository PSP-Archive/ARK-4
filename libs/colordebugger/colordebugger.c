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
#include "macros.h"

// Framebuffer
u32* g_vram_base = (u32*)0x44000000;

// screen handler (for psx)
void (*screen_handler)(u32 vram);

// Framebuffer Painter (for debugging)
void colorDebug(u32 color)
{
    // Paint Framebuffer
    unsigned int i = 0; for(; i < 0x100000; i++)
    {
        // Set Pixel Color
        g_vram_base[i] = color;
    }
    if (screen_handler) screen_handler(g_vram_base);
}

void setScreenHandler(void (*handler)(u32 vram)){
    screen_handler = handler;
}

// Framebuffer Color Freeze Loop (for debugging)
void doBreakpoint(void)
{
    // Screen Color Red
    colorDebug(0xFF);
    // Endless Loop
    while(1){};
}

void setBreakpoint(u32 addr){
    _sw(JUMP(doBreakpoint), addr);
    _sw(NOP, addr+4);
}
