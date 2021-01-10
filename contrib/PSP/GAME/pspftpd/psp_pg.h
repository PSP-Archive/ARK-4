/*
 *  xtiger port on PSP 
 *
 *  Copyright (C) 2006 Ludovic Jacomme (ludovic.jacomme@gmail.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
// primitive graphics for Hello World PSP
# ifndef _PSP_PG_H_
# define _PSP_PG_H_

//#define PG_RGB(r, g, b) ((r)|((g)<<5)|((b)<<10))
# define PG_RGB(r, g, b)  (((b & 0xf8) << 7) | ((g & 0xf8) << 2) | (r >> 3))
extern void pgWaitV();
extern void pgWaitVn(unsigned long count);
extern void pgScreenFrame(long mode,long frame);
extern void pgScreenFlip();
extern void pgScreenFlipV();
extern void pgFillvram(unsigned long color);
extern void pgBitBlt(unsigned long x,unsigned long y,unsigned long w,unsigned long h,unsigned long mag,const unsigned short *d);
extern void pgBitTransp(unsigned long x,unsigned long y,unsigned long w,unsigned long h,unsigned long mag,const unsigned short *d, unsigned short transparency);
extern char *pgGetVramAddr(unsigned long x,unsigned long y);

/* New Stuff */
extern void pgDrawBox(int x, int y, int w, int h, int border, int color);
extern void pgDrawRectangle(int x, int y, int w, int h, int border, int mode);
extern void pgFillRectangle(int x, int y, int w, int h, int color, int mode);
extern void pgPutPixel(int x, int y,int color);

extern void pgPutChar(int x, int y, int color,int bgcolor, char c, int drawfg, int drawbg);
extern void pgFramePutChar(int x, int y, int color, char c);
extern void pgPrint(int x,int y,const char *str, int color);
extern void pgFramePrint(int x,int y,const char *str, int color);
extern void pgFillPrint(int x,int y,const char *str, int color, int bgcolor);

extern void pgInit(void);
extern void pgClear(void);

#define PG_SCREEN_WIDTH  480
#define PG_SCREEN_HEIGHT 272

/* Cyan */
#define PG_TEXT_COLOR       PG_RGB(0x00,205,205)
#define PG_TEXT_RED         PG_RGB(0xff,0,0)
#define PG_TEXT_GREEN       PG_RGB(0x0,0xff,0)
#define PG_TEXT_YELLOW      PG_RGB(0xff,0xff,0)
#define PG_TEXT_BLUE        PG_RGB(0x00,0,0xff)
#define PG_TEXT_WHITE       PG_RGB(0xff,0xff,0xff)

#define PG_DEF       0
#define PG_XOR       1

# endif

