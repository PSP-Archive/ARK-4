#include <pspsdk.h>
#include <pspkernel.h>
#include <pspiofilemgr_kernel.h>
#include <pspumd.h>
#include <systemctrl.h>
#include <macros.h>
#include <stdio.h>
#include <string.h>
#include <ark.h>
#include "graphics.h"

PSP_MODULE_INFO("BlueScreenOfDeath", 0, 1, 0);

extern int registerExceptionHandler(PspDebugErrorHandler handler, PspDebugRegBlock * regs);

// for screen debugging
int (* DisplaySetFrameBuf)(void*, int, int, int) = NULL;

// Previous Module Start Handler
STMOD_HANDLER previous = NULL;

// Return Boot Status
int isSystemBooted(void)
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

int OnModuleStart(SceModule2* mod){

    static int booted = 0;

    if (DisplaySetFrameBuf && !booted){
        initScreen(DisplaySetFrameBuf);
        PRTSTR1("Module: %s", mod->modname);
    }

    if (strcmp(mod->modname, "sceDisplay_Service") == 0)
    {
        // can use screen now
        DisplaySetFrameBuf = (void*)sctrlHENFindFunction("sceDisplay_Service", "sceDisplay", 0x289D82FE);
    }

    if (!booted && isSystemBooted()){
        booted = 1;
    }

    if (previous) return previous(mod);
    return 0;
}

void textlog(char* text){
    int fd = sceIoOpen("ms0:/bsod.log", PSP_O_RDONLY|PSP_O_CREAT|PSP_O_APPEND, 0777);
    sceIoWrite(fd, text, strlen(text));
    sceIoClose(fd);
}

int module_start(){

    int res = 0;
    //int intc = pspSdkDisableInterrupts();

    // flash green screen
    _sw(0x44000000, 0xBC800100);

    // Register Default Exception Handler
    if ((res=registerExceptionHandler(NULL, NULL)) < 0){
        colorDebug(0xFF); // red color on init error
        static char text[128];
        sprintf(text, "exception handler result = %p\n", res);
        textlog(text);
    }
    else colorDebug(0xFF00);

    DisplaySetFrameBuf = (void*)sctrlHENFindFunction("sceDisplay_Service", "sceDisplay", 0x289D82FE);

    if (!DisplaySetFrameBuf){ // can't use this function now?
        // Register module handler, see if we can find it later
        previous = sctrlHENSetStartModuleHandler(OnModuleStart);
    }
    else {
        initScreen(DisplaySetFrameBuf);
        PRTSTR1("BSoD Started: %p", res);
        if (res<0) sceKernelDelayThread(10000000);
    }

    static u32 patch_nids[] = {0x565C0B0E, 0x3FB264FC, 0x5A837AD4};
    for (int i=0; i<NELEMS(patch_nids); i++){
        u32 patch_addr = sctrlHENFindFunction("sceExceptionManager", "ExceptionManagerForKernel", patch_nids[i]);
        if (patch_addr){
            MAKE_DUMMY_FUNCTION_RETURN_0(patch_addr);
        }
    }

    //pspSdkEnableInterrupts(intc);

    return 0;
}