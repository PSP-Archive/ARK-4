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

#ifndef _YA2D_IMAGE_H_
#define _YA2D_IMAGE_H_

#include "ya2d_texture.h"
#include <pspkerneltypes.h>

struct ya2d_texture* ya2d_load_PNG_file(const char* filename, int place);
struct ya2d_texture* ya2d_load_PNG_file_offset(const char* filename, int place, SceOff offset);
struct ya2d_texture* ya2d_load_PNG_buffer(void* buffer, int place);

struct ya2d_texture* ya2d_load_BMP_file(const char* filename, int place);
struct ya2d_texture* ya2d_load_BMP_buffer(void* buffer, int place);

struct ya2d_texture* ya2d_load_JPEG_file(const char* filename, int place);
struct ya2d_texture* ya2d_load_JPEG_buffer(void* buffer, unsigned long buffer_size, int place);


#endif
