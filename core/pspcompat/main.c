#include <pspsdk.h>
#include <pspinit.h>
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
#include "high_mem.h"
#include "exitgame.h"
#include "libs/graphics/graphics.h"

PSP_MODULE_INFO("ARKPSPCompat", 0x3007, 1, 0);

// Previous Module Start Handler
STMOD_HANDLER previous = NULL;

// for some model specific patches
u32 psp_model = 0;

static ARKConfig _ark_conf;
ARKConfig* ark_config = &_ark_conf;

extern void PSPOnModuleStart(SceModule2 * mod);

// Flush Instruction and Data Cache
void flushCache()
{
    // Flush Instruction Cache
    sceKernelIcacheInvalidateAll();
    
    // Flush Data Cache
    sceKernelDcacheWritebackInvalidateAll();
}

static void processArkConfig(ARKConfig* ark_config){
    sctrlHENGetArkConfig(ark_config);
    if (ark_config->exec_mode == DEV_UNK){
        ark_config->exec_mode = PSP_ORIG; // assume running on PSP
        sctrlHENSetArkConfig(ark_config); // notify SystemControl
    }
}

// Boot Time Entry Point
int module_start(SceSize args, void * argp)
{
    #ifdef DEBUG
    // set LCD framebuffer in hardware reg so we can do color debbuging
    _sw(0x44000000, 0xBC800100);
    #endif
    
    // get psp model
    psp_model = sceKernelGetModel();
    // get ark config
    processArkConfig(ark_config);
    // Register Module Start Handler
    previous = sctrlHENSetStartModuleHandler(PSPOnModuleStart);
    // Return Success
    return 0;
}
