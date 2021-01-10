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

	char* path = ark_config->menupath;
	
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
	
	// Load Sony Reboot Buffer
	return _LoadReboot(arg1, arg2, arg3, arg4);
}

// Patch loadexec_01g.prx
void patchLoadExec(void)
{
	// Find LoadExec Module
	SceModule2 * loadexec = (SceModule2*)sceKernelFindModuleByName("sceLoadExec");
	
	// Find Reboot Loader Function
	_LoadReboot = (void *)loadexec->text_addr; //+ LOADEXEC_LOAD_REBOOT;
	
	patchLoadExecCommon(loadexec, (u32)LoadReboot);

	// Patch sceKernelExitGame Syscalls
	sctrlHENPatchSyscall((void *)sctrlHENFindFunction(loadexec->modname, "LoadExecForUser", 0x05572A5F), returnToLauncher);
	sctrlHENPatchSyscall((void *)sctrlHENFindFunction(loadexec->modname, "LoadExecForUser", 0x2AC9954B), returnToLauncher);
}
