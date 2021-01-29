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
#include <pspthreadman_kernel.h>
#include <pspsysevent.h>
#include <pspiofilemgr.h>
#include <pspsysmem_kernel.h>
#include <pspinit.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <systemctrl_private.h>
#include <version.h>
#include <stdio.h>
#include <string.h>
#include <module2.h>
#include <globals.h>
#include <macros.h>
#include "rebootex.h"
#include "nidresolver.h"
#include "modulemanager.h"
#include "loadercore.h"
#include "imports.h"

char *GetUmdFile(void) __attribute__((alias("sctrlSEGetUmdFile")));

void SetUmdFile(char *file) __attribute__((alias("sctrlSESetUmdFile")));

// Return Reboot Configuration UMD File
char * sctrlSEGetUmdFile(void)
{
    // Return Reboot Configuration UMD File
    return reboot_config_isopath;
}

// Set Reboot Configuration UMD File
void sctrlSESetUmdFile(char * file)
{
    // Overwrite Reboot Configuration UMD File
    strncpy(reboot_config_isopath, file, REBOOTEX_CONFIG_ISO_PATH_MAXSIZE - 1);
    
    // Terminate String
    reboot_config_isopath[REBOOTEX_CONFIG_ISO_PATH_MAXSIZE - 1] = 0;
}

void sctrlSESetBootConfFileIndex(int index)
{
    reboot_config.iso_mode = index;
}

unsigned int sctrlSEGetBootConfFileIndex(void)
{
    return reboot_config.iso_mode;
}

void sctrlSESetDiscType(int type)
{
    reboot_config.iso_disc_type = type;
}

int sctrlSEGetDiscType(void)
{
    return reboot_config.iso_disc_type;
}
