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

#include <pspsdk.h>
#include <pspsysmem_kernel.h>
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <pspsysevent.h>
#include <pspiofilemgr.h>
#include <stdio.h>
#include <string.h>
#include "globals.h"
#include <systemctrl.h>
#include "sysmem.h"

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

typedef struct _MemPart {
    u32 *meminfo;
    int offset;
    int size;
} MemPart;

int g_high_memory_enabled = 0;

static u8 g_p8_size = 4;

static u32 * (*get_memory_partition)(int pid) = NULL;

extern u32 psp_model;

static u32 findGetPartition(){
    int found = 0;
    for (u32 addr = SYSMEM_TEXT; !found; addr+=4){
        if (_lw(addr) == 0x2C85000D){
            return addr-4;
        }
    }
    return 0;
}

static inline u32 *get_partition(int pid)
{
    if (get_memory_partition == NULL) get_memory_partition = findGetPartition();
    return (*get_memory_partition)(pid);
}

static void modify_partition(MemPart *part)
{
    u32 *meminfo;
    u32 offset, size;

    meminfo = part->meminfo;
    offset = part->offset;
    size = part->size;

    if(meminfo == NULL) {
        return;
    }

    if (offset != 0) {
        meminfo[1] = (offset << 20) + 0x88800000;
    }

    meminfo[2] = size << 20;
    ((u32*)(meminfo[4]))[5] = (size << 21) | 0xFC;
}

int prevent_highmem(){
    if (psp_model == PSP_1000) return 1;
    int apitype = sceKernelInitApitype();
    return (apitype == 0x144 || apitype == 0x155 || apitype ==  0x210 || apitype ==  0x220);
}

void prepatch_partitions(void)
{
    if(prevent_highmem()){
        return;
    }

    MemPart p8, p11;

    p8.meminfo = get_partition(8);
    p11.meminfo = get_partition(11);

    g_p8_size = p8.size = 1;
    
    if(p11.meminfo != NULL) {
        p8.offset = 56-4-p8.size;
    } else {
        p8.offset = 56-p8.size;
    }

    modify_partition(&p8);

    p11.size = 4;
    p11.offset = 56-4;
    modify_partition(&p11);
}

void patch_partitions(void) 
{

    if(prevent_highmem()){
        return;
    }

    MemPart p2, p9;
    int max_user_part_size;

    p2.meminfo = get_partition(2);
    p9.meminfo = get_partition(9);

    p2.size = MAX_HIGH_MEMSIZE;
    p9.size = 0;

    if(get_partition(11) != NULL) {
        max_user_part_size = 56 - 4 - g_p8_size;
    } else {
        max_user_part_size = 56 - g_p8_size;
    }

    if (p2.size + p9.size > max_user_part_size) {
        // reserved 4MB for P11
        int reserved_len;

        reserved_len = p2.size + p9.size - max_user_part_size;

        if(p9.size > reserved_len) {
            p9.size -= reserved_len;
        } else {
            p2.size -= reserved_len - p9.size; 
            p9.size = 0;
        }
    }

    p2.offset = 0;
    modify_partition(&p2);

    p9.offset = p2.size;
    modify_partition(&p9);

    g_high_memory_enabled = 1;
    
    sctrlHENSetMemory(MAX_HIGH_MEMSIZE, 0);
}