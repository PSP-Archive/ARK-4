#include <pspsdk.h>
#include <globals.h>
#include <graphics.h>
#include <macros.h>
#include <module2.h>
#include <pspdisplay_kernel.h>
#include <pspsysmem_kernel.h>
#include <systemctrl.h>
#include <systemctrl_private.h>
#include <pspiofilemgr.h>
#include <pspgu.h>
#include <functions.h>
#include "popsdisplay.h"

PSP_MODULE_INFO("VitaPops", 0x3007, 1, 0);

// Previous Module Start Handler
STMOD_HANDLER previous = NULL;

static ARKConfig _ark_conf;
ARKConfig* ark_config = &_ark_conf;

extern void patchVitaPopsDisplay(SceModule2* mod);

KernelFunctions _ktbl = {
    .KernelDcacheInvalidateRange = &sceKernelDcacheInvalidateRange,
    .KernelIcacheInvalidateAll = &sceKernelIcacheInvalidateAll,
    .KernelDcacheWritebackInvalidateAll = &sceKernelDcacheWritebackInvalidateAll,
    .KernelIOOpen = &sceIoOpen,
    .KernelIORead = &sceIoRead,
    .KernelIOClose = &sceIoClose,
    .KernelDelayThread = &sceKernelDelayThread,
}; // for vita flash patcher

// Flush Instruction and Data Cache
void flushCache()
{
    // Flush Instruction Cache
    sceKernelIcacheInvalidateAll();
    
    // Flush Data Cache
    sceKernelDcacheWritebackInvalidateAll();
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

static void ARKVitaPopsOnModuleStart(SceModule2 * mod){

    static int booted = 0;

    // Patch display in PSX exploits
    if(strcmp(mod->modname, "sceDisplay_Service") == 0) {
        patchVitaPopsDisplay(mod);
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
    // copy configuration
    sctrlHENGetArkConfig(ark_config);
    // Register Module Start Handler
    previous = sctrlHENSetStartModuleHandler(ARKVitaPopsOnModuleStart);
    // Return Success
    return 0;
}
