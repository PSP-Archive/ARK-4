#include <pspsdk.h>
#include <pspinit.h>
#include <globals.h>
#include <graphics.h>
#include <macros.h>
#include <module2.h>
#include <pspdisplay_kernel.h>
#include <pspsysmem_kernel.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <systemctrl_private.h>
#include <pspiofilemgr.h>
#include <pspgu.h>
#include <functions.h>
#include "exitgame.h"
#include "libs/graphics/graphics.h"

#include "core/compat/psp/rebootex/payload.h"

PSP_MODULE_INFO("ARKCompatLayer", 0x3007, 1, 0);

// Previous Module Start Handler
STMOD_HANDLER previous = NULL;

// for some model specific patches
u32 psp_model = 0;

ARKConfig* ark_config = NULL;
SEConfig* se_config = NULL;

extern void (*prevPluginHandler)(const char* path, int modid);
extern void pluginHandler(const char* path, int modid);
extern void PSPOnModuleStart(SceModule2 * mod);
extern int (*prev_start)(int modid, SceSize argsize, void * argp, int * modstatus, SceKernelSMOption * opt);
extern int StartModuleHandler(int modid, SceSize argsize, void * argp, int * modstatus, SceKernelSMOption * opt);

// Flush Instruction and Data Cache
void flushCache()
{
    // Flush Instruction Cache
    sceKernelIcacheInvalidateAll();
    
    // Flush Data Cache
    sceKernelDcacheWritebackInvalidateAll();
}

void processArkConfig(){
    se_config = sctrlSEGetConfig(NULL);
    ark_config = sctrlHENGetArkConfig(NULL);
    if (ark_config->exec_mode == DEV_UNK){
        ark_config->exec_mode = PSP_ORIG; // assume running on PSP
    }
}

// Boot Time Entry Point
int module_start(SceSize args, void * argp)
{
    // set rebootex for PSP
    sctrlHENSetRebootexOverride(rebootbuffer_psp);
    
    // get psp model
    psp_model = sceKernelGetModel();
    
    // get ark config
    processArkConfig();

    // Register Module Start Handler
    previous = sctrlHENSetStartModuleHandler(PSPOnModuleStart);
    
    // Register custom start module
    prev_start = sctrlSetStartModuleExtra(StartModuleHandler);
    
    // Register plugin handler
    prevPluginHandler = sctrlHENSetPluginHandler(&pluginHandler);
    
    // Return Success
    return 0;
}
