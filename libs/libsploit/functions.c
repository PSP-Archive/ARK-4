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

#include <stdio.h>
#include <pspsdk.h>
#include <pspiofilemgr.h>
#include <psploadexec.h>
#include <psploadexec_kernel.h>
#include <psputility_modules.h>
#include <pspumd.h>
#include <module2.h>
#include <macros.h>
#include <rebootconfig.h>
#include <systemctrl_se.h>
#include <string.h>
#include <ark.h>
#include "functions.h"

static UserFunctions _g_tbl;
static KernelFunctions _k_tbl;
static KxploitFunctions _kxf;

UserFunctions* g_tbl = &_g_tbl;
KernelFunctions* k_tbl = &_k_tbl;
KxploitFunctions* kxf = &_kxf;


// fill UserFunctions
void scanUserFunctions(UserFunctions* tbl){

    memset(g_tbl, 0, sizeof(UserFunctions));
    
    tbl->IoOpen = (void*)RelocImport("IoFileMgrForUser", 0x109F50BC, 0);
    tbl->IoRead = (void*)RelocImport("IoFileMgrForUser", 0x6A638D83, 0);
    tbl->IoClose = (void*)RelocImport("IoFileMgrForUser", 0x810C4BC3, 0);
    tbl->IoWrite = (void*)RelocImport("IoFileMgrForUser", 0x42EC03AC, 0);
    tbl->IoRemove = (void*)RelocImport("IoFileMgrForUser", 0xF27A9C51, 0);
    
    tbl->KernelLibcTime = (void*)RelocImport("UtilsForUser", 0x27CC57F0, 0);
    tbl->KernelLibcClock = (void*)RelocImport("UtilsForUser", 0x91E4F6A7, 0);
    tbl->KernelPowerLock = (void*)RelocImport("sceNetIfhandle_lib", 0xE80F00A4, 0);
    tbl->KernelDcacheWritebackAll = (void*)RelocImport("UtilsForUser", 0x79D1C3FA, 0);
    tbl->KernelIcacheInvalidateAll = (void*)RelocImport("UtilsForUser", 0x920F104A, 0);
    tbl->DisplaySetFrameBuf = (void*)RelocImport("sceDisplay", 0x289D82FE, 0);

    tbl->KernelCreateThread = (void*)RelocImport("ThreadManForUser", 0x446D8DE6, 0);
    tbl->KernelDelayThread = (void*)RelocImport("ThreadManForUser", 0xCEADEB47, 0);
    tbl->KernelStartThread = (void*)RelocImport("ThreadManForUser", 0xF475845D, 0);
    tbl->KernelWaitThreadEnd = (void*)RelocImport("ThreadManForUser", 0x278C0DF5, 0);
    tbl->KernelExitThread = (void*)RelocImport("ThreadManForUser", 0xAA73C935, 0);
    tbl->KernelExitDeleteThread = (void*)RelocImport("ThreadManForUser", 0x809CE29B, 0);

    tbl->KernelCreateVpl = (void*)RelocImport("ThreadManForUser", 0x56C039B5, 0);
    tbl->KernelTryAllocateVpl = (void*)RelocImport("ThreadManForUser", 0xAF36D708, 0),
    tbl->KernelFreeVpl = (void*)RelocImport("ThreadManForUser", 0xB736E9FF, 0);
    tbl->KernelDeleteVpl = (void*)RelocImport("ThreadManForUser", 0x89B3D48C, 0);
    tbl->KernelDeleteFpl = (void*)RelocImport("ThreadManForUser", 0xED1410E0, 0);
    
    tbl->UtilityLoadModule = (void*)RelocImport("sceUtility", 0x2A2B3DE0, 0);
    tbl->UtilityUnloadModule = (void*)RelocImport("sceUtility", 0xE49BFE92, 0);
    tbl->UtilityLoadNetModule = (void*)RelocImport("sceUtility", 0x1579A159, 0);
    tbl->UtilityUnloadNetModule = (void*)RelocImport("sceUtility", 0x64D50C56, 0);
    
    //tbl->SysMemUserForUser_91DE343C = (void*)RelocImport("SysMemUserForUser", 0x91DE343C, 0);
    tbl->KernelFreePartitionMemory = (void*)RelocImport("SysMemUserForUser", 0xB6D61D02, 0);
    tbl->KernelAllocPartitionMemory = (void*)RelocImport("SysMemUserForUser", 0x237DBD4F, 0);
    tbl->KernelGetBlockHeadAddr = (void*)RelocImport("SysMemUserForUser", 0x9D9A5BA1, 0);

    // savedata functions
    tbl->UtilitySavedataGetStatus = (void*)RelocImport("sceUtility", 0x8874DBE0, 0);
    tbl->UtilitySavedataInitStart = (void*)RelocImport("sceUtility", 0x50C4CD57, 0);
    tbl->UtilitySavedataUpdate = (void*)RelocImport("sceUtility", 0xD4B95FFB, 0);
    tbl->UtilitySavedataShutdownStart = (void*)RelocImport("sceUtility", 0x9790B33C, 0);
    
    tbl->KernelVolatileMemUnlock = (void*)RelocImport("sceSuspendForUser", 0xA569E425, 0);
    
}

void scanArkFunctions(UserFunctions* tbl){
    // ARK Functions
    tbl->freeMem = &freeMem;
    tbl->FindImportUserRam = &FindImportUserRam;
    tbl->FindImportVolatileRam = &FindImportVolatileRam;
    tbl->FindImportRange = &FindImportRange;
    tbl->RelocSyscall = &RelocSyscall;
    tbl->p5_open_savedata = &p5_open_savedata;
    tbl->p5_close_savedata = &p5_close_savedata;
    tbl->qwikTrick = &qwikTrick;
}

/*
 * These functions are ment for using when initial kernel access has been
 * granted, for example through the mean of a kernel exploit.
 */
void scanKernelFunctions(KernelFunctions* kfuncs){

    memset(kfuncs, 0, sizeof(KernelFunctions));

    kfuncs->KernelIOOpen = (void*)FindFunction("sceIOFileManager", "IoFileMgrForKernel", 0x109F50BC);
    kfuncs->KernelIORead = (void*)FindFunction("sceIOFileManager", "IoFileMgrForKernel", 0x6A638D83);
    kfuncs->KernelIOLSeek = (void*)FindFunction("sceIOFileManager", "IoFileMgrForKernel", 0x27EB27B8);
    kfuncs->KernelIOClose = (void*)FindFunction("sceIOFileManager", "IoFileMgrForKernel", 0x810C4BC3);
    kfuncs->KernelIOWrite = (void*)FindFunction("sceIOFileManager", "IoFileMgrForKernel", 0x42EC03AC);
    kfuncs->KernelIOMkdir = (void*)FindFunction("sceIOFileManager", "IoFileMgrForKernel", 0x06A70004);
    kfuncs->KernelIORmdir = (void*)FindFunction("sceIOFileManager", "IoFileMgrForKernel", 0x1117C65F);
    kfuncs->KernelIODopen = (void*)FindFunction("sceIOFileManager", "IoFileMgrForKernel", 0xB29DDF9C);
    kfuncs->KernelIODread = (void*)FindFunction("sceIOFileManager", "IoFileMgrForKernel", 0xE3EB004C);
    kfuncs->KernelIODclose = (void*)FindFunction("sceIOFileManager", "IoFileMgrForKernel", 0xEB092469);
    kfuncs->KernelIOGetStat = (void*)FindFunction("sceIOFileManager", "IoFileMgrForKernel", 0xACE946E8);
    kfuncs->KernelIORemove = (void*)FindFunction("sceIOFileManager", "IoFileMgrForKernel", 0xF27A9C51);
    kfuncs->IoAssign = (void*)FindFunction("sceIOFileManager", "IoFileMgrForKernel", 0xB2A628C1);
    kfuncs->IoUnassign = (void*)FindFunction("sceIOFileManager", "IoFileMgrForKernel", 0x6D08A871);
    
    kfuncs->KernelAllocPartitionMemory = (void*)FindFunction("sceSystemMemoryManager", "SysMemForKernel", 0x237DBD4F);
    kfuncs->KernelGetBlockHeadAddr = (void*)FindFunction("sceSystemMemoryManager", "SysMemForKernel", 0x9D9A5BA1);
    kfuncs->KernelFreePartitionMemory = (void*)FindFunction("sceSystemMemoryManager", "SysMemForKernel", 0xB6D61D02);
    kfuncs->KernelIcacheInvalidateAll = (void*)FindFunction("sceSystemMemoryManager", "UtilsForKernel", 0x920F104A);
    kfuncs->KernelDcacheWritebackInvalidateAll = (void*)FindFunction("sceSystemMemoryManager", "UtilsForKernel", 0xB435DEC5);
    kfuncs->KernelDcacheInvalidateRange = (void*)FindFunction("sceSystemMemoryManager", "UtilsForKernel", 0xBFA98062);
    kfuncs->KernelGzipDecompress = (void*)FindFunction("sceSystemMemoryManager", "UtilsForKernel", 0x78934841);
    
    kfuncs->KernelFindModuleByName = (void*)FindFunction("sceLoaderCore", "LoadCoreForKernel", 0xF6B1BF0F);
    
    kfuncs->KernelCreateThread = (void*)FindFunction("sceThreadManager", "ThreadManForKernel", 0x446D8DE6);
    kfuncs->KernelStartThread = (void*)FindFunction("sceThreadManager", "ThreadManForKernel", 0xF475845D);
    kfuncs->KernelDelayThread = (void*)FindFunction("sceThreadManager", "ThreadManForKernel", 0xCEADEB47);
    kfuncs->KernelExitThread = (void*)FindFunction("sceThreadManager", "ThreadManForKernel", 0xAA73C935);
    kfuncs->KernelDeleteThread = (void*)FindFunction("sceThreadManager", "ThreadManForKernel", 0x9FA03CD3);
    kfuncs->waitThreadEnd = (void*)FindFunction("sceThreadManager", "ThreadManForKernel", 0x278C0DF5);
    
    // ARK kernel functions
    kfuncs->FindTextAddrByName = &FindTextAddrByName;
    kfuncs->FindFunction = &FindFunction;
}

int isKernel(){
    u32 ra;
    __asm__ volatile ("move %0, $ra;" : "=r"(ra));
    return (ra&0x80000000) != 0;
}
