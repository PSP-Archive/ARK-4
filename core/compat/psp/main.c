#include <pspsdk.h>
#include <pspinit.h>
#include <ark.h>
#include <graphics.h>
#include <macros.h>
#include <module2.h>
#include <pspdisplay_kernel.h>
#include <pspsysmem_kernel.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <systemctrl_private.h>
#include <pspiofilemgr.h>
#include <pspsysevent.h>
#include <pspgu.h>
#include <functions.h>
#include "libs/graphics/graphics.h"

#include "core/compat/psp/rebootex/payload.h"

PSP_MODULE_INFO("ARKCompatLayer", 0x3007, 1, 0);

// for some model specific patches
u32 psp_model = 0;

ARKConfig* ark_config = NULL;
SEConfig* se_config = NULL;

void processArkConfig(){
    if (ark_config->exec_mode == DEV_UNK){
        ark_config->exec_mode = PSP_ORIG; // assume running on PSP
    }
}

// Boot Time Entry Point
int module_start(SceSize args, void * argp)
{
    
    // get psp model
    psp_model = sceKernelGetModel();

    se_config = sctrlSEGetConfig(NULL);
    ark_config = sctrlArkGetConfig(NULL);

    if (ark_config == NULL){
        return 1;
    }
    
    // get ark config
    processArkConfig();

    if (!IS_PSP(ark_config)){
        return 2;
    }

    // set rebootex for PSP
    sctrlHENSetRebootexOverride(rebootbuffer_psp);

    PSPSyspatchStart();

    sctrlFlushCache();
    
    // Return Success
    return 0;
}
