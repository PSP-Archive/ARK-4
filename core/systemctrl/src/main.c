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
#include <globals.h> 
#include "rebootex.h"
#include "modulemanager.h"
#include "loadercore.h"
#include "interruptman.h"
#include "cryptography.h"
#include "syspatch.h"
#include "sysmem.h"
#include "exception.h"
#include "ansi_c_functions.h"
#include "libs/graphics/graphics.h"

PSP_MODULE_INFO("SystemControl", 0x3007, 1, 0);

static ARKConfig _ark_conf;
ARKConfig* ark_config = &_ark_conf;

// Boot Time Entry Point
int module_start(SceSize args, void * argp)
{

  #ifdef DEBUG
    printkInit(NULL);
    printk("ARK SystemControl started.\r\n");
  #endif

    // copy configuration from user ram
    memcpy(ark_config, ark_conf_backup, sizeof(ARKConfig));

    // Apply Module Patches
    patchSystemMemoryManager();
    SceModule2* loadcore = patchLoaderCore();
    setupNidResolver(loadcore);
    patchModuleManager();
    patchInterruptMan();
    patchMemlmd();
    
    // Backup Reboot Buffer (including configuration)
    backupRebootBuffer();
    
    // Initialize Malloc
    oe_mallocinit();
    
    // Initialize Module Start Patching
    syspatchInit();
    
    // Flush Cache
    flushCache();
    
    // Register Default Exception Handler
    registerExceptionHandler(NULL, NULL);
    
    // Return Success
    return 0;
}

