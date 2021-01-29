/*
	TN SaveState Plugin
	Copyright (C) 2014, Total_Noob

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __COMMON_H__
#define __COMMON_H__

#include <pspsdk.h>
#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <pspgu.h>
#include <pspgum.h>
#include "systemctrl.h"
#include <psploadcore.h>

#include <stdio.h>
#include <string.h>
#include <malloc.h>

/* Definitions */
#define MAKE_JUMP(a, f) _sw(0x08000000 | (((u32)(f) & 0x0FFFFFFC) >> 2), a);
#define MAKE_CALL(a, f) _sw(0x0C000000 | (((u32)(f) >> 2) & 0x03FFFFFF), a);

//by Davee
#define HIJACK_FUNCTION(a, f, ptr) \
{ \
	u32 func = a; \
	static u32 patch_buffer[3]; \
	_sw(_lw(func), (u32)patch_buffer); \
	_sw(_lw(func + 4), (u32)patch_buffer + 8);\
	MAKE_JUMP((u32)patch_buffer + 4, func + 8); \
	_sw(0x08000000 | (((u32)(f) >> 2) & 0x03FFFFFF), func); \
	_sw(0, func + 4); \
	ptr = (void *)patch_buffer; \
}

#define ALIGN(p, x) ((((u32)p) & ~(x - 1)) + x);

/* RAM structure */
#define KERNEL_1_ADDR 0x88000000
#define KERNEL_1_SIZE 3 * 1024 * 1024

#define KERNEL_2_ADDR 0x88300000
#define KERNEL_2_SIZE 1 * 1024 * 1024

#define VOLATILE_ADDR 0x88400000
#define VOLATILE_SIZE 4 * 1024 * 1024

#define USER_1_ADDR 0x88800000
#define USER_1_SIZE 12 * 1024 * 1024

#define USER_2_ADDR 0x89400000
#define USER_2_SIZE 12 * 1024 * 1024

/* Other definitions */
#define SVST_MAGIC 0x54535653
#define SVST_VERSION 2

#define PSP_MEMORY_PARTITION_CUSTOM 11

enum SaveStateModes
{
	MODE_NONE = -1,
	MODE_SAVE = 0,
	MODE_LOAD = 1,
};

typedef struct
{
	char gameid[16];
	char gametitle[64];
} StateInfo;

typedef struct
{
	u32 magic;
	u32 version;
	u32 mode;
	u32 sp;
	u32 ra;
	u32 kernel_offs;
	u32 kernel_size;
	u32 volatile_offs;
	u32 volatile_size;
	u32 user_1_offs;
	u32 user_1_size;
	u32 user_2_offs;
	u32 user_2_size;
} StateHeader;

typedef struct
{
	u32 magic;
	u32 version;
	u32 param_offset;
	u32 icon0_offset;
	u32 icon1_offset;
	u32 pic0_offset;
	u32 pic1_offset;
	u32 snd0_offset;
	u32 elf_offset;
	u32 psar_offset;
} PBPHeader;

typedef struct  __attribute__((packed))
{
	u32 signature;
	u32 version;
	u32 fields_table_offs;
	u32 values_table_offs;
	int nitems;
} SFOHeader;

typedef struct __attribute__((packed))
{
	u16 field_offs;
	u8  unk;
	u8  type; // 0x2 -> string, 0x4 -> number
	u32 unk2;
	u32 unk3;
	u16 val_offs;
	u16 unk4;
} SFODir;

#endif
