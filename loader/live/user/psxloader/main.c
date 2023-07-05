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
#include "graphics.h"

PSP_MODULE_INFO("ARK VitaPOPS Loader", 0, 1, 0);

#define ARK_LOADADDR 0x08D30000
#define ARK_SIZE 0x8000

// ARK.BIN requires these imports
//int SysMemUserForUser_91DE343C(void* unk);
extern int sceKernelPowerLock(unsigned int, unsigned int);

volatile void* KernelExitGame = &sceKernelExitGame;
volatile void* ReadController = &sceCtrlReadBufferPositive;

volatile ARKConfig config = {
    .magic = ARK_CONFIG_MAGIC,
    .arkpath = DEFAULT_ARK_PATH, // only ms0 available anyways
    .launcher = ARK_XMENU, // use xMenu
    .exec_mode = PSV_POPS, // set to VitaPops mode
    .exploit_id = "ePSX", // ps1 loader name
    .recovery = 0,
};
volatile UserFunctions funcs = {
    .config = &config,
    // File IO
    .IoOpen = &sceIoOpen,
    .IoRead = &sceIoRead,
    .IoWrite = &sceIoWrite,
    .IoClose = &sceIoClose,
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
};

int psxloader_thread(int argc, void* argv){

    // wait for system to finish booting
    sceKernelDelayThread(1000000);

    sceDisplaySetFrameBuf((void *)0x04000000, 512, PSP_DISPLAY_PIXEL_FORMAT_8888, 1);
    colorDebugSetIsVitaPops(1);

    SceUID fd = sceIoOpen("ms0:/PSP/SAVEDATA/ARK_01234/ARK4.BIN", PSP_O_RDONLY, 0);

    if (fd < 0){
        setScreenHandler(&copyPSPVram);
        initScreen(NULL);
        PRTSTR1("ERROR: %p", fd);
        sceKernelExitGame();
        return 0;
    }

    sceIoRead(fd, (void *)(ARK_LOADADDR), ARK_SIZE);
    sceIoClose(fd);
    sceKernelDcacheWritebackAll();

    // execute main function
    void (* hEntryPoint)(ARKConfig*, UserFunctions*, char*) = (void*)ARK_LOADADDR;
    hEntryPoint(&config, &funcs, "ms0:/PSP/GAME/SCPS10084/K.BIN");

    return 0;
}

int module_start(SceSize args, void* argp)
{

    sceKernelAllocPartitionMemory(PSP_MEMORY_PARTITION_USER, "ARK4.BIN", PSP_SMEM_Addr, ARK_SIZE, ARK_LOADADDR);
    sceKernelAllocPartitionMemory(PSP_MEMORY_PARTITION_USER, "K.BIN", PSP_SMEM_Addr, 0x4000, 0x08D20000);

    int thid = sceKernelCreateThread("psxloader", &psxloader_thread, 0x10, 0x20000, PSP_THREAD_ATTR_USER|PSP_THREAD_ATTR_VFPU, NULL);
    sceKernelStartThread(thid, 0, NULL);

    return 0;
}