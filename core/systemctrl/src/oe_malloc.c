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

static SceUID heapid = -1;

// Initialize Heap
int oe_mallocinit(void)
{
    int size;
    // Get Application Type
    int key_config = sceKernelApplicationType();
    
    if(key_config == PSP_INIT_KEYCONFIG_POPS) return 0; // PS1 Emulator = No Heap
    else if (key_config == PSP_INIT_KEYCONFIG_VSH) size = 14*1024; // VSH = smaller heap
    else size = 45 * 1024; // Everything else leaves us room to brawl
    
    // Prevent Double Tapping
    if(heapid >= 0) return 0;
    
    // Create Heap
    heapid = sceKernelCreateHeap(PSP_MEMORY_PARTITION_KERNEL, size, 1, "SystemCtrlHeap");
    
    // Return Error Code on Error
    return (heapid < 0) ? heapid : 0;
}

// Allocate Memory from Heap
void * oe_malloc(unsigned int size)
{
    // Forward Call
    return sceKernelAllocHeapMemory(heapid, size);
}

// Return Memory to Heap
void oe_free(void * p)
{
    // Forward Call
    sceKernelFreeHeapMemory(heapid, p);
}

// Terminate Heap
int oe_mallocterminate(void)
{
    // Forward Call
    return sceKernelDeleteHeap(heapid);
}

void* malloc(unsigned int size){
    return oe_malloc(size);
}

void free(void* p){
    oe_free(p);
}
