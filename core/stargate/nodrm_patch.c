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
#include <pspiofilemgr.h>
#include <systemctrl.h>
#include <systemctrl_private.h>
#include <macros.h>
#include <string.h>
#include "nodrm_patch.h"

// NODRM Hook Entry
typedef struct _NoDrmHookEntry {
	char * modname;
	char * libname;
	unsigned int nid;
	void * hook_addr;
} NoDrmHookEntry;

// NODRM File Descriptor List Node
typedef struct NoDrmFd {
	int fd;
	int asyncKeySetup;
	struct NoDrmFd * next;
} NoDrmFd;

// File Descriptor List Head Node
static NoDrmFd g_head;

// File Descriptor List Tail
static NoDrmFd * g_tail = &g_head;

// NODRM Semaphore
static int g_nodrm_sema = -1;

// DRM Magic #1 "PSPEDATA"
static unsigned char g_drm_magic_1[8] = {
	0x00, 0x50, 0x53, 0x50, 0x45, 0x44, 0x41, 0x54
};

// DRM Magic #2 "PGD"
static unsigned char g_drm_magic_2[4] = {
	0x00, 0x50, 0x47, 0x44
};

// Original Sony Function Pointer
static int (* _sceNpDrmRenameCheck)(char * fn);
static int (* _sceNpDrmEdataSetupKey)(int fd);
static SceOff (* _sceNpDrmEdataGetDataSize)(int fd);
static int (* _sceKernelLoadModuleNpDrm)(char * fn, int flag, void * opt);

// Import from loadmodule_patch.c
extern int (* _sceKernelLoadModule)(char * fname, int flag, void * opt);

// Initialization Function Prototypes
void nodrmGetFunctions(void);
void nodrmHookFunctions(void);

// Helper Function Prototypes
int check_memory(const void * addr, int size);
int is_encrypted_flag(int flag);
int check_file_is_encrypted(int fd);
int check_file_is_encrypted_by_path(const char * path);
static void lock(void);
static void unlock(void);

// Hook Function Prototypes
int myIoOpen(const char * file, int flag, int mode);
int myIoOpenAsync(const char * file, int flag, int mode);
int myNpDrmRenameCheck(char * fn);
int myNpDrmEdataSetupKey(int fd);
SceOff myNpDrmEdataGetDataSize(int fd);
int myKernelLoadModuleNpDrm(char * fn, int flag, void * opt);
int myIoIoctl(SceUID fd, unsigned int cmd, void * indata, int inlen, void * outdata, int outlen);
int myIoIoctlAsync(SceUID fd, unsigned int cmd, void * indata, int inlen, void * outdata, int outlen);
int myIoClose(SceUID fd);
int myIoCloseAsync(SceUID fd);
int myIoWaitAsyncCB(SceUID fd, SceIores *result);
int myIoPollAsync(SceUID fd, SceIores *result);

// Hook Table
static NoDrmHookEntry g_nodrm_hook_map[] = {
	{ "sceIOFileManager", "IoFileMgrForUser", 0x109F50BC, myIoOpen },
	{ "sceIOFileManager", "IoFileMgrForUser", 0x89AA9906, myIoOpenAsync },
	{ "sceIOFileManager", "IoFileMgrForUser", 0x810C4BC3, myIoClose },
	{ "sceIOFileManager", "IoFileMgrForUser", 0xFF5940B6, myIoCloseAsync },
	{ "sceIOFileManager", "IoFileMgrForUser", 0x63632449, myIoIoctl },
	{ "sceIOFileManager", "IoFileMgrForUser", 0xE95A012B, myIoIoctlAsync },
	{ "sceIOFileManager", "IoFileMgrForUser", 0x35DBD746, myIoWaitAsyncCB },
	{ "sceIOFileManager", "IoFileMgrForUser", 0x3251EA56, myIoPollAsync },
	{ "scePspNpDrm_Driver", "scePspNpDrm_user", 0x275987D1, myNpDrmRenameCheck },
	{ "scePspNpDrm_Driver", "scePspNpDrm_user", 0x08D98894, myNpDrmEdataSetupKey },
	{ "scePspNpDrm_Driver", "scePspNpDrm_user", 0x219EF5CC, myNpDrmEdataGetDataSize },
	{ "sceModuleManager", "ModuleMgrForUser", 0xF2D8D1B4, myKernelLoadModuleNpDrm },
};

// Initialize NODRM Engine
void nodrmInit(void)
{
	// Create Semaphore
	g_nodrm_sema = sceKernelCreateSema("", 0, 1, 1, NULL);
	
	// Find Functions in Memory
	nodrmGetFunctions();
	
	// Hook Functions via Syscall Table
	nodrmHookFunctions();
}

// Find Functions in Memory
void nodrmGetFunctions(void)
{
	// Find Functions
	_sceNpDrmRenameCheck = (void *)sctrlHENFindFunction("scePspNpDrm_Driver", "scePspNpDrm_user", 0x275987D1);
	_sceNpDrmEdataSetupKey = (void *)sctrlHENFindFunction("scePspNpDrm_Driver", "scePspNpDrm_user", 0x08D98894);
	_sceNpDrmEdataGetDataSize = (void *)sctrlHENFindFunction("scePspNpDrm_Driver", "scePspNpDrm_user", 0x219EF5CC);
	_sceKernelLoadModuleNpDrm = (void *)sctrlHENFindFunction("sceModuleManager", "ModuleMgrForUser", 0xF2D8D1B4);
}

// Hook Functions via Syscall Table
void nodrmHookFunctions(void)
{
	// Iterate Hook Table Items
	unsigned int i = 0; for(; i < NELEMS(g_nodrm_hook_map); i++)
	{
		// Patch System Call
		sctrlHENPatchSyscall((void *)sctrlHENFindFunction(g_nodrm_hook_map[i].modname, g_nodrm_hook_map[i].libname, g_nodrm_hook_map[i].nid), g_nodrm_hook_map[i].hook_addr);
	}
}

// K1 Memory Address Check
int check_memory(const void * addr, int size)
{
	// Buffer End Address
	const void * end_addr = addr + size - 1;
	
	// K1 Value
	unsigned int k1 = pspSdkGetK1();
	
	// Invalid Address
	if((int)(((unsigned int)end_addr | (unsigned int)addr) & (k1 << 11)) < 0)
	{
		// Return Error
		return 0;
	}
	
	// Return Success
	return 1;
}

// File Crypto Flag Matcher
int is_encrypted_flag(int flag)
{
	// File Crypto Flag Match
	if(flag == 0x40004001 || flag == 0x40000001)
		return 1;
	
	// Other Flags
	return 0;
}

// File Crypto Checker
int check_file_is_encrypted(int fd)
{
	// Work Buffer (unaligned)
	char p[8 + 64];
	
	// Work Buffer (aligned)
	char * buf = (char*)((((unsigned int)p) & ~(64-1)) + 64);
	
	// Elevate Permission Level
	unsigned int k1 = pspSdkSetK1(0);
	
	// Read Crypto Magic
	int result = sceIoRead(fd, buf, 8);
	
	// Restore Permission Level
	pspSdkSetK1(k1);
	
	// Rewind File
	sceIoLseek32(fd, 0, PSP_SEEK_SET);
	
	// Read Error (we assume its decrypted because of small size)
	if(result != 8) return 0;
	
	// DRM Magic #1 "PSPEDATA" Match
	if(memcmp(buf, g_drm_magic_1, sizeof(g_drm_magic_1)) == 0)
	{
		return 1;
	}
	
	// DRM Magic #2 "PGD"
	if(memcmp(buf, g_drm_magic_2, sizeof(g_drm_magic_2)) == 0)
	{
		return 1;
	}
	
	// Decrypted File
	return 0;
}

// File Crypto Checker (Path Variant)
int check_file_is_encrypted_by_path(const char * path)
{
	// Result
	int result = 0;
	
	// Elevate Permission Level
	unsigned int k1 = pspSdkSetK1(0);
	
	// Open File in Binary Mode
	int fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
	
	// Opened File
	if(fd >= 0)
	{
		// 2nd Stage Crypto Check
		result = check_file_is_encrypted(fd);
		
		// Close File
		sceIoClose(fd);
	}
	
	// Open Error (we assume its encrypted)
	else result = 1;
	
	// Restore Permission Level
	pspSdkSetK1(k1);
	
	// Return Result
	return result;
}

// Lock Semaphore
static void lock(void)
{
	// Elevate Permission Level
	unsigned int k1 = pspSdkSetK1(0);
	
	// Lock Semaphore
	sceKernelWaitSema(g_nodrm_sema, 1, 0);
	
	// Restore Permission Level
	pspSdkSetK1(k1);
}

// Unlock Semaphore
static void unlock(void)
{
	// Elevate Permission Level
	unsigned int k1 = pspSdkSetK1(0);
	
	// Unlock Semaphore
	sceKernelSignalSema(g_nodrm_sema, 1);
	
	// Restore Permission Level
	pspSdkSetK1(k1);
}

// Add NODRM File Descriptor to Internal List
void add_nodrm_fd(int fd)
{
	// Invalid File Descriptor
	if(fd < 0) return;
	
	// Lock List Access
	lock();
	
	// Allocate Internal List Item
	NoDrmFd * slot = (NoDrmFd *)oe_malloc(sizeof(NoDrmFd));
	
	// Allocation Error
	if(slot == NULL)
	{
		// Unlock List Access
		unlock();
		
		// Exit Function
		return;
	}
	
	// Save File Descriptor
	slot->fd = fd;
	slot->asyncKeySetup = 0;
	
	// Link Item into Internal List
	g_tail->next = slot;
	g_tail = slot;
	slot->next = NULL;
	
	// Unlock List Access
	unlock();
}

// Remove NODRM File Descriptor from Internal List
void remove_nodrm_fd(int fd)
{
	// Invalid File Descriptor
	if(fd < 0) return;
	
	// Lock List Access
	lock();
	
	// Iterator Variables
	NoDrmFd * prev = &g_head;
	NoDrmFd * fds = g_head.next;
	
	// Iterate File Descriptor List
	for(; fds != NULL; prev = fds, fds = fds->next)
	{
		// Found File Descriptor Match
		if(fd == fds->fd) break;
	}
	
	// Found File Descriptor
	if(fds != NULL)
	{
		// Unlick Item from Internal List
		prev->next = fds->next;
		if(g_tail == fds) g_tail = prev;
		
		// Free Item Memory
		oe_free(fds);
	}
	
	// Unlock List Access
	unlock();
}

struct NoDrmFd *find_nodrm_fd(SceUID fd)
{
	struct NoDrmFd *fds;

	if (fd < 0)
		return NULL;

	for(fds = g_head.next; fds != NULL; fds = fds->next)
	{
		if(fds->fd == fd)
			break;
	}

	return fds;
}

// Check if File Descriptor is a NODRM File Descriptor
int is_nodrm_fd(int fd)
{
	return find_nodrm_fd(fd) != NULL ? 1 : 0;
}

// sceIoOpen Hook
int myIoOpen(const char * file, int flag, int mode)
{
	// File Descriptor
	int fd = -1;
	
	// Encrypted File requested
	if(is_encrypted_flag(flag))
	{
		// Open File in Binary Mode
		fd = sceIoOpen(file, PSP_O_RDONLY, mode);
		
		// Opened File
		if(fd >= 0)
		{
			// Encrypted File encountered
			if(check_file_is_encrypted(fd))
			{
				// Close Binary File Descriptor
				sceIoClose(fd);
			}
			
			// Decrypted File encountered
			else
			{
				// Add File Descriptor to Internal List
				add_nodrm_fd(fd); 
				
				// Pass Binary Mode File Descriptor to Caller
				goto exit;
			}
		}
	}
	
	// Forward Call
	fd = sceIoOpen(file, flag, mode);
	
exit:
	
	// Return File Descriptor
	return fd;
}

// sceIoOpenAsync Hook
int myIoOpenAsync(const char * file, int flag, int mode)
{
	// File Descriptor
	int fd = -1;
	
	// Decrypted File Flag
	int is_plain = 0;
	
	// Encrypted File requested
	if(is_encrypted_flag(flag))
	{
		// Open File in Binary Mode
		fd = sceIoOpen(file, PSP_O_RDONLY, mode);
		
		// Opened File
		if(fd >= 0)
		{
			// Decrypted File encountered
			if(!check_file_is_encrypted(fd))
			{
				// Set Decrypted File Flag
				is_plain = 1;
			}
		}
		
		// Close Binary File Descriptor
		sceIoClose(fd);
	}
	
	// Decrypted File encountered
	if(is_plain)
	{
		// Open File in Binary Asynchronous Mode
		fd = sceIoOpenAsync(file, PSP_O_RDONLY, mode);
		
		// Opened File
		if(fd >= 0)
		{
			// Add File Descriptor to Internal List
			add_nodrm_fd(fd);
		}
	}
	
	// Encrypted File encountered
	else
	{
		// Forward Call
		fd = sceIoOpenAsync(file, flag, mode);
	}
	
	// Return File Descriptor
	return fd;
}

// sceNpDrmRenameCheck Hook
int myNpDrmRenameCheck(char * fn)
{
	// Result
	int result = -1;
	
	// Invalid Filename Buffer Memory Location
	if(!check_memory(fn, strlen(fn) + 1))
	{
		// Return Internal NPDRM Error for Invalid Memory Address
		result = 0x80550910;
		
		// Exit Function
		goto exit;
	}
	
	// Decrypted File encountered
	if(!check_file_is_encrypted_by_path(fn))
	{
		// File Information
		SceIoStat stat;
		
		// Elevate Permission Level
		unsigned int k1 = pspSdkSetK1(0);
		
		// Check File Existance and set File not found NPDRM Error if needed
		result = sceIoGetstat(fn, &stat) == 0 ? 0 : 0x80550901;
		
		// Restore Permission Level
		pspSdkSetK1(k1);
	}
	
	// Encrypted File encountered
	else
	{
		// Forward Call
		result = _sceNpDrmRenameCheck(fn);
	}
	
exit:
	
	// Return Result
	return result;
}

// sceNpDrmEdataSetupKey Hook
int myNpDrmEdataSetupKey(int fd)
{
	// Result
	int result = -1;
	
	// Decrypted File encountered
	if(is_nodrm_fd(fd))
	{
		// Fake Success
		result = 0;
	}
	
	// Encrypted File encountered
	else
	{
		// Forward Call
		result = _sceNpDrmEdataSetupKey(fd);
	}
	
	// Return Result
	return result;
}

// sceNpDrmEdataGetDataSize Hook
SceOff myNpDrmEdataGetDataSize(int fd)
{
	// File Size
	SceOff end;
	
	// Decrypted File encountered
	if(is_nodrm_fd(fd))
	{
		// Backup Offset
		SceOff off = sceIoLseek(fd, 0, PSP_SEEK_CUR);
		
		// Get File Size
		end = sceIoLseek(fd, 0, PSP_SEEK_END);
		
		// Restore Offset
		sceIoLseek(fd, off, PSP_SEEK_SET);
	}
	
	// Encrypted File encountered
	else
	{
		// Forward Call
		end = _sceNpDrmEdataGetDataSize(fd);
	}
	
	// Return File Size
	return end;
}

// sceKernelLoadModuleNpDrm Hook
int myKernelLoadModuleNpDrm(char * fn, int flag, void * opt)
{
	// Attempt Decrypted Module Load
	int result = _sceKernelLoadModule(fn, flag, opt);
	
	// Return Decrypted Module UID
	if(result >= 0) return result;
	
	// Forward Call
	return _sceKernelLoadModuleNpDrm(fn, flag, opt);
}

// sceIoIoctl Hook
int myIoIoctl(SceUID fd, unsigned int cmd, void * indata, int inlen, void * outdata, int outlen)
{
	// Result
	int result = -1;
	
	// Alternative Way of Setting Crypto Key (next to sceNpDrmEdataSetupKey)
	if(cmd == 0x04100001 || cmd == 0x04100002)
	{
		// Decrypted File encountered
		if(is_nodrm_fd(fd))
		{
			// Fake Success
			result = 0;
			
			// Exit Function
			goto exit;
		}
	}
	
	// Forward Call
	result = sceIoIoctl(fd, cmd, indata, inlen, outdata, outlen);
	
exit:
	
	// Return Result
	return result;
}

// sceIoIoctlAsync Hook
int myIoIoctlAsync(SceUID fd, unsigned int cmd, void * indata, int inlen, void * outdata, int outlen)
{
	// Result
	int result = -1;
	
	// Alternative Way of Setting Crypto Key (next to sceNpDrmEdataSetupKey)
	if(cmd == 0x04100001 || cmd == 0x04100002)
	{
		// Decrypted File encountered
		if (is_nodrm_fd(fd))
		{
			// Fake Success
			result = 0;

			// async key setup
			find_nodrm_fd(fd)->asyncKeySetup = 1;
			
			// Exit function
			goto exit;
		}
	} 
	
	// Forward Call
	result = sceIoIoctlAsync(fd, cmd, indata, inlen, outdata, outlen);
	
exit:
	
	// Return Result
	return result;
}

// sceIoClose Hook
int myIoClose(SceUID fd)
{
	// Forward Call
	int result = sceIoClose(fd);
	
	// Decrypted File encountered
	if(is_nodrm_fd(fd))
	{
		// Close Success
		if(result == 0)
		{
			// Remove File Descriptor from Internal List
			remove_nodrm_fd(fd);
		}
	}
	
	// Return Result
	return result;
}

// sceIoCloseAsync Hook
int myIoCloseAsync(SceUID fd)
{
	// Forward Call
	int result = sceIoCloseAsync(fd);
	
	// Decrypted File encountered
	if(is_nodrm_fd(fd))
	{
		// Close Success
		if(result == 0)
		{
			// Remove File Descriptor from Internal List
			remove_nodrm_fd(fd);
		}
	}
	
	// Return Result
	return result;
}

int myIoWaitAsyncCB(SceUID fd, SceIores *result)
{
	int ret;

	ret = sceIoWaitAsyncCB(fd, result);

	{
		struct NoDrmFd *fds;

		fds = find_nodrm_fd(fd);

		if(fds != NULL && fds->asyncKeySetup)
		{
			fds->asyncKeySetup = 0;
			*result = 0LL;
			ret = 0;
		}
	}

//	printk("%s: %d -> 0x%08X\r\n", __func__, fd, ret);

	return ret;
}

int myIoPollAsync(SceUID fd, SceIores *result)
{
	int ret;

	ret = sceIoPollAsync(fd, result);

	{
		struct NoDrmFd *fds;

		fds = find_nodrm_fd(fd);

		if(fds != NULL && fds->asyncKeySetup)
		{
			fds->asyncKeySetup = 0;
			*result = 0LL;
			ret = 0;
		}
	}

//	printk("%s: %d -> 0x%08X\r\n", __func__, fd, ret);

	return ret;
}
