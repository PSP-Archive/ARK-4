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
#include <systemctrl_se.h>
#include <systemctrl_private.h>
#include <ark.h>
#include "functions.h"
#include "macros.h"
#include "libs/graphics/graphics.h"

#include "core/compat/pentazemin/rebootex/payload.h"

PSP_MODULE_INFO("ARKCompatLayer", 0x1007, 1, 0);

ARKConfig* ark_config = NULL;
SEConfig* se_config = NULL;

static void processArkConfig(){
    if (ark_config->exec_mode == DEV_UNK){
        ark_config->exec_mode = PSV_ADR; // assume running on Adrenaline
    }
}

// Boot Time Entry Point
int module_start(SceSize args, void * argp)
{

    se_config = sctrlSEGetConfig(NULL);
    ark_config = sctrlArkGetConfig(NULL);

    if (ark_config == NULL){
        return 1;
    }

    // copy configuration
    processArkConfig();

    if (ark_config->exec_mode != PSV_ADR){
        return 2;
    }

    // set rebootex for Vita
    sctrlHENSetRebootexOverride(rebootbuffer_pentazemin);

    // Vita patches
    AdrenalineSysPatch();
   
    sctrlFlushCache();
    
    // Return Success
    return 0;
}
