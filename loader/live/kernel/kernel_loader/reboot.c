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

#include "reboot.h"
#include "globals.h"
#include "main.h"

u8* rebootbuffer = NULL;
u32 size_rebootbuffer = REBOOTEX_MAX_SIZE;
int iso_mode = MODE_INFERNO;

// Sony Reboot Buffer Loader
int (* _LoadReboot)(void *, unsigned int, void *, unsigned int) = NULL;

// Build Reboot Configuration
void buildRebootBufferConfig(int rebootBufferSize)
{
    // Fetch Memory Range
    RebootConfigARK* conf = (RebootConfigARK*)(REBOOTEX_CONFIG);
    
    // Clear Reboot Configuration
    memset((char *)REBOOTEX_CONFIG, 0, sizeof(RebootConfigARK));
    
    // Write Configuration Magic
    conf->magic = ARK_CONFIG_MAGIC;
    
    // Write PROCFW Reboot Buffer Size (for backup in System Control)
    conf->reboot_buffer_size = rebootBufferSize;

    // Default ISO driver for homebrew and ISO
    conf->iso_mode = iso_mode;
    // Default ISO disc type
    conf->iso_disc_type = PSP_UMD_TYPE_GAME;
    
    // backup runtime ARK config
    memcpy(ARK_CONFIG, ark_config, sizeof(ARKConfig));
}

// PROCFW Reboot Buffer Loader
int LoadReboot(void * arg1, unsigned int arg2, void * arg3, unsigned int arg4)
{

    // Copy PROCFW Reboot Buffer into Memory
    memset((char *)REBOOTEX_TEXT, 0, REBOOTEX_MAX_SIZE);
    int rebootBufferSize = REBOOTEX_MAX_SIZE;
    if (rebootbuffer[0] == 0x1F && rebootbuffer[1] == 0x8B) // gzip packed rebootex
        rebootBufferSize = k_tbl->KernelGzipDecompress((unsigned char *)REBOOTEX_TEXT, REBOOTEX_MAX_SIZE, rebootbuffer, NULL);
    else // plain payload
        memcpy(REBOOTEX_TEXT, rebootbuffer, REBOOTEX_MAX_SIZE);
        
    // Build Configuration
    buildRebootBufferConfig(rebootBufferSize);
    
    // Load Sony Reboot Buffer
    return _LoadReboot(arg1, arg2, arg3, arg4);
}
