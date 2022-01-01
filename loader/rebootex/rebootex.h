#ifndef REBOOTEX_H
#define REBOOTEX_H

#include "rebootconfig.h"

extern RebootConfigARK* reboot_conf;
extern ARKConfig* ark_config;

extern u32 reboot_start;
extern u32 reboot_end;

// sceReboot Main Function
extern int (* sceReboot)(int, int, int, int);

// Instruction Cache Invalidator
extern void (* sceRebootIcacheInvalidateAll)(void);

// Data Cache Invalidator
extern void (* sceRebootDacheWritebackInvalidateAll)(void);

// Sony PRX Decrypter Function Pointer
extern int (* SonyPRXDecrypt)(void *, unsigned int, unsigned int *);
extern int (* origCheckExecFile)(unsigned char * addr, void * arg2);

// LfatOpen on PS Vita
extern int (*pspemuLfatOpen)(char** filename, int unk);

// UnpackBootConfig on PSP
extern int (* UnpackBootConfig)(char * buffer, int length);
extern u32 UnpackBootConfigCall;
extern u32 UnpackBootConfigArg;

// Rebootex functions
void loadCoreModuleStartCommon();
void patchRebootBufferPSP();
void patchRebootBufferVita();
void patchRebootIoPSP();

#endif
