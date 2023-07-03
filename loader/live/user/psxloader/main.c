#include <pspsdk.h>
#include <pspdebug.h>
#include <pspiofilemgr.h>
#include <pspctrl.h>
#include <pspinit.h>
#include <pspdisplay.h>
#include <pspkernel.h>
#include <psputility.h>
#include <pspsuspend.h>
#include <psputilsforkernel.h>
#include <psppower.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>

#include "globals.h"
#include "functions.h"
#include "colordebugger.h"

PSP_MODULE_INFO("ARK VitaPOPS Loader", 0, 1, 0);

#define ARK_LOADADDR 0x08D30000
#define ARK_SIZE 0x8000

//#include "simple.h"

// ARK.BIN requires these imports
//int SysMemUserForUser_91DE343C(void* unk);
extern int sceKernelPowerLock(unsigned int, unsigned int);

volatile ARKConfig config = {
    .magic = ARK_CONFIG_MAGIC,
    .arkpath = DEFAULT_ARK_PATH, // only ms0 available anyways
    .launcher = ARK_XMENU, // use default (if needed)
    .exec_mode = PSV_POPS, // set to Vita Pops mode
    .exploit_id = "ePSX", // ps1 loader name
    .recovery = 0,
};
volatile UserFunctions funcs = {
    .config = &config,
    // File IO
    .IoOpen = &sceIoOpen,
    .IoRead = &sceIoRead,
    .IoWrite = &sceIoClose,
    .IoClose = &sceIoWrite,
    .IoRemove = &sceIoRemove,
    // System
    .KernelLibcTime = &sceKernelLibcTime,
    .KernelLibcClock = &sceKernelLibcClock,
    .KernelPowerLock = &sceKernelPowerLock,
    .KernelDcacheWritebackAll = &sceKernelDcacheWritebackAll,
    .KernelIcacheInvalidateAll = &sceKernelIcacheInvalidateAll,
    .DisplaySetFrameBuf = &sceDisplaySetFrameBuf,
    // Threads
    .KernelCreateThread = &sceKernelCreateThread,
    .KernelDelayThread = &sceKernelDelayThread,
    .KernelStartThread = &sceKernelStartThread,
    .KernelExitThread = &sceKernelExitThread,
    .KernelExitDeleteThread = &sceKernelExitDeleteThread,
    .KernelWaitThreadEnd = &sceKernelWaitThreadEnd,
    .KernelCreateVpl = &sceKernelCreateVpl,
    .KernelTryAllocateVpl = &sceKernelTryAllocateVpl,
    .KernelFreeVpl = &sceKernelFreeVpl,
    .KernelDeleteVpl = &sceKernelDeleteVpl,
    .KernelDeleteFpl = &sceKernelDeleteFpl,
    // Modules
    .UtilityLoadModule = &sceUtilityLoadModule,
    .UtilityUnloadModule = &sceUtilityUnloadModule,
    .UtilityLoadNetModule = &sceUtilityLoadNetModule,
    .UtilityUnloadNetModule = &sceUtilityUnloadNetModule,
    // Sysmem
    //.SysMemUserForUser_91DE343C = &SysMemUserForUser_91DE343C,
    .KernelAllocPartitionMemory = &sceKernelAllocPartitionMemory,
    .KernelFreePartitionMemory = &sceKernelFreePartitionMemory,
    .KernelGetBlockHeadAddr = &sceKernelGetBlockHeadAddr,
    // Intr
    .KernelCpuSuspendIntr = &sceKernelCpuSuspendIntr,
    .KernelCpuResumeIntr = &sceKernelCpuResumeIntr,
    .KernelVolatileMemUnlock = &sceKernelVolatileMemUnlock,
    // Savedata
    .KernelAllocPartitionMemory = &sceKernelAllocPartitionMemory,
};

int psxloader_thread(int argc, void* argv){

    //int modid = sceKernelLoadModuleBuffer(simple, size_simple, 0, NULL);
    //sceKernelStartModule(modid, 0, NULL, NULL, NULL);

    sceDisplaySetFrameBuf((void *)0x04000000, 512, PSP_DISPLAY_PIXEL_FORMAT_8888, 1);
    colorDebugSetIsVitaPops(1);

    char loadpath[ARK_PATH_SIZE];
    strcpy(loadpath, config.arkpath);
    strcat(loadpath, ARK4_BIN);

    SceUID fd = sceIoOpen(loadpath, PSP_O_RDONLY, 0);

    u32 color;
    if (fd < 0){
        color = 0xff;
    }
    else {
        color = 0xff00;
    }

    colorDebug(color);
    sceKernelExitGame();
    return 0;

    sceIoRead(fd, (void *)(ARK_LOADADDR), ARK_SIZE);
    sceIoClose(fd);
    sceKernelDcacheWritebackAll();

    // execute main function
    void (* hEntryPoint)(ARKConfig*, UserFunctions*, char*) = (void*)ARK_LOADADDR;
    hEntryPoint(&config, &funcs, NULL);

    return 0;
}

int module_start(SceSize args, void* argp)
{

    int thid = sceKernelCreateThread("psxloader", &psxloader_thread, 0x10, 0x10000, PSP_THREAD_ATTR_USER|PSP_THREAD_ATTR_VFPU, NULL);
    sceKernelStartThread(thid, 0, NULL);

    return 0;
}