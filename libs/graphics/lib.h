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

#ifndef ELOADER_LIBC
#define ELOADER_LIBC

#include <pspiofilemgr.h>
#include <stdlib.h>

// Typedefs
typedef unsigned char byte;

void numtohex8(char *dst, int n);
void numtohex4(char *dst, int n);
void numtohex2(char *dst, int n);

// limited sprintf function - avoids pulling in large library
int writeFormat(char *xibuff, const char *xifmt, u32 xidata);
void mysprintf11(char *xobuff, const char *xifmt,
   u32 xidata,
   u32 xidata2,
   u32 xidata3,
   u32 xidata4,
   u32 xidata5,
   u32 xidata6,
   u32 xidata7,
   u32 xidata8,
   u32 xidata9,
   u32 xidata10,
   u32 xidata11);

void mysprintf0(char *xobuff, const char *xifmt);

#endif
