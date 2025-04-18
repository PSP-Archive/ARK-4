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
#include <systemctrl.h>
#include <systemctrl_se.h>
#include "macros.h"
#include <ark.h>
#include "functions.h"

PSP_MODULE_INFO("VshCtrl", 0x1007, 1, 2);

u32 psp_model = 0;
ARKConfig* ark_config = NULL;
SEConfig* se_config = NULL;
int has_umd_iso = 0;
int _150_addon_enabled = 0;
char mounted_iso[64];

void enable_150_addon()
{
    SceIoStat stat;

    if (psp_model == PSP_1000 && sceKernelDevkitVersion() == FW_661) {
        memset(&stat, 0, sizeof(stat));
        if (sceIoGetstat(ARK_DC_PATH "/150", &stat) >= 0)
            _150_addon_enabled = 1;
    }
}

// Flush Instruction and Data Cache
void sync_cache()
{
    // Flush Instruction Cache
    sceKernelIcacheInvalidateAll();
    
    // Flush Data Cache
    sceKernelDcacheWritebackInvalidateAll();
}

void* vsh_malloc(size_t size){
    SceUID uid = sceKernelAllocPartitionMemory(2, "", PSP_SMEM_High, size+sizeof(u32), NULL);
    int* ptr = sceKernelGetBlockHeadAddr(uid);
    if (ptr){
        ptr[0] = uid;
        return &(ptr[1]);
    }
    return NULL;
}

void vsh_free(int* ptr){
    if (ptr){
        int uid = ptr[-1];
        sceKernelFreePartitionMemory(uid);
    }
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

int module_start(SceSize args, void* argp)
{
    #ifdef DEBUG
    printk_init("ms0:/log_vshctrl.txt");
    printk("VshCtrl started\n");
    #endif
    
    psp_model = sceKernelGetModel();
    ark_config = sctrlHENGetArkConfig(NULL);
    se_config = sctrlSEGetConfig(NULL);

    if (ark_config->recovery) return 0;

    vshpatch_init();
    load_server_file();
    
    // always reset to NORMAL mode in VSH
    // to avoid ISO mode is used in homebrews in next reboot
    char* umdfile = sctrlSEGetUmdFile();
    has_umd_iso = (umdfile[0] != 0 && sctrlSEGetBootConfFileIndex() == MODE_VSHUMD);
    if (has_umd_iso) strcpy(mounted_iso, umdfile);
    sctrlSESetUmdFile("");
    sctrlSESetBootConfFileIndex(MODE_UMD);

    if (has_umd_iso){
        // disable launcher mode if using VSH ISO
        ark_config->launcher[0] = 0;
    }

    enable_150_addon();

    return 0;
}
