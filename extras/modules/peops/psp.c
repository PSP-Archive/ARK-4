/*
    Custom Emulator Firmware
    Copyright (C) 2012-2014, ColdBird/Total_Noob/Acid_Snake

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

#include <common.h>
#include "main.h"

#include "spu/stdafx.h"
#include "spu/externals.h"

void *malloc(size_t size)
{
    SceUID uid = sceKernelAllocPartitionMemory(PSP_MEMORY_PARTITION_USER, "", PSP_SMEM_Low, size + 8, NULL);
    if(uid >= 0)
    {
        unsigned int *p = (unsigned int *)sceKernelGetBlockHeadAddr(uid);
        *p = uid;
        *(p + 4) = size;
        return (void *)(p + 8);
    }

    return NULL;
}

void free(void *ptr)
{
    if(ptr)
    {
        sceKernelFreePartitionMemory(*((SceUID *)ptr - 8));
    }
}

unsigned long timeGetTime()
{
    return sceKernelLibcClock();
}

void ReadConfig(void)
{
}