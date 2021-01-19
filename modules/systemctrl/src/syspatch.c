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

int is_homebrews_runlevel(void)
{
	int apitype;

	apitype = sceKernelInitApitype();
	
	if(apitype == 0x152 || apitype == 0x141) {
		return 1;
	}

	return 0;
}

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

static void patch_sceWlan_Driver(SceModule2* mod)
{
	_sw(NOP, mod->text_addr + 0x000026C0);
}

static void patch_scePower_Service(SceModule2* mod)
{
	// scePowerGetBacklightMaximum always returns 4
	_sw(NOP, mod->text_addr + 0x00000E68);
}

static int _sceKernelBootFromForUmdMan(void)
{
	return 0x20;
}

static void patch_sceUmdMan_driver(SceModule2* mod)
{
	if(is_homebrews_runlevel()) {
		hookImportByNID(mod, "InitForKernel", 0x27932388, _sceKernelBootFromForUmdMan);
	}
}

//prevent umd-cache in homebrew, so we can drain the cache partition.
void patch_umdcache(SceModule2* mod)
{
    /*
	int ret;

	ret = sceKernelApplicationType();

	if (ret != PSP_INIT_KEYCONFIG_GAME)
		return;

	ret = sctrlKernelBootFrom();

	if (ret != 0x40 && ret != 0x50)
		return;
	*/
	//kill module start
	if (is_homebrews_runlevel()){
	    _sw(JR_RA, mod->text_addr + 0x000009C8);
	    _sw(LI_V0(1), mod->text_addr + 0x000009C8 + 4);
	}
	//MAKE_DUMMY_FUNCTION_RETURN_1(mod->text_addr + 0x000009C8);
}

// Module Start Handler
static void ARKSyspatchOnModuleStart(SceModule2 * mod)
{

	// System fully booted Status
	static int booted = 0;
	
#ifdef DEBUG
	int apitype = sceKernelInitApitype();
	
	printk("syspatch: %s(0x%04X)\r\n", mod->modname, apitype);
	hookImportByNID(mod, "KDebugForKernel", 0x84F370BC, printk);
#endif

    if (IS_VITA(ark_config->exec_mode)){
	    // Apply Directory IO PSP Emulation
	    patchFileManagerImports(mod);
	    // Apply Game ID Patch
    	patchGameInfoGetter(mod);
	}
	
	if (DisplaySetFrameBuf){
	    PRTSTR1("Loading module %s", mod->modname);
	}
	
	if (strcmp(mod->modname, "sceController_Service")==0){
	    // can use screen now
	    DisplaySetFrameBuf = (void *)sctrlHENFindFunction("sceDisplay_Service", "sceDisplay", 0x289D82FE);
	    initScreen(DisplaySetFrameBuf);
	    goto flush;
	}
	
	// load after lflash
	/*
	if(0 == strcmp(mod->modname, "sceDisplay_Service")) {
	    if (IS_VITA_POPS(ark_config->exec_mode)) patchVitaPopsDisplay();
	    //patchLoadExec(loadexec_mod);
		goto flush;
	}*/
	
	if(strcmp(mod->modname, "sceLoadExec") == 0)
	{
		// Patch loadexec_01g.prx
		patchLoadExec(mod);
		// Initialize Malloc
    	oe_mallocinit();
		goto flush;
	}
	
	// Media Sync about to start...
	if(0 == strcmp(mod->modname, "sceMediaSync"))
	{
		// Patch mediasync.prx
		patchMediaSync(mod);
		/*
		int (*IoDevctl)(char*, u32, void*, int, int*, int) = sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForKernel", 0x54F5FB11);
	    int status = -1;
        PRTSTR1("Ioctl: %d", IoDevctl("fatms0:", 0x02425823, NULL, 0, &status, sizeof(status)));
        PRTSTR1("ms0 status: %p", status);
	    if (status != 1){
	        PRTSTR("Reassigning ms0");
	        int (*IoAssign)(char*, char*, char*, int, void*, int) = sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForKernel", 0xB2A628C1);
	        int (*IoUnassign)(char*) = sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForKernel", 0x6D08A871);
	        int res = IoUnassign("ms0:");
	        PRTSTR1("Unassign: %p", res);
	        res = IoAssign("ms0:", "msstor0p:", "fatms0:", 0, NULL, 0);
	        PRTSTR1("Assign: %p", res);
	        PRTSTR("ms0 assigned");
	    }
        while(1){};
        */
		// Exit Handler
		goto flush;
	}
	
	// MesgLed Cryptography about to start...
	if(0 == strcmp(mod->modname, "sceMesgLed"))
	{
		// Patch mesg_led_01g.prx
		patchMesgLed(mod);
		// Exit Handler
		goto flush;
	}
	
	// VLF Module Patches
	/*
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
	*/
	
	/*
	if(0 == strcmp(mod->modname, "sceSYSCON_Driver") && IS_PSP(ark_config->exec_mode)) {
		resolve_syscon_driver(mod);
		goto flush;
	}
	
	if(0 == strcmp(mod->modname, "sceWlan_Driver")) {
		patch_sceWlan_Driver(mod);
		//patch_Libertas_MAC(mod);
		goto flush;
	}

	if(0 == strcmp(mod->modname, "scePower_Service")) {
		patch_scePower_Service(mod);
		goto flush;
	}

    if(0 == strcmp(mod->modname, "sceUmdMan_driver")) {
		patch_sceUmdMan_driver((SceModule*)mod);
		goto flush;
	}
	if(0 == strcmp(mod->modname, "sceUmdCache_driver")) {
		patch_umdcache(mod);
		goto flush;
	}
    */
    /*
	if(0 == strcmp(mod->modname, "sceKernelLibrary")) {
		printkSync();
		goto flush;
	}*/
	
	/*
	if (strcmp(mod->modname, "sceAudio_Driver")==0){
	    int status;
        sceIoDevctl("fatms0:", 0x02425823, NULL, 0, &status, sizeof(status));
        if (status != 1)
        {
            _sw(0,0);
        }
	}
	*/

	// Boot Complete Action not done yet
	/*
	if(booted == 0)
	{
		// Boot is complete
		if(isSystemBooted())
		{
			// Initialize Memory Stick Speedup Cache
			msstorCacheInit();
			
			if (IS_VITA(ark_config->exec_mode)) patchFileSystemDirSyscall();
			printkSync();

			// Boot Complete Action done
			booted = 1;
			goto flush;
		}
	}
	*/
	
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
