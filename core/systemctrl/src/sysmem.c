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
#include <string.h>
#include <ark.h>
#include <systemctrl.h>
#include <systemctrl_private.h>
#include "sysmem.h"
#include "functions.h"

void uprotectExtraMemory()
{
    //unlock memory
    unsigned int i = 0; for(; i < 0x40; i += 4) {
        _sw(0xFFFFFFFF, 0xBC000040 + i);
    }
}

// Patch System Memory Manager
void patchSystemMemoryManager(void)
{

    // Force branching
    u32 nids[] = {0x7591C7DB, 0x342061E5, 0x315AD3A0, 0xEBD5C3E6, 0x057E7380,\
                    0x91DE343C, 0x7893F79A, 0x35669D4C, 0x1B4217BC, 0x358CA1BB };
    int i;
    for (i=0; i<sizeof(nids)/sizeof(u32); i++)
        _sh(0x1000, FindFirstBEQ(sctrlHENFindFunction("sceSystemMemoryManager", "SysMemUserForUser", nids[i])) + 2);
    // Flush Cache
    flushCache();
}
