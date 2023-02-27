#ifndef REBOOTEX_H
#define REBOOTEX_H

#include "rebootconfig.h"

#define REBOOT_MODULE "/rtm.prx"

typedef struct {
	char *name;
	void *buffer;
	u32 size;
} BootFile;

typedef struct {
    u32 addr;
    u32 size;
} SceSysmemPartInfo;

typedef struct {
    u32 memSize;
    u32 unk4;
    u32 unk8;
    SceSysmemPartInfo other1; // 12
    SceSysmemPartInfo other2; // 20
    SceSysmemPartInfo vshell; // 28
    SceSysmemPartInfo scUser; // 36
    SceSysmemPartInfo meUser; // 44
    SceSysmemPartInfo extSc2Kernel; // 52
    SceSysmemPartInfo extScKernel; // 60
    SceSysmemPartInfo extMeKernel; // 68
    SceSysmemPartInfo extVshell; // 76
} SceSysmemPartTable;

extern RebootConfigARK* reboot_conf;
extern ARKConfig* ark_config;

extern u32 reboot_start;
extern u32 reboot_end;

// sceReboot Main Function
extern int (* sceReboot)(int, int, int, int, int, int, int);

// Instruction Cache Invalidator
extern void (* sceRebootIcacheInvalidateAll)(void);

// Data Cache Invalidator
extern void (* sceRebootDacheWritebackInvalidateAll)(void);

// Sony PRX Decrypter Function Pointer
extern int (* SonyPRXDecrypt)(void *, unsigned int, unsigned int *);
extern int (* origCheckExecFile)(unsigned char * addr, void * arg2);

// UnpackBootConfig on PSP
extern int (* UnpackBootConfig)(char * buffer, int length);
extern u32 UnpackBootConfigCall;
extern u32 UnpackBootConfigArg;

// Rebootex functions
u32 loadCoreModuleStartCommon(u32 entry);
void patchRebootBufferPSP();
#ifdef REBOOTEX
void patchRebootBufferVita();
#endif
void patchRebootIoPSP();

#endif
