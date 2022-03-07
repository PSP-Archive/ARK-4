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
#include <zlib.h>
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

// Static Buffer
static char bignamebuffer[256];

u32 g_p2_size = 24;
u32 g_p9_size = 0; //24;
int sctrlHENSetMemory(u32 p2, u32 p9)
{
    if(p2 != 0 && (p2 + p9) <= MAX_HIGH_MEMSIZE) {
        g_p2_size = p2;
        g_p9_size = p9;
    }

    return 0;
}

int sctrlKernelLoadExecVSHDisc(const char *file, struct SceKernelLoadExecVSHParam *param)
{
	int k1 = pspSdkSetK1(0);
	int res = -1;
	int (*LoadExecVSHDisc)() = FindFunction("sceLoadExec", "LoadExecForKernel", 0xD8320A28);
	res = LoadExecVSHDisc(file, param);
	pspSdkSetK1(k1);
	return res;
}

int sctrlKernelLoadExecVSHDiscUpdater(const char *file, struct SceKernelLoadExecVSHParam *param)
{
	int k1 = pspSdkSetK1(0);
	int res = -1;
	int (*LoadExecVSHDiscUpdater)() = FindFunction("sceLoadExec", "LoadExecForKernel", 0xD4B49C4B);
	res = LoadExecVSHDiscUpdater(file, param);
	pspSdkSetK1(k1);
	return res;
}

int sctrlKernelLoadExecVSHMs1(const char *file, struct SceKernelLoadExecVSHParam *param)
{
	int k1 = pspSdkSetK1(0);
	int res = -1;
	int (*LoadExecVSHMs1)() = FindFunction("sceLoadExec", "LoadExecForKernel", 0x4FB44D27);
	res = LoadExecVSHMs1(file, param);
	pspSdkSetK1(k1);
	return res;
}

int sctrlKernelLoadExecVSHMs2(const char *file, struct SceKernelLoadExecVSHParam *param)
{
	int k1 = pspSdkSetK1(0);	
	int res = -1;
	int (*LoadExecVSHMs2)() = FindFunction("sceLoadExec", "LoadExecForKernel", 0xD940C83C);
	res = LoadExecVSHMs2(file, param);
	pspSdkSetK1(k1);
	return res;
}

int sctrlKernelLoadExecVSHMs3(const char *file, struct SceKernelLoadExecVSHParam *param)
{
	int k1 = pspSdkSetK1(0);	
	int res = -1;
	int (*LoadExecVSHMs3)() = FindFunction("sceLoadExec", "LoadExecForKernel", 0xCC6A47D2);
	res = LoadExecVSHMs3(file, param);
	pspSdkSetK1(k1);
	return res;
}

int sctrlKernelLoadExecVSHMs4(const char *file, struct SceKernelLoadExecVSHParam *param)
{
	int k1 = pspSdkSetK1(0);	
	int res = -1;
	int (*LoadExecVSHMs4)() = FindFunction("sceLoadExec", "LoadExecForKernel", 0x00745486);
	res = LoadExecVSHMs4(file, param);
	pspSdkSetK1(k1);
	return res;
}

int sctrlKernelLoadExecVSHEf2(const char *file, struct SceKernelLoadExecVSHParam *param)
{
	int k1 = pspSdkSetK1(0);
	int res = -1;
	int (*LoadExecVSHEf2)() = FindFunction("sceLoadExec", "LoadExecForKernel", 0x032A7938);
	res = LoadExecVSHEf2(file, param);
	pspSdkSetK1(k1);
	return res;
}

int sctrlKernelExitVSH(struct SceKernelLoadExecVSHParam *param)
{
    u32 k1;
    int ret = -1;

    k1 = pspSdkSetK1(0);

    int (*_KernelExitVSH)(void*) = FindFunction("sceLoadExec", "LoadExecForKernel", 0x08F7166C);
    ret = _KernelExitVSH(param);
    
    pspSdkSetK1(k1);

    return ret;
}

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
    return ( (ARK_MAJOR_VERSION << 16) | (ARK_MINOR_VERSION << 8) | ARK_MICRO_VERSION );
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
    __asm__ volatile("cfc0 %0, $12\n" : "=r"(syscalls));
    
    // Invalid Syscall Table
    if(syscalls == NULL) return;
    
    // Skip Table Header
    syscalls += 4; // 4 * 4 = 16
    
    // Iterate Syscalls
    for(int i = 0; i < 0xFF4; ++i)
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
    
    // Find Target Function
    int (* _LoadExecVSHWithApitype)(int, const char*, struct SceKernelLoadExecVSHParam*, unsigned int)
        = (void *)findFirstJALForFunction("sceLoadExec", "LoadExecForKernel", 0xD8320A28);

    // Load Execute Module
    int result = _LoadExecVSHWithApitype(apitype, file, param, 0x10000);
    
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
    for (int i = 0; i < pMod->ent_size;)
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
                for(int j = 0; j < total; j++)
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
    if(alloc == NULL) __asm__ volatile ("break");
    
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
    for(int i = 0; i < nidTableSize; i++)
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

int sctrlKernelSetInitKeyConfig(int key)//old sctrlKernelSetInitMode
{
	int k1 = pspSdkSetK1(0);
	int r = sceKernelInitKeyConfig();

	if (kernel_init_keyconfig != NULL){
	    *kernel_init_keyconfig = key;
	}
	
	pspSdkSetK1(k1);
	return r;
}

int sctrlKernelMsIsEf(){
    int k1 = pspSdkSetK1(0);
    int apitype = sceKernelInitApitype();
    int res = (apitype == 0x155 || apitype == 0x125 || apitype == 0x152 || apitype ==  0x220);
    pspSdkSetK1(k1);
    return res;
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

void* sctrlSetStartModuleExtra(int (*func)() )
{
	void * ret = (void *)customStartModule;
	customStartModule = func;
	return ret;
}

int sctrlDeflateDecompress(void* dest, void* src, int size){
    if (dest == NULL || src == NULL || size == 0)
        return -1;
        
    unsigned int k1 = pspSdkSetK1(0);
    
    int ret = sceKernelDeflateDecompress(dest, size, src, 0);
    
    pspSdkSetK1(k1);
    
    return ret;
}

int sctrlDaxDecompress(void* output, void* input, u32 in_size){
    static const int DAX_BLOCK_SIZE = 0x2000;
    static const int DAX_COMP_BUF = 0x2400;
    u32 s = DAX_BLOCK_SIZE;
    uncompress(output, &s, input, MIN(DAX_COMP_BUF, in_size));
    return s;
}

u32 sctrlHENFindImport(const char *szMod, const char *szLib, u32 nid)
{
    SceModule2 *mod = sceKernelFindModuleByName(szMod);
    if(!mod) return 0;

    for(int i = 0; i < mod->stub_size;)
    {
        SceLibraryStubTable *stub = (SceLibraryStubTable *)(mod->stub_top + i);

        if(stub->libname && strcmp(stub->libname, szLib) == 0)
        {
            u32 *table = stub->nidtable;

            for(int j = 0; j < stub->stubcount; j++)
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

void sctrlHENGetArkConfig(ARKConfig* conf){
    u32 k1 = pspSdkSetK1(0);
    memcpy(conf, ark_config, sizeof(ARKConfig));
    pspSdkSetK1(k1);
}

void sctrlHENSetArkConfig(ARKConfig* conf){
    u32 k1 = pspSdkSetK1(0);
    if ( ( (k1<<11) & 0x80000000 ) == 0 )
        memcpy(ark_config, conf, sizeof(ARKConfig));
    pspSdkSetK1(k1);
}

void sctrlHENLoadModuleOnReboot(char *module_before, void *buf, int size, int flags)
{
    if (reboot_funcs)
        reboot_funcs->SetRebootModule(module_before, buf, size, flags);
}

int sctrlKernelSetUMDEmuFile(const char *filename)
{
	// Invalid Argument
    if(filename == NULL) return -1;
    
    // Field unavailable
    if(kernel_init_filename == NULL) return -2;
    
    // Copy Filename
    strncpy(bignamebuffer, filename, sizeof(bignamebuffer) - 1);
    
    // Link Buffer
    kernel_init_filename[1] = bignamebuffer;
    
    // Return Success
    return 0;
}

void sctrlHENSetSpeed(int cpuspd, int busspd)
{
    int (*_scePowerSetClockFrequency)(int, int, int);
    _scePowerSetClockFrequency = sctrlHENFindFunction("scePower_Service", "scePower", 0x545A7F3C);
    _scePowerSetClockFrequency(cpuspd, cpuspd, busspd);
}

int sctrlKernelBootFrom()
{
	/*
	int k1 = pspSdkSetK1(0);
	int (* BootFrom)() = (void *)(init_addrs[3]);
	int ret = BootFrom();
	pspSdkSetK1(k1);
	return ret;
	*/
	return sceKernelBootFrom();
}

int sctrlKernelQuerySystemCall(void *func_addr)
{
	int ret = -1;
	u32 k1;

	k1 = pspSdkSetK1(0);

	ret = sceKernelQuerySystemCall(func_addr);

	pspSdkSetK1(k1);

	return ret;
}

void *sctrlKernelMalloc(size_t size)
{
	return oe_malloc(size); 
}

int sctrlKernelFree(void *p)
{
	oe_free(p);
	return 0;
}

u32 sctrlKernelResolveNid(const char *szLib, u32 nid){
    return resolveMissingNid(szLib, nid);
}
