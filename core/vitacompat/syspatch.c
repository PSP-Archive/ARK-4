#include <pspsdk.h>
#include <pspsysmem_kernel.h>
#include <systemctrl.h>
#include <systemctrl_private.h>
#include <globals.h> 
#include "functions.h"
#include "macros.h"
#include "exitgame.h"
#include "popspatch.h"
#include "libs/graphics/graphics.h"

extern STMOD_HANDLER previous;

KernelFunctions _ktbl = { // for vita flash patcher
    .KernelDcacheInvalidateRange = &sceKernelDcacheInvalidateRange,
    .KernelIcacheInvalidateAll = &sceKernelIcacheInvalidateAll,
    .KernelDcacheWritebackInvalidateAll = &sceKernelDcacheWritebackInvalidateAll,
    .KernelIOOpen = &sceIoOpen,
    .KernelIORead = &sceIoRead,
    .KernelIOClose = &sceIoClose,
    .KernelDelayThread = &sceKernelDelayThread,
};

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

int doColorDebug(){
    colorDebug(0xff);
    _sw(0x44000000, 0xBC800100);
    return 0;
}

void ARKVitaOnModuleStart(SceModule2 * mod){

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
    
    if (strcmp(mod->modname, "PopcornManager") == 0){
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
            // Allow exiting through key combo
            patchExitGame();
            // Initialize Memory Stick Speedup Cache
            msstorCacheInit("ms", 16 * 1024);
            // Apply Directory IO PSP Emulation
            patchFileSystemDirSyscall();
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
