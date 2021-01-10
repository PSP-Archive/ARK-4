#include <pspsdk.h>
#include <pspdebug.h>
#include <pspiofilemgr.h>
#include <pspctrl.h>
#include <pspinit.h>
#include <pspdisplay.h>
#include <pspkernel.h>
#include <psputility.h>
#include <pspsuspend.h>
#include <psppower.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>

#include "globals.h"
#include "functions.h"
#include "graphics.h"

PSP_MODULE_INFO("ARK Loader", PSP_MODULE_USER, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER | PSP_THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(4096);

#define ARK_LOADADDR 0x08D20000
#define ARK_SIZE 0x8000
#define ARK_BIN "ARK.BIN"
#define ARK_MENU "VBOOT.PBP"
#define INSTALL_PATH "ms0:/PSP/SAVEDATA/ARK_01234/"
#define LOADPATH (INSTALL_PATH ARK_BIN)

// ARK's linkless loader must be able to find this string
//static const char* loadpath = INSTALL_PATH ARK_BIN;

// ARK.BIN requires these imports
int SysMemUserForUser_91DE343C(void* unk);
int sceKernelPowerLock(unsigned int, unsigned int);

volatile ARKConfig config = {
    .arkpath = INSTALL_PATH,
    .menupath = (INSTALL_PATH ARK_MENU),
    .exec_mode = REAL_PSP,
    .exploit_id = "PSP_LIVE",
};
volatile FunctionTable funcs = {
    .config = &config,
    // File IO
	.IoOpen = &sceIoOpen,
	.IoRead = &sceIoRead,
	.IoWrite = &sceIoClose,
	.IoClose = &sceIoWrite,
    // System
	.KernelLibcTime = &sceKernelLibcTime,
	.KernelPowerLock = &sceKernelPowerLock,
	.KernelDcacheWritebackAll = &sceKernelDcacheWritebackAll,
	.KernelIcacheInvalidateAll = &sceKernelIcacheInvalidateAll,
	.DisplaySetFrameBuf = &sceDisplaySetFrameBuf,
    // Threads
	.KernelCreateThread = &sceKernelCreateThread,
	.KernelDelayThread = &sceKernelDelayThread,
	.KernelStartThread = &sceKernelStartThread,
	.KernelExitThread = &sceKernelExitThread,
	.KernelWaitThreadEnd = &sceKernelWaitThreadEnd,
	.KernelDeleteVpl = &sceKernelDeleteVpl,
	.KernelDeleteFpl = &sceKernelDeleteFpl,
    // Modules
	.UtilityLoadModule = &sceUtilityLoadModule,
	.UtilityUnloadModule = &sceUtilityUnloadModule,
	.UtilityLoadNetModule = &sceUtilityLoadNetModule,
	.UtilityUnloadNetModule = &sceUtilityUnloadNetModule,
    // Sysmem
	.SysMemUserForUser_91DE343C = &SysMemUserForUser_91DE343C,
	.KernelFreePartitionMemory = &sceKernelFreePartitionMemory,
    // Intr
	.KernelCpuSuspendIntr = &sceKernelCpuSuspendIntr,
	.KernelCpuResumeIntr = &sceKernelCpuResumeIntr,
	.KernelVolatileMemUnlock = &sceKernelVolatileMemUnlock,
    // Savedata
	.UtilitySavedataGetStatus = &sceUtilitySavedataGetStatus,
	.UtilitySavedataInitStart = &sceUtilitySavedataInitStart,
	.UtilitySavedataUpdate = &sceUtilitySavedataUpdate,
	.UtilitySavedataShutdownStart = &sceUtilitySavedataShutdownStart,
};

int main(){

    initScreen(funcs.DisplaySetFrameBuf);
    
    PRTSTR("Stage 1 Starting");
	SceUID fd = sceIoOpen(LOADPATH, PSP_O_RDONLY, 0);
	sceIoRead(fd, (void *)(ARK_LOADADDR), ARK_SIZE);
	sceIoClose(fd);
	sceKernelDcacheWritebackAll();

	PRTSTR("Executing ARK");
	void (* hEntryPoint)(ARKConfig*, FunctionTable*) = (void*)ARK_LOADADDR;
	hEntryPoint(&config, &funcs);
	
	return 0;
}
