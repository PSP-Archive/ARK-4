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
#include <pspsysmem_kernel.h>
#include <systemctrl.h>
#include <systemctrl_private.h>
#include <globals.h> 
#include "functions.h"
#include "macros.h"
#include "exitgame.h"
#include "psxspu.h"
#include "libs/graphics/graphics.h"

PSP_MODULE_INFO("ARKVitaCompat", 0x3007, 1, 0);

static ARKConfig _ark_conf;
ARKConfig* ark_config = &_ark_conf;

KernelFunctions _ktbl = { // for vita flash patcher
    .KernelDcacheInvalidateRange = &sceKernelDcacheInvalidateRange,
    .KernelIcacheInvalidateAll = &sceKernelIcacheInvalidateAll,
    .KernelDcacheWritebackInvalidateAll = &sceKernelDcacheWritebackInvalidateAll,
    .KernelIOOpen = &sceIoOpen,
    .KernelIORead = &sceIoRead,
    .KernelIOClose = &sceIoClose,
    .KernelDelayThread = &sceKernelDelayThread,
};

// Previous Module Start Handler
STMOD_HANDLER previous = NULL;

// Flush Instruction and Data Cache
void flushCache()
{
    // Flush Instruction Cache
    sceKernelIcacheInvalidateAll();
    
    // Flush Data Cache
    sceKernelDcacheWritebackInvalidateAll();
}

// Return Game Product ID of currently running Game
int sctrlARKGetGameID(char gameid[GAME_ID_MINIMUM_BUFFER_SIZE])
{
    // Invalid Arguments
    if(gameid == NULL) return -1;
    
    // Elevate Permission Level
    unsigned int k1 = pspSdkSetK1(0);
    
    // Fetch Game Information Structure
    void * gameinfo = SysMemForKernel_EF29061C_Fixed();
    
    // Restore Permission Level
    pspSdkSetK1(k1);
    
    // Game Information unavailable
    if(gameinfo == NULL) return -3;
    
    // Copy Product Code
    memcpy(gameid, gameinfo + 0x44, GAME_ID_MINIMUM_BUFFER_SIZE - 1);
    
    // Terminate Product Code
    gameid[GAME_ID_MINIMUM_BUFFER_SIZE - 1] = 0;
    
    // Return Success
    return 0;
}

// Return Boot Status
static int isSystemBooted(void)
{

    // Find Function
    int (* _sceKernelGetSystemStatus)(void) = (void*)sctrlHENFindFunction("sceSystemMemoryManager", "SysMemForKernel", 0x452E3696);
    
    // Get System Status
    int result = _sceKernelGetSystemStatus();
        
    // System booted
    if(result == 0x20000) return 1;
    
    // Still booting
    return 0;
}

static void ARKVitaOnModuleStart(SceModule2 * mod){

    // System fully booted Status
    static int booted = 0;

    if(strcmp(mod->modname, "sceLoadExec") == 0)
    {
        // Patch sceKernelExitGame Syscalls
        sctrlHENPatchSyscall((void*)sctrlHENFindFunction(mod->modname, "LoadExecForUser", 0x05572A5F), exitToLauncher);
        sctrlHENPatchSyscall((void*)sctrlHENFindFunction(mod->modname, "LoadExecForUser", 0x2AC9954B), exitToLauncher);
        sctrlHENPatchSyscall((void*)sctrlHENFindFunction(mod->modname, "LoadExecForUser", 0x08F7166C), exitToLauncher);
        goto flush;
    }
    
    if (strcmp(mod->modname, "pops") == 0)
    {
        // Patch POPS SPU
        patchVitaPopsSpu(mod);
        goto flush;
    }
    
    if (strcmp(mod->modname, "scePops_Manager") == 0){
        patchVitaPopsman(mod);
        goto flush;
    }
    
    // Kermit Peripheral Patches
    if(strcmp(mod->modname, "sceKermitPeripheral_Driver") == 0)
    {
        // Patch Kermit Peripheral Module to load flash0
        patchKermitPeripheral(&_ktbl);
        // Exit Handler
        goto flush;
    }
       
    // Boot Complete Action not done yet
    if(booted == 0)
    {
        // Boot is complete
        if(isSystemBooted())
        {
            // Apply Directory IO PSP Emulation
            patchFileSystemDirSyscall();
            // Allow exiting through key combo
            //patchExitGame();
            // Boot Complete Action done
            booted = 1;
            goto flush;
        }
    }

flush:
    flushCache();

exit:
       // Forward to previous Handler
    if(previous) previous(mod);
}

// Boot Time Entry Point
int module_start(SceSize args, void * argp)
{

    // ask iso driver load fake.cso when no iso available
    char* umdfile = GetUmdFile();
    if(umdfile[0] == '\0')
    {
        SetUmdFile("flash0:/fake.cso");
    }

    // copy configuration
    sctrlHENGetArkConfig(ark_config);
    
    initFileSystem();
    
    //unprotectVitaMemory();
    
    //patchFileManager();
    
    // Register Module Start Handler
    previous = sctrlHENSetStartModuleHandler(ARKVitaOnModuleStart);

    // Return Success
    return 0;
}

int module_stop(SceSize args, void *argp)
{
    spuShutdown();
    return 0;
}
