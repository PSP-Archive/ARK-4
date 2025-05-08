#include <pspsdk.h>
#include <pspkernel.h>
#include <pspiofilemgr_kernel.h>
#include <pspumd.h>
#include <systemctrl.h>
#include <macros.h>
#include <stdio.h>
#include <string.h>
#include <ark.h>

PSP_MODULE_INFO("BlueScreenOfDeath", 0x3007, 1, 0);

extern int registerExceptionHandler(PspDebugErrorHandler handler, PspDebugRegBlock * regs);

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

    // flash green screen
    _sw(0x44000000, 0xBC800100);
    colorDebug(0xFF00);

    // Register Default Exception Handler
    if (registerExceptionHandler(NULL, NULL) < 0)
        colorDebug(0xFF); // red color on init error

    DisplaySetFrameBuf = (void*)sctrlHENFindFunction("sceDisplay_Service", "sceDisplay", 0x289D82FE);

    if (!DisplaySetFrameBuf){ // can't use this function now?
        // Register module handler, see if we can find it later
        previous = sctrlHENSetStartModuleHandler(OnModuleStart);
    }

    return 0;
}