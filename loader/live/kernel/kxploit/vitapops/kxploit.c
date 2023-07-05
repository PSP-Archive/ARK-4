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
Dummy kernel exploit to use when running NoPspEmuDrm
*/

UserFunctions* g_tbl = NULL;


int stubScanner(UserFunctions* tbl){
    g_tbl = tbl;
    return 0;
}

void repairInstruction(KernelFunctions* k_tbl){
}

int doExploit(void){
    int res = g_tbl->IoOpen("ms0:/__dokxploit__", 0, 0);
    PRTSTR1("res: %d", res);
    return 0;
}

void executeKernel(u32 kfuncaddr){
    int (*libctime)(u32, u32) = g_tbl->KernelLibcTime;
    libctime(0x08800000, 0x88380000);
}
