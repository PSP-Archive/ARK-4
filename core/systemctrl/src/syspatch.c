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
extern SEConfig se_config;

// Previous Module Start Handler
STMOD_HANDLER previous = NULL;

#ifdef DEBUG
// for screen debugging
int (* DisplaySetFrameBuf)(void*, int, int, int) = NULL;
#endif

// Return Boot Status
int isSystemBooted(void)
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

static unsigned int fakeFindFunction(char * szMod, char * szLib, unsigned int nid){
    if (nid == 0x221400A6 && strcmp(szMod, "SystemControl") == 0)
        return 0; // Popsloader V4 looks for this function to check for ME, let's pretend ARK doesn't have it ;)
    return sctrlHENFindFunction(szMod, szLib, nid);
}

int _sceChkreg_6894A027(u8* a0, u32 a1){
    if (a0 && a1 == 0){
        *a0 = 1;
        return 0;
    }
    return -1;
}

void patch_qaflags(){
    u32 fp;
   
    // sceChkregGetPsCode
    fp = sctrlHENFindFunction("sceChkreg", "sceChkreg_driver", 0x6894A027); 

    if (fp) {
        _sw(JUMP(_sceChkreg_6894A027), fp);
        _sw(NOP, fp+4);
    }
}

// Module Start Handler
static void ARKSyspatchOnModuleStart(SceModule2 * mod)
{

    // System fully booted Status
    static int booted = 0;

    patchGameInfoGetter(mod);

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
        // Allow exiting through key combo
        patchController(mod);
        goto flush;
    }

    if(strcmp(mod->modname, "sceLoadExec") == 0)
    {
        // Find Reboot Loader Function
        OrigLoadReboot = (void *)mod->text_addr;
        // Patch loadexec
        patchLoadExec(mod, (u32)LoadReboot, (u32)sctrlHENFindFunction("sceThreadManager", "ThreadManForKernel", 0xF6427665), 3);

        // Hijack all execute calls
        extern int (* _sceLoadExecVSHWithApitype)(int, const char*, struct SceKernelLoadExecVSHParam*, unsigned int);
        extern int sctrlKernelLoadExecVSHWithApitype(int apitype, const char * file, struct SceKernelLoadExecVSHParam * param);
        u32 _LoadExecVSHWithApitype = findFirstJAL(sctrlHENFindFunction("sceLoadExec", "LoadExecForKernel", 0xD8320A28));
        HIJACK_FUNCTION(_LoadExecVSHWithApitype, sctrlKernelLoadExecVSHWithApitype, _sceLoadExecVSHWithApitype);

        // Hijack exit calls
        extern int (*_sceKernelExitVSH)(void*);
        extern int sctrlKernelExitVSH(struct SceKernelLoadExecVSHParam *param);
        u32 _KernelExitVSH = sctrlHENFindFunction("sceLoadExec", "LoadExecForKernel", 0x08F7166C);
        HIJACK_FUNCTION(_KernelExitVSH, sctrlKernelExitVSH, _sceKernelExitVSH);
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

    // unlocks mp3 variable bitrate and qwerty osk on old games/homebrew
    if (strcmp(mod->modname, "sceMp3_Library") == 0 || strcmp(mod->modname, "sceVshOSK_Module") == 0){
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

    if (strcmp(mod->modname, "popsloader") == 0 || strcmp(mod->modname, "popscore") == 0){
        // fix for 6.60 check on 6.61
        hookImportByNID(mod, "SysMemForKernel", 0x3FC9AE6A, &sctrlHENFakeDevkitVersion);
        // fix to prevent ME detection
        hookImportByNID(mod, "SystemCtrlForKernel", 0x159AF5CC, &fakeFindFunction);
        goto flush;
    }

    if (strcmp(mod->modname, "DayViewer_User") == 0){
        // fix scePaf imports in DayViewer
        static u32 nids[] = {
            0x2BE8DDBB, 0xE8CCC611, 0xCDDCFFB3, 0x48BB05D5, 0x22FB4177, 0xBC8DC92B, 0xE3D530AE
        };
        for (int i=0; i<NELEMS(nids); i++){
            hookImportByNID(mod, "scePaf", nids[i], sctrlHENFindFunction("scePaf_Module", "scePaf", nids[i]));
        }
        goto flush;
    }

    // Boot Complete Action not done yet
    if(booted == 0)
    {
        // Boot is complete
        if(isSystemBooted())
        {

            if (se_config.qaflags){
                patch_qaflags();
            }

            if (se_config.umdseek || se_config.umdspeed){
                se_config.iso_cache = 0;
                void (*SetUmdDelay)(int, int) = sctrlHENFindFunction("PRO_Inferno_Driver", "inferno_driver", 0xB6522E93);
                if (SetUmdDelay) SetUmdDelay(se_config.umdseek, se_config.umdspeed);
            }

            if (se_config.iso_cache){
                int (*CacheInit)(int, int, int) = sctrlHENFindFunction("PRO_Inferno_Driver", "inferno_driver", 0x8CDE7F95);
                if (CacheInit){
                    CacheInit(se_config.iso_cache_size, se_config.iso_cache_num, se_config.iso_cache_partition);
                }
                if (se_config.iso_cache == 2){
                    int (*CacheSetPolicy)(int) = sctrlHENFindFunction("PRO_Inferno_Driver", "inferno_driver", 0xC0736FD6);
                    if (CacheSetPolicy){
                        CacheSetPolicy(CACHE_POLICY_RR);
                    }
                }
            }

            // handle CPU speed
            switch (se_config.cpubus_clock){
                case 1: sctrlHENSetSpeed(333, 166); break;
                case 2: sctrlHENSetSpeed(133, 66); break;
                case 3: sctrlHENSetSpeed(222, 111); break;
            }
            
            #ifdef DEBUG
            // syncronize printk
            printkSync();
            #endif

            ark_config->recovery = 0; // reset recovery mode for next reboot

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
