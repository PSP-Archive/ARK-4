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

PSP_MODULE_INFO("ARKVitaPopsCompat", 0x3007, 1, 0);

// Previous Module Start Handler
STMOD_HANDLER previous = NULL;

static ARKConfig _ark_conf;
ARKConfig* ark_config = &_ark_conf;

extern void ARKVitaPopsOnModuleStart(SceModule2* mod);

// Flush Instruction and Data Cache
void flushCache()
{
    // Flush Instruction Cache
    sceKernelIcacheInvalidateAll();
    
    // Flush Data Cache
    sceKernelDcacheWritebackInvalidateAll();
}

void* sctrlARKSetPSXVramHandler(void (*handler)(u32* psp_vram, u16* ps1_vram)){
    int k1 = pspSdkSetK1(0);
    void* prev = registerPSXVramHandler(handler);
    pspSdkSetK1(k1);
    return prev;
}

static void processArkConfig(ARKConfig* ark_config){
    sctrlHENGetArkConfig(ark_config);
    if (ark_config->exec_mode == DEV_UNK){
        ark_config->exec_mode = PSV_POPS; // assume running on PS Vita Pops
        sctrlHENSetArkConfig(ark_config); // notify SystemControl
    }
}

#ifdef DEBUG
static void pops_vram_handler(u32 vram){
    SoftRelocateVram(vram, NULL);
}
#endif

// Boot Time Entry Point
int module_start(SceSize args, void * argp)
{
    #ifdef DEBUG
    setScreenHandler(&pops_vram_handler);
    #endif
    
    // copy configuration
    processArkConfig(ark_config);
    // Register Module Start Handler
    previous = sctrlHENSetStartModuleHandler(ARKVitaPopsOnModuleStart);
    // Return Success
    return 0;
}
