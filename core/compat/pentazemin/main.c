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
#include <pspsysmem_kernel.h>
#include <systemctrl.h>
#include <systemctrl_private.h>
#include <globals.h>
#include "functions.h"
#include "macros.h"
#include "exitgame.h"
#include "libs/graphics/graphics.h"

#include "core/compat/pentazemin/rebootex/payload.h"

PSP_MODULE_INFO("ARKCompatLayer", 0x3007, 1, 0);

static ARKConfig _ark_conf;
ARKConfig* ark_config = &_ark_conf;

// Previous Module Start Handler
STMOD_HANDLER previous = NULL;

extern void AdrenalineOnModuleStart(SceModule2 * mod);

// Flush Instruction and Data Cache
void flushCache()
{
    // Flush Instruction Cache
    sceKernelIcacheInvalidateAll();
    
    // Flush Data Cache
    sceKernelDcacheWritebackInvalidateAll();
}

static void processArkConfig(ARKConfig* ark_config){
    sctrlHENGetArkConfig(ark_config);
    if (ark_config->exec_mode == DEV_UNK){
        ark_config->exec_mode = PSV_ADR; // assume running on Adrenaline
    }
    if (ark_config->launcher[0] == '\0'){
        strcpy(ark_config->launcher, ARK_MENU);
    }
    sctrlHENSetArkConfig(ark_config); // notify SystemControl
}

// Boot Time Entry Point
int module_start(SceSize args, void * argp)
{

    _sw(0x44000000, 0xBC800100);

    // set rebootex for Vita
    sctrlHENSetRebootexOverride(rebootbuffer_pentazemin);

    // copy configuration
    processArkConfig(ark_config);

    // Vita patches
    AdrenalineSysPatch();

    // Register Module Start Handler
    previous = sctrlHENSetStartModuleHandler(AdrenalineOnModuleStart);
   
    flushCache();

    colorDebug(0xff0000);
    
    // Return Success
    return 0;
}

int module_stop(SceSize args, void *argp)
{
    return 0;
}
