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
#include <psploadcore.h>
#include <pspiofilemgr.h>
#include <stdio.h>
#include "macros.h"

static char *g_blacklist[] = {
    "iso",
    "seplugins",
    "isocache.bin",
    "irshell",
};

static inline int is_in_blacklist(const char *dname)
{
    // lower string
    char temp[255]; memset(temp, 0, sizeof(temp));
    strncpy(temp, dname, sizeof(temp));
    lowerString(temp, temp, strlen(temp)+1);

    for (int i=0; i<NELEMS(g_blacklist); ++i) {
        if (strstr(temp, g_blacklist[i])) {
        	return 1;
        }
    }

    return 0;
}

int hideIoDread(SceUID fd, SceIoDirent * dir)
{
    int k1 = pspSdkSetK1(0);
    int result = sceIoDread(fd, dir);

    if(result > 0 && is_in_blacklist(dir->d_name)) {
        result = sceIoDread(fd, dir);
    }

    pspSdkSetK1(k1);
    return result;
}

int hideIoOpen(const char *path, int flags, SceMode mode){
    if (is_in_blacklist(path)){
        return 0x80010002; //SCE_ERROR_ERRNO_FILE_NOT_FOUND;
    }
    return sceIoOpen(path, flags, mode);
}

int hideIoDopen(const char *path){
    if (is_in_blacklist(path)){
        return 0x80010002; //SCE_ERROR_ERRNO_FILE_NOT_FOUND;
    }
    return sceIoDopen(path);
}

int hideIoGetstat(const char *path, SceIoStat *stat){
    if (is_in_blacklist(path)){
        return 0x80010002; //SCE_ERROR_ERRNO_FILE_NOT_FOUND;
    }
    return sceIoGetstat(path, stat);
}

int hideIoRemove(const char *path){
    if (is_in_blacklist(path)){
        return 0x80010002; //SCE_ERROR_ERRNO_FILE_NOT_FOUND;
    }
    return sceIoRemove(path);
}

int hideIoRmdir(const char *path){
    if (is_in_blacklist(path)){
        return 0x80010002; //SCE_ERROR_ERRNO_FILE_NOT_FOUND;
    }
    return sceIoRmdir(path);
}

// hide cfw folders, this avoids crashing the weird dj max portable 3 savegame algorithm
void hide_cfw_folder(SceModule * mod)
{
    // hide dread
    sctrlHookImportByNID(mod, "IoFileMgrForUser", 0xE3EB004C, &hideIoDread);

    // hide file open
    sctrlHookImportByNID(mod, "IoFileMgrForUser", 0x109F50BC, &hideIoOpen);

    // hide dir open
    sctrlHookImportByNID(mod, "IoFileMgrForUser", 0xB29DDF9C, &hideIoDopen);

    // hide getstat
    sctrlHookImportByNID(mod, "IoFileMgrForUser", 0xACE946E8, &hideIoGetstat);

    // hide remove
    sctrlHookImportByNID(mod, "IoFileMgrForUser", 0xF27A9C51, &hideIoRemove);

    // hide rmdir
    sctrlHookImportByNID(mod, "IoFileMgrForUser", 0x1117C65F, &hideIoRmdir);
}
