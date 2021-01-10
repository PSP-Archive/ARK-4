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
#include <pspusb.h>
#include <pspctrl.h>
#include <stdio.h>
#include <string.h>
#include "utils.h"
#include "libs.h"
#include "printk.h"
#include "systemctrl.h"
#include "systemctrl_se.h"
#include "main.h"
#include "virtual_pbp.h"
#include "pspusbdevice.h"
#include "config.h"

static SceUID g_usbdevice_modid = -1;

static SceUID load_start_usbdevice(void)
{
	SceUID modid = -1;
	int ret;
	const char *mod = PATH_USBDEVICE;

	modid = sceKernelLoadModule(mod, 0, NULL);

	if (modid < 0) {
		return -1;
	}

	ret = sceKernelStartModule(modid, 0, NULL, NULL, NULL);

	if (ret < 0) {
		printk("%s: sceKernelStartModule -> 0x%08X\n", __func__, ret);
		sceKernelUnloadModule(modid);

		return -1;
	}

	return modid;
}

static void stop_unload_usbdevice(void)
{
	int ret;

	ret = sceKernelStopModule(g_usbdevice_modid, 0, NULL, NULL, NULL);

#ifdef DEBUG
	if(ret < 0) {
		printk("%s: sceKernelStopModule(0x%08X) -> 0x%08X\n", __func__, g_usbdevice_modid, ret);
	}
#endif

	ret = sceKernelUnloadModule(g_usbdevice_modid);

#ifdef DEBUG
	if(ret < 0) {
		printk("%s: sceKernelUnloadModule(0x%08X) -> 0x%08X\n", __func__, g_usbdevice_modid, ret);
	}
#endif

	if (ret >= 0) {
		g_usbdevice_modid = -1;
	}
}

static int (*sceUsbStartOrig)(const char *driverName, int size, void *args) = NULL;
static int (*sceUsbStopOrig)(const char *driverName, int size, void *args) = NULL;

static int _sceUsbStart(const char *driverName, int size, void *args)
{
	int ret;
	u32 k1;

	k1 = pspSdkSetK1(0);

	if (0 == strcmp(driverName, "USBStor_Driver")) {
		if(conf.usbdevice > 0 && conf.usbdevice <= 5) {
			if (g_usbdevice_modid < 0) {
				g_usbdevice_modid = load_start_usbdevice();
			}

			if (g_usbdevice_modid >= 0) {
				ret = pspUsbDeviceSetDevice(conf.usbdevice - 1, conf.flashprot, 0);
				printk("%s: pspUsbDeviceSetDevice %d %d -> 0x%08X\n", __func__,
						conf.usbdevice-1, conf.flashprot, ret);
			}
		}
	}

	pspSdkSetK1(k1);
	ret = (*sceUsbStartOrig)(driverName, size, args);

	return ret;
}

static int _sceUsbStop(const char *driverName, int size, void *args)
{
	int ret;
	u32 k1;

	ret = (*sceUsbStopOrig)(driverName, size, args);
	k1 = pspSdkSetK1(0);

	if (0 == strcmp(driverName, "USBStor_Driver")) {
		if(conf.usbdevice > 0 && conf.usbdevice <= 5) {
			if (g_usbdevice_modid >= 0) {
				int result;

				result = pspUsbDeviceFinishDevice();
				printk("%s: pspUsbDeviceFinishDevice -> 0x%08X\n", __func__, result);
				stop_unload_usbdevice();
			}
		}
	}

	pspSdkSetK1(k1);
	
	return ret;
}

void patch_sceUSB_Driver(u32 text_addr)
{
	sceUsbStartOrig = (void*)sctrlHENFindFunction("sceUSB_Driver", "sceUsb", 0xAE5DE6AF);
	sctrlHENPatchSyscall(sceUsbStartOrig, &_sceUsbStart);

	sceUsbStopOrig = (void*)sctrlHENFindFunction("sceUSB_Driver", "sceUsb", 0xC2464FA0);
	sctrlHENPatchSyscall(sceUsbStopOrig, &_sceUsbStop);
}
