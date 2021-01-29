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

// Build Reboot Configuration
void buildRebootBufferConfig(int rebootBufferSize)
{
    // Fetch Memory Range
    RebootBufferConfiguration * conf = (RebootBufferConfiguration *)(REBOOTEX_CONFIG);
    
    // Write Configuration Magic
    conf->magic = REBOOTEX_CONFIG_MAGIC;
    
    // Write PROCFW Reboot Buffer Size (for backup in System Control)
    conf->reboot_buffer_size = rebootBufferSize;

    if (IS_VITA_POPS(ark_conf_backup->exec_mode)){
        conf->iso_mode = MODE_NP9660;
        conf->iso_disc_type = PSP_UMD_TYPE_GAME;
    }
    else{
        // Default ISO driver for homebrew and ISO
        conf->iso_mode = MODE_INFERNO;
        // Default ISO disc type
        conf->iso_disc_type = PSP_UMD_TYPE_GAME;
    }
}

// PROCFW Reboot Buffer Loader
int LoadReboot(void * arg1, unsigned int arg2, void * arg3, unsigned int arg4)
{

    // Copy PROCFW Reboot Buffer into Memory
    int decompressSize;
    memset((char *)REBOOTEX_TEXT, 0, REBOOTEX_MAX_SIZE);
    decompressSize = k_tbl->KernelGzipDecompress((unsigned char *)REBOOTEX_TEXT, REBOOTEX_MAX_SIZE, rebootbuffer, NULL);
    
    // Clear Reboot Configuration
    memset((char *)REBOOTEX_CONFIG, 0, REBOOTEX_CONFIG_MAXSIZE);
    memset((char *)REBOOTEX_CONFIG_ISO_PATH, 0, REBOOTEX_CONFIG_ISO_PATH_MAXSIZE);
    
    // Build Configuration
    buildRebootBufferConfig(decompressSize);
    
    // Load Sony Reboot Buffer
    return _LoadReboot(arg1, arg2, arg3, arg4);
}
