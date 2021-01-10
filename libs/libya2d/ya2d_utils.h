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

#ifndef _YA2D_UTILS_H_
#define _YA2D_UTILS_H_

#define YA2D_LOGFILE "ms0:/ya2d_log.txt"

struct ya2d_vertex_2s3s
{
    short u, v;
    short x, y, z;
};

struct ya2d_vertex_1ui2s3s
{
    short u, v;
    unsigned int color;
    short x, y, z;
};

struct ya2d_vertex_1ui3s
{
    unsigned int color;
    short x, y, z;
};

void ya2d_log(const char* s, ...);

unsigned int next_pow2(unsigned int v);
void swizzle_fast(unsigned char* out, const unsigned char* in, unsigned int width, unsigned int height);

#endif
