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

#include "globals.h"
#include "functions.h"
#include "kxploit.h"

/*
Dummy kernel exploit to use when ARK.BIN is already loaded with kernel priviledges.
*/


int stubScanner(FunctionTable* tbl){
    return 0;
}

void repairInstruction(void){
}

int doExploit(void){
    return 0;
}

void executeKernel(u32 kfuncaddr){
    void (*kernelContentFunction)(void) = (void*)kfuncaddr;
    kernelContentFunction();
}

