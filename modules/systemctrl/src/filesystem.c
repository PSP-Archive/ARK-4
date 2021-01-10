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
#include "custom_png.h"
#include "filesystem.h"
#include "lflash0.h"

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

// sceIoMkdirHook semaphore
//static SceUID mkDirSema = -1;

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
//int sceIoMsOpenHook(PspIoDrvFileArg * arg, char * file, int flags, SceMode mode);

// sceIoDopen Hook
int sceIoDopenHook(const char * dirname);

// sceIoDread Hook
int sceIoDreadHook(int fd, SceIoDirent * dir);

// sceIoDclose Hook
int sceIoDcloseHook(int fd);

// sceIoMkdir Hook
int	(* _sceIoMkdir)(char *dir, SceMode mode) = (void*)NULL;
//int	sceIoMkdirHook(char *dir, SceMode mode);

int flashLoadPatch(int cmd);

// "flash" Driver IoOpen Original Call
int (* sceIoFlashOpen)(PspIoDrvFileArg * arg, char * file, int flags, SceMode mode) = NULL;

// "ms" Driver IoOpen Original Call
int (* sceIoMsOpen)(PspIoDrvFileArg * arg, char * file, int flags, SceMode mode) = NULL;

// "ms" Driver Reference
PspIoDrvArg * ms_driver = NULL;

// Homebrew Runlevel Check
int isHomebrewRunlevel(void)
{
	// Get Apitype
	int apitype = sceKernelInitApitype();
	
	if (apitype == 0x141) {
		// Homebrew Runlevel
		return 1;
	}
	
	// Other Runlevel
	return 0;
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

u32 iojal;

int patchio(const char *a0, u32 a1, u32 a2, u32 a3, u32 t0, u32 t1)
{
	if (!IS_VITA_POPS && !strcmp(a0, "flash0:/kd/npdrm.prx")){
		char path[SAVE_PATH_SIZE];
		strcpy(path, ARKPATH);
		strcat(path, "NPDRM.PRX");
		a0 = path;
	}

	int (* _iojal)(const char *, u32, u32, u32, u32, u32) = (void *)iojal;

	return _iojal(a0, a1, a2, a3, t0, t1);
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

// sceIOFileManager Patch
void patchFileManager(void)
{

	// Find Module
	SceModule2 * mod = (SceModule2 *)sceKernelFindModuleByName("sceIOFileManager");

	u32 IOAddDrv = sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForKernel", 0x8E982A74);
	
	u32 AddDrv = findRefInGlobals("IoFileMgrForKernel", IOAddDrv, IOAddDrv);

	// Hooking sceIoAddDrv
	_sw((unsigned int)sceIoAddDrvHook, AddDrv);

	iojal = mod->text_addr + 0x49F4;
	ioctl = mod->text_addr + 0x41A0;

	backup[0] = _lw(ioctl);
	backup[1] = _lw(ioctl + 4);

	_sw((((u32)&patchio >> 2) & 0x03FFFFFF) | 0x0C000000, mod->text_addr + 0x3FD8);
	_sw((((u32)&patchio >> 2) & 0x03FFFFFF) | 0x0C000000, mod->text_addr + 0x4060);
	_sw((((u32)&patchioctl >> 2) & 0x03FFFFFF) | 0x08000000, mod->text_addr + 0x41A0);
	_sw(0, mod->text_addr + 0x41A4);

	//HIJACK_FUNCTION(K_EXTRACT_IMPORT(sceIoRead), sceIoRfilesystem.ceadHook, sceIoRead_);
	//HIJACK_FUNCTION(K_EXTRACT_IMPORT(sceIoWrite), sceIoWriteHook, sceIoWrite_);
	
	/*
	// determine how we can install EBOOT.PBP
	if (IS_PSP)
		ark_config->eboot_install_mode = NO_EBOOT_HACK;
	else{
		SceUID fd = sceIoOpen("ms0:/EBOOT.PBP.", PSP_O_CREAT|PSP_O_TRUNC|PSP_O_WRONLY, 0777);
		sceIoClose(fd);
		if (sceIoRemove("ms0:/EBOOT.PBP.")>=0)
			ark_config->eboot_install_mode = EBOOT_DOT_HACK;
		else{
			SceUID fd = sceIoOpen("ms0:/EBOOT.PBP/", PSP_O_CREAT|PSP_O_TRUNC|PSP_O_WRONLY, 0777);
			sceIoClose(fd);
			if (sceIoRemove("ms0:/EBOOT.PBP/")>=0)
				ark_config->eboot_install_mode = EBOOT_SLASH_HACK;
			else
				ark_config->eboot_install_mode = VBOOT_HACK;
		}
	}
	
	// determine how we can install homebrews
	if (IS_PSP || !NEWER_FIRMWARE)
		ark_config->game_install_mode = NO_GAME_HACK;
	else{
		sceIoRename("ms0:/PSP.", "ms0:/QSP");
		if (sceIoRename("ms0:/QSP", "ms0:/PSP.")>=0)
			ark_config->game_install_mode = QSP_HACK;
		else
			ark_config->game_install_mode = APPS_HACK;
	}
	*/
}

void patchFileSystemDirSyscall(void)
{
	if (!isHomebrewRunlevel())
		return;
	
	// Create Semaphore
	dreadSema = sceKernelCreateSema("", 0, 1, 1, NULL);

	// Hooking sceIoDopen for User Modules
	sctrlHENPatchSyscall((void *)sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0xB29DDF9C), sceIoDopenHook);
	
	// Hooking sceIoDread for User Modules
	sctrlHENPatchSyscall((void *)sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0xE3EB004C), sceIoDreadHook);
	
	// Hooking sceIoDclose for User Modules
	sctrlHENPatchSyscall((void *)sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0xEB092469), sceIoDcloseHook);
	
	// Hooking sceIoMkdir for User Modules
	//sctrlHENPatchSyscall((void *)sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0x06A70004), sceIoMkdirHook);
	
	//sctrlHENPatchSyscall((void *)sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0x6A638D83), sceIoReadHook);
	//sctrlHENPatchSyscall((void *)sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0x42EC03AC), sceIoWriteHook);
}

// Directory IO Patch for PSP-like Behaviour
void patchFileManagerImports(SceModule2 * mod)
{
	//if (!isHomebrewRunlevel())
	//	return;
	
	// Hooking sceIoDopen for Kernel Modules
	hookImportByNID(mod, "IoFileMgrForKernel", 0xB29DDF9C, sceIoDopenHook);
	
	// Hooking sceIoDread for Kernel Modules
	hookImportByNID(mod, "IoFileMgrForKernel", 0xE3EB004C, sceIoDreadHook);
	
	// Hooking sceIoDclose for Kernel Modules
	hookImportByNID(mod, "IoFileMgrForKernel", 0xEB092469, sceIoDcloseHook);
	
	// Hooking sceIoMkdir for Kernel Modules
	//hookImportByNID(mod, "IoFileMgrForKernel", 0x06A70004, sceIoMkdirHook);
	
	//hookImportByNID(mod, "IoFileMgrForKernel", 0x6A638D83, sceIoReadHook);
	//hookImportByNID(mod, "IoFileMgrForKernel", 0x42EC03AC, sceIoWriteHook);
}

void lowerString(char* orig, char* ret, int strSize){
	int i=0;
	while (*(orig+i) && i<strSize-1){
		*(ret+i) = *(orig+i);
		if (*(orig+i) >= 'A' && *(orig+i) <= 'Z')
			*(ret+i) += 0x20;
		i++;
	}
	*(ret+i) = 0;
}

// sceIoMkdir Hook to allow creating folders in ms0:/PSP/GAME
/*
int	sceIoMkdirHook(char *dir, SceMode mode)
{

	u32 k1 = pspSdkSetK1(0);
	
	if (mkDirSema < 0)
		mkDirSema = sceKernelCreateSema("sceMkdirPatch", 0, 1, 1, NULL);

	sceKernelWaitSema(mkDirSema, 1, NULL);
	
	char lower[14];
	lowerString(dir, lower, 14);
	int needsPatch = (ark_config->game_install_mode != NO_GAME_HACK && strncmp(lower, "ms0:/psp/game", 13)==0);
	char bak[4];
	
	if (needsPatch){
		switch (ark_config->game_install_mode){
			case QSP_HACK:{
				sceIoRename("ms0:/PSP.", "ms0:/QSP");
				bak[0] = *(dir+5);
				*(dir+5) = 'Q';
				break;
			}
			case APPS_HACK:{
				*(unsigned int*)bak = *(unsigned int*)(&dir[9]);
				*(unsigned int*)(&dir[9]) = 0x73707061; // redirect to ms0:/psp/apps
				break;
			}
			default: break;
		}
	}
	
	int ret = sceIoMkdir(dir, mode);
	
	if (needsPatch){
		switch (ark_config->game_install_mode){
			case QSP_HACK:{
				sceIoRename("ms0:/QSP", "ms0:/PSP.");
				*(dir+5) = bak[0];
				break;
			}
			case APPS_HACK:{
				*(unsigned int*)(&dir[9]) = *(unsigned int*)bak;
				break;
			}
			default: break;
		}
	}
	
	sceKernelSignalSema(mkDirSema, 1);
	
	pspSdkSetK1(k1);
	
	return ret;
}
*/

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
	else if(strcmp(driver->name, "ms") == 0) { // "ms" Driver
		// Hook IoOpen Function
		sceIoMsOpen = driver->funcs->IoOpen;
		//driver->funcs->IoOpen = sceIoMsOpenHook;
		
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
	// flash0 File Access Attempt
	if (arg->fs_num == 0) {
		// File Path Buffer
		char msfile[256];
		
		// Create "ms" File Path (links to flash0 folder on ms0)
		sprintf(msfile, "/flash0%s", file);
		
		// Backup "flash" Filesystem Driver
		PspIoDrvArg * backup = arg->drv;
		
		// Exchange Filesystem Driver for "ms"
		arg->drv = ms_driver;
		
		// Open File from "ms"
		int fd = sceIoMsOpen(arg, msfile, flags, mode);
		
		// Open Success
		if (fd >= 0)
			return fd;
		
		// Restore Filesystem Driver to "flash"
		arg->drv = backup;
	}
	
	// Forward Call
	return sceIoFlashOpen(arg, file, flags, mode);
}

// "ms" Driver IoOpen Hook
/*
int sceIoMsOpenHook(PspIoDrvFileArg * arg, char * file, int flags, SceMode mode)
{
	// Update Internal "ms" Driver Reference
	ms_driver = arg->drv;
	
	// File Write Open Request
	if ((flags & PSP_O_WRONLY) != 0 && ark_config->eboot_install_mode != NO_EBOOT_HACK) {
		// Search for EBOOT.PBP Filename
		char * occurence = strstr(file, "/EBOOT.PBP");
		
		// Trying to write a EBOOT.PBP File
		if (occurence != NULL) {
			switch (ark_config->eboot_install_mode){
				case VBOOT_HACK:{
					strcpy(occurence, "/VBOOT.PBP");
					break;
				}
				case EBOOT_DOT_HACK:{
					char new_path[256];
					strcpy(new_path, file);
					strcpy(new_path+(occurence-file), "/EBOOT.PBP.");
					return sceIoMsOpen(arg, new_path, flags, mode);
				}
				case EBOOT_SLASH_HACK:{
					char new_path[256];
					strcpy(new_path, file);
					strcpy(new_path+(occurence-file), "/EBOOT.PBP/");
					return sceIoMsOpen(arg, new_path, flags, mode);
				}
				default: break;
			}
		}
	}
	
	// Forward Call
	return sceIoMsOpen(arg, file, flags, mode);
}
*/

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
			break;
	}
	
	// Directory not found
	return item;
}

// sceIoDopen Hook
int sceIoDopenHook(const char * dirname)
{
	// Forward Call
	int result = sceIoDopen(dirname);

	// Open Success
	if (result >= 0 && 0 == strncmp(dirname, "ms0:/", sizeof("ms0:/") - 1)) {
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

			if (err != 0)
				goto forward;

			// Valid Output Argument
			if (dir != NULL) {
				// Clear Memory
				memset(dir, 0, sizeof(SceIoDirent));
				
				// Copy Status
				memcpy(&dir->d_stat, &stat, sizeof(stat));
				
				// Fake "." Output Stage
				if (item->stage == 0) {
					// Set File Name
					strcpy(dir->d_name, ".");
				} else { // Fake ".." Output Stage
					// Set File Name
					strcpy(dir->d_name, "..");
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

forward:
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

// kermit_peripheral's sub_000007CC clone, called by loadexec + 0x0000299C with a0=8 (was a0=7 for fw <210)
// Returns 0 on success
u64 kermit_flash_load(int cmd)
{
	u8 buf[128];
	u64 resp = -1;
	void *alignedBuf = (void*)ALIGN_64((int)buf + 63);
	sceKernelDcacheInvalidateRange(alignedBuf, 0x40);
	
	KermitPacket *packet = (KermitPacket *)KERMIT_PACKET((int)alignedBuf);

	// Find Kermit command execute function
	int (* _sceKermit_driver_4F75AA05)(KermitPacket *packet, u32 cmd_mode, u32 cmd, u32 argc, u32 allow_callback, u64 *resp) = NULL;
	_sceKermit_driver_4F75AA05 = (void *)sctrlHENFindFunction("sceKermit_Driver", "sceKermit_driver", 0x4F75AA05);
	if (_sceKermit_driver_4F75AA05 == NULL)
		_sceKermit_driver_4F75AA05 = (void *)sctrlHENFindFunction("sceKermit_Driver", "sceKermit_driver", 0x36666181);

	u32 argc = 0;
	_sceKermit_driver_4F75AA05(packet, KERMIT_MODE_PERIPHERAL, cmd, argc, KERMIT_CALLBACK_DISABLE, &resp);


	return resp;
}

int patchFlash0Archive(void)
{
	int fd;

	// Base Address
	uint32_t procfw = 0x8BA00000;
	uint32_t sony = FLASH_SONY;

	// flash0 Filecounts
	uint32_t procfw_filecount = 0;
	uint32_t flash0_filecount = 0;

	// Cast PROCFW flash0 Filesystem
	VitaFlashBufferFile * prof0 = (VitaFlashBufferFile *)procfw;
	
	// Cast Sony flash0 Filesystem
	VitaFlashBufferFile * f0 = (VitaFlashBufferFile *)sony;
	
	// Prevent Double Tapping
	//if (prof0[0].name == (char*)f0)
	//	return 0;

	char path[SAVE_PATH_SIZE];
	strcpy(path, ARKPATH);
	strcat(path, "FLASH0.ARK");

	fd = sceIoOpen(path, PSP_O_RDONLY, 0777);

	if (fd < 0)
		return fd;

	sceIoRead(fd, &procfw_filecount, sizeof(procfw_filecount));
	sceIoClose(fd);

	// Count Sony flash0 Files
	while (f0[flash0_filecount].content != NULL)
		flash0_filecount++;

	// Copy Sony flash0 Filesystem into PROCFW flash0
	memcpy(&prof0[procfw_filecount], f0, (flash0_filecount + 1) * sizeof(VitaFlashBufferFile));
	
	// Cast Namebuffer
	char * namebuffer = (char *)sony;
	
	// Cast Contentbuffer
	unsigned char * contentbuffer = (unsigned char *)&prof0[procfw_filecount + flash0_filecount + 1];
	
	// Ammount of linked in Files
	unsigned int linked = 0;
	
	fd = sceIoOpen(path, PSP_O_RDONLY, 0777);

	if (fd < 0)
		return fd;

	int fileSize, ret, i;
	unsigned char lenFilename;

	// skip file counter
	sceIoRead(fd, &fileSize, sizeof(fileSize));

	for (i = 0; i < procfw_filecount; ++i) {
		ret = sceIoRead(fd, &fileSize, sizeof(fileSize));

		if (ret != sizeof(fileSize))
			break;

		ret = sceIoRead(fd, &lenFilename, sizeof(lenFilename));

		if (ret != sizeof(lenFilename))
			break;

		ret = sceIoRead(fd, namebuffer, lenFilename);

		if (ret != lenFilename)
			break;

		namebuffer[lenFilename] = '\0';

		// Content Buffer 64 Byte Alignment required
		// (if we don't align this buffer by 64 PRXDecrypt will fail on 1.67+ FW!)
		if ((((unsigned int)contentbuffer) % 64) != 0) {
			// Align Content Buffer
			contentbuffer += 64 - (((unsigned int)contentbuffer) % 64);
		}
		
		ret = sceIoRead(fd, contentbuffer, fileSize);

		if (ret != fileSize)
			break;

		// Link File into virtual flash0 Filesystem
		prof0[linked].name = namebuffer;
		prof0[linked].content = contentbuffer;
		prof0[linked++].size = fileSize;

		// Move Buffer
		namebuffer += lenFilename + 1;
		contentbuffer += fileSize;
	}

	sceIoClose(fd);

	// Injection Error
	if (procfw_filecount == 0 || linked != procfw_filecount)
		return -1;
	
	// Return Number of Injected Files
	return linked;
}

int flashLoadPatch(int cmd)
{
	int ret = kermit_flash_load(cmd);

	// Custom handling on loadFlash mode, else nothing
	if ((cmd == KERMIT_CMD_ERROR_EXIT || cmd == KERMIT_CMD_ERROR_EXIT_2) && (ret != -1)) {
		int linked;

		// Wait for flash to load
		sceKernelDelayThread(10000);

		// Patch flash0 Filesystem Driver
		if ((linked = patchFlash0Archive()) < 0) {
			// Failed xD
		}
	}

	return ret;
}

int patchKermitPeripheral(SceModule2 * kermit_peripheral)
{
	// Redirect KERMIT_CMD_ERROR_EXIT loadFlash function
	u32 knownnids[2] = { 0x3943440D, 0x0648E1A3 /* 3.3X */ };
	u32 swaddress = 0;
	u32 i;
	for (i = 0; i < 2; i++)
	{
		swaddress = findFirstJALForFunction("sceKermitPeripheral_Driver", "sceKermitPeripheral_driver", knownnids[i]);
		if (swaddress != 0)
			break;
	}
	_sw(JUMP(flashLoadPatch), swaddress);
	_sw(NOP, swaddress+4);

	return 0;
}

SceUID sceIoOpenPSX(const char *file, int flags, SceMode mode)
{
	return sceIoOpen(file, (flags & 0x40000000) ? (flags & ~0x40000000) : flags, mode);
}

int sceIoIoctlPSX(SceUID fd, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen)
{
	int res = sceIoIoctl(fd, cmd, indata, inlen, outdata, outlen);

	if(res < 0)
	{
		if(cmd == 0x04100001)
		{
			return 0;
		}
		else if(cmd == 0x04100002)
		{
			u32 offset = *(u32 *)indata;		
			sceIoLseek(fd, offset, PSP_SEEK_SET);
			return 0;
		}
	}
	
	return res;
}

int sceIoReadPSX(SceUID fd, void *data, SceSize size)
{
	int k1 = pspSdkSetK1(0);
	int res = sceIoRead(fd, data, size);

	if(res == size)
	{
		if(size == size_custom_png)
		{
			if(use_custom_png)
			{
				u32 magic = 0x474E5089;
				if(memcmp(data, &magic, sizeof(u32)) == 0)
				{
					memcpy(data, custom_png, size_custom_png);
					res = size_custom_png;
					goto RETURN;
				}
			}
		}
		else if(size == 4)
		{
			u32 magic = 0x464C457F;
			if(memcmp(data, &magic, sizeof(u32)) == 0)
			{
				magic = 0x5053507E;
				memcpy(data, &magic, sizeof(u32));
				goto RETURN;
			}
		}

		/* Castlevania sound patch */
		if(size >= 0x420)
		{
			if(((u8 *)data)[0x41B] == 0x27 && ((u8 *)data)[0x41C] == 0x19)
			{
				if(((u8 *)data)[0x41D] == 0x22 && ((u8 *)data)[0x41E] == 0x41)
				{
					if(((u8 *)data)[0x41A] == ((u8 *)data)[0x41F])
					{
						((u8 *)data)[0x41B] = 0x55;
					}
				}
			}
		}
	}

RETURN:
	pspSdkSetK1(k1);
	return res;
}
