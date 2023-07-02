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

PSP_MODULE_INFO("ARK PS1 Loader", 0, 1, 0);

#define ARK_LOADADDR 0x08D30000
#define ARK_SIZE 0x8000

// ARK.BIN requires these imports
//int SysMemUserForUser_91DE343C(void* unk);
extern int sceKernelPowerLock(unsigned int, unsigned int);

volatile ARKConfig config = {
    .magic = ARK_CONFIG_MAGIC,
    .arkpath = DEFAULT_ARK_PATH, // only ms0 available anyways
    .launcher = {0}, // use default (if needed)
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

// PSP to PSX Color Conversion
static u16 RGBA8888_to_RGBA5551(u32 color)
{
    int r, g, b, a;
    a = (color >> 24) ? 0x8000 : 0;
    b = (color >> 19) & 0x1F;
    g = (color >> 11) & 0x1F;
    r = (color >> 3) & 0x1F;
    return a | r | (g << 5) | (b << 10);
}

static u32 GetPopsVramAddr(u32 framebuffer, int x, int y)
{
    return framebuffer + x * 2 + y * 640 * 4;
}

static u32 GetPspVramAddr(u32 framebuffer, int x, int y)
{
    return framebuffer + x * 4 + y * 512 * 4;
}

// Copy PSP VRAM to PSX VRAM
void SoftRelocateVram(u32* psp_vram, u16* ps1_vram)
{
    if(psp_vram)
    {
        int y;
        for(y = 0; y < 272; y++)
        {
            int x;
            for(x = 0; x < 480; x++)
            {
                u32 color = *(u32 *)GetPspVramAddr((u32)psp_vram, x, y);
                *(u16 *)GetPopsVramAddr(ps1_vram, x, y) = RGBA8888_to_RGBA5551(color);
            }
        }
    }
}

int module_start(SceSize args, void* argp)
{

    char loadpath[ARK_PATH_SIZE];
    strcpy(loadpath, config.arkpath);
    strcat(loadpath, ARK4_BIN);

    SceUID fd = sceIoOpen(loadpath, PSP_O_RDONLY, 0);

    if (fd < 0){
        while (1){
            colorDebug(0xff);
            SoftRelocateVram((u32*)0x44000000, (u16*)0x490C0000);
        }
    }
    else {
        while (1){
            colorDebug(0xff00);
            SoftRelocateVram((u32*)0x44000000, (u16*)0x490C0000);
        }
    }

    sceIoRead(fd, (void *)(ARK_LOADADDR), ARK_SIZE);
    sceIoClose(fd);
    sceKernelDcacheWritebackAll();

    // execute main function
    void (* hEntryPoint)(ARKConfig*, UserFunctions*, char*) = (void*)ARK_LOADADDR;
    hEntryPoint(&config, &funcs, NULL);

}