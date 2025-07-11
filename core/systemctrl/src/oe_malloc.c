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
#include <pspkernel.h>
#include <pspsysmem_kernel.h>
#include <malloc.h>
#include "imports.h"

void* generic_malloc(int size, int partition){
    int uid = sceKernelAllocPartitionMemory(partition, "", PSP_SMEM_High, size+sizeof(int), NULL);
    int* ptr = sceKernelGetBlockHeadAddr(uid);
    if (ptr){
        ptr[0] = uid;
        return &(ptr[1]);
    }
    return NULL;
}

void* oe_malloc(size_t size){
    return generic_malloc(size, PSP_MEMORY_PARTITION_KERNEL);
}

void oe_free(void* ptr){
    if (ptr){
        SceUID uid = ((SceUID*)ptr)[-1];
        sceKernelFreePartitionMemory(uid);
    }
}

void* user_malloc(size_t size){
    return generic_malloc(size, PSP_MEMORY_PARTITION_USER);
}

void* user_memalign(unsigned int align, unsigned int size){
    int uid = sceKernelAllocPartitionMemory(PSP_MEMORY_PARTITION_USER, "", PSP_SMEM_High, size+align+sizeof(int), NULL);
    int* ptr = sceKernelGetBlockHeadAddr(uid);
    if (ptr){
        ptr = (u32)ptr + sizeof(int);
        ptr = (void*)(((u32)ptr & (~(align-1))) + align); // align
        ptr[-1] = uid;
        return ptr;
    }
    return NULL;
}