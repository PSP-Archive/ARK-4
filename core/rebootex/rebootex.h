#ifndef REBOOTEX_H
#define REBOOTEX_H

#include "rebootconfig.h"

// Original Sony functions
extern int (* sceReboot)(int, int, int, int);
extern void (* sceRebootIcacheInvalidateAll)(void);
extern void (* sceRebootDacheWritebackInvalidateAll)(void);
extern int (* SonyPRXDecrypt)(void *, unsigned int, unsigned int *);
extern int (* origCheckExecFile)(unsigned char * addr, void * arg2);
extern int (* UnpackBootConfig)(char * buffer, int length);
extern void* origLfatOpen;

void loadCoreModuleStartCommon();
void patchRebootBufferPSP(u32 reboot_start, u32 reboot_end);
void patchRebootBufferVita(u32 reboot_start, u32 reboot_end);

#endif
