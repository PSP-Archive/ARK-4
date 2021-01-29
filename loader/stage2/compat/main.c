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
#include <psptypes.h>
#include <graphics.h>
#include "globals.h"
#include "functions.h"

#define ARK_LOADADDR 0x08D30000
#define ARK_SIZE 0x8000
#define EXPLOIT_ID "ARK_PBOOT_LOADER"
#define ARK_PATH "ms0:/PSP/VHBL/ARK_Live/"
#define BIN_PATH ARK_PATH "ARK.BIN"

static ARKConfig conf = {
    .arkpath = ARK_PATH,
    .exec_mode = PS_VITA,
    .exploit_id = EXPLOIT_ID,
};

static FunctionTable tbl = {.config=&conf};

void memset(u8* start, u8 data, u32 size){
    u32 i = 0;
    for (; i<size; i++){
        start[i] = data;
    }
}

// Clear BSS Segment of Payload
void clearBSS(void)
{
    // BSS Start and End Address from Linkfile
    extern char __bss_start, __bss_end;
    
    // Clear Memory
    memset(&__bss_start, 0, &__bss_end - &__bss_start);
}

// Entry Point
int _start(char*) __attribute__((section(".text.startup")));
int _start(char* arg0)
{

    clearBSS();

    /*
    if (is_kernel(0)){
        tbl.IoOpen = (void*)FindFunction("sceIOFileManager", "IoFileMgrForKernel", 0x109F50BC);
        tbl.IoRead = (void*)FindFunction("sceIOFileManager", "IoFileMgrForKernel", 0x6A638D83);
        tbl.IoClose = (void*)FindFunction("sceIOFileManager", "IoFileMgrForKernel", 0x810C4BC3);
        tbl.KernelDcacheWritebackAll = (void*)FindFunction("sceSystemMemoryManager", "UtilsForKernel", 0xB435DEC5);
        tbl.DisplaySetFrameBuf = (void*)FindFunction("sceDisplay_Service", "sceDisplay", 0x289D82FE);
    }
    else{
    */
        tbl.IoOpen = (void*)FindImportUserRam("IoFileMgrForUser", 0x109F50BC);
        tbl.IoRead = (void*)FindImportUserRam("IoFileMgrForUser", 0x6A638D83);
        tbl.IoClose = (void*)FindImportUserRam("IoFileMgrForUser", 0x810C4BC3);
        tbl.KernelDcacheWritebackAll = (void*)FindImportUserRam("UtilsForUser", 0x79D1C3FA);
        tbl.DisplaySetFrameBuf = (void*)FindImportUserRam("sceDisplay", 0x289D82FE);
        tbl.KernelLibcTime = (void*)FindImportUserRam("UtilsForUser", 0x27CC57F0);
    //}
    
    int fd = tbl.IoOpen(BIN_PATH, PSP_O_RDONLY, 0);
    tbl.IoRead(fd, (void *)(ARK_LOADADDR), ARK_SIZE);
    tbl.IoClose(fd);
    
    tbl.KernelDcacheWritebackAll();
    
    void (* hEntryPoint)(ARKConfig*, FunctionTable*) = (void*)ARK_LOADADDR;
    hEntryPoint(&conf, &tbl);
    
}
