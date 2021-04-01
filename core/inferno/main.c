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
#include <systemctrl_private.h>
#include <pspsysmem_kernel.h>
#include <pspsysevent.h>
#include <pspumd.h>
#include <psprtc.h>
#include <globals.h>
#include <macros.h>
#include "systemctrl.h"
#include "systemctrl_se.h"
#include "inferno.h"

PSP_MODULE_INFO("Inferno_Driver", 0x1000, 1, 1);

extern int sceKernelApplicationType(void);
extern int sceKernelSetQTGP3(void *unk0);
extern char *GetUmdFile();

// 00002790
const char *g_iso_fn = NULL;

// 0x00002248
unsigned char g_umddata[16] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

extern int powerEvtHandler(int ev_id, char *ev_name, void *param, int *result);

PspSysEventHandler g_powerEvt = {
    .size = sizeof(g_powerEvt),
    .name = "infernoSysEvent",
    .type_mask = 0x00FFFF00, // both suspend / resume
    .handler = &powerEvtHandler,
};

// 00000090
int setupUMDDevice(void)
{
    int ret;

    g_iso_fn = GetUmdFile();
    
    if (g_iso_fn==NULL || g_iso_fn[0] == 0) return -1; // no ISO file
    
    printk("UMDFile = %s\r\n", g_iso_fn);

    infernoSetDiscType(sctrlSEGetDiscType());
    ret = sceIoAddDrv(&g_iodrv);

    if(ret < 0)
    {
        return ret;
    }

    sceKernelSetQTGP3(g_umddata);
    ret = 0;

    return ret;
}

// 00001514
int infernoInitialize(void)
{
    g_drive_status = PSP_UMD_INITING;
    g_umd_cbid = -1;
    g_umd_error_status = 0;
    g_drive_status_evf = sceKernelCreateEventFlag("SceMediaManUser", 0x201, 0, NULL);
    sceKernelRegisterSysEventHandler(&g_powerEvt);

    if (g_drive_status_evf < 0)
        return g_drive_status_evf;

    return 0;
}

// 0x00000000
int module_start(SceSize args, void* argp)
{

    int ret;

    ret = setupUMDDevice();

    if(ret < 0)
    {
        return ret;
    }

    ret = infernoInitialize();

    if (ret < 0)
        return ret;

    return 0;
}

// 0x0000006C
int module_stop(SceSize args, void *argp)
{
    sceIoDelDrv("umd");
    sceKernelDeleteEventFlag(g_drive_status_evf);
    sceKernelUnregisterSysEventHandler(&g_powerEvt);

    return 0;
}
