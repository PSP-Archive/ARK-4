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
#include <psputilsforkernel.h>
#include <stdio.h>
#include <string.h>
#include <macros.h>
#include <module2.h>
#include <globals.h>
#include <systemctrl.h>
#include <systemctrl_private.h>
#include <loadexec_patch.h>
#include "imports.h"
#include "modulemanager.h"
#include "filesystem.h"
#include "rebootex.h"

// Original Load Reboot Buffer Function
int (* _LoadReboot)(void * arg1, unsigned int arg2, void * arg3, unsigned int arg4) = NULL;

// sceKernelExitGame Replacement
int returnToLauncher(void)
{
	// Load Execute Parameter
	struct SceKernelLoadExecVSHParam param;
	
	// Clear Memory
	memset(&param, 0, sizeof(param));

	char path[ARK_PATH_SIZE];
	strcpy(path, ark_config->arkpath);
	strcat(path, ARK_MENU);
	
	// Configure Parameters
	param.size = sizeof(param);
	param.args = strlen(path) + 1;
	param.argp = path;
	param.key = "game";
	
	// Trigger Reboot
	return sctrlKernelLoadExecVSHWithApitype(0x141, path, &param);
}

// Reboot Buffer Loader
int LoadReboot(void * arg1, unsigned int arg2, void * arg3, unsigned int arg4)
{
	// Restore Reboot Buffer Configuration
	restoreRebootBuffer();
	
	// backup ARK configuration to user ram
	memcpy(ark_conf_backup, ark_config, sizeof(ARKConfig));
	
	// Load Sony Reboot Buffer
	return _LoadReboot(arg1, arg2, arg3, arg4);
}

// Patch loadexec_01g.prx
void patchLoadExec(SceModule2* loadexec)
{
	// Find Reboot Loader Function
	_LoadReboot = (void *)loadexec->text_addr; //+ LOADEXEC_LOAD_REBOOT;
	int k1_patches;
	if (IS_PSP(ark_config->exec_mode)){
	    k1_patches = 2;
	    //replace LoadReboot function
	    _sw(JAL(LoadReboot), loadexec->text_addr + 0x00002D5C);

	    //patch Rebootex position to 0x88FC0000
	    _sw(0x3C0188FC, loadexec->text_addr + 0x00002DA8); // lui $at, 0x88FC

	    //allow user $k1 configs to call sceKernelLoadExecWithApiType
	    _sw(0x1000000C, loadexec->text_addr + 0x000023D0);
	    //allow all user levels to call sceKernelLoadExecWithApiType
	    _sw(NOP, loadexec->text_addr + 0x00002414);

	    //allow all user levels to call sceKernelExitVSHVSH
	    _sw(0x10000008, loadexec->text_addr + 0x000016A4);
	    _sw(NOP, loadexec->text_addr + 0x000016D8);
	}
	else{
	    k1_patches = 3;
	    // Patch sceKernelExitGame Syscalls
	    patchLoadExecCommon(loadexec, (u32)LoadReboot, k1_patches);
	    sctrlHENPatchSyscall((void *)sctrlHENFindFunction(loadexec->modname, "LoadExecForUser", 0x05572A5F), returnToLauncher);
	    sctrlHENPatchSyscall((void *)sctrlHENFindFunction(loadexec->modname, "LoadExecForUser", 0x2AC9954B), returnToLauncher);
	}
	// Flush Cache
	flushCache();
}
