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

// we keep this here for compatibility
// ARK doesn't use this and it isn't persistent
SEConfig se_config = { // default SE/PRO configuration
    .magic = 0x47434554,
    .umdmode = MODE_INFERNO,
    .usbcharge = 0,
    .machidden = 1,
    .skipgameboot = 0,
    .hidepic = 0,
    .plugvsh = 1,
    .pluggame = 1,
    .plugpop = 1,
    .usbdevice = 0,
    .flashprot = 1,
    .fakeregion = FAKE_REGION_DISABLED,
    .skiplogo = 0,
    .useversion = 0,
    .useownupdate = 1,
    .usenodrm = 1,
    .hibblock = 1,
    .noanalog = 0,
    .oldplugin = 1,
    .htmlviewer_custom_save_location = 1,
    .hide_cfw_dirs = 1,
    .chn_iso = 1,
    .msspeed = MSSPEED_NONE,
    .slimcolor = 1,
    .iso_cache = 1,
    .iso_cache_total_size = 20,
    .iso_cache_num = 256,
    .iso_cache_policy = CACHE_POLICY_LRU,
    .usbversion = 0,
    .language = -1,
    .retail_high_memory = 0,
};

/**
 * Gets the SE configuration.
 * Avoid using this function, it may corrupt your program.
 * Use sctrlSEGetCongiEx function instead.
 *
 * @param config - pointer to a SEConfig structure that receives the SE configuration
 * @returns 0 on success
*/
int sctrlSEGetConfig(SEConfig *config){
    u32 k1 = pspSdkSetK1(0);
    memcpy(config, &se_config, sizeof(SEConfig));
    pspSdkSetK1(k1);
    return 0;
}

/**
 * Gets the SE configuration
 *
 * @param config - pointer to a SEConfig structure that receives the SE configuration
 * @param size - The size of the structure
 * @returns 0 on success
*/
int sctrlSEGetConfigEx(SEConfig *config, int size){
    u32 k1 = pspSdkSetK1(0);
    memcpy(config, &se_config, size);
    pspSdkSetK1(k1);
    return 0;
}

/**
 * Sets the SE configuration
 * This function can corrupt the configuration in flash, use
 * sctrlSESetConfigEx instead.
 *
 * @param config - pointer to a SEConfig structure that has the SE configuration to set
 * @returns 0 on success
*/
int sctrlSESetConfig(SEConfig *config){
    u32 k1 = pspSdkSetK1(0);
    memcpy(&se_config, config, sizeof(SEConfig));
    pspSdkSetK1(k1);
    return 0;
}

/**
 * Sets the SE configuration
 *
 * @param config - pointer to a SEConfig structure that has the SE configuration to set
 * @param size - the size of the structure
 * @returns 0 on success
*/
int sctrlSESetConfigEx(SEConfig *config, int size){
    u32 k1 = pspSdkSetK1(0);
    memcpy(&se_config, config, size);
    pspSdkSetK1(k1);
    return 0;
}

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
    reboot_config->iso_mode = index;
}

unsigned int sctrlSEGetBootConfFileIndex(void)
{
    return reboot_config->iso_mode;
}

void sctrlSESetDiscType(int type)
{
    reboot_config->iso_disc_type = type;
}

int sctrlSEGetDiscType(void)
{
    return reboot_config->iso_disc_type;
}
