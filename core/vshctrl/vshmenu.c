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

#include <pspsdk.h>
#include <pspsysmem_kernel.h>
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <pspsysevent.h>
#include <pspiofilemgr.h>
#include <pspctrl.h>
#include <stdio.h>
#include <string.h>
#include "globals.h"
#include "systemctrl.h"
#include "systemctrl_se.h"
#include "systemctrl_private.h"

#define SENSE_KEY (PSP_CTRL_CIRCLE|PSP_CTRL_TRIANGLE|PSP_CTRL_CROSS|PSP_CTRL_SQUARE|PSP_CTRL_START|PSP_CTRL_SELECT)

#define ALL_ALLOW    (PSP_CTRL_UP|PSP_CTRL_RIGHT|PSP_CTRL_DOWN|PSP_CTRL_LEFT)
#define ALL_BUTTON   (PSP_CTRL_TRIANGLE|PSP_CTRL_CIRCLE|PSP_CTRL_CROSS|PSP_CTRL_SQUARE)
#define ALL_TRIGGER  (PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER)
#define ALL_FUNCTION (PSP_CTRL_SELECT|PSP_CTRL_START|PSP_CTRL_HOME|PSP_CTRL_HOLD|PSP_CTRL_NOTE)
#define ALL_CTRL     (ALL_ALLOW|ALL_BUTTON|ALL_TRIGGER|ALL_FUNCTION)

extern ARKConfig* ark_config;
extern int cur_usbdevice;
extern int usb_readonly;

SEConfig conf;

static int (*g_VshMenuCtrl) (SceCtrlData *, int);
static SceUID g_satelite_mod_id = -1;

int (*g_sceCtrlReadBufferPositive) (SceCtrlData *, int) = NULL;

int vctrlVSHRegisterVshMenu(int (*ctrl)(SceCtrlData *, int))
{
    u32 k1;
   
    k1 = pspSdkSetK1(0);
    g_VshMenuCtrl = (void *) ((u32) ctrl | 0x80000000);
    pspSdkSetK1(k1);

    return 0;
}

int vctrlVSHUpdateConfig(SEConfig *config)
{
    u32 k1;
    int ret;
    k1 = pspSdkSetK1(0);
    memcpy(&conf, config, sizeof(conf));
    ret = sctrlSESetConfig(&conf);
    cur_usbdevice = config->usbdevice;
    usb_readonly = config->usbdevice_rdonly;
    pspSdkSetK1(k1);
    return ret;
}

int vctrlVSHExitVSHMenu(SEConfig *config, char *videoiso, int disctype)
{
    u32 k1;
    int ret = 0;

    k1 = pspSdkSetK1(0);
    if (config){
        ret = vctrlVSHUpdateConfig(config);
        cur_usbdevice = config->usbdevice;
        usb_readonly = config->usbdevice_rdonly;
    }

    g_VshMenuCtrl = NULL;
    pspSdkSetK1(k1);
    
    return ret;
}

static SceUID load_satelite(void)
{
    SceUID modid;
    char path[ARK_PATH_SIZE];
    strcpy(path, ark_config->arkpath);
    strcat(path, VSH_MENU);
    
    SceKernelLMOption opt = {
        .size = 0x14,
        .flags = 0,
        .access = 1,
        .position = 1,
    };

    modid = sceKernelLoadModule(path, 0, &opt);

    if (modid < 0){
        // try flash0
        modid = sceKernelLoadModule("flash0:/vsh/module/ark_satelite.prx", 0, &opt);
    }

    return modid;
}

SceUID get_thread_id(const char *name)
{
    int ret, count, i;
    SceUID ids[128];

    ret = sceKernelGetThreadmanIdList(SCE_KERNEL_TMID_Thread, ids, sizeof(ids), &count);

    if(ret < 0) {
        return -1;
    }

    for(i=0; i<count; ++i) {
        SceKernelThreadInfo info;

        info.size = sizeof(info);
        ret = sceKernelReferThreadStatus(ids[i], &info);

        if(ret < 0) {
            continue;
        }

        if(0 == strcmp(info.name, name)) {
            return ids[i];
        }
    }

    return -2;
}

int _sceCtrlReadBufferPositive(SceCtrlData *ctrl, int count)
{
    int ret;
    u32 k1;
    SceUID modid;

    ret = (*g_sceCtrlReadBufferPositive)(ctrl, count);
    k1 = pspSdkSetK1(0);

    if (sceKernelFindModuleByName("VshCtrlSatelite")) {
        if (g_VshMenuCtrl) {
            (*g_VshMenuCtrl)(ctrl, count);
        } else {
            if (g_satelite_mod_id >= 0) {
                if (sceKernelStopModule(g_satelite_mod_id, 0, 0, 0, 0) >= 0) {
                    sceKernelUnloadModule(g_satelite_mod_id);
                }
            }
        }
    } else {
        /* filter out fault PSP sending dead keyscan */
        if ((ctrl->Buttons & ALL_CTRL) != PSP_CTRL_SELECT) {
            goto exit;
        }
        
        // Block Satellite Menu in OSK
        if (sceKernelFindModuleByName("sceVshOSK_Module"))
            goto exit;

        // Block Satellite while using Skype
        if (sceKernelFindModuleByName("Skyhost"))
            goto exit;

        // Block Satellite Menu in HTML Viewer (Bugged on Thread Monitor too, this is faster...)
        if (sceKernelFindModuleByName("htmlviewer_plugin_module"))
            goto exit;

        // Block Satellite while mounting USB
        if (sceKernelFindModuleByName("sceUSB_Stor_Driver"))
            goto exit;

        // Block Recovery menu
        if (sceKernelFindModuleByName("Recovery"))
            goto exit;
        
        // Block Satellite Menu in NP Signup Module (Blue PSN Login Screen)
        if (get_thread_id("SceNpSignupEvent") >= 0)
            goto exit;

        // Block Satellite Menu while playing Music
        if (get_thread_id("VshCacheIoPrefetchThread") >= 0)
            goto exit;

        // Block Satellite Menu while watching Videos
        if (get_thread_id("VideoDecoder") >= 0 || get_thread_id("AudioDecoder") >= 0)
            goto exit;

        // Block Satellite Menu while a Standard Dialog / Error is displayed
        if (get_thread_id("ScePafJob") >= 0)
            goto exit;

        // Block Satellite Menu in PSN Store
        if (get_thread_id("ScePSStoreBrowser2") >= 0)
            goto exit;

        // Block Satellite while accessing a DHCP Router
        if (get_thread_id("SceNetDhcpClient") >= 0)
            goto exit;

        // Block Satellite Menu in Go!cam [Yoti]
        if (sceKernelFindModuleByName("camera_plugin_module"))
            goto exit;

        #ifdef DEBUG
        printk("%s: loading satelite\n", __func__);
        #endif
        modid = load_satelite();

        if (modid >= 0) {
            g_satelite_mod_id = modid;
            modid = sceKernelStartModule(g_satelite_mod_id, 0, 0, 0, 0);
            #ifdef DEBUG
            if (modid < 0) {
                printk("%s: start module -> 0x%08X\n", __func__, modid);
            }
            #endif
            ctrl->Buttons &= (~PSP_CTRL_SELECT); // Filter SELECT
        }
    }
    
exit:
    pspSdkSetK1(k1);
    
    return ret;
}
