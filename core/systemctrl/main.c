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
#include "rebootex.h"
#include "modulemanager.h"
#include "loadercore.h"
#include "interruptman.h"
#include "cryptography.h"
#include "syspatch.h"
#include "sysmem.h"
#include "exception.h"
#include "libs/graphics/graphics.h"

PSP_MODULE_INFO("SystemControl", 0x3007, 4, 0);

// default config when none provided by the bootloader
static ARKConfig _ark_conf = {
    .magic = ARK_CONFIG_MAGIC,
    .arkpath = DEFAULT_ARK_PATH,
    .exploit_id = {0}, // None by default
    .exec_mode = DEV_UNK, // set by compat layer
    .recovery = 0,
};
ARKConfig* ark_config = &_ark_conf;

extern void copyPSPVram(u32* psp_vram);

// Boot Time Entry Point
int module_start(SceSize args, void * argp)
{

    #ifdef DEBUG
    printkInit(NULL);
    printk("ARK SystemControl started.\r\n");
    // set LCD framebuffer in hardware reg so we can do color debbuging
    _sw(0x44000000, 0xBC800100);
    colorDebug(0xFF00);
    #endif

    // Apply Module Patches
    patchSystemMemoryManager();
    SceModule2* loadcore = patchLoaderCore();
    SceModule2* modman = patchModuleManager();
    SceModule2* intrman = patchInterruptMan();
    SceModule2* memlmd = patchMemlmd();

    // Flush Cache
    flushCache();

    // setup NID resolver on loadercore
    setupNidResolver(loadcore);

    // Initialize Module Start Patching
    syspatchInit();

    // Backup Reboot Buffer (including configuration)
    backupRebootBuffer();

    #ifdef DEBUG
    // Register Default Exception Handler
    registerExceptionHandler(NULL, NULL);
    #endif

    uprotectExtraMemory();

    // Flush Cache
    flushCache();

    // Return Success
    return 0;
}

