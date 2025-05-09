#include <pspsdk.h>
#include <pspkernel.h>
#include <pspdisplay.h>
#include <systemctrl.h>
#include <kubridge.h>
#include <macros.h>
#include <stdio.h>
#include <string.h>
#include <ark.h>

PSP_MODULE_INFO("BlueScreenOfDeath_User", 0x0007, 1, 0);

// for screen debugging
int (* DisplaySetFrameBuf)(void*, int, int, int) = NULL;

// Exception Handler
PspDebugErrorHandler curr_handler = NULL;

// Register Snapshot
PspDebugRegBlock * exception_regs = NULL;

// ASM Exception Handler Payload
extern void _pspDebugExceptionHandler(void);

// Bluescreen Register Snapshot
static PspDebugRegBlock cpuRegs;

void DefaultHandler(PspDebugRegBlock * regs){
    struct KernelCallArg args;
    args.arg1 = regs;
    kuKernelCall(sctrlHENFindFunction("BlueScreenOfDeath_Kernel", "BlueScreenOfDeathLib", 0xCDA22F1B), &args);
}

int module_start(){

    // Register Default Exception Handler
    curr_handler = &DefaultHandler;
    exception_regs = &cpuRegs;
     
    // Register Exception Handler
    struct KernelCallArg args;
    args.arg1 = _pspDebugExceptionHandler;
    kuKernelCall(sctrlHENFindFunction("sceExceptionManager", "ExceptionManagerForKernel", 0x565C0B0E), &args);

    return 0;
}