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

#include <pspdebug.h>
#include <pspctrl.h>
#include <pspsdk.h>
#include <pspiofilemgr.h>
#include <psputility.h>
#include <psputility_htmlviewer.h>
#include <psploadexec.h>
#include <psputils.h>
#include <psputilsforkernel.h>
#include <pspsysmem.h>
#include <psppower.h>
#include <string.h>

#include "macros.h"
#include "globals.h"
#include "functions.h"
#include "kxploit.h"

/*
Kernel exploit to use when ARK.BIN is already loaded within a Custom Firmware.
*/

UserFunctions* g_tbl = NULL;

void* (*set_start_module_handler)(void*) = NULL;
void (*prev)(void*) = NULL;
void (*kfunc)() = NULL;

int stubScanner(UserFunctions* tbl){
    g_tbl = tbl;
    set_start_module_handler = tbl->FindImportUserRam("SystemCtrlForUser", 0x1C90BECB);
    return (set_start_module_handler==NULL || *(u32*)set_start_module_handler == JR_RA);
}

void repairInstruction(KernelFunctions* k_tbl){
}

void my_mod_handler(void* mod){
    if (kfunc){
        kfunc();
    }
    if (prev){
        prev(mod);
    }
}

int doExploit(void){
    prev = set_start_module_handler(my_mod_handler);
    return (prev == NULL);
    return 0;
}

void kthread(SceSize args, void** argp){
    kfunc();
}

void executeKernel(u32 kfuncaddr){
    kfunc = KERNELIFY(kfuncaddr);
    g_tbl->UtilityLoadModule(PSP_MODULE_NP_COMMON);
    g_tbl->UtilityUnloadModule(PSP_MODULE_NP_COMMON);
}