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
#include <stdio.h>
#include <string.h>
#include <ark.h>
#include <macros.h>
#include "module2.h"

extern ARKConfig* ark_config;

// Interrupt Manager Patch
SceModule2* patchInterruptMan(void)
{
    // Find Module
    SceModule2* mod = (SceModule2 *)sceKernelFindModuleByName("sceInterruptManager");
    
    // Fetch Text Address
    u32 addr = mod->text_addr;
    u32 topaddr = mod->text_addr + mod->text_size;
    int patches = 2;
    for (; addr<topaddr && patches; addr+=4){
        u32 data = _lw(addr);
        // Override Endless Loop of breaking Death with EPC = t7
        if (data == 0x0003FF8D){
            _sw(0x408F7000, addr-4);
            _sw(NOP, addr);
            patches--;
        }
        // Prevent Hardware Register Writing
        else if ((data & 0x0000FFFF) == 0xBC00){
            _sw(NOP, addr+4);
            _sw(NOP, addr+8);
            patches--;
        }
    }
    // Flush Cache
    flushCache();
    return mod;
}

