#include <pspsdk.h>
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
#include <pspsysevent.h>
#include <functions.h>
#include "popsdisplay.h"

#include "core/compat/vitapops/rebootex/payload.h"

PSP_MODULE_INFO("ARKCompatLayer", 0x3007, 1, 0);

// Previous Module Start Handler
STMOD_HANDLER previous = NULL;

ARKConfig* ark_config = NULL;
SEConfig* se_config = NULL;

extern void ARKVitaPopsOnModuleStart(SceModule2* mod);
extern int (*prev_start)(int modid, SceSize argsize, void * argp, int * modstatus, SceKernelSMOption * opt);
extern int StartModuleHandler(int modid, SceSize argsize, void * argp, int * modstatus, SceKernelSMOption * opt);

void* sctrlARKSetPSXVramHandler(void (*handler)(u32* psp_vram, u16* ps1_vram)){
    int k1 = pspSdkSetK1(0);
    void* prev = registerPSXVramHandler(handler);
    pspSdkSetK1(k1);
    return prev;
}

static void processArkConfig(){
    se_config = sctrlSEGetConfig(NULL);
    ark_config = sctrlHENGetArkConfig(NULL);
    if (ark_config->exec_mode == DEV_UNK){
        ark_config->exec_mode = PSV_POPS; // assume running on PS Vita Pops
    }
    if (ark_config->launcher[0] == '\0'){
        strcpy(ark_config->launcher, ARK_XMENU);
    }
}

// Boot Time Entry Point
int module_start(SceSize args, void * argp)
{
    
    // copy configuration
    processArkConfig();

    if (ark_config->exec_mode != PSV_POPS){
        return 1;
    }

    #ifdef DEBUG
    _sw(0x44000000, 0xBC800100);
    setScreenHandler(&copyPSPVram);
    initVitaPopsVram();
    colorDebug(0xFF00);
    #endif

    // set rebootex for VitaPOPS
    sctrlHENSetRebootexOverride(rebootbuffer_vitapops);

    //initFileSystem();

    // Register Module Start Handler
    previous = sctrlHENSetStartModuleHandler(ARKVitaPopsOnModuleStart);
    
    // Return Success
    return 0;
}
