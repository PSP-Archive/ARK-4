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

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "../colordebugger/colordebugger.h"

#define    PSP_LINE_SIZE 512
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272

typedef u32 Color;
#define A(color) ((u8)(color >> 24 & 0xFF))
#define B(color) ((u8)(color >> 16 & 0xFF))
#define G(color) ((u8)(color >> 8 & 0xFF))
#define R(color) ((u8)(color & 0xFF))

extern void printTextScreen(int x, int y, const char * text, u32 color);

void print_to_screen(const char * text);
extern void print_to_screen_color(const char * text, u32 color);
void PRTSTR11(const char* A, unsigned long B, unsigned long C, unsigned long D, unsigned long E, unsigned long F, unsigned long G, unsigned long H, unsigned long I, unsigned long J, unsigned long K, unsigned long L);
#define PRTSTR10(text, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10) PRTSTR11(text, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, 0)
#define PRTSTR9(text, x1, x2, x3, x4, x5, x6, x7, x8, x9) PRTSTR10(text, x1, x2, x3, x4, x5, x6, x7, x8, x9, 0)
#define PRTSTR8(text, x1, x2, x3, x4, x5, x6, x7, x8) PRTSTR9(text, x1, x2, x3, x4, x5, x6, x7, x8, 0)
#define PRTSTR7(text, x1, x2, x3, x4, x5, x6, x7) PRTSTR8(text, x1, x2, x3, x4, x5, x6, x7, 0)
#define PRTSTR6(text, x1, x2, x3, x4, x5, x6) PRTSTR7(text, x1, x2, x3, x4, x5, x6, 0)
#define PRTSTR5(text, x1, x2, x3, x4, x5) PRTSTR6(text, x1, x2, x3, x4, x5, 0)
#define PRTSTR4(text, x1, x2, x3, x4) PRTSTR5(text, x1, x2, x3, x4, 0)
#define PRTSTR3(text, x1, x2, x3) PRTSTR4(text, x1, x2, x3, 0)
#define PRTSTR2(text, x1, x2) PRTSTR3(text, x1, x2, 0)
#define PRTSTR1(text, x1) PRTSTR2(text, x1, 0)
#define PRTSTR(text) PRTSTR1(text, 0)

//init screen
void initScreen(int (*DisplaySetFrameBuf)(void*, int, int, int));

//clear screen
void cls();

#endif
