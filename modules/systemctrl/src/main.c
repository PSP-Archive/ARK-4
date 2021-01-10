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
#include <systemctrl.h>
#include <systemctrl_private.h>
#include <globals.h> 
#include "rebootex.h"
#include "modulemanager.h"
#include "loadercore.h"
#include "filesystem.h"
#include "interruptman.h"
#include "cryptography.h"
#include "syspatch.h"
#include "sysmem.h"
#include "exception.h"
#include "ansi_c_functions.h"
#include "vitapops.h"

PSP_MODULE_INFO("SystemControl", 0x3007, 1, 0);

void UnprotectExtraMemory()
{
	if (IS_VITA_POPS)
		return;

	u32 *prot = (u32 *)0xBC000040;

	int i;
	for (i = 0; i < 0x10; i++)
		prot[i] = 0xFFFFFFFF;
}

void protectMemory(){
	if (IS_VITA_POPS){
		sceKernelAllocPartitionMemory(6, "POPS VRAM CONFIG", 2, 0x1B0, (void *)0x09FE0000);
		sceKernelAllocPartitionMemory(6, "POPS VRAM", 2, 0x3C0000, (void *)0x090C0000);
	}
}

// Boot Time Entry Point
int module_bootstart(SceSize args, void * argp)
{


	setIsVitaPops((int)(IS_VITA_POPS));

	UnprotectExtraMemory();
	protectMemory();

#ifdef DEBUG
	colorDebug(0xFF);

	char path[SAVE_PATH_SIZE];
	memset(path, 0, SAVE_PATH_SIZE);
	strcpy(path, SAVEPATH);
	strcat(path, "LOG.TXT");
	
	printkInit(path);
	printk("systemctrl started: compiled at %s %s\r\n", __DATE__, __TIME__);
#endif

	// Apply Module Patches
	patchSystemMemoryManager();
	patchLoaderCore();
	patchModuleManager();
	patchInterruptMan();
	patchMemlmd();
	patchFileManager();
	
	// Backup Reboot Buffer (including configuration)
	backupRebootBuffer();
	
	// Initialize Module Start Patching
	syspatchInit();
	
	// Initialize Malloc
	oe_mallocinit();
	
	// Flush Cache
	flushCache();
	
	// Register Exception Handler
	// registerExceptionHandler(NULL, NULL);
	
	// Return Success
	return 0;
}

// Run Time Entry Point
int module_start(SceSize args, void * argp)
{
	// Simply there to ensure our Entry Point gets called...
	return module_bootstart(args, argp);
}

