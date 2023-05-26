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
#include "rebootconfig.h"
#include "sysmem.h"

extern u32 sctrlHENFakeDevkitVersion();
extern int is_plugins_loading;

// Previous Module Start Handler
STMOD_HANDLER previous = NULL;

#ifdef DEBUG
// for screen debugging
int (* DisplaySetFrameBuf)(void*, int, int, int) = NULL;
#endif

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

    // Fix 6.60 plugins on 6.61
    if (is_plugins_loading){
        hookImportByNID(mod, "SysMemForKernel", 0x3FC9AE6A, &sctrlHENFakeDevkitVersion);
        hookImportByNID(mod, "SysMemUserForUser", 0x3FC9AE6A, &sctrlHENFakeDevkitVersion);
    }
    
    #ifdef DEBUG
    printk("syspatch: %s(0x%04X)\r\n", mod->modname, sceKernelInitApitype());
    hookImportByNID(mod, "KDebugForKernel", 0x84F370BC, printk);

    if (sceKernelFindModuleByName("vsh_module") == NULL){
        initScreen(DisplaySetFrameBuf);
        PRTSTR1("Module: %s", mod->modname);
    }

    if(strcmp(mod->modname, "sceDisplay_Service") == 0)
    {
        // can use screen now
        DisplaySetFrameBuf = (void*)sctrlHENFindFunction("sceDisplay_Service", "sceDisplay", 0x289D82FE);
        goto flush;
    }

    #endif

    if (strcmp(mod->modname, "sceController_Service") == 0){
        initController(mod);
        goto flush;
    }

    if(strcmp(mod->modname, "sceLoadExec") == 0)
    {
        // Find Reboot Loader Function
        OrigLoadReboot = (void *)mod->text_addr;
        // Patch loadexec
        patchLoadExec(mod, (u32)LoadReboot, (u32)sctrlHENFindFunction("sceThreadManager", "ThreadManForKernel", 0xF6427665), 3);
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

    // unlocks variable bitrate on old homebrew
    if (strcmp(mod->modname, "sceMp3_Library") == 0){
        hookImportByNID(mod, "SysMemUserForUser", 0xFC114573, &sctrlHENFakeDevkitVersion);
        goto flush;
    }

    if (strcmp(mod->modname, "sceNpSignupPlugin_Module") == 0) {
        patch_npsignup(mod);
        goto flush;
    }

    if (strcmp(mod->modname, "sceVshNpSignin_Module") == 0) {
        patch_npsignin(mod);
        goto flush;
    }

    if (strcmp(mod->modname, "sceNp") == 0) {
        patch_np(mod, 9, 90);
        goto flush;
    }

    // Boot Complete Action not done yet
    if(booted == 0)
    {
        // Boot is complete
        if(isSystemBooted())
        {

            // Allow exiting through key combo
            patchController();
            
            #ifdef DEBUG
            // syncronize printk
            printkSync();
            #endif

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
