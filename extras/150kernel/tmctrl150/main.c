#include <string.h>

#include <pspidstorage.h>
#include <pspkernel.h>
#include <pspreg.h>
#include <pspsdk.h>
#include <pspsysevent.h>
#include <psputilsforkernel.h>

#include <module2.h>
#include <macros.h>
#include <systemctrl.h>

#include "flashemu.h"

PSP_MODULE_INFO("TimeMachine_Control", PSP_MODULE_KERNEL | PSP_MODULE_SINGLE_START | PSP_MODULE_SINGLE_LOAD | PSP_MODULE_NO_STOP, 1, 0);

int SysEventHandler(int eventId, char *eventName, void *param, int *result);

PspSysEventHandler sysEventHandler =
    {
        .size = sizeof(PspSysEventHandler),
        .name = "",
        .type_mask = 0x0000FF00,
        .handler = SysEventHandler};

extern SceUID flashemu_sema;
extern int msNotReady;

STMOD_HANDLER previous;

void ClearCaches()
{
    sceKernelDcacheWritebackAll();
    sceKernelIcacheInvalidateAll();
}

int OnModuleStart(SceModule2 *mod)
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

   if (previous)
      return previous(mod);

    return 0;
}

int module_start(SceSize args, void *argp)
{
   previous = sctrlHENSetStartModuleHandler(OnModuleStart);
    InstallFlashEmu();
    ClearCaches();

    return 0;
}

int module_reboot_before(SceSize args, void *argp)
{
    SceUInt timeout = 500000;
    sceKernelWaitSema(flashemu_sema, 1, &timeout);
    sceKernelDeleteSema(flashemu_sema);
    sceIoUnassign("flash0:");
    sceIoUnassign("flash1:");
    sceKernelUnregisterSysEventHandler(&sysEventHandler);

    return 0;
}

int SysEventHandler(int eventId, char *eventName, void *param, int *result)
{
    if (eventId == 0x10009) //resume
        msNotReady = 1;

    return 0;
}
