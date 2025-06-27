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

#include <pspinit.h>
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <string.h>
#include <stdio.h>
#include <module2.h>
#include <ark.h>
#include <systemctrl.h>
#include <systemctrl_private.h>
#include <macros.h>
#include <functions.h>
#include "filesystem.h"
#include "vitaflash.h"
#include "rebootconfig.h"
#include "fatef.h"
#include "core/compat/pentazemin/flashfs.h"

extern RebootConfigARK* reboot_config;

// sceIoDread Open List Item
typedef struct OpenDirectory
{
    // Previous Item in List
    struct OpenDirectory * prev;
    // Next Item in List
    struct OpenDirectory * next;
    // Directory File Descriptor
    int fd;
    // Directory IO Stage
    int stage;
    // sceIoDopen Path
    char * path;
} OpenDirectory;

// Open Directory List
OpenDirectory * opendirs = NULL;

// Directory IO Semaphore
static SceUID dreadSema = -1;

// Directory IO Semaphore Lock
void dreadLock();

// Directory IO Semaphore Unlock
void dreadUnlock();

// Directory List Scanner
OpenDirectory * findOpenDirectory(int fd);

// sceIoAddDrv Hook
int sceIoAddDrvHook(PspIoDrv * driver);
int sceIoDelDrvHook(const char *drv_name);

int sceIoAssignHook(const char *dev1, const char *dev2, const char *dev3, int mode, void* unk1, long unk2);
int sceIoUnassignHook(const char *dev);

// "ms" Driver IoOpen Hook
int sceIoMsOpenHook(PspIoDrvFileArg * arg, char * file, int flags, SceMode mode);

// sceIoDopen Hook
int sceIoDopenHook(const char * dirname);

// sceIoDread Hook
int sceIoDreadHook(int fd, SceIoDirent * dir);

// sceIoDclose Hook
int sceIoDcloseHook(int fd);

// "ms" Driver Reference
PspIoDrv * ms_drv = NULL;
PspIoDrv * flashfat_drv = NULL;

PspIoDrvFuncs ms_funcs;
PspIoDrvFuncs flash_funcs;

// "ef" driver
PspIoDrv ef_drv;
PspIoDrv fatef_drv;
PspIoDrvFuncs ef_funcs;

int (* _sceIoAddDrv)(PspIoDrv *drv);
int (* _sceIoDelDrv)(const char *drv_name);

int (* _sceIoUnassign)(const char *dev);
int (* _sceIoAssign)(const char *dev1, const char *dev2, const char *dev3, int mode, void* unk1, long unk2);

void initFileSystem(){
    // Create Semaphore
    dreadSema = sceKernelCreateSema("sceIoDreadSema", 0, 1, 1, NULL);

    // patch Driver
    u32 IoAddDrv = sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForKernel", 0x8E982A74);
    u32 IoDelDrv = sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForKernel", 0xC7F35804);
    u32 IoAssign = sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForKernel", 0xB2A628C1);
    u32 IoUnassign = sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForKernel", 0x6D08A871);

    HIJACK_FUNCTION(IoAddDrv, sceIoAddDrvHook, _sceIoAddDrv);
    HIJACK_FUNCTION(IoDelDrv, sceIoDelDrvHook, _sceIoDelDrv);

    HIJACK_FUNCTION(IoUnassign, sceIoUnassignHook, _sceIoUnassign);
    HIJACK_FUNCTION(IoAssign, sceIoAssignHook, _sceIoAssign);
}

// "ms" Driver IoOpen Hook
int (* sceIoMsOpen)(PspIoDrvFileArg * arg, char * file, int flags, SceMode mode) = NULL;
int sceIoMsOpenHook(PspIoDrvFileArg * arg, char * file, int flags, SceMode mode)
{
    
    // File Write Open Request
    if ((flags & PSP_O_WRONLY) != 0) {
        // Search for EBOOT.PBP Filename
        char * occurence = strstr(file, "/EBOOT.PBP");
        // Trying to write an EBOOT.PBP file in PS Vita fails, redirect to VBOOT.PBP
        if (occurence != NULL) {
            occurence[1] = 'V';
        }
    }
    
    // Forward Call
    return sceIoMsOpen(arg, file, flags, mode);
}

void patchFileSystemDirSyscall(void)
{
    if (sceKernelInitApitype() != 0x141)
        return;
    
    // Hooking sceIoDopen for User Modules
    sctrlHENPatchSyscall((void *)sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0xB29DDF9C), sceIoDopenHook);
    
    // Hooking sceIoDread for User Modules
    sctrlHENPatchSyscall((void *)sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0xE3EB004C), sceIoDreadHook);
    
    // Hooking sceIoDclose for User Modules
    sctrlHENPatchSyscall((void *)sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0xEB092469), sceIoDcloseHook);
}

// Directory IO Patch for PSP-like Behaviour
void patchFileManagerImports(SceModule2 * mod)
{
    if (sceKernelInitApitype() != 0x141)
        return;
    
    // Hooking sceIoDopen for Kernel Modules
    sctrlHookImportByNID(mod, "IoFileMgrForKernel", 0xB29DDF9C, sceIoDopenHook);
    
    // Hooking sceIoDread for Kernel Modules
    sctrlHookImportByNID(mod, "IoFileMgrForKernel", 0xE3EB004C, sceIoDreadHook);
    
    // Hooking sceIoDclose for Kernel Modules
    sctrlHookImportByNID(mod, "IoFileMgrForKernel", 0xEB092469, sceIoDcloseHook);
}

__attribute__((noinline)) int BuildMsPathChangeFsNum(PspIoDrvFileArg *arg, const char *name, char *ms_path) {
    sprintf(ms_path, "/flash/%d%s", (int)arg->fs_num, name);
    int fs_num = arg->fs_num;
    arg->fs_num = 0;
    return fs_num;
}

// sceIoAddDrv Hook
int sceIoAddDrvHook(PspIoDrv * driver)
{
    // "flash" Driver
    if (strcmp(driver->name, "flash") == 0) {
        memcpy(&flash_funcs, driver->funcs, sizeof(PspIoDrvFuncs));

        driver->funcs->IoOpen = flashIoOpen;
        driver->funcs->IoClose = flashIoClose;
        driver->funcs->IoRead = flashIoRead;
        driver->funcs->IoWrite = flashIoWrite;
        driver->funcs->IoLseek = flashIoLseek;
        driver->funcs->IoIoctl = flashIoIoctl;
        driver->funcs->IoRemove = flashIoRemove;
        driver->funcs->IoMkdir = flashIoMkdir;
        driver->funcs->IoRmdir = flashIoRmdir;
        driver->funcs->IoDopen = flashIoDopen;
        driver->funcs->IoDclose = flashIoDclose;
        driver->funcs->IoDread = flashIoDread;
        driver->funcs->IoGetstat = flashIoGetstat;
        driver->funcs->IoChstat = flashIoChstat;
        driver->funcs->IoChdir = flashIoChdir;
        driver->funcs->IoDevctl = flashIoDevctl;

        // Add flashfat driver
        flashfat_drv->funcs = driver->funcs;
        _sceIoAddDrv(flashfat_drv);
    }
    // "ms" Driver
    else if (strcmp(driver->name, "fatms") == 0) {

        // Configure ms driver
        memcpy(&ms_funcs, driver->funcs, sizeof(PspIoDrvFuncs));

        sceIoMsOpen = driver->funcs->IoOpen;
        driver->funcs->IoOpen = sceIoMsOpenHook;

        ms_drv->funcs = driver->funcs;

        // Configure ef driver
        memcpy(&ef_funcs, driver->funcs, sizeof(PspIoDrvFuncs));
        ef_funcs.IoOpen = sceIoEfOpenHook;
        ef_funcs.IoRemove = sceIoEfRemoveHook;
        ef_funcs.IoMkdir = sceIoEfMkdirHook;
        ef_funcs.IoRmdir = sceIoEfRmdirHook;
        ef_funcs.IoDopen = sceIoEfDopenHook;
        ef_funcs.IoGetstat = sceIoEfGetStatHook;
        ef_funcs.IoChstat = sceIoEfChStatHook;
        ef_funcs.IoRename = sceIoEfRenameHook;
        ef_funcs.IoChdir = sceIoEfChdirHook;

        memcpy(&ef_drv, ms_drv, sizeof(PspIoDrv));
        ef_drv.name = "ef";
        ef_drv.name2 = "EF";
        ef_drv.funcs = &ef_funcs;

        memcpy(&fatef_drv, driver, sizeof(PspIoDrv));
        fatef_drv.name = "fatef";
        fatef_drv.name2 = "FATEF";
        fatef_drv.funcs = &ef_funcs;

        // redirect ms to ef
        int apitype = reboot_config->fake_apitype;
        if (apitype == 0x152 || apitype == 0x125 || apitype == 0x126 || apitype == 0x155){
            memcpy(ms_drv->funcs, &ef_funcs, sizeof(PspIoDrvFuncs));
        }

        // Add drivers
        _sceIoAddDrv(ms_drv);
        _sceIoAddDrv(&ef_drv);
        _sceIoAddDrv(&fatef_drv);
    }
    else if(strcmp(driver->name, "ms") == 0) {
        ms_drv = driver;
        return 0;
    }
    else if (strcmp(driver->name, "flashfat") == 0){
        flashfat_drv = driver;
        return 0;
    }
    
    // Register Driver
    return _sceIoAddDrv(driver);
}

int sceIoDelDrvHook(const char *drv_name) {
    if (strcmp(drv_name, "ms") == 0 || strcmp(drv_name, "flashfat") == 0) {
        return 0;
    } else if (strcmp(drv_name, "fatms") == 0) {
        _sceIoDelDrv("ms");
    } else if (strcmp(drv_name, "flash") == 0) {
        _sceIoDelDrv("flashfat");
    }

    return _sceIoDelDrv(drv_name);
}

int sceIoUnassignHook(const char *dev) {
    int k1 = pspSdkSetK1(0);

    if (strncmp(dev, "ms", 2) == 0 || strncmp(dev, "flash", 5) == 0) {
        pspSdkSetK1(k1);
        return 0;
    }

    pspSdkSetK1(k1);
    return _sceIoUnassign(dev);
}

int sceIoAssignHook(const char *dev1, const char *dev2, const char *dev3, int mode, void* unk1, long unk2) {
    int k1 = pspSdkSetK1(0);

    if (strncmp(dev1, "ms", 2) == 0 || strncmp(dev1, "flash", 5) == 0) {
        pspSdkSetK1(k1);
        return 0;
    }

    pspSdkSetK1(k1);
    return _sceIoAssign(dev1, dev2, dev3, mode, unk1, unk2);
}

// Directory IO Semaphore Lock
void dreadLock()
{
    // Elevate Permission Level
    unsigned int k1 = pspSdkSetK1(0);
    
    // Wait for Semaphore & Lock It
    sceKernelWaitSema(dreadSema, 1, 0);
    
    // Restore Permission Level
    pspSdkSetK1(k1);
}

// Directory IO Semaphore Unlock
void dreadUnlock()
{
    // Elevate Permission Level
    unsigned int k1 = pspSdkSetK1(0);
    
    // Unlock Semaphore
    sceKernelSignalSema(dreadSema, 1);
    
    // Restore Permission Level
    pspSdkSetK1(k1);
}

// Directory List Scanner
OpenDirectory * findOpenDirectory(int fd)
{
    // Iterate Open Directory List
    OpenDirectory * item = opendirs;

    for(; item != NULL; item = item->next) {
        // Matching File Descriptor
        if (item->fd == fd)
            return item;
    }
    // Directory not found
    return NULL;
}

// sceIoDopen Hook
int sceIoDopenHook(const char * dirname)
{
    // Forward Call
    int result = sceIoDopen(dirname);

    // Open Success
    if (result >= 0 && 0 == strncmp(dirname, "ms0:/", 4)) {
        // Allocate Memory
        OpenDirectory * item = (OpenDirectory *)oe_malloc(sizeof(OpenDirectory));
        
        // Allocate Success
        if (item != NULL) {
            // Clear Memory
            memset(item, 0, sizeof(OpenDirectory));
            
            // Save Descriptor
            item->fd = result;

            item->path = (char*)oe_malloc(strlen(dirname) + 1);

            if (item->path == NULL) {
                oe_free(item);
                return result;
            }

            strcpy(item->path, dirname);

            {
                int len = strlen(item->path);

                while (len > 0 && item->path[len-1] == '/') {
                    item->path[len-1] = '\0';
                    len = strlen(item->path);
                }
            }
            
            // Lock Semaphore
            dreadLock();
            
            // Link Item into List
            item->next = opendirs;

            if (opendirs != NULL)
                opendirs->prev = item;
            
            // Replace List Start Node
            opendirs = item;
            
            // Unlock Semaphore
            dreadUnlock();
            
        }
    }
    
    // Return Result
    return result;
}

// sceIoDread Hook
int sceIoDreadHook(int fd, SceIoDirent * dir)
{

    // Lock List Access
    dreadLock();
    
    // Find Directory in List
    OpenDirectory * item = findOpenDirectory(fd);
    
    // Found Directory in List
    if (item != NULL) {
        // Fake Directory Stage
        if (item->stage < 2) {
            // Kernel Address Status Structure
            static SceIoStat stat;
            int err;
            
            // Elevate Permission Level
            unsigned int k1 = pspSdkSetK1(0);
            
            // Fetch Folder Status
            err = sceIoGetstat(item->path, &stat);

            // Restore Permission Level
            pspSdkSetK1(k1);

            // Valid Output Argument
            if (err == 0 && dir != NULL) {
                // Clear Memory
                memset(dir, 0, sizeof(SceIoDirent));
                
                // Copy Status
                memcpy(&dir->d_stat, &stat, sizeof(stat));
                
                dir->d_name[0] = '.';
                dir->d_name[1] = '.';
                
                // Fake "." Output Stage
                if (item->stage == 0) {
                    dir->d_name[1] = 0;
                }
                
                // Move to next Stage
                item->stage++;
                
                // Unlock List Access
                dreadUnlock();
                
                // Return "More files"
                return 1;
            }
        }
    }

    // Unlock List Access
    dreadUnlock();
    
    // Forward Call
    return sceIoDread(fd, dir);
}

// sceIoDclose Hook
int sceIoDcloseHook(int fd)
{
    // Lock Semaphore
    dreadLock();
    
    // Find Directory in List
    OpenDirectory * item = findOpenDirectory(fd);
    
    // Found Directory in List
    if (item != NULL) {
        // Unlink Item from List
        if (item->prev != NULL)
            item->prev->next = item->next;
        else
            opendirs = item->next;

        if (item->next != NULL)
            item->next->prev = item->prev;

        // Free path string
        oe_free(item->path);
        
        // Return Memory to Heap
        oe_free(item);
    }
    
    // Unlock Semaphore
    dreadUnlock();
        
    // Forward Call
    return sceIoDclose(fd);
}
