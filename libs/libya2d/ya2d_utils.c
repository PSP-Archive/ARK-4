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

#include "ya2d_utils.h"
#include <pspiofilemgr.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

void ya2d_log(const char* s, ...)
{
    SceUID fd;
    if((fd = sceIoOpen(YA2D_LOGFILE, PSP_O_WRONLY|PSP_O_APPEND|PSP_O_CREAT, 0777))) {
        char buffer[256];
        va_list args;
        va_start(args, s);
        vsnprintf(buffer, 256, s, args);
        sceIoWrite(fd, buffer, strlen(buffer));
        va_end(args);
        sceIoClose(fd);
    }
}

//Grabbed from: http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
unsigned int next_pow2(unsigned int v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    return v+1;
}

//Grabbed from: PSPSDK Samples
void swizzle_fast(unsigned char* out, const unsigned char* in, unsigned int width, unsigned int height)
{
   unsigned int blockx, blocky;
   unsigned int j;
 
   unsigned int width_blocks = (width / 16);
   unsigned int height_blocks = (height / 8);
 
   unsigned int src_pitch = (width-16)/4;
   unsigned int src_row = width * 8;
 
   const unsigned char* ysrc = in;
   unsigned int* dst = (unsigned int*)out;
 
   for (blocky = 0; blocky < height_blocks; ++blocky)
   {
      const unsigned char* xsrc = ysrc;
      for (blockx = 0; blockx < width_blocks; ++blockx)
      {
         const unsigned int* src = (unsigned int*)xsrc;
         for (j = 0; j < 8; ++j)
         {
            *(dst++) = *(src++);
            *(dst++) = *(src++);
            *(dst++) = *(src++);
            *(dst++) = *(src++);
            src += src_pitch;
         }
         xsrc += 16;
     }
     ysrc += src_row;
   }
}
