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
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <pspthreadman_kernel.h>
#include <pspsysevent.h>
#include <pspiofilemgr.h>
#include <pspsysmem_kernel.h>
#include <pspinit.h>
#include <systemctrl.h>
#include <systemctrl_private.h>
#include <version.h>
#include <stdio.h>
#include <string.h>
#include <module2.h>
#include <globals.h>
#include <macros.h>
#include <functions.h>
#include "rebootex.h"
#include "nidresolver.h"
#include "modulemanager.h"
#include "loadercore.h"
#include "imports.h"
#include "sysmem.h"

// Set User Level
int sctrlKernelSetUserLevel(int level)
{
	// Elevate Permission Level
	unsigned int k1 = pspSdkSetK1(0);
	
	// Backup User Level
	int previouslevel = sceKernelGetUserLevel();
	
	
	u32 _sceKernelReleaseThreadEventHandler = FindFunction("sceThreadManager", "ThreadManForKernel", 0x72F3C145);
	
	u32 threadman_userlevel_struct = _lh(_sceKernelReleaseThreadEventHandler + 0x4)<<16;
	threadman_userlevel_struct += (short)_lh(_sceKernelReleaseThreadEventHandler + 0x18);
	
	
	// Set User Level
	_sw((level ^ 8) << 28, *(unsigned int *)(threadman_userlevel_struct) + 0x14);
	
	// Flush Cache
	flushCache();
	
	// Restore Permission Level
	pspSdkSetK1(k1);
	
	// Return previous User Level
	return previouslevel;
}

// Set System Firmware Version
int sctrlKernelSetDevkitVersion(int version)
{
	// Elevate Permission Level
	unsigned int k1 = pspSdkSetK1(0);
	
	// Backup Firmware Version
	int previousversion = sceKernelDevkitVersion();
	
	// Overwrite Firmware Version
	u32 DevkitVersion = sctrlHENFindFunction("sceSystemMemoryManager", "SysMemForKernel", 0xC886B169);
	_sh((version >> 16), DevkitVersion);
	_sh((version & 0xFFFF), DevkitVersion+8);
	
	// Flush Cache
	flushCache();
	
	// Restore Permission Level
	pspSdkSetK1(k1);
	
	// Return previous Firmware Version
	return previousversion;
}

// Get HEN Version
int sctrlHENGetVersion()
{
	return PRO_VERSION;
}

// Get HEN Minor Version
int sctrlHENGetMinorVersion()
{
	return PRO_MINOR_VERSION;
}

// Find Filesystem Driver
PspIoDrv * sctrlHENFindDriver(char * drvname)
{
	// Elevate Permission Level
	unsigned int k1 = pspSdkSetK1(0);
	
	// Find Function
	int * (* findDriver)(char * drvname) = (void*)findFirstJALForFunction("sceIOFileManager", "IoFileMgrForKernel", 0x76DA16E3);
	
	// Find Driver Wrapper
	int * wrapper = findDriver(drvname);
	
	// Search Result
	PspIoDrv * driver = NULL;
	
	// Found Driver Wrapper
	if(wrapper != NULL)
	{
		// Save Driver Pointer
		driver = (PspIoDrv *)(wrapper[1]);
	}
	
	// Restore Permission Level
	pspSdkSetK1(k1);
	
	// Return Driver
	return driver;
}

// Replace Function in Syscall Table
void sctrlHENPatchSyscall(void * addr, void * newaddr)
{
	// Syscall Table
	unsigned int * syscalls = NULL;
	
	// Get Syscall Table Pointer from Coprocessor
	asm("cfc0 %0, $12\n" : "=r"(syscalls));
	
	// Invalid Syscall Table
	if(syscalls == NULL) return;
	
	// Skip Table Header
	syscalls += 4; // 4 * 4 = 16
	
	// Iterate Syscalls
	int i = 0; for(; i < 0xFF4; ++i)
	{
		// Found Matching Function
		if((syscalls[i] & 0x0FFFFFFF) == (((unsigned int)addr) & 0x0FFFFFFF))
		{
			// Replace Syscall Function
			syscalls[i] = (unsigned int)newaddr;
		}
	}
}

// Dword Poker (relative to module text_addr)
int sctrlPatchModule(char * modname, unsigned int inst, unsigned int offset)
{
	// Poke Result
	int result = 0;
	
	// Elevate Permission Level
	unsigned int k1 = pspSdkSetK1(0);
	
	// Find Target Module
	SceModule2 * mod = (SceModule2 *)sceKernelFindModuleByName(modname);
	
	// Found Module
	if(mod != NULL)
	{
		// Poke Dword
		_sw(inst, mod->text_addr + offset);
		
		// Flush Cache
		flushCache();
	}
	
	// Module not found
	else result = -1;
	
	// Restore Permission Level
	pspSdkSetK1(k1);
	
	// Return Result
	return result;
}

// Text Address Getter
unsigned int sctrlModuleTextAddr(char * modname)
{
	// Result Value
	unsigned int text_addr = 0;
	
	// Elevate Permission Level
	unsigned int k1 = pspSdkSetK1(0);
	
	// Find Target Module
	SceModule2 * mod = (SceModule2 *)sceKernelFindModuleByName(modname);
	
	// Found Module
	if(mod != NULL)
	{
		// Set Text Address
		text_addr = mod->text_addr;
	}
	
	// Restore Permission Level
	pspSdkSetK1(k1);
	
	// Return Result
	return text_addr;
}

// Load Execute Module via Kernel Internal Function
int sctrlKernelLoadExecVSHWithApitype(int apitype, const char * file, struct SceKernelLoadExecVSHParam * param)
{
	// Elevate Permission Level
	unsigned int k1 = pspSdkSetK1(0);
	
	// Open Target File
	int result = sceIoOpen(file, PSP_O_RDONLY, 0777);
	
	// File Open Success
	if(result >= 0)
	{
		// Close File
		sceIoClose(result);
	}
	
	// File Open Error
	else
	{
		// EBOOT.PBP File Check
		char * eboot = strstr(file, "EBOOT.PBP");
		
		// Is EBOOT.PBP File
		if(eboot != NULL)
		{
			// Try VBOOT.PBP
			strcpy(eboot, "VBOOT.PBP");
			
			// Open Target File
			result = sceIoOpen(file, PSP_O_RDONLY, 0777);
			
			// File Open Success
			if(result >= 0)
			{
				// Close File
				sceIoClose(result);
			}
		}
	}
	
	// Open Success
	if(result >= 0)
	{
		// Find Target Function
		int (* _sctrlKernelLoadExecVSHWithApitype)(int apitype, const char * file, struct SceKernelLoadExecVSHParam * param, unsigned int unk) = (void *)findFirstJALForFunction("sceLoadExec", "LoadExecForKernel", 0xD8320A28);
		
		// Load Execute Module
		result = _sctrlKernelLoadExecVSHWithApitype(apitype, file, param, 0x10000);
	}
	
	// Restore Permission Level on Failure
	pspSdkSetK1(k1);
	
	// Return Error Code
	return result;
}

// Register Prologue Module Start Handler
STMOD_HANDLER sctrlHENSetStartModuleHandler(STMOD_HANDLER new_handler)
{
	// Get Previous Handler
	STMOD_HANDLER on_module_start = g_module_start_handler;
	
	// Register Handler
	g_module_start_handler = (void *)(KERNELIFY(new_handler));
	
	// Return Previous Handler
	return on_module_start;
}

// Find Function Address
unsigned int sctrlHENFindFunction(char * szMod, char * szLib, unsigned int nid)
{
	// Get NID Resolver
	NidResolverLib * resolver = getNidResolverLib(szLib);
	
	// Found Resolver for Library
	if(resolver != NULL)
	{
		// Resolve NID
		nid = getNidReplacement(resolver, nid);
	}
	
	// Find Target Module
	SceModule2 * pMod = (SceModule2 *)sceKernelFindModuleByName(szMod);
	
	// Module not found
	if(pMod == NULL)
	{
		// Attempt to find it by Address
		pMod = (SceModule2 *)sceKernelFindModuleByAddress((unsigned int)szMod);
		
		// Module not found
		if(pMod == NULL) return 0;
	}
	
	// Fetch Export Table Start Address
	void * entTab = pMod->ent_top;
	
	// Iterate Exports
	int i = 0; while(i < pMod->ent_size)
	{
		// Cast Export Table Entry
		struct SceLibraryEntryTable * entry = (struct SceLibraryEntryTable *)(entTab + i);
		
		// Found Matching Library
		if(entry->libname != NULL && szLib != NULL && 0 == strcmp(entry->libname, szLib))
		{
			// Accumulate Function and Variable Exports
			unsigned int total = entry->stubcount + entry->vstubcount;
			
			// NID + Address Table
			unsigned int * vars = entry->entrytable;
			
			// Exports available
			if(total > 0)
			{
				// Iterate Exports
				int j = 0; for(; j < total; j++)
				{
					// Found Matching NID
					if(vars[j] == nid) return vars[total + j];
				}
			}
		}
		
		// Move Pointer
		i += (entry->len * 4);
	}
	
	// Function not found
	return 0;
}

// Calculate Random Number via KIRK
unsigned int sctrlKernelRand(void)
{
	// Elevate Permission Level
	unsigned int k1 = pspSdkSetK1(0);
	
	// Allocate KIRK Buffer
	unsigned char * alloc = oe_malloc(20 + 4);
	
	// Allocation Error
	if(alloc == NULL) asm("break");
	
	// Align Buffer to 4 Bytes
	unsigned char * buffer = (void *)(((unsigned int)alloc & (~(4-1))) + 4);
	
	// KIRK Random Generator Opcode
	enum {
		KIRK_PRNG_CMD=0xE,
	};
	
	// Create 20 Random Bytes
	sceUtilsBufferCopyWithRange(buffer, 20, NULL, 0, KIRK_PRNG_CMD);
	
	// Fetch Random Number
	unsigned int random = *(unsigned int *)buffer;
	
	// Free Buffer
	oe_free(alloc);
	
	// Restore Permission Level
	pspSdkSetK1(k1);
	
	// Return Random Number
	return random;
}

// Enable or Disable NID Resolver for Library
int sctrlKernelSetNidResolver(char * libname, unsigned int enabled)
{
	// Iterate Libraries
	unsigned int i = 0; for(; i < nidTableSize; i++)
	{
		// Found Matching Library
		if(0 == strcmp(libname, nidTable[i].name))
		{
			// Fetch current Value
			unsigned int old = nidTable[i].enabled;
			
			// Overwrite Value
			nidTable[i].enabled = enabled;
			
			// Return current Value
			return old;
		}
	}
	
	// Library not found
	return -1;
}

// Set Init Apitype
int sctrlKernelSetInitApitype(int apitype)
{
	// Field unavailable
	if(kernel_init_apitype == NULL) return -1;
	
	// Read current Value
	int previousapitype = *kernel_init_apitype;
	
	// Set new Apitype
	*kernel_init_apitype = apitype;
	
	// Return old Apitype
	return previousapitype;
}

// Set Init Filename
int sctrlKernelSetInitFileName(char * filename)
{
	// Static Buffer
	static char bignamebuffer[256];
	
	// Invalid Argument
	if(filename == NULL) return -1;
	
	// Field unavailable
	if(kernel_init_filename == NULL) return -2;
	
	// Copy Filename
	strncpy(bignamebuffer, filename, sizeof(bignamebuffer) - 1);
	
	// Link Buffer
	*kernel_init_filename = bignamebuffer;
	
	// Return Success
	return 0;
}

// Return Text Address of init.prx
unsigned int sctrlGetInitTextAddr(void)
{
	// Return logged sceInit Text Address
	return sceInitTextAddr;
}

// Register Custom init.prx sceKernelStartModule Handler
void sctrlSetCustomStartModule(int (* func)(int modid, SceSize argsize, void * argp, int * modstatus, SceKernelSMOption * opt))
{
	// Register Handler
	customStartModule = func;
}

// Return PSID Hash
int sctrlKernelGetPSIDHash(unsigned char psidHash[16])
{
	memcpy(psidHash, reboot_config.psidHash, sizeof(reboot_config.psidHash));

	return 0;
}

int sctrlKernelDummyFunction(void)
{
	return 0;
}

// Return Game Product ID of currently running Game
int sctrlKernelGetGameID(char gameid[GAME_ID_MINIMUM_BUFFER_SIZE])
{
	// Invalid Arguments
	if(gameid == NULL) return -1;
	
	// Elevate Permission Level
	unsigned int k1 = pspSdkSetK1(0);
	
	// Fetch Game Information Structure
	void * gameinfo = SysMemForKernel_EF29061C_Fixed();
	
	// Restore Permission Level
	pspSdkSetK1(k1);
	
	// Game Information unavailable
	if(gameinfo == NULL) return -3;
	
	// Copy Product Code
	memcpy(gameid, gameinfo + 0x44, GAME_ID_MINIMUM_BUFFER_SIZE - 1);
	
	// Terminate Product Code
	gameid[GAME_ID_MINIMUM_BUFFER_SIZE - 1] = 0;
	
	// Return Success
	return 0;
}

int sctrlDeflateDecompress(void* dest, void* src, int size){
	if (dest == NULL || src == NULL || size == 0)
		return -1;
		
	unsigned int k1 = pspSdkSetK1(0);
	
	int ret = sceKernelDeflateDecompress(dest, size, src, 0);
	
	pspSdkSetK1(k1);
	
	return ret;
}

void sctrlHENGetConfig(ARKConfig* dest){
	if (dest == NULL)
		return;
	
	unsigned int k1 = pspSdkSetK1(0);
	
	memcpy(dest, ark_config, sizeof(ARKConfig));
	
	pspSdkSetK1(k1);
}

void sctrlHENSetConfig(ARKConfig* src){
	if (src == NULL)
		return;
	
	unsigned int k1 = pspSdkSetK1(0);
	
	memcpy(ark_config, src, sizeof(ARKConfig));
	
	pspSdkSetK1(k1);
}

u32 sctrlHENFindImport(const char *szMod, const char *szLib, u32 nid)
{
	SceModule2 *mod = sceKernelFindModuleByName(szMod);
	if(!mod) return 0;

	int i = 0;
	while(i < mod->stub_size)
	{
		SceLibraryStubTable *stub = (SceLibraryStubTable *)(mod->stub_top + i);

		if(stub->libname && strcmp(stub->libname, szLib) == 0)
		{
			u32 *table = stub->nidtable;

			int j;
			for(j = 0; j < stub->stubcount; j++)
			{
				if(table[j] == nid)
				{
					return ((u32)stub->stubtable + (j * 8));
				}
			}
		}

		i += (stub->len * 4);
	}

	return 0;
}

// EBOOT.PBP Parameter Getter
int sctrlGetInitPARAM(const char * paramName, u16 * paramType, u32 * paramLength, void * paramBuffer)
{
	// Enable Full Kernel Permission for Syscalls
	u32 k1 = pspSdkSetK1(0);
	
	// Invalid Argument
	if (paramName == NULL || paramName[0] == 0 || paramLength == NULL)
	{
		// Restore Syscall Permissions
		pspSdkSetK1(k1);
		
		// Return Error Code
		return 0x800001FF;
	}
		
	// Invalid Size
	if (*paramLength <= 0)
	{
		// Restore Syscall Permissions
		pspSdkSetK1(k1);
		
		// Return Error Code
		return 0x80000104;
	}
		
	// Get Init Filename
	const char * pbpPath = sceKernelInitFileName();
	
	// Init Filename not found
	if (pbpPath == NULL)
	{
		// Restore Syscall Permissions
		pspSdkSetK1(k1);
		
		// Return Error Code
		return 0x80010002;
	}
		
	// Get File Extension
	const char * pbpExtension = pbpPath;
	pbpExtension += strlen(pbpExtension) - 4;
	if (pbpExtension < pbpPath)
		pbpExtension = ".BIN";
	
	// Invalid Format
	if (pbpExtension[0] != '.' ||
		(pbpExtension[1] != 'P' && pbpExtension[1] != 'p') ||
		(pbpExtension[2] != 'B' && pbpExtension[2] != 'b') ||
		(pbpExtension[3] != 'P' && pbpExtension[3] != 'p'))
	{
		// Restore Syscall Permissions
		pspSdkSetK1(k1);
		
		// Return Error Code
		return 0x80000108;
	}
	
	// Open PBP File
	int fd = sceIoOpen(pbpPath, PSP_O_RDONLY, 0);
	
	// PBP File not found
	if (fd < 0)
	{
		// Restore Syscall Permissions
		pspSdkSetK1(k1);
		
		// Return Error Code
		return 0x80010002;
	}
	
	// seek to PARAM.SFO offset variable
	sceIoLseek(fd, 0x08, PSP_SEEK_SET);
	
	// read PARAM.SFO offset
	u32 paramOffset = 0;
	sceIoRead(fd, &paramOffset, sizeof(u32));
	
	// seek to PARAM.SFO offset
	sceIoLseek(fd, paramOffset, PSP_SEEK_SET);
	
	// seek to key table offset variable
	sceIoLseek(fd, 0x08, PSP_SEEK_CUR);
	
	// read variables of interest
	u32 keyTableOffset = 0;
	u32 dataTableOffset = 0;
	u32 entryCount = 0;
	sceIoRead(fd, &keyTableOffset, sizeof(keyTableOffset));
	sceIoRead(fd, &dataTableOffset, sizeof(dataTableOffset));
	sceIoRead(fd, &entryCount, sizeof(entryCount));
	
	// iterate entries
	u32 entry;
	for (entry = 0; entry < entryCount; entry++)
	{
		// read variables of interest
		u16 entryKeyOffset = 0;
		u16 entryFormat = 0;
		u32 entryUsedLength = 0;
		u32 entryFullLength = 0;
		u32 entryDataOffset = 0;
		sceIoRead(fd, &entryKeyOffset, sizeof(entryKeyOffset));
		sceIoRead(fd, &entryFormat, sizeof(entryFormat));
		sceIoRead(fd, &entryUsedLength, sizeof(entryUsedLength));
		sceIoRead(fd, &entryFullLength, sizeof(entryFullLength));
		sceIoRead(fd, &entryDataOffset, sizeof(entryDataOffset));
		
		// save offset for next entry
		SceOff nextEntryOffset = sceIoLseek(fd, 0, PSP_SEEK_CUR);
		
		// move to key name
		sceIoLseek(fd, paramOffset + keyTableOffset + entryKeyOffset, PSP_SEEK_SET);
		
		// read key name
		char keyName[128];
		memset(keyName, 0, sizeof(keyName));
		char symbol = 0;
		while (sceIoRead(fd, &symbol, sizeof(symbol)) > 0)
		{
			keyName[strlen(keyName)] = symbol;
			if (symbol == 0)
				break;
		}
		
		// found the parameter
		if (strcmp(keyName, paramName) == 0)
		{
			// Report the normal length
			u32 requiredLength = entryUsedLength;
			
			// We will terminate UTF-8 without terminators as an act of courtesy
			if (entryFormat == 0x0004)
				requiredLength = entryUsedLength + 1;
				
			// Parameter Output Buffer isn't big enough
			if (paramBuffer != NULL && *paramLength < requiredLength)
			{
				// Close File
				sceIoClose(fd);
				
				// Restore Syscall Permissions
				pspSdkSetK1(k1);
				
				// Invalid Size
				return 0x80000104;
			}
			
			// Buffer Length Request detected
			if (paramLength != NULL)
			{
				// Output required Buffer Length
				*paramLength = requiredLength;
			}
			
			// Data Type Request detected
			if (paramType != NULL)
			{
				// Output Data Type
				*paramType = entryFormat;
			}
			
			// move to entry data
			sceIoLseek(fd, paramOffset + dataTableOffset + entryDataOffset, PSP_SEEK_SET);
			
			// Output Buffer has been provided
			if (paramBuffer != NULL)
			{
				// reset buffer (also serves as termination of strings)
				memset(paramBuffer, 0, *paramLength);
				
				// read data into buffer
				sceIoRead(fd, paramBuffer, entryUsedLength);
			}
			
			// Close File
			sceIoClose(fd);
			
			// Restore Syscall Permissions
			pspSdkSetK1(k1);
			
			// Return Success
			return 0;
		}
		
		// resume processing at next entry
		sceIoLseek(fd, nextEntryOffset, PSP_SEEK_SET);
	}
	
	// close pbp file
	sceIoClose(fd);
	
	// Return Error Code (we just treat a missing parameter as file not found, it should work)
	return 0x80010002;
}

void sctrlHENEnableCustomPeopsConfig(void* config){
	u32 k1 = pspSdkSetK1(0);
	memcpy(&(ark_config->peops_config), config, sizeof(PeopsConfig));
	ark_config->override_peops_config = -1;
	pspSdkSetK1(k1);
}

void sctrlHENDisableCustomPeopsConfig(){
	u32 k1 = pspSdkSetK1(0);
	ark_config->override_peops_config = 0;
	pspSdkSetK1(k1);
}
