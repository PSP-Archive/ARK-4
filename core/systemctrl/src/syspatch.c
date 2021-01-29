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
#include <systemctrl.h>
#include <systemctrl_private.h>
#include <module2.h>
#include <stdio.h>
#include <string.h>
#include <macros.h>
#include <graphics.h>
#include "libs/colordebugger/colordebugger.h"
#include "imports.h"
#include "modulemanager.h"
#include "cryptography.h"
#include "mediasync.h"
#include "msstor_cache.h"
#include "rebootex.h"
#include "vlffix.h"
#include "rebootconfig.h"
#include "sysmem.h"
#include "core/vitacompat/exitgame.h"

// Previous Module Start Handler
STMOD_HANDLER previous = NULL;

// Return Boot Status
static int isSystemBooted(void)
{

	// Find Function
	int (* _sceKernelGetSystemStatus)(void) = (void *)sctrlHENFindFunction("sceSystemMemoryManager", "SysMemForKernel", 0x452E3696);
	
	// Get System Status
	int result = _sceKernelGetSystemStatus();
		
	// System booted
	if(result == 0x20000) return 1;
	
	// Still booting
	return 0;
}

// Module Start Handler
static void ARKSyspatchOnModuleStart(SceModule2 * mod)
{

	// System fully booted Status
	static int booted = 0;
	static SceModule2* loadexec = NULL;
	
  #ifdef DEBUG
	int apitype = sceKernelInitApitype();
	
	printk("syspatch: %s(0x%04X)\r\n", mod->modname, apitype);
	hookImportByNID(mod, "KDebugForKernel", 0x84F370BC, printk);
	if (DisplaySetFrameBuf){
	    PRTSTR1("Loading module %s", mod->modname);
	}

  #endif
  
	if(strcmp(mod->modname, "sceLoadExec") == 0)
	{
		loadexec = mod;
		if (ark_config->recovery){
		    // Patch sceKernelExitGame Syscalls
    	    sctrlHENPatchSyscall((void *)sctrlHENFindFunction(mod->modname, "LoadExecForUser", 0x05572A5F), exitToLauncher);
	        sctrlHENPatchSyscall((void *)sctrlHENFindFunction(mod->modname, "LoadExecForUser", 0x2AC9954B), exitToLauncher);
		}
		goto flush;
	}
	
	if(strcmp(mod->modname, "sceDisplay_Service") == 0)
	{
	    // can use screen now
	    DisplaySetFrameBuf = (void *)sctrlHENFindFunction("sceDisplay_Service", "sceDisplay", 0x289D82FE);
	    #ifdef DEBUG
	    initScreen(DisplaySetFrameBuf);
	    #endif
		// Patch loadexec_01g.prx
		patchLoadExec(loadexec);
		goto flush;
	}
	
	// Media Sync about to start...
	if(strcmp(mod->modname, "sceMediaSync") == 0)
	{
		// Patch mediasync.prx
		patchMediaSync(mod);
		// Exit Handler
		goto flush;
	}
	
	// MesgLed Cryptography about to start...
	if(strcmp(mod->modname, "sceMesgLed") == 0)
	{
		// Patch mesg_led_01g.prx
		patchMesgLed(mod);
		// Exit Handler
		goto flush;
	}
	
	// VLF Module Patches
	if(strcmp(mod->modname, "VLF_Module") == 0)
	{
		// Patch VLF Module
		patchVLF(mod);
		// Exit Handler
		goto flush;
	}
	
	if(strcmp(mod->modname, "sceSYSCON_Driver") == 0) {
		resolve_syscon_driver((SceModule*)mod);
		goto flush;
	}
	
	// Boot Complete Action not done yet
	if(booted == 0)
	{
		// Boot is complete
		if(isSystemBooted())
		{
			// Initialize Memory Stick Speedup Cache
			msstorCacheInit();
			// syncronize printk
			printkSync();
			// Boot Complete Action done
			booted = 1;
			goto flush;
		}
	}
	
	// No need to flush
	goto exit;
	
flush:
	// Flush Cache
	flushCache();
	
exit:
	// Forward to previous Handler
	if(previous) previous(mod);
}

// Add Module Start Patcher
void syspatchInit(void)
{
	// Register Module Start Handler
	previous = sctrlHENSetStartModuleHandler(ARKSyspatchOnModuleStart);
}
