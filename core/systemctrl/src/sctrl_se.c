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
#include <stdio.h>
#include <string.h>
#include <module2.h>
#include <ark.h>
#include <macros.h>
#include "rebootex.h"
#include "nidresolver.h"
#include "modulemanager.h"
#include "loadercore.h"
#include "imports.h"

SEConfig se_config = {
    .magic = ARK_CONFIG_MAGIC,
    .umdseek = 0,
    .cpubus_clock = 0,
    .disable_pause = 0,
    .hidedlc = 0,
    .umdregion = 0,
    .vshregion = 0,
    .usbdevice = 0,
    .usbcharge = 0,
    .hidemac = 0,
    .launcher_mode = 0,
    .hidepics = 0,
    .qaflags = 0,
    .usbdevice_rdonly = 0,
    .skiplogos = 0,
    .noumd = 0,
    .hibblock = 0,
    .noanalog = 0,
    .oldplugin = 0,
    .msspeed = 0,
    .iso_cache = 0,
    .iso_cache_size = 4 * 1024,
    .iso_cache_num = 8,
    .iso_cache_partition = PSP_MEMORY_PARTITION_KERNEL,
    .noled = 0, // always false
    .wpa2 = 0, /* not used by default */
    .force_high_memory = 0,
};

char *GetUmdFile(void) __attribute__((alias("sctrlSEGetUmdFile")));

void SetUmdFile(char *file) __attribute__((alias("sctrlSESetUmdFile")));

void sctrlSEApplyConfig(SEConfig *config) __attribute__((alias("sctrlSESetConfig")));

// we keep this here for compatibility
// ARK doesn't use this and it isn't persistent

/**
 * Gets the SE configuration.
 * Avoid using this function, it may corrupt your program.
 * Use sctrlSEGetCongiEx function instead.
 *
 * @param config - pointer to a SEConfig structure that receives the SE configuration
 * @returns pointer to original SEConfig structure in SystemControl
*/
SEConfig* sctrlSEGetConfig(SEConfig *config)
{
    if (config) memcpy(config, &se_config, sizeof(SEConfig));
    return &se_config;
}

/**
 * Gets the SE configuration
 *
 * @param config - pointer to a SEConfig structure that receives the SE configuration
 * @param size - The size of the structure
 * @returns pointer to original SEConfig structure in SystemControl
*/
SEConfig* sctrlSEGetConfigEx(SEConfig *config, int size)
{
    if (config && size == sizeof(SEConfig)){
        memcpy(config, &se_config, size);
    }
    return &se_config;
}

/**
 * Sets the SE configuration
 * This function can corrupt the configuration in flash, use
 * sctrlSESetConfigEx instead.
 *
 * @param config - pointer to a SEConfig structure that has the SE configuration to set
 * @returns 0 on success
*/
int sctrlSESetConfig(SEConfig *config)
{
    memcpy(&se_config, config, sizeof(SEConfig));
    return 0;
}

/**
 * Sets the SE configuration
 *
 * @param config - pointer to a SEConfig structure that has the SE configuration to set
 * @param size - the size of the structure
 * @returns 0 on success
*/
int sctrlSESetConfigEx(SEConfig *config, int size)
{
    if (config && size == sizeof(SEConfig)){
        memcpy(&se_config, config, size);
        return 0;
    }
    return -1;
}

// Return Reboot Configuration UMD File
char * sctrlSEGetUmdFile(void)
{
    // Return Reboot Configuration UMD File
    return rebootex_config.iso_path;
}

char *sctrlSEGetUmdFileEx(char *input)
{
    char* umdfilename = sctrlSEGetUmdFile();
    sctrlSESetUmdFile(input);
    return umdfilename;
}

// Set Reboot Configuration UMD File
void sctrlSESetUmdFile(char * file)
{
    if (file == NULL || file[0] == 0){
        rebootex_config.iso_path[0] = 0;
    }
    else {
        // Overwrite Reboot Configuration UMD File
        strncpy(rebootex_config.iso_path, file, REBOOTEX_CONFIG_ISO_PATH_MAXSIZE - 1);
        // Terminate String
        rebootex_config.iso_path[REBOOTEX_CONFIG_ISO_PATH_MAXSIZE - 1] = 0;
    }
}

void sctrlSESetUmdFileEx(const char *umd, char *input)
{
    strcpy(input, rebootex_config.iso_path);
    sctrlSESetUmdFile(umd);
}

void sctrlSESetBootConfFileIndex(int index)
{
    rebootex_config.iso_mode = index;
}

unsigned int sctrlSEGetBootConfFileIndex(void)
{
    return rebootex_config.iso_mode;
}

void sctrlSESetDiscType(int type)
{
    rebootex_config.iso_disc_type = type;
}

int sctrlSEGetDiscType(void)
{
    return rebootex_config.iso_disc_type;
}

int sctrlSEGetVersion()
{
    return ( (ARK_MAJOR_VERSION << 24) | (ARK_MINOR_VERSION << 16) | (ARK_MICRO_VERSION << 8) | ARK_REVISION );
}

int sctrlSEMountUmdFromFile(char *file, int noumd, int isofs){
    return -1;
}

int sctrlSEUmountUmd(){
    return 0;
}

void sctrlSESetDiscOut(int out){
    return;
}
