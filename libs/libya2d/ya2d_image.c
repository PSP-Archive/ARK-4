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

#include "ya2d_image.h"
#include "ya2d_texture.h"
#include "ya2d_utils.h"
#include <pspiofilemgr.h>
#include <pspgu.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <png.h>
#include <jpeglib.h>


#define YA2D_PNGSIGSIZE   (8)
#define YA2D_BMPSIGNATURE (0x4D42)


typedef struct {
    unsigned short  bfType;
    unsigned int    bfSize;
    unsigned short  bfReserved1;
    unsigned short  bfReserved2;
    unsigned int    bfOffBits;
}__attribute__((packed)) BITMAPFILEHEADER;


typedef struct {
    unsigned int    biSize;
    unsigned int    biWidth;
    unsigned int    biHeight;
    unsigned short  biPlanes;
    unsigned short  biBitCount;
    unsigned int    biCompression;
    unsigned int    biSizeImage;
    unsigned int    biXPelsPerMeter;
    unsigned int    biYPelsPerMeter;
    unsigned int    biClrUsed;
    unsigned int    biClrImportant;
}__attribute__((packed)) BITMAPINFOHEADER;


static void _ya2d_read_png_file_fn(png_structp png_ptr, png_bytep data, png_size_t length)
{
    SceUID fd = *(SceUID*) png_get_io_ptr(png_ptr);
    sceIoRead(fd, data, length);
}

static void _ya2d_read_png_buffer_fn(png_structp png_ptr, png_bytep data, png_size_t length)
{
    unsigned int* address = png_get_io_ptr(png_ptr);
    memcpy(data, (void*)(*address), length);
    *address += length;
}


static struct ya2d_texture* _ya2d_load_PNG_generic(void* io_ptr, png_rw_ptr read_data_fn, int place)
{

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                                 NULL, NULL, NULL);
    if(png_ptr == NULL) {
        goto exit_error;
    }
    
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if(info_ptr == NULL) {
        goto exit_destroy_read;
    }
    
    png_bytep* row_ptrs = NULL;
    
    if(setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)0);
        if(row_ptrs != NULL) free(row_ptrs);
        goto exit_error;    
    }

    png_set_read_fn(png_ptr, (png_voidp)io_ptr, read_data_fn);
    png_set_sig_bytes(png_ptr, YA2D_PNGSIGSIZE);
    png_read_info(png_ptr, info_ptr);
    
    unsigned int width, height;
    int bit_depth, color_type;
    
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,
                 &color_type, NULL, NULL, NULL);
    
    if (color_type == PNG_COLOR_TYPE_PALETTE && bit_depth <= 8) png_set_expand(png_ptr);
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_expand(png_ptr);
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_expand(png_ptr);
    if (bit_depth == 16) png_set_strip_16(png_ptr);
    
    //PSP: 16 or 32 bpp (not 0xRRGGBB)
    if (bit_depth == 8 && color_type == PNG_COLOR_TYPE_RGB) png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_ptr);
        png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
    }
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_expand_gray_1_2_4_to_8(png_ptr);
    
    if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png_ptr);
    }
    
    if(bit_depth < 8) png_set_packing(png_ptr);

    png_read_update_info(png_ptr, info_ptr);
    
    row_ptrs = (png_bytep*)malloc(sizeof(png_bytep) * height);
    struct ya2d_texture* texture = ya2d_create_texture(width, height,
                                                GU_PSM_8888, place);
                                                
    texture->has_alpha = 1;
    int i;
    for (i = 0; i < height; ++i) {
        row_ptrs[i] = (png_bytep)(texture->data + i*texture->stride);
    }
    
    png_read_image(png_ptr, row_ptrs);
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)0);
    free(row_ptrs);
    ya2d_flush_texture(texture);
    return texture;
exit_destroy_read:
    png_destroy_read_struct(&png_ptr, (png_infopp)0, (png_infopp)0);
exit_error:
    return NULL;
}


struct ya2d_texture* ya2d_load_PNG_file_offset(const char* filename, int place, SceOff offset)
{
    png_byte pngsig[YA2D_PNGSIGSIZE];
    SceUID fd;
    
    if ((fd = sceIoOpen(filename, PSP_O_RDONLY, 0777)) < 0) {
        goto exit_error;
    }
    
    sceIoLseek(fd, offset, SEEK_SET);
    
    if (sceIoRead(fd, pngsig, YA2D_PNGSIGSIZE) != YA2D_PNGSIGSIZE) {
        goto exit_close;
    }
    
    if (png_sig_cmp(pngsig, 0, YA2D_PNGSIGSIZE) != 0) {
        goto exit_close;
    }
    
    struct ya2d_texture* texture = _ya2d_load_PNG_generic((void*) &fd, _ya2d_read_png_file_fn, place);
    sceIoClose(fd);
    return texture;
    
exit_close:
    sceIoClose(fd);
exit_error:
    return NULL;
}


struct ya2d_texture* ya2d_load_PNG_file(const char* filename, int place)
{
    return ya2d_load_PNG_file_offset(filename, place, 0);
}


struct ya2d_texture* ya2d_load_PNG_buffer(void *buffer, int place)
{
    if(png_sig_cmp((png_byte*) buffer, 0, YA2D_PNGSIGSIZE) != 0) {
        return NULL;
    }
    unsigned int buffer_address = (unsigned int)buffer + YA2D_PNGSIGSIZE;

    return _ya2d_load_PNG_generic((void*) &buffer_address, _ya2d_read_png_buffer_fn, place);    
}

static struct ya2d_texture* _ya2d_load_BMP_generic(BITMAPFILEHEADER* bmp_fh,
                                            BITMAPINFOHEADER* bmp_ih,
                                            void* user_data,
                                            void (*seek_fn)(void* user_data, unsigned int offset),
                                            void (*read_fn)(void* user_data, void* buffer, unsigned int length),
                                            int place)
{
    unsigned int row_size;
    if (bmp_ih->biBitCount == 32) {
        row_size = bmp_ih->biWidth * 4;
    } else if (bmp_ih->biBitCount == 24) {
        row_size = bmp_ih->biWidth * 3;
    } else if (bmp_ih->biBitCount == 16) {
        row_size = bmp_ih->biWidth * 2;
    } else {
        goto exit_error;
    }
    
    if(row_size%4 != 0) {
        row_size += 4-(row_size%4);
    }
    
    struct ya2d_texture* texture = ya2d_create_texture(bmp_ih->biWidth, bmp_ih->biHeight,
                                                GU_PSM_8888, place);
    
    seek_fn(user_data, bmp_fh->bfOffBits);
    
    void *buffer = malloc(row_size);
    unsigned int* tex_ptr, color;
    int i, x, y=bmp_ih->biHeight-1;
    for(i = 0; i < bmp_ih->biHeight; ++i, y=bmp_ih->biHeight-1-i) {
        read_fn(user_data, buffer, row_size);
        tex_ptr = (unsigned int*)(texture->data + y*texture->stride);
        for(x = 0; x < bmp_ih->biWidth; ++x) { 
            if (bmp_ih->biBitCount == 32) { //ABGR8888
                color = *(unsigned int*)(buffer + x*4);
                *tex_ptr = (color&0xFF)<<24 | ((color>>8)&0xFF)<<16 |
                           ((color>>16)&0xFF)<<8 | (color>>24);
            } else if (bmp_ih->biBitCount == 24) { //BGR888
                unsigned char *address = buffer + x*3;
                *tex_ptr =  (*address)<<16 | (*(address+1))<<8 |
                            (*(address+2)) | (0xFF<<24);
            } else if (bmp_ih->biBitCount == 16) { //BGR565
                color = *(unsigned short*)(buffer + x*2);
                unsigned char r = (color&0x1F)*((float)255/31);
                unsigned char g = ((color>>5)&0x3F)*((float)255/63);
                unsigned char b = ((color>>11)&0x1F)*((float)255/31);
                *tex_ptr = ((r<<16) | (g<<8) | b | (0xFF<<24));
            }
            tex_ptr++;
        }
    }
    
    free(buffer);
    ya2d_flush_texture(texture);
    return texture;
    
exit_error:    
    return NULL;
}

static void _ya2d_read_bmp_file_seek_fn(void* user_data, unsigned int offset)
{
    sceIoLseek(*(SceUID*) user_data, offset, SEEK_SET);
}

static void _ya2d_read_bmp_file_read_fn(void* user_data, void* buffer, unsigned int length)
{
    sceIoRead(*(SceUID*) user_data, buffer, length);
}

static void _ya2d_read_bmp_buffer_seek_fn(void* user_data, unsigned int offset)
{
    *(unsigned int*)user_data += offset;
}

static void _ya2d_read_bmp_buffer_read_fn(void* user_data, void* buffer, unsigned int length)
{
    memcpy(buffer, (void*)*(unsigned int*)user_data, length);
    *(unsigned int*)user_data += length;
}

struct ya2d_texture* ya2d_load_BMP_file(const char* filename, int place)
{
    SceUID fd;
    if((fd = sceIoOpen(filename, PSP_O_RDONLY, 0777)) < 0) {
        goto exit_error;
    }
    
    BITMAPFILEHEADER bmp_fh;
    sceIoRead(fd, (void*)&bmp_fh, sizeof(BITMAPFILEHEADER));    
    if(bmp_fh.bfType != YA2D_BMPSIGNATURE) {
        goto exit_close;
    }
    
    BITMAPINFOHEADER bmp_ih;
    sceIoRead(fd, (void*)&bmp_ih, sizeof(BITMAPINFOHEADER));
    
    struct ya2d_texture* texture = _ya2d_load_BMP_generic(&bmp_fh,
                                                       &bmp_ih,
                                                       (void*)&fd,
                                                       _ya2d_read_bmp_file_seek_fn,
                                                       _ya2d_read_bmp_file_read_fn,
                                                       place);
                                                        
    sceIoClose(fd);
    return texture;
    
exit_close:
    sceIoClose(fd);
exit_error:    
    return NULL;
}


struct ya2d_texture* ya2d_load_BMP_buffer(void* buffer, int place)
{
    if(buffer == NULL) {
        goto exit_error;
    }
    
    unsigned int buffer_address = (unsigned int)buffer;
    
    BITMAPFILEHEADER bmp_fh;
    memcpy(&bmp_fh, (void*)buffer_address, sizeof(BITMAPFILEHEADER));
    if(bmp_fh.bfType != YA2D_BMPSIGNATURE) {
        goto exit_error;
    }
    
    BITMAPINFOHEADER bmp_ih;
    memcpy(&bmp_ih, (void*)buffer_address + sizeof(BITMAPFILEHEADER), sizeof(BITMAPINFOHEADER));
    
    
    struct ya2d_texture* texture = _ya2d_load_BMP_generic(&bmp_fh,
                                                       &bmp_ih,
                                                       (void*)&buffer_address,
                                                       _ya2d_read_bmp_buffer_seek_fn,
                                                       _ya2d_read_bmp_buffer_read_fn,
                                                       place);
    
    return texture;
exit_error:    
    return NULL;    
}

static struct ya2d_texture* _ya2d_load_JPEG_generic(struct jpeg_decompress_struct* jinfo, struct jpeg_error_mgr* jerr, int place)
{
    int row_bytes;
    switch (jinfo->out_color_space) {
    case JCS_RGB:
        row_bytes = jinfo->image_width * 3;
        break;
    default:
        goto exit_error;
    }
    
    struct ya2d_texture *texture = ya2d_create_texture(jinfo->image_width,
                                                jinfo->image_height,
                                                GU_PSM_8888, place);
    JSAMPARRAY buffer = (JSAMPARRAY)malloc(sizeof(JSAMPROW));
    buffer[0] = (JSAMPROW)malloc(sizeof(JSAMPLE) * row_bytes);
    
    unsigned int i, color, *tex_ptr;
    unsigned char* jpeg_ptr;
    void* row_ptr = texture->data;
    jpeg_start_decompress(jinfo);
    
    while (jinfo->output_scanline < jinfo->output_height) {
        jpeg_read_scanlines(jinfo, buffer, 1);
        tex_ptr = (row_ptr += texture->stride);
        for (i = 0, jpeg_ptr = buffer[0]; i < jinfo->output_width; ++i) {
            color = *(jpeg_ptr++);
            color |= *(jpeg_ptr++)<<8;
            color |= *(jpeg_ptr++)<<16;
            *(tex_ptr++) = color | 0xFF000000; 
        }
    }

    
    free(buffer[0]);
    free(buffer);
    ya2d_flush_texture(texture);
    return texture;
exit_error:    
    return NULL;
}


struct ya2d_texture* ya2d_load_JPEG_file(const char* filename, int place)
{
    FILE *fd;
    if ((fd = fopen(filename, "rb")) < 0) {
        fclose(fd);
        return NULL;
    }
    
    struct jpeg_decompress_struct jinfo;
    struct jpeg_error_mgr jerr;
    jinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&jinfo);
    jpeg_stdio_src(&jinfo, fd);
    jpeg_read_header(&jinfo, 1);
    
    struct ya2d_texture* texture = _ya2d_load_JPEG_generic(&jinfo, &jerr, place);
    
    jpeg_finish_decompress(&jinfo);
    jpeg_destroy_decompress(&jinfo);
    
    fclose(fd);
    return texture;
}


struct ya2d_texture* ya2d_load_JPEG_buffer(void* buffer, unsigned long buffer_size, int place)
{
    if (buffer == NULL) {
        return NULL;
    }
    
    struct jpeg_decompress_struct jinfo;
    struct jpeg_error_mgr jerr;
    jinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&jinfo);
    jpeg_mem_src(&jinfo, buffer, buffer_size);
    jpeg_read_header(&jinfo, 1);
    
    struct ya2d_texture* texture = _ya2d_load_JPEG_generic(&jinfo, &jerr, place);
    
    jpeg_finish_decompress(&jinfo);
    jpeg_destroy_decompress(&jinfo);
    
    return texture;
}
