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
#include <globals.h>
#include <systemctrl.h>
#include <systemctrl_private.h>
#include <macros.h>
#include <functions.h>
#include "filesystem.h"
#include "vitaflash.h"

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

// define all possible file replacements here
static struct{
    char* orig;
    char* new;
    unsigned char len;
} ioreplacements[] = {
    // Replace flash0
    {.orig = NULL, .new = NULL, .len=0}
};

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

// "flash" Driver IoOpen Hook
int sceIoFlashOpenHook(PspIoDrvFileArg * arg, char * file, int flags, SceMode mode);

// "ms" Driver IoOpen Hook
int sceIoMsOpenHook(PspIoDrvFileArg * arg, char * file, int flags, SceMode mode);

// sceIoDopen Hook
int sceIoDopenHook(const char * dirname);

// sceIoDread Hook
int sceIoDreadHook(int fd, SceIoDirent * dir);

// sceIoDclose Hook
int sceIoDcloseHook(int fd);

// sceIoMkdir Hook
int    (* _sceIoMkdir)(char *dir, SceMode mode) = (void*)NULL;
int    sceIoMkdirHook(char *dir, SceMode mode);

int flashLoadPatch(int cmd);

// "flash" Driver IoOpen Original Call
int (* sceIoFlashOpen)(PspIoDrvFileArg * arg, char * file, int flags, SceMode mode) = NULL;

// "ms" Driver IoOpen Original Call
int (* sceIoMsOpen)(PspIoDrvFileArg * arg, char * file, int flags, SceMode mode) = NULL;

// "ms" Driver Reference
PspIoDrvArg * ms_driver = NULL;
PspIoDrvArg * flash_driver = NULL;

void initFileSystem(){
    // Create Semaphore
    dreadSema = sceKernelCreateSema("sceIoDreadSema", 0, 1, 1, NULL);
    // patch Driver
    u32 IOAddDrv = sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForKernel", 0x8E982A74);
    u32 AddDrv = findRefInGlobals("IoFileMgrForKernel", IOAddDrv, IOAddDrv);
    // Hooking sceIoAddDrv
    _sw((unsigned int)sceIoAddDrvHook, AddDrv);
    // Patch IO for file replacements
    //SceModule2* ioman = patchFileIO();
}

u32 UnprotectAddress(u32 addr){
    if ((addr >= 0x0B000000 && addr < 0x0BC00000)
        || (addr >= 0x0A000000 && addr < 0x0C000000)
        || (addr >= 0x00010000 && addr < 0x00014000))
        return addr | 0x80000000;
    if ((addr >= 0x4A000000 && addr < 0x4C000000)
        || (addr >= 0x40010000 && addr < 0x40014000))
        return addr | 0x60000000;
    return 0;
}

int (* sceIoMsRead_)(SceUID fd, void *data, SceSize size);
int (* sceIoFlashRead_)(SceUID fd, void *data, SceSize size);
int sceIoReadHookCommon(SceUID fd, void *data, SceSize size, int (* sceIoRead_)(SceUID fd, void *data, SceSize size))
{
    int ret;
    
    u32 addr = UnprotectAddress((u32)data);

    if (addr) {
        u32 k1 = pspSdkSetK1(0);

        ret = sceIoRead_(fd, (void *)(addr), size);

        pspSdkSetK1(k1);

        return ret;
    }

    return sceIoRead_(fd, data, size);
}

int sceIoMsReadHook(SceUID fd, void* data, SceSize size){
    return sceIoReadHookCommon(fd, data, size, sceIoMsRead_);
}

int sceIoFlashReadHook(SceUID fd, void* data, SceSize size){
    return sceIoReadHookCommon(fd, data, size, sceIoFlashRead_);
}

int (* sceIoMsWrite_)(SceUID fd, const void *data, SceSize size);
int (* sceIoFlashWrite_)(SceUID fd, const void *data, SceSize size);
int sceIoWriteHookCommon(SceUID fd, const void *data, SceSize size, int (* sceIoWrite_)(SceUID fd, void *data, SceSize size))
{
    int ret;

    u32 addr = UnprotectAddress((u32)data);

    if (addr) {
        u32 k1 = pspSdkSetK1(0);

        ret = sceIoWrite_(fd, (void *)(addr), size);

        pspSdkSetK1(k1);

        return ret;
    }
    return sceIoWrite_(fd, data, size);
}

int sceIoMsWriteHook(SceUID fd, void* data, SceSize size){
    return sceIoWriteHookCommon(fd, data, size, sceIoMsWrite_);
}

int sceIoFlashWriteHook(SceUID fd, void* data, SceSize size){
    return sceIoWriteHookCommon(fd, data, size, sceIoFlashWrite_);
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
    
    // Hooking sceIoMkdir for User Modules
    sctrlHENPatchSyscall((void *)sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0x06A70004), sceIoMkdirHook);
    
    //sctrlHENPatchSyscall((void *)sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0x6A638D83), sceIoReadHookCommon);
    //sctrlHENPatchSyscall((void *)sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0x42EC03AC), sceIoWriteHookCommon);
}

// Directory IO Patch for PSP-like Behaviour
void patchFileManagerImports(SceModule2 * mod)
{
    if (sceKernelInitApitype() != 0x141)
        return;
    
    // Hooking sceIoDopen for Kernel Modules
    hookImportByNID(mod, "IoFileMgrForKernel", 0xB29DDF9C, sceIoDopenHook);
    
    // Hooking sceIoDread for Kernel Modules
    hookImportByNID(mod, "IoFileMgrForKernel", 0xE3EB004C, sceIoDreadHook);
    
    // Hooking sceIoDclose for Kernel Modules
    hookImportByNID(mod, "IoFileMgrForKernel", 0xEB092469, sceIoDcloseHook);
    
    // Hooking sceIoMkdir for Kernel Modules
    hookImportByNID(mod, "IoFileMgrForKernel", 0x06A70004, sceIoMkdirHook);
    
    //hookImportByNID(mod, "IoFileMgrForKernel", 0x6A638D83, sceIoReadHookCommon);
    //hookImportByNID(mod, "IoFileMgrForKernel", 0x42EC03AC, sceIoWriteHookCommon);
}

// sceIoMkdir Hook to allow creating folders in ms0:/PSP/GAME
int sceIoMkdirHook(char *dir, SceMode mode)
{

    u32 k1 = pspSdkSetK1(0);
    
    int ret = sceIoMkdir(dir, mode);
    
    pspSdkSetK1(k1);
    
    return ret;
}

// sceIoAddDrv Hook
int sceIoAddDrvHook(PspIoDrv * driver)
{
    // "flash" Driver
    if (strcmp(driver->name, "flash") == 0) {
        // Hook IoOpen Function
        sceIoFlashOpen = driver->funcs->IoOpen;
        driver->funcs->IoOpen = sceIoFlashOpenHook;
        
    }
    else if (strcmp(driver->name, "flashfat") == 0){
        
        sceIoFlashRead_ = driver->funcs->IoRead;
        driver->funcs->IoRead = sceIoFlashReadHook;
    
        sceIoFlashWrite_ = driver->funcs->IoWrite;
        driver->funcs->IoWrite = sceIoFlashWriteHook;
        
    }
    // "ms" Driver
    else if(strcmp(driver->name, "ms") == 0) {
        // Hook IoOpen Function
        sceIoMsOpen = driver->funcs->IoOpen;
        driver->funcs->IoOpen = sceIoMsOpenHook;
        
        sceIoMsRead_ = driver->funcs->IoRead;
        driver->funcs->IoRead = sceIoMsReadHook;
        
        sceIoMsWrite_ = driver->funcs->IoWrite;
        driver->funcs->IoWrite = sceIoMsWriteHook;
    }
    
    // Register Driver
    return sceIoAddDrv(driver);
}

// "flash" Driver IoOpen Hook
int sceIoFlashOpenHook(PspIoDrvFileArg * arg, char * file, int flags, SceMode mode)
{
    flash_driver = arg->drv;
    // flash0 File Access Attempt
    if (arg->fs_num < 4) {
        // File Path Buffer
        char msfile[256];
        
        // Create "ms" File Path (links to flash0 folder on ms0)
        sprintf(msfile, "/flash/%d%s", arg->fs_num, file);
        
        // Exchange Filesystem Driver for "ms"
        arg->drv = ms_driver;
        
        // Open File from "ms"
        int fd = sceIoMsOpen(arg, msfile, flags, mode);
        
        // Open Success
        if (fd >= 0)
            return fd;
        
        // Restore Filesystem Driver to "flash"
        arg->drv = flash_driver;
    }
    
    // Forward Call
    return sceIoFlashOpen(arg, file, flags, mode);
}

// "ms" Driver IoOpen Hook
int sceIoMsOpenHook(PspIoDrvFileArg * arg, char * file, int flags, SceMode mode)
{
    // Update Internal "ms" Driver Reference
    ms_driver = arg->drv;
    
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

int (*iojal)(const char *, u32, u32, u32, u32, u32) = NULL;
int patchio(const char *a0, u32 a1, u32 a2, u32 a3, u32 t0, u32 t1)
{

    int res;
    for (int i=0; ioreplacements[i].orig; i++){
        if (strncmp(a0, ioreplacements[i].orig, ioreplacements[i].len) == 0){
            char path[ARK_PATH_SIZE];
            strcpy(path, ark_config->arkpath);
            strcat(path, ioreplacements[i].new);
            res = iojal(path, a1, a2, a3, t0, t1);
            if (res>=0) return res;
            break;
        }
    }
    res = iojal(a0, a1, a2, a3, t0, t1);
    
    return res;
}

u32 backup[2], ioctl;
int patchioctl(SceUID a0, u32 a1, void *a2, int a3, void *t0, int t1)
{
    _sw(backup[0], ioctl);
    _sw(backup[1], ioctl + 4);

    sceKernelDcacheWritebackInvalidateRange((void *)ioctl, 8);
    sceKernelIcacheInvalidateRange((void *)ioctl, 8);

    int ret = sceIoIoctl(a0, a1, a2, a3, t0, t1);

    _sw((((u32)&patchioctl >> 2) & 0x03FFFFFF) | 0x08000000, ioctl);
    _sw(0, ioctl + 4);

    sceKernelDcacheWritebackInvalidateRange((void *)ioctl, 8);
    sceKernelIcacheInvalidateRange((void *)ioctl, 8);

    if (ret < 0 && ((a1 & 0x208000) == 0x208000))
        return 0;

    return ret;
}

SceModule2* patchFileIO(){
    SceModule2 * mod = (SceModule2 *)sceKernelFindModuleByName("sceIOFileManager");
    u32 topaddr = mod->text_addr+mod->text_size;
    // find functions
    int patches = 2;
    for (u32 addr=mod->text_addr; addr<topaddr && patches; addr+=4){
        u32 data = _lw(addr);
        if (data == 0x03641824){
            iojal = addr-4;
            patches--;
        }
        else if (data == 0x00005021){
            ioctl = addr-12;
            patches--;
        }
    }
    // redirect IO
    patches = 2;
    for (u32 addr=mod->text_addr; addr<topaddr && patches; addr+=4){
        u32 data = _lw(addr);
        if (data == JAL(iojal)){
            _sw(JAL(patchio), addr);
            patches--;
        }
    }
    
    // redirect ioctl to patched
    backup[0] = _lw(ioctl);
    backup[1] = _lw(ioctl + 4);
    _sw(JUMP(patchioctl), ioctl);
    _sw(0, ioctl+4);
    
    // Flush Cache
    flushCache();
    
    return mod;
}
