#include <pspsdk.h>
#include <pspkernel.h>
#include <pspiofilemgr_kernel.h>
#include <pspumd.h>
#include <systemctrl.h>
#include <macros.h>
#include <stdio.h>
#include <string.h>
#include <ark.h>

PSP_MODULE_INFO("BlueScreenOfDeath", 0x1007, 1, 0);

extern void registerExceptionHandler(PspDebugErrorHandler handler, PspDebugRegBlock * regs);

ARKConfig* ark_config = NULL;

// for screen debugging
int (* DisplaySetFrameBuf)(void*, int, int, int) = NULL;

// Previous Module Start Handler
STMOD_HANDLER previous = NULL;

int OnModuleStart(SceModule2* mod){

    if(strcmp(mod->modname, "sceDisplay_Service") == 0)
    {
        // can use screen now
        DisplaySetFrameBuf = (void*)sctrlHENFindFunction("sceDisplay_Service", "sceDisplay", 0x289D82FE);
    }

    if (previous) return previous;
    return 0;
}

int module_start(){

    // get ARK Config
    ark_config = sctrlHENGetArkConfig(NULL);

    // Register Default Exception Handler
    registerExceptionHandler(NULL, NULL);

    // Register module handler
    previous = sctrlHENSetStartModuleHandler(OnModuleStart);

    return 0;
}