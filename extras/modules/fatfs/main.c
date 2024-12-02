#include <pspkernel.h>
#include <pspreg.h>
#include <stdio.h>
#include <string.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <pspsysmem_kernel.h>
#include <pspsysevent.h>
#include <pspumd.h>
#include <psprtc.h>
#include <pspinit.h>
#include "systemctrl_private.h"
#include "inferno.h"
#include <ark.h>
#include "macros.h"

PSP_MODULE_INFO("ARK_fatfs", 0x1000, 2, 1);

extern PspIoDrvFuncs g_drv_funcs;
extern int power_event_handler(int ev_id, char *ev_name, void *param, int *result);

// 0x00002444
PspIoDrv g_iodrv = {
    .name = "ms",
    .dev_type = 4, // block device
    .unk2 = 0x800,
    .name2 = "fatms",
    .funcs = &g_drv_funcs,
};

PspSysEventHandler g_power_event = {
    .size = sizeof(g_power_event),
    .name = "fatfsSysEvent",
    .type_mask = 0x00FFFF00, // both suspend / resume
    .handler = &power_event_handler,
};

int module_start(SceSize args, void* argp)
{
    int ret;

    ret = sceKernelRegisterSysEventHandler(&g_power_event);
    if (ret < 0) return ret;

    ret = sceIoAddDrv(&g_iodrv);
    if (ret < 0) return ret;

    ret = InitFS();
    if (ret != 0) return -ret;

    return 0;
}

// 0x0000006C
int module_stop(SceSize args, void *argp)
{
    sceIoDelDrv("ms");
    sceKernelDeleteEventFlag(g_drive_status_evf);
    sceKernelUnregisterSysEventHandler(&g_power_event);

    return 0;
}