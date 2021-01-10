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
#include "imports.h"
#include "modulemanager.h"
#include "filesystem.h"
#include "cryptography.h"
#include "mediasync.h"
#include "msstor_cache.h"
#include "loadexec.h"
#include "vlffix.h"
#include "rebootconfig.h"
#include "sysmem.h"
#include "lflash0.h"

// Previous Module Start Handler
STMOD_HANDLER previous = NULL;

// Return Boot Status
static int isSystemBooted(void)
{

	// Find Function
	int (* sceKernelGetSystemStatus)(void) = (void *)sctrlHENFindFunction("sceSystemMemoryManager", "SysMemForKernel", 0x452E3696);
	
	// Get System Status
	int result = sceKernelGetSystemStatus();
		
	// System booted
	if(result == 0x20000) return 1;
	
	// Still booting
	return 0;
}

void vitaPopsOnModuleStart(SceModule2* mod){

	#ifdef DEBUG
	cls();
	PRTSTR(mod->modname);
	copyPSPVram(NULL);
	#endif
	
	if(strcmp(mod->modname, "sceLoadExec") == 0){
		// Patch loadexec_01g.prx
		patchLoadExec();
		flushCache();
	}
	else if(strcmp(mod->modname, "sceDisplay_Service") == 0){
		// Patch Vita POPS display
		patchVitaPopsDisplay();
		flushCache();
	}
	else if(strcmp(mod->modname, "sceMediaSync") == 0){
		// Patch mediasync.prx
		patchMediaSync(mod->text_addr);
		flushCache();
	}
	else if(strcmp(mod->modname, "sceMesgLed") == 0){
		// Patch mesg_led_01g.prx
		patchMesgLed(mod->text_addr);
		flushCache();
	}
	/*
	else if(strcmp(mod->modname, "scePops_Manager") == 0){
		// Patch popsman
		patchVitaPopsManager(mod);
		flushCache();
	}
	else if(strcmp(mod->modname, "pops") == 0){
		// Patch pops
		patchVitaPops(mod);
		flushCache();
	}
	*/
	else if(strcmp(mod->modname, "sceKermitPeripheral_Driver") == 0){
		// Patch Kermit Peripheral Module
		patchKermitPeripheral(mod);
		flushCache();
	}
	/*
	else if(strcmp(mod->modname, "complex") == 0 || strcmp(mod->modname, "simple") == 0){
		installModuleJALTrace(mod);
		flushCache();
	}
	*/
}

// Module Start Handler
static void PROSyspatchOnModuleStart(SceModule2 * mod)
{

	// System fully booted Status
	static int booted = 0;

/*
#ifdef DEBUG
	int apitype = sceKernelInitApitype();
	
	printk("syspatch: %s(0x%04X)\r\n", mod->modname, apitype);
	hookImportByNID(mod, "KDebugForKernel", 0x84F370BC, printk);
#endif
*/

	// Apply Directory IO PSP Emulation
	patchFileManagerImports(mod);
	
	// Apply Game ID Patch
	patchGameInfoGetter(mod);
	
	PRTSTR1("Loading module %s", mod->modname);
	
	if(strcmp(mod->modname, "sceLoadExec") == 0)
	{
		// Patch loadexec_01g.prx
		patchLoadExec();
		
		goto flush;
	}
	
	// Media Sync about to start...
	if(0 == strcmp(mod->modname, "sceMediaSync"))
	{
		// Patch mediasync.prx
		patchMediaSync(mod->text_addr);
		// Exit Handler
		goto flush;
	}
	
	// MesgLed Cryptography about to start...
	if(0 == strcmp(mod->modname, "sceMesgLed"))
	{
		// Patch mesg_led_01g.prx
		patchMesgLed(mod->text_addr);
		// Exit Handler
		goto flush;
	}
	
	// VLF Module Patches
	if(0 == strcmp(mod->modname, "VLF_Module"))
	{
		// Patch VLF Module
		patchVLF(mod);
		
		// Exit Handler
		goto flush;
	}

	// Kermit Peripheral Patches
	if(0 == strcmp(mod->modname, "sceKermitPeripheral_Driver"))
	{
		// Patch Kermit Peripheral Module
		patchKermitPeripheral(mod);
		// Exit Handler
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
			
			patchFileSystemDirSyscall();
			printkSync();

			// Boot Complete Action done
			booted = 1;
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
	//PSXFlashScreen(0xff00);
}

// Add Module Start Patcher
void syspatchInit(void)
{
	// Register Module Start Handler
	if (!IS_VITA_POPS)
		previous = sctrlHENSetStartModuleHandler(PROSyspatchOnModuleStart);
}
