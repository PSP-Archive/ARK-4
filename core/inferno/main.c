/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

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
#include "systemctrl.h"
#include "systemctrl_se.h"
#include "systemctrl_private.h"
#include "inferno.h"
#include "globals.h"
#include "macros.h"

PSP_MODULE_INFO("PRO_Inferno_Driver", 0x1000, 1, 1);

//u32 psp_model;
//u32 psp_fw_version;

extern int sceKernelApplicationType(void);
extern int sceKernelSetQTGP3(void *unk0);
extern char *GetUmdFile();

// 00002790
const char *g_iso_fn = NULL;

// 0x00002248
u8 g_umddata[16] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

extern int power_event_handler(int ev_id, char *ev_name, void *param, int *result);

PspSysEventHandler g_power_event = {
    .size = sizeof(g_power_event),
    .name = "infernoSysEvent",
    .type_mask = 0x00FFFF00, // both suspend / resume
    .handler = &power_event_handler,
};

// 00000090
int setup_umd_device(void)
{
    int ret;

    g_iso_fn = GetUmdFile();
    infernoSetDiscType(sctrlSEGetDiscType());
    ret = sceIoAddDrv(&g_iodrv);

    if(ret < 0) {
        return ret;
    }

    sceKernelSetQTGP3(g_umddata);
    ret = 0;

    return ret;
}

// 00001514
int init_inferno(void)
{
    g_drive_status = PSP_UMD_INITING;
    g_umd_cbid = -1;
    g_umd_error_status = 0;
    if (g_drive_status_evf < 0) g_drive_status_evf = sceKernelCreateEventFlag("SceMediaManUser", 0x201, 0, NULL);
    sceKernelRegisterSysEventHandler(&g_power_event);

    return MIN(g_drive_status_evf, 0);
}

// 0x00000000
int module_start(SceSize args, void* argp)
{
    int ret;

    #ifdef DEBUG
    printk("Inferno started\n");
    #endif

    ret = setup_umd_device();

    if(ret < 0) {
        return ret;
    }

    ret = init_inferno();

    return MIN(ret, 0);
}

// 0x0000006C
int module_stop(SceSize args, void *argp)
{
    sceIoDelDrv("umd");
    sceKernelDeleteEventFlag(g_drive_status_evf);
    sceKernelUnregisterSysEventHandler(&g_power_event);

    return 0;
}
