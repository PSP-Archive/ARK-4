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
#include <globals.h>
#include <systemctrl.h>
#include <systemctrl_private.h>
#include "sysmem.h"
#include "functions.h"

// Patch System Memory Manager
void patchSystemMemoryManager(void)
{
    if (IS_PSP(ark_config->exec_mode)){
	    _sw(0x10000000|0x1F, SYSMEM_TEXT+0x000098F0);
	    _sw(0x10000000|0x12, SYSMEM_TEXT+0x00009A10);
	    _sw(0x10000000|0x18, SYSMEM_TEXT+0x00009AA8);
	    _sw(0x10000000|0x1C, SYSMEM_TEXT+0x00009B58);
	    _sw(0x10000000|0x15, SYSMEM_TEXT+0x00009C2C);
	    _sw(0x10000000|0x15, SYSMEM_TEXT+0x00009CD0);
	    _sw(0x10000000|0x12, SYSMEM_TEXT+0x00009D74);
	    _sw(0x10000000|0x18, SYSMEM_TEXT+0x00009E0C);
	    _sw(0x10000000|0x18, SYSMEM_TEXT+0x00009EBC);
	    _sw(0x10000000|0x12, SYSMEM_TEXT+0x00009F6C);
    }
    else{
	    // Force branching
	    u32 nids[] = {0x7591C7DB, 0x342061E5, 0x315AD3A0, 0xEBD5C3E6, 0x057E7380,\
					    0x91DE343C, 0x7893F79A, 0x35669D4C, 0x1B4217BC, 0x358CA1BB };
	    int i;
	    for (i=0; i<sizeof(nids)/sizeof(u32); i++)
	    	_sh(0x1000, FindFirstBEQ(sctrlHENFindFunction("sceSystemMemoryManager", "SysMemUserForUser", nids[i])) + 2);
    }
    // Flush Cache
	flushCache();
}

// Patch Game ID Getter
void patchGameInfoGetter(SceModule2 * mod)
{
	// Kernel Module
	if((mod->text_addr & 0x80000000) != 0)
	{
		// Hook Import
		hookImportByNID(mod, "SysMemForKernel", 0xEF29061C, SysMemForKernel_EF29061C_Fixed);
	}
}

// Fixed Game Info Getter Function
void * SysMemForKernel_EF29061C_Fixed(void)
{

	// Default Game ID
	const char * defaultid = "HOME00000";

	if (IS_VITA_POPS(ark_config->exec_mode))
		return defaultid;

	// Find Function
	void * (* SysMemForKernel_EF29061C)(void) = (void *)sctrlHENFindFunction("sceSystemMemoryManager", "SysMemForKernel", 0xEF29061C);
	
	// Function unavailable (how?!)
	if(SysMemForKernel_EF29061C == NULL) return NULL;
	
	// Get Game Info Structure
	void * gameinfo = SysMemForKernel_EF29061C();
	
	// Structure unavailable
	if(gameinfo == NULL) return NULL;
	
	// Open Disc Identifier
	int disc = sceIoOpen("disc0:/UMD_DATA.BIN", PSP_O_RDONLY, 0777);
	
	// Opened Disc Identifier
	if(disc >= 0)
	{
		// Read Country Code
		sceIoRead(disc, gameinfo + 0x44, 4);
		
		// Skip Delimiter
		sceIoLseek32(disc, 1, PSP_SEEK_CUR);
		
		// Read Game ID
		sceIoRead(disc, gameinfo + 0x48, 5);
		
		// Close Disc Identifier
		sceIoClose(disc);
	}
	else{
	    // Set Default Game ID
	    memcpy(gameinfo + 0x44, defaultid, strlen(defaultid));
	}
	
	// Return Game Info Structure
	return gameinfo;
}

void handleVitaMemory(){
    if (IS_VITA_POPS(ark_config->exec_mode)){ // protect POPS Vram from being overwritten by homebrew
		sceKernelAllocPartitionMemory(6, "POPS VRAM CONFIG", 2, 0x1B0, (void *)0x09FE0000);
		sceKernelAllocPartitionMemory(6, "POPS VRAM", 2, 0x3C0000, (void *)0x090C0000);
	}
	else{ // unprotect extra RAM available on PS Vita
	    u32 *prot = (u32 *)0xBC000040;
	    int i;
	    for (i = 0; i < 0x10; i++)
		    prot[i] = 0xFFFFFFFF;
	}
}
