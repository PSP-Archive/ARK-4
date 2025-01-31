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

#ifndef _GAME_FUNCTIONS_H_
#define _GAME_FUNCTIONS_H_

#include <sdk.h>
#include <psploadexec_kernel.h>
#include "module2.h"
#include <ark.h>

struct minZipHeader {
    char pk[2];
    unsigned nb;
    char space[12];
    unsigned fileSize;
    unsigned fileSizeClone;
    unsigned pathLen;
    /*
    path
    data
    */
};

// K.BIN communication interface
typedef struct KxploitFunctions{
    int (*stubScanner)(struct UserFunctions*);
    int (*doExploit)(void);
    void (*executeKernel)(u32 kernelContentFunction);
    void (*repairInstruction)(struct KernelFunctions*);
}KxploitFunctions;

/*
 * These functions are usermode syscalls, not meant to be used from kernelmode.
 */
typedef struct UserFunctions
{
    ARKConfig* config;
    int (* IoOpen)(char *, int, int);
    int (* IoRead)(int, void *, int);
    int (* IoWrite)(int, void *, int);
    int (* IoClose)(int);
    int (* IoRemove)(char*);
    void (* KernelLibcTime)(int);
    void (* KernelLibcClock)();
    int (* KernelPowerLock)(unsigned int, unsigned int);
    void (* KernelDcacheWritebackAll)(void);
    void (* KernelIcacheInvalidateAll)(void);
    int (* DisplaySetFrameBuf)(void *topaddr, int bufferwidth, int pixelformat, int sync);
    SceUID (* KernelCreateThread)(const char *name, SceKernelThreadEntry entry, int initPriority, int stackSize, SceUInt attr, SceKernelThreadOptParam *option);
    int (* KernelStartThread)(SceUID thid, SceSize arglen, void *argp);
    void (* KernelDelayThread)(uint32_t);
    void (* KernelExitThread)(uint32_t);
    int (* KernelExitDeleteThread)(int status);
    int (* KernelWaitThreadEnd)(SceUID thid, SceUInt *timeout);
    SceUID (* KernelCreateVpl)(const char*, int, int, unsigned int, void*);
    int (* KernelTryAllocateVpl)(SceUID, unsigned int, void**);
    int (* KernelFreeVpl)(SceUID uid, void *data);
    int (* KernelDeleteVpl)(int);
    int (* KernelDeleteFpl)(int);
    unsigned int (*KernelCpuSuspendIntr)();
    void (*KernelCpuResumeIntr)(unsigned int flags);
    int (* UtilityLoadModule)(int);
    int (* UtilityUnloadModule)(int);
    int (* UtilityLoadNetModule)(int);
    int (* UtilityUnloadNetModule)(int);
    //int (* SysMemUserForUser_91DE343C)( void *unk );
    SceUID (*KernelAllocPartitionMemory)(SceUID partitionid, const char *name, int type, SceSize size, void *addr);
    void * (*KernelGetBlockHeadAddr)(SceUID blockid);
    int (* KernelFreePartitionMemory)(int);
    int (* UtilitySavedataGetStatus)();
    int (* UtilitySavedataInitStart)(void* params);
    void (* UtilitySavedataUpdate)(int a0);
    int (* UtilitySavedataShutdownStart)();
    int (* KernelVolatileMemUnlock)(int unknown);
    // common ark functions
    void (* freeMem)(struct UserFunctions*);
    u32 (* FindImportUserRam)(char *libname, u32 nid);
    u32 (* FindImportVolatileRam)(char *libname, u32 nid);
    u32 (* FindImportRange)(char *libname, u32 nid, u32 lower, u32 higher);
    void* (* RelocSyscall)(u32 call);
    int (* p5_open_savedata)(int mode);
    int (* p5_close_savedata)();
    u32 (* qwikTrick)(char* lib, u32 nid, u32 version);
    void (*prtstr)(const char* A, unsigned long B, unsigned long C, unsigned long D, unsigned long E, unsigned long F, unsigned long G, unsigned long H, unsigned long I, unsigned long J, unsigned long K, unsigned long L);
} UserFunctions;

// fills a UserFunctions instance with all available imports
extern void scanUserFunctions(UserFunctions* tbl);
extern void scanArkFunctions(UserFunctions* tbl);

// check if a pointer is in a range
extern int AddressInRange(u32 addr, u32 lower, u32 higher);

// find an import within a given range
extern u32 FindImportRange(char *libname, u32 nid, u32 lower, u32 higher);
// find an import within volatile memory
extern u32 FindImportVolatileRam(char *libname, u32 nid);
// find an import within user ram
extern u32 FindImportUserRam(char *libname, u32 nid);

/**
 * Given a library name, a nid and the version of the library, returns a pointer to the function identified by the nid
 * This is qwikrazor87's hack to obtain any import we want.
 *
 * param lib: the name of the library the function is in
 * param nid: the identifier of the function
 * param version: the version of the library
 * return u32: the raw pointer of the function
 *
 */
extern u32 qwikTrick(char* lib, u32 nid, u32 version);

// relocate a syscall
extern void *RelocSyscall(u32 call);
extern void* RelocImport(char* libname, u32 nid, int ram);

// algorithm to free all possible memory
extern void freeMem();

/*
 * These functions are ment for using when initial kernel access has been
 * granted.
 */
typedef struct KernelFunctions{
    // iofilemgr.prx Functions
    SceUID (* KernelIOOpen)(const char *, int, int);
    int (* KernelIOWrite)(SceUID, const void *, unsigned);
    int (* KernelIORead)(SceUID, void *, unsigned);
    int (* KernelIOLSeek)(int fd, s64 offset, int whence);
    int (* KernelIOClose)(SceUID);
    SceUID (* KernelIODopen)(char *);
    int (* KernelIODread)(SceUID, SceIoDirent *);
    int (* KernelIODclose)(SceUID);
    int (* KernelIOMkdir)(const char*, SceMode);
    int (* KernelIORmdir)(const char* path);
    int (* KernelIOGetStat)(const char *file, SceIoStat *stat);
    int (* KernelIORemove)(const char* file);
    int (* IoAssign)(const char *dev1, const char *dev2, const char *dev3, int mode, void *unk1, long unk2);
    int (* IoUnassign)(const char *dev);
    
    // sysmem.prx Functions
    SceUID     (*KernelAllocPartitionMemory)(SceUID partitionid, const char *name, int type, SceSize size, void *addr);
    void *     (*KernelGetBlockHeadAddr)(SceUID blockid);
    int (* KernelFreePartitionMemory)(int);
    void (* KernelIcacheInvalidateAll)(void);
    void (* KernelDcacheWritebackInvalidateAll)(void);
    int (* KernelGzipDecompress)(unsigned char *dest, unsigned int destSize, const unsigned char *src, void *unknown);
    void (* KernelDcacheInvalidateRange)(const void *p, unsigned int size);

    // loadcore.prx Functions
    SceModule2 * (* KernelFindModuleByName)(char *);

    // threadman.prx Functions
    SceUID (* KernelCreateThread)(const char *name, SceKernelThreadEntry entry,\
            int initPriority, int stackSize, SceUInt attr, SceKernelThreadOptParam *option);
    int (* KernelStartThread)(SceUID thid, SceSize arglen, void *argp);
    int (* KernelDelayThread)(int);
    int (*KernelDeleteThread)(int);
    int (*KernelExitThread)(int);
    void (*waitThreadEnd)(int, int*);
    
    // ARK functions
    u32 (* FindTextAddrByName)(const char *modulename);
    u32 (* FindFunction)(const char *module, const char *library, u32 nid);
    
}KernelFunctions;

extern UserFunctions* g_tbl;
extern KernelFunctions* k_tbl;
extern KxploitFunctions* kxf;

extern void scanKernelFunctions(KernelFunctions* kfuncs);

extern u32 FindTextAddrByName(const char *);
extern u32 FindFunction(const char *modulename, const char *library, u32 nid);

extern u32 _findJAL(u32 addr, int reversed, int skip);
#define findFirstJAL(addr) _findJAL(addr, 0, 0)
#define findFirstJALReverse(addr) _findJAL(addr, 1, 0)
#define findJAL(addr, pos) _findJAL(addr, 0, pos)
#define findJALReverse(addr, pos) _findJAL(addr, 1, pos)
#define findFirstJALForFunction(modname, libname, uid) findFirstJAL(FindFunction(modname, libname, uid))
#define findJALForFunction(modname, libname, uid, pos) findJAL(FindFunction(modname, libname, uid), pos)
#define findFirstJALReverseForFunction(modname, libname, uid) findFirstJALReverse(FindFunction(modname, libname, uid))
#define findJALReverseForFunction(modname, libname, uid, pos) findJALReverse(FindFunction(modname, libname, uid), pos)

u32 FindFirstBEQ(u32 addr);
extern u32 findRefInGlobals(char* libname, u32 addr, u32 ptr);

// search for (starting from given address) and replace the next instruction with the one given (optional skip parameter to skips first few)
extern u32 patchNextInstruction(u32 orig, u32 instr, u32 addr, int skip);

extern int p5_open_savedata(int mode);
extern int p5_close_savedata();

extern void flashPatch();

extern int isKernel();

extern void AccurateError(u32 text_addr, u32 text_size, u16 error);

#endif
