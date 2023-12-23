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

// Initialize Heap
int oe_mallocinit(void)
{
   return 0;
}

void* oe_malloc(size_t size){
    SceUID uid = sceKernelAllocPartitionMemory(PSP_MEMORY_PARTITION_KERNEL, "", PSP_SMEM_High, size+sizeof(SceUID), NULL);
    SceUID* ptr = sceKernelGetBlockHeadAddr(uid);
    ptr[0] = uid;
    return &(ptr[1]);
}

void oe_free(void* ptr){
    SceUID uid = ((SceUID*)ptr)[-1];
    sceKernelFreePartitionMemory(uid);
}

// Terminate Heap
int oe_mallocterminate(void)
{
    // Forward Call
    return 0;
}
