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
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <string.h>
#include <rebootconfig.h>
#include <systemctrl.h>
#include "imports.h"
#include "rebootex.h"

extern ARKConfig* ark_config;

// Original Load Reboot Buffer Function
int (* _LoadReboot)(void * arg1, unsigned int arg2, void * arg3, unsigned int arg4) = NULL;

// Reboot Buffer Backup Size
unsigned int reboot_size = 0;

// Reboot Buffer Backup
void * reboot_backup = NULL;

// Reboot Buffer Configuration
union {
    RebootBufferConfiguration config;
    u8 buf[REBOOTEX_CONFIG_MAXSIZE];
} reboot;

RebootBufferConfiguration* reboot_config = &(reboot.config);

int recovery_launch = 0;

// Reboot ISO Path
char reboot_config_isopath[REBOOTEX_CONFIG_ISO_PATH_MAXSIZE];

// Backup Reboot Buffer
void backupRebootBuffer(void)
{
    // Copy ARK runtime Config
    if (*(u32*)ARK_CONFIG == ARK_CONFIG_MAGIC)
        memcpy(ark_config, ARK_CONFIG, sizeof(ARKConfig));
    
    // Reboot Buffer Configuration
    RebootBufferConfiguration * conf = (RebootBufferConfiguration *)(REBOOTEX_CONFIG);
    
    // Invalid Reboot Buffer Magic
    if(conf->magic != REBOOTEX_CONFIG_MAGIC) return;
    
    // Backup Reboot Buffer Size
    reboot_size = conf->reboot_buffer_size;
    
    // Allocate Reboot Buffer Backup Memory
    int bufferid = sceKernelAllocPartitionMemory(1, "RebootBuffer", PSP_SMEM_Low, reboot_size, 0);
    
    // Get Memory Pointer
    reboot_backup = sceKernelGetBlockHeadAddr(bufferid);
    
    // Copy Reboot Buffer Payload
    memcpy(reboot_backup, (void *)REBOOTEX_TEXT, reboot_size);
    
    // Copy Reboot Buffer Configuration
    memcpy(&reboot, (void *)REBOOTEX_CONFIG, REBOOTEX_CONFIG_MAXSIZE);
    
    // Copy Reboot ISO Path
    strncpy(reboot_config_isopath, (void *)REBOOTEX_CONFIG_ISO_PATH, REBOOTEX_CONFIG_ISO_PATH_MAXSIZE);
    reboot_config_isopath[REBOOTEX_CONFIG_ISO_PATH_MAXSIZE - 1] = 0;
    
    // Flush Cache
    flushCache();
}

// Restore Reboot Buffer
void restoreRebootBuffer(void)
{

    // Reboot Buffer Configuration
    RebootBufferConfiguration * conf = (RebootBufferConfiguration *)(REBOOTEX_CONFIG);

    // Restore Reboot Buffer Configuration
    memcpy((void *)REBOOTEX_CONFIG, &reboot, REBOOTEX_CONFIG_MAXSIZE);
    
    // Restore ARK Config
    ark_config->recovery = recovery_launch; // reset recovery mode for next reboot
    memcpy(ARK_CONFIG, ark_config, sizeof(ARKConfig));
    
    // Restore Reboot ISO Path
    strncpy((char*)REBOOTEX_CONFIG_ISO_PATH, sctrlSEGetUmdFile(), REBOOTEX_CONFIG_ISO_PATH_MAXSIZE);
    ((char*)REBOOTEX_CONFIG_ISO_PATH)[REBOOTEX_CONFIG_ISO_PATH_MAXSIZE - 1] = '\0';
    
    // Restore Reboot Buffer Payload
    memcpy((void *)REBOOTEX_TEXT, reboot_backup, reboot_size);
}

// Reboot Buffer Loader
int LoadReboot(void * arg1, unsigned int arg2, void * arg3, unsigned int arg4)
{
    // Restore Reboot Buffer Configuration
    restoreRebootBuffer();
    
    // Load Sony Reboot Buffer
    return _LoadReboot(arg1, arg2, arg3, arg4);
}

// Patch loadexec
void patchLoadExec(SceModule2* loadexec)
{
    // Find Reboot Loader Function
    _LoadReboot = (void *)loadexec->text_addr;
    patchLoadExecCommon(loadexec, (u32)LoadReboot, 2);
    // Flush Cache
    flushCache();
}
