#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <pspkernel.h>
#include <pspsysevent.h>
#include <psputilsforkernel.h>
#include <pspthreadman_kernel.h>
#include <pspsysmem_kernel.h>

#include <systemctrl.h>
#include <macros.h>

#include "flashemu.h"
#include "rebootex/payload.h"

PSP_MODULE_INFO("TimeMachine_Control", PSP_MODULE_KERNEL | PSP_MODULE_SINGLE_START | PSP_MODULE_SINGLE_LOAD | PSP_MODULE_NO_STOP, 1, 0);

int psp_model = 0;

int SysEventHandler(int eventId, char *eventName, void *param, int *result);

PspSysEventHandler sysEventHandler =
    {
        .size = sizeof(PspSysEventHandler),
        .name = "",
        .type_mask = 0x00FFFF00,
        .handler = SysEventHandler};

extern int msNotReady;

STMOD_HANDLER previous;

void ClearCaches()
{
    sceKernelDcacheWritebackAll();
    sceKernelIcacheInvalidateAll();
}

void OnModuleStart(SceModule2 *mod)
{
    char *moduleName = mod->modname;

    if (strcmp(moduleName, "sceLflashFatfmt") == 0)
    {
        u32 funcAddr = sctrlHENFindFunction("sceLflashFatfmt", "LflashFatfmt", 0xb7a424a4); // sceLflashFatfmtStartFatfmt
        if (funcAddr)
        {
        	MAKE_DUMMY_FUNCTION_RETURN_0(funcAddr);
        	ClearCaches();
        }
    }
    else if (strcmp(moduleName, "sceMediaSync") == 0)
    {
        MAKE_DUMMY_FUNCTION_RETURN_0(mod->text_addr + 0x135c);
        ClearCaches();
    }

    if (previous) previous(mod);
}

int module_start(SceSize args, void *argp)
{
    psp_model = sceKernelGetModel();
    InstallFlashEmu();
    previous = sctrlHENSetStartModuleHandler(OnModuleStart);
    sctrlHENSetRebootexOverride(rebootbuffer_ms_psp);
    ClearCaches();

    return 0;
}

int module_reboot_before(SceSize args, void *argp)
{
    sceIoUnassign("flash0:");
    sceIoUnassign("flash1:");
    sceIoUnassign("flash2:");
    sceIoUnassign("flash3:");
    sceKernelUnregisterSysEventHandler(&sysEventHandler);

    return 0;
}

int SysEventHandler(int eventId, char *eventName, void *param, int *result)
{
    if (eventId == 0x10009) // resume
        msNotReady = 1;
    return 0;
}
