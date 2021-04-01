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

extern int is_launcher_mode;
extern int use_mscache;
extern void settingsHandler(char* path);

// Flush Instruction and Data Cache
void flushCache()
{
    // Flush Instruction Cache
    sceKernelIcacheInvalidateAll();
    
    // Flush Data Cache
    sceKernelDcacheWritebackInvalidateAll();
}

void PROOnModuleStart(SceModule2 * mod){
    // System fully booted Status
    static int booted = 0;
    
    if(booted == 0)
    {
        // Boot is complete
        if(isSystemBooted())
        {
            if (use_mscache){
                if (psp_model == PSP_GO)
                    msstorCacheInit("eflash0a0f1p", 8 * 1024);
                else
                    msstorCacheInit("msstor0p", 16 * 1024);
            }
            // Boot Complete Action done
            booted = 1;
            goto flush;
        }
    }
    
flush:
    flushCache();

    // Forward to previous Handler
    if(previous) previous(mod);
}

void PROSysPatch(){
    SceModule2* mod = NULL;
    if((mod = sceKernelFindModuleByName("sceUmdMan_driver")) != NULL) {
        patch_sceUmdMan_driver(mod);
    }

    if((mod = sceKernelFindModuleByName("sceUmdCache_driver")) != NULL) {
        patch_umdcache(mod);
    }

    if((mod = sceKernelFindModuleByName("sceWlan_Driver")) != NULL) {
        patch_sceWlan_Driver(mod);
    }

    if((mod = sceKernelFindModuleByName("scePower_Service")) != NULL) {
        patch_scePower_Service(mod);
    }
    
    if (psp_model > PSP_1000 && sceKernelApplicationType() == PSP_INIT_KEYCONFIG_GAME) {
        prepatch_partitions();
    }
    
    loadSettings(&settingsHandler);
    
    if (is_launcher_mode){
        strcpy(ark_config->launcher, ARK_MENU); // set CFW in launcher mode
    }
    else{
        ark_config->launcher[0] = 0; // disable launcher mode
    }
    sctrlHENSetArkConfig(ark_config);
    
    flushCache();
}

void processArkConfig(ARKConfig* ark_config){
    sctrlHENGetArkConfig(ark_config);
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
    // Do PRO patches
    PROSysPatch();
    // notify SystemControl of changes in runtime config
    sctrlHENSetArkConfig(ark_config);
    // Register Module Start Handler
    previous = sctrlHENSetStartModuleHandler(PROOnModuleStart);
    // Return Success
    return 0;
}
