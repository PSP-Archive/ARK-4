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

PSP_MODULE_INFO("VshCtrl", 0x1007, 1, 0);

u32 psp_model = 0;
ARKConfig* ark_config = NULL;

int has_umd_iso = 0;
int hidepics = 0;
int hibblock = 0;
int skiplogos = 0;
int hidemac = 0;
int hidedlc = 0;

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

void settingsHandler(char* path){
    if (strcasecmp(path, "hidepics") == 0){ // hide PIC0 and PIC1
        hidepics = 1;
    }
    else if (strcasecmp(path, "hibblock") == 0){ // block hibernation
        hibblock = 1;
    }
    else if (strcasecmp(path, "skiplogos") == 0){ // skip sony logo and gameboot
        skiplogos = 1;
    }
    else if (strcasecmp(path, "hidemac") == 0){ // hide mac address
        hidemac = 1;
    }
    else if (strcasecmp(path, "hidedlc") == 0){ // hide mac address
        hidedlc = 1;
    }
}

int module_start(SceSize args, void* argp)
{
    #ifdef DEBUG
    printk_init("ms0:/log_vshctrl.txt");
    printk("VshCtrl started\n");
    #endif
    
    psp_model = sceKernelGetModel();
    ark_config = sctrlHENGetArkConfig(NULL);
    isoInit();
    vshpatch_init();
    load_server_file();
    loadSettings(&settingsHandler);
    
    // always reset to NORMAL mode in VSH
    // to avoid ISO mode is used in homebrews in next reboot
    has_umd_iso = (sctrlSEGetUmdFile()[0] != 0 && sctrlSEGetBootConfFileIndex() == MODE_VSHUMD);
    sctrlSESetUmdFile("");
    sctrlSESetBootConfFileIndex(MODE_UMD);

    if (has_umd_iso){
        // disable launcher mode if using VSH ISO
        ark_config->launcher[0] = 0;
    }

    return 0;
}
