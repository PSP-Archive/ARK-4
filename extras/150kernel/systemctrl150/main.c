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
#include <systemctrl.h>
#include <systemctrl_private.h>
#include <ark.h> 
#include "modulemanager.h"
#include "loadexec.h"

PSP_MODULE_INFO("SystemControl150", 0x3007, 1, 0);

u8 keyseed[0x20];

// default config when none provided by the bootloader
static ARKConfig _ark_conf = {
    .magic = ARK_CONFIG_MAGIC,
    .arkpath = DEFAULT_ARK_PATH,
    .exploit_id = {0}, // None by default
    .exec_mode = DEV_UNK, // set by compat layer
    .recovery = 0,
};
ARKConfig* ark_config = &_ark_conf;

// Boot Time Entry Point
int module_start(SceSize args, void * argp)
{
    pspSdkInstallNoDeviceCheckPatch();
    pspSdkInstallNoPlainModuleCheckPatch(); 

    // Apply Module Patches
    patchModuleManager();
    patchLoadExec();

    memcpy(keyseed, 0x883f0000, 0x20); 

    // Flush Cache
    flushCache();

    // Initialize Module Start Patching
    syspatchInit();

    backupRebootBuffer();

    // Flush Cache
    flushCache();


    // Return Success
    return 0;
}

