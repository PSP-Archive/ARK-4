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

// Reboot Buffer Backup Size
unsigned int reboot_size = 0;

// Reboot Buffer Backup
void * reboot_backup = NULL;

// Reboot Buffer Configuration
RebootBufferConfiguration reboot_config;

// Reboot ISO Path
char reboot_config_isopath[REBOOTEX_CONFIG_ISO_PATH_MAXSIZE];

// Backup Reboot Buffer
void backupRebootBuffer(void)
{
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
	memcpy(&reboot_config, (void *)REBOOTEX_CONFIG, sizeof(reboot_config));
	
	// Copy Reboot ISO Path
	strncpy(reboot_config_isopath, (void *)REBOOTEX_CONFIG_ISO_PATH, REBOOTEX_CONFIG_ISO_PATH_MAXSIZE);
	reboot_config_isopath[REBOOTEX_CONFIG_ISO_PATH_MAXSIZE - 1] = 0;
}

// Restore Reboot Buffer
void restoreRebootBuffer(void)
{
	// No Backup available
	if(reboot_size == 0 || reboot_backup == NULL) return;
	
	// Restore Reboot Buffer Payload
	memcpy((void *)REBOOTEX_TEXT, reboot_backup, reboot_size);
	
	// Restore Reboot Buffer Configuration
	memcpy((void *)REBOOTEX_CONFIG, &reboot_config, sizeof(reboot_config));
	
	// Restore Reboot ISO Path
	strncpy((char*)REBOOTEX_CONFIG_ISO_PATH, sctrlSEGetUmdFile(), REBOOTEX_CONFIG_ISO_PATH_MAXSIZE);
	((char*)REBOOTEX_CONFIG_ISO_PATH)[REBOOTEX_CONFIG_ISO_PATH_MAXSIZE - 1] = '\0';
}

