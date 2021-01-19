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
#include <pspsysmem_kernel.h>
#include <pspthreadman_kernel.h>
#include <pspdebug.h>
#include <pspinit.h>
#include <string.h>
#include <stdio.h>
#include <systemctrl.h>
#include <systemctrl_private.h>
#include <macros.h>
#include <gameinfo.h>
#include <globals.h>
#include <functions.h>
#include "loadmodule_patch.h"
#include "savedata_patch.h"
#include "nodrm_patch.h"
#include "psid.h"

PSP_MODULE_INFO("stargate", 0x1007, 1, 0);
PSP_MAIN_THREAD_ATTR(0);

// Previous Module Start Handler
STMOD_HANDLER previous;

ARKConfig config;

// Module Start Handler
void stargateSyspatchModuleOnStart(SceModule2 * mod)
{
	// Call Previous Module Start Handler
	if(previous) previous(mod);
	
	// Patch LoadModule Function
	patchLoadModuleFuncs(mod);
	
	// Fix Exploit Game Save
	//if (IS_GAME_EXPLOIT)
	//    fixExploitGameModule(mod);
}

// Boot Time Module Start Handler
int stargateStartModuleHandler(int modid, SceSize argsize, void * argp, int * modstatus, SceKernelSMOption * opt)
{
	// Fetch Module
	SceModule2 * mod = (SceModule2 *)sceKernelFindModuleByUID(modid);
	
	// Module not found
	if(mod == NULL) return -1;
	
	// Fix Prometheus Patch #1
	SceLibraryStubTable * import = findImportLib(mod, "Kernel_LibrarZ");
	if(import != NULL)
	{
		strcpy((char *)import->libname, "Kernel_Library");
	}
	
	// Fix Prometheus Patch #2
	import = findImportLib(mod, "Kernel_Librar0");
	if(import != NULL)
	{
		strcpy((char* )import->libname, "Kernel_Library");
	}
	
	// Fix Prometheus Patch #3
	import = findImportLib(mod, "sceUtilitO");
	if(import != NULL)
	{
		strcpy((char*)import->libname, "sceUtility");
	}
	
	// Why do we return an error...?
	return -1;
}

// Idol Master Fix
static void patchLoadExec(void)
{
	// Fix Load Execute CFW Detection
	u32 addr = sctrlHENFindFunction("sceLoadExec", "LoadExecForUser", 0x362A956B);
	for (;;addr+=4){
		if (_lw(addr) == 0xAC800004)
			break;
	}
	*(u32*)(addr-4) = 0;
}

// Entry Point
int module_start(SceSize args, void * argp)
{
    int apitype = sceKernelInitApitype();
    if (apitype == 0x152 || apitype == 0x141) {
		return 0;
	}
	
	// Hello Message
	printk("stargate started: compiled at %s %s\r\n", __DATE__, __TIME__);

    memcpy(&config, ark_conf_backup, sizeof(ARKConfig)); // copy configuration from user ram
	
	// Fix Idol Master
	patchLoadExec();
	
	// Fetch sceUtility Load Module Functions
	getLoadModuleFuncs();
	
	// Initialize NPDRM
	nodrmInit();
	
	// Register Start Module Handler
	previous = sctrlHENSetStartModuleHandler(stargateSyspatchModuleOnStart);
	
	// Register Boot Start Module Handler
	sctrlSetCustomStartModule(stargateStartModuleHandler);
	
	// Flush Cache
	flushCache();
	
	// Module Start Success
	return 0;
}
