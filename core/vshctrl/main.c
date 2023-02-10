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
#include <stdio.h>
#include <string.h>
#include <pspumd.h>
#include "main.h"
#include "systemctrl.h"
#include "systemctrl_se.h"
#include "macros.h"
#include "globals.h"
#include "functions.h"

#define DAX_BLOCK_SIZE 0x2000
#define DAX_COMP_BUF 0x2400

PSP_MODULE_INFO("VshCtrl", 0x1007, 1, 0);

u32 psp_model = 0;
ARKConfig _ark_conf;
ARKConfig* ark_config = &_ark_conf;

// Flush Instruction and Data Cache
void sync_cache()
{
    // Flush Instruction Cache
    sceKernelIcacheInvalidateAll();
    
    // Flush Data Cache
    sceKernelDcacheWritebackInvalidateAll();
}

int get_device_name(char *device, int size, const char* path)
{
    const char *p;

    if (path == NULL || device == NULL) {
        return -1;
    }

    p = strchr(path, '/');

    if (p == NULL) {
        return -2;
    }

    strncpy(device, path, MIN(size, p-path+1));
    device[MIN(size-1, p-path)] = '\0';

    return 0;
}

int hidepics = 0;
void settingsHandler(char* path){
    if (strcasecmp(path, "hidepics") == 0){ // hide PIC0 and PIC1
        hidepics = 1;
    }
}

int module_start(SceSize args, void* argp)
{
    #ifdef DEBUG
    printk_init("ms0:/log_vshctrl.txt");
    printk("VshCtrl started\n");
    #endif
    
    psp_model = sceKernelGetModel();
    sctrlHENGetArkConfig(ark_config);
    isoInit();
    vshpatch_init();
    loadSettings(&settingsHandler);
    
    // always reset to NORMAL mode in VSH
    // to avoid ISO mode is used in homebrews in next reboot
    sctrlSESetUmdFile("");
    sctrlSESetBootConfFileIndex(MODE_UMD);

    return 0;
}
