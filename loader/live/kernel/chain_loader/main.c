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
#include "graphics.h"
#include "globals.h"
#include "functions.h"

#define ARK_LOADADDR 0x08D30000
#define ARK_SIZE 0x8000

static ARKConfig conf = {
    .magic = ARK_CONFIG_MAGIC,
    .arkpath = DEFAULT_ARK_PATH,
    .exploit_id = LIVE_EXPLOIT_ID,
    .launcher = {0},
    .exec_mode = DEV_UNK, // best to autodetect
    .recovery = 0,
};

static UserFunctions tbl;

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
int _start(char* savepath)
{

    clearBSS();
    
    if (savepath){
        // adjust ms/ef
        conf.arkpath[0] = savepath[0];
        conf.arkpath[1] = savepath[1];
    }

    tbl.IoOpen = (void*)FindImportUserRam("IoFileMgrForUser", 0x109F50BC);
    tbl.IoRead = (void*)FindImportUserRam("IoFileMgrForUser", 0x6A638D83);
    tbl.IoClose = (void*)FindImportUserRam("IoFileMgrForUser", 0x810C4BC3);
    tbl.KernelDcacheWritebackAll = (void*)FindImportUserRam("UtilsForUser", 0x79D1C3FA);
    tbl.DisplaySetFrameBuf = (void*)FindImportUserRam("sceDisplay", 0x289D82FE);
    tbl.KernelLibcTime = (void*)FindImportUserRam("UtilsForUser", 0x27CC57F0);
    
    char binpath[ARK_PATH_SIZE];
    strcpy(binpath, conf.arkpath);
    strcat(binpath, ARK4_BIN);
    int fd = tbl.IoOpen(binpath, PSP_O_RDONLY, 0);
    tbl.IoRead(fd, (void *)(ARK_LOADADDR), ARK_SIZE);
    tbl.IoClose(fd);
    
    tbl.KernelDcacheWritebackAll();
    
    void (* hEntryPoint)(ARKConfig*, UserFunctions*, char*) = (void*)ARK_LOADADDR;
    hEntryPoint(&conf, NULL, NULL);
    
}
