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

/*
// define all possible file replacements here
static struct{
    char* orig;
    char* new;
    unsigned char len;
} ioreplacements[] = {
    // Replace flash0
    {.orig = NULL, .new = NULL, .len=0}
};
*/

static int flash_redirect = 0;

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

int flashLoadPatch(int cmd);

// "flash" Driver IoOpen Original Call
int (* sceIoFlashOpen)(PspIoDrvFileArg * arg, char * file, int flags, SceMode mode) = NULL;

// "ms" Driver IoOpen Original Call
int (* sceIoMsOpen)(PspIoDrvFileArg * arg, char * file, int flags, SceMode mode) = NULL;

// "ms" Driver Reference
PspIoDrv * ms_drv = NULL;
PspIoDrv * flashfat_drv = NULL;
PspIoDrvArg * ms_driver = NULL;
PspIoDrvArg * flash_driver = NULL;

PspIoDrvFuncs ms_funcs;
PspIoDrvFuncs flash_funcs;


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
    return addr;
}

int (* sceIoMsRead_)(PspIoDrvFileArg* arg, void* data, SceSize size);
int (* sceIoFlashRead_)(PspIoDrvFileArg* arg, void* data, SceSize size);
int sceIoReadHookCommon(PspIoDrvFileArg* arg, void* data, SceSize size, int (*sceIoRead_)(PspIoDrvFileArg*, void*, SceSize))
{
    int ret;
    
    u32 addr = UnprotectAddress((u32)data);

    if (addr) {
        u32 k1 = pspSdkSetK1(0);
        ret = sceIoRead_(arg, (void *)(addr), size);
        pspSdkSetK1(k1);

        return ret;
    }

    return sceIoRead_(arg, data, size);
}

int sceIoMsReadHook(PspIoDrvFileArg* arg, void* data, SceSize size){
    return sceIoReadHookCommon(arg, data, size, sceIoMsRead_);
}

int sceIoFlashReadHook(PspIoDrvFileArg* arg, void* data, SceSize size){
    return sceIoReadHookCommon(arg, data, size, sceIoFlashRead_);
}

int (* sceIoMsWrite_)(PspIoDrvFileArg* arg, const void* data, SceSize size);
int (* sceIoFlashWrite_)(PspIoDrvFileArg* arg, const void* data, SceSize size);
int sceIoWriteHookCommon(PspIoDrvFileArg* arg, const void* data, SceSize size, int (*sceIoWrite_)(PspIoDrvFileArg*, void*, SceSize))
{
    int ret;

    u32 addr = UnprotectAddress((u32)data);

    if (addr) {
        u32 k1 = pspSdkSetK1(0);
        ret = sceIoWrite_(arg, (void *)(addr), size);
        pspSdkSetK1(k1);

        return ret;
    }
    return sceIoWrite_(arg, data, size);
}

int sceIoMsWriteHook(PspIoDrvFileArg * arg, void* data, SceSize size){
    return sceIoWriteHookCommon(arg, data, size, sceIoMsWrite_);
}

int sceIoFlashWriteHook(PspIoDrvFileArg * arg, void* data, SceSize size){
    return sceIoWriteHookCommon(arg, data, size, sceIoFlashWrite_);
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
    hookImportByNID(mod, "IoFileMgrForKernel", 0xB29DDF9C, sceIoDopenHook);
    
    // Hooking sceIoDread for Kernel Modules
    hookImportByNID(mod, "IoFileMgrForKernel", 0xE3EB004C, sceIoDreadHook);
    
    // Hooking sceIoDclose for Kernel Modules
    hookImportByNID(mod, "IoFileMgrForKernel", 0xEB092469, sceIoDcloseHook);
}

#if 0
__attribute__((noinline)) int BuildMsPathChangeFsNum(PspIoDrvFileArg *arg, const char *name, char *ms_path) {
	sprintf(ms_path, "/flash/%d%s", (int)arg->fs_num, name);
	int fs_num = arg->fs_num;
	arg->fs_num = 0;
	return fs_num;
}

int _flashIoOpen(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	char *file = (char *)args[1];
	int flags = args[2];
	SceMode mode = (SceMode)args[3];

	char ms_path[128];
	int fs_num = BuildMsPathChangeFsNum(arg, file, ms_path);
	int res = ms_funcs.IoOpen(arg, ms_path, flags, mode);
	if (res >= 0) {
		((u32 *)arg)[18] = 1;
	}

	if (fs_num == 1 || res >= 0)
		return res;

	arg->fs_num = fs_num;

	return flash_funcs.IoOpen(arg, file, flags, mode);	
}

int _flashIoClose(PspIoDrvFileArg *arg) {
	if (((u32 *)arg)[18]) {
		arg->fs_num = 0;
		return ms_funcs.IoClose(arg);
	}

	return flash_funcs.IoClose(arg);
}

int _flashIoRead(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	char *data = (char *)args[1];
	int len = args[2];

    if (!data || !len) return 0;

    data = (char*)UnprotectAddress(data);

	if (((u32 *)arg)[18]) {
		arg->fs_num = 0;
		return ms_funcs.IoRead(arg, data, len);
	}

	return flash_funcs.IoRead(arg, data, len);
}

int _flashIoWrite(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *data = (const char *)args[1];
	int len = args[2];

	if (!data || !len) return 0;

    data = (char*)UnprotectAddress(data);

	if (((u32 *)arg)[18]) {
		arg->fs_num = 0;
		return ms_funcs.IoWrite(arg, data, len);
	}

	return flash_funcs.IoWrite(arg, data, len);
}

SceOff _flashIoLseek(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	u32 ofs = (u32)args[1];
	int whence = args[2];

	if (((u32 *)arg)[18]) {
		arg->fs_num = 0;
		return ms_funcs.IoLseek(arg, ofs, whence);
	}

	return flash_funcs.IoLseek(arg, ofs, whence);
}

int _flashIoRemove(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *name = (const char *)args[1];

	char ms_path[128];
	int fs_num = BuildMsPathChangeFsNum(arg, name, ms_path);
	int res = ms_funcs.IoRemove(arg, ms_path);

	if (fs_num == 1 || res >= 0)
		return res;

	arg->fs_num = fs_num;
	return flash_funcs.IoRemove(arg, name);
}

int _flashIoMkdir(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *name = (const char *)args[1];
	SceMode mode = (SceMode)args[2];

	char ms_path[128];
	int fs_num = BuildMsPathChangeFsNum(arg, name, ms_path);
	int res = ms_funcs.IoMkdir(arg, ms_path, mode);

	if (fs_num == 1 || res >= 0)
		return res;

	arg->fs_num = fs_num;
	return flash_funcs.IoMkdir(arg, name, mode);
}

int _flashIoRmdir(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *name = (const char *)args[1];

	char ms_path[128];
	int fs_num = BuildMsPathChangeFsNum(arg, name, ms_path);
	int res = ms_funcs.IoRmdir(arg, ms_path);

	if (fs_num == 1 || res >= 0)
		return res;

	arg->fs_num = fs_num;
	return flash_funcs.IoRmdir(arg, name);
}

int _flashIoDopen(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *dirname = (const char *)args[1];

	char ms_path[128];
	int fs_num = BuildMsPathChangeFsNum(arg, dirname, ms_path);
	int res = ms_funcs.IoDopen(arg, ms_path);
	if (res >= 0) {
		((u32 *)arg)[18] = 1;
	}

	if (fs_num == 1 || res >= 0)
		return res;

	arg->fs_num = fs_num;
	return flash_funcs.IoDopen(arg, dirname);
}

int _flashIoDclose(PspIoDrvFileArg *arg) {
	if (((u32 *)arg)[18]) {
		arg->fs_num = 0;
		return ms_funcs.IoDclose(arg);
	}

	return flash_funcs.IoDclose(arg);
}

int _flashIoDread(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	SceIoDirent *dir = (SceIoDirent *)args[1];

    if (dir) dir = UnprotectAddress(dir);

	if (((u32 *)arg)[18]) {
		arg->fs_num = 0;
		return ms_funcs.IoDread(arg, dir);
	}

	return flash_funcs.IoDread(arg, dir);
}

int _flashIoGetstat(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *file = (const char *)args[1];
	SceIoStat *stat = (SceIoStat *)args[2];

	char ms_path[128];
	int fs_num = BuildMsPathChangeFsNum(arg, file, ms_path);
	int res = ms_funcs.IoGetstat(arg, ms_path, stat);

	if (fs_num == 1 || res >= 0)
		return res;

	arg->fs_num = fs_num;
	return flash_funcs.IoGetstat(arg, file, stat);
}

int _flashIoChstat(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *file = (const char *)args[1];
	SceIoStat *stat = (SceIoStat *)args[2];
	int bits = (int)args[3];

	char ms_path[128];
	int fs_num = BuildMsPathChangeFsNum(arg, file, ms_path);
	int res = ms_funcs.IoChstat(arg, ms_path, stat, bits);

	if (fs_num == 1 || res >= 0)
		return res;

	arg->fs_num = fs_num;
	return flash_funcs.IoChstat(arg, file, stat, bits);
}

int _flashIoChdir(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *dir = (const char *)args[1];

	char ms_path[128];
	int fs_num = BuildMsPathChangeFsNum(arg, dir, ms_path);
	int res = ms_funcs.IoChdir(arg, ms_path);

	if (fs_num == 1 || res >= 0)
		return res;

	arg->fs_num = fs_num;
	return flash_funcs.IoChdir(arg, dir);
}

int flashIoOpen(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode) {
	u32 args[4];
	args[0] = (u32)arg;
	args[1] = (u32)file;
	args[2] = (u32)flags;
	args[3] = (u32)mode;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoOpen, args);
}

int flashIoClose(PspIoDrvFileArg *arg) {
	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoClose, arg);
}

int flashIoRead(PspIoDrvFileArg *arg, char *data, int len) {
	u32 args[3];
	args[0] = (u32)arg;
	args[1] = (u32)data;
	args[2] = (u32)len;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoRead, args);
}

int flashIoWrite(PspIoDrvFileArg *arg, const char *data, int len) {
	u32 args[3];
	args[0] = (u32)arg;
	args[1] = (u32)data;
	args[2] = (u32)len;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoWrite, args);
}

SceOff flashIoLseek(PspIoDrvFileArg *arg, SceOff ofs, int whence) {
	u32 args[3];
	args[0] = (u32)arg;
	args[1] = (u32)ofs;
	args[2] = (u32)whence;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoLseek, args);
}

int flashIoIoctl(PspIoDrvFileArg *arg, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen) {
	return 0;
}

int flashIoRemove(PspIoDrvFileArg *arg, const char *name) {
	u32 args[2];
	args[0] = (u32)arg;
	args[1] = (u32)name;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoRemove, args);
}

int flashIoMkdir(PspIoDrvFileArg *arg, const char *name, SceMode mode) {
	u32 args[3];
	args[0] = (u32)arg;
	args[1] = (u32)name;
	args[2] = (u32)mode;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoMkdir, args);
}

int flashIoRmdir(PspIoDrvFileArg *arg, const char *name) {
	u32 args[2];
	args[0] = (u32)arg;
	args[1] = (u32)name;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoRmdir, args);
}

int flashIoDopen(PspIoDrvFileArg *arg, const char *dirname) {
	u32 args[2];
	args[0] = (u32)arg;
	args[1] = (u32)dirname;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoDopen, args);
}

int flashIoDclose(PspIoDrvFileArg *arg) {
	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoDclose, arg);
}

int flashIoDread(PspIoDrvFileArg *arg, SceIoDirent *dir) {
	u32 args[2];
	args[0] = (u32)arg;
	args[1] = (u32)dir;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoDread, args);
}

int flashIoGetstat(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat) {
	u32 args[3];
	args[0] = (u32)arg;
	args[1] = (u32)file;
	args[2] = (u32)stat;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoGetstat, args);
}

int flashIoChstat(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat, int bits) {
	u32 args[4];
	args[0] = (u32)arg;
	args[1] = (u32)file;
	args[2] = (u32)stat;
	args[3] = (u32)bits;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoChstat, args);
}

int flashIoChdir(PspIoDrvFileArg *arg, const char *dir) {
	u32 args[2];
	args[0] = (u32)arg;
	args[1] = (u32)dir;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoChdir, args);
}

int flashIoDevctl(PspIoDrvFileArg *arg, const char *devname, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen) {
	return 0;
}
#endif

void redirectFlashFileSystem(){
    flash_redirect = 1;
}

// sceIoAddDrv Hook
int sceIoAddDrvHook(PspIoDrv * driver)
{
    // "flash" Driver
    if (strcmp(driver->name, "flash") == 0) {
        memcpy(&flash_funcs, driver->funcs, sizeof(PspIoDrvFuncs));

        #if 1
        // Hook IoOpen Function
        sceIoFlashOpen = driver->funcs->IoOpen;
        driver->funcs->IoOpen = sceIoFlashOpenHook;

        sceIoFlashRead_ = driver->funcs->IoRead;
        driver->funcs->IoRead = sceIoFlashReadHook;
    
        sceIoFlashWrite_ = driver->funcs->IoWrite;
        driver->funcs->IoWrite = sceIoFlashWriteHook;

        #else
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
        #endif

        // Add flashfat driver
		flashfat_drv->funcs = driver->funcs;
		sceIoAddDrv(flashfat_drv);
    }
    // "ms" Driver
    else if (strcmp(driver->name, "fatms") == 0) {
		memcpy(&ms_funcs, driver->funcs, sizeof(PspIoDrvFuncs));

        // Hook IoOpen Function
        sceIoMsOpen = driver->funcs->IoOpen;
        driver->funcs->IoOpen = sceIoMsOpenHook;
        
        sceIoMsRead_ = driver->funcs->IoRead;
        driver->funcs->IoRead = sceIoMsReadHook;
        
        sceIoMsWrite_ = driver->funcs->IoWrite;
        driver->funcs->IoWrite = sceIoMsWriteHook;

		// Add ms driver
		ms_drv->funcs = driver->funcs;
		sceIoAddDrv(ms_drv);
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
    return sceIoAddDrv(driver);
}

// "flash" Driver IoOpen Hook
int sceIoFlashOpenHook(PspIoDrvFileArg * arg, char * file, int flags, SceMode mode)
{
    flash_driver = arg->drv;
    // flash0 File Access Attempt
    if (arg->fs_num < 4 && flash_redirect) {
        // File Path Buffer
        char msfile[256];
        int fs_num = arg->fs_num;
        
        // Create "ms" File Path (links to flash0 folder on ms0)
        sprintf(msfile, "/flash/%d%s", fs_num, file);
        
        // Exchange Filesystem Driver for "ms"
        arg->drv = ms_driver;
        arg->fs_num = 0;
        
        // Open File from "ms"
        int k1 = pspSdkSetK1(0);
        int fd = sceIoMsOpen(arg, msfile, flags, mode);
        pspSdkSetK1(k1);
        
        // Open Success
        if (fd >= 0)
            return fd;
        
        // Restore Filesystem Driver to "flash"
        arg->drv = flash_driver;
        arg->fs_num = fs_num;
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

#if 0
int (*iojal)(const char *, u32, u32, u32, u32, u32) = NULL;
int patchio(const char *a0, u32 a1, u32 a2, u32 a3, u32 t0, u32 t1)
{

    int res;
    if (strncmp(a0, "flash", 5) == 0){
        char newpath[ARK_PATH_SIZE];
        sprintf(newpath, "ms0:/flash/%c%s", a0[5], a0+7);
        res = iojal(newpath, a1, a2, a3, t0, t1);
        if (res>=0) return res;
    }
    /*
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
    */
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
#endif

