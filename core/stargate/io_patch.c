#include <pspsdk.h>
#include <pspkernel.h>
#include <pspsysmem_kernel.h>
#include <pspthreadman_kernel.h>
#include <pspdebug.h>
#include <pspinit.h>
#include <string.h>
#include <stdio.h>
#include <systemctrl.h>
#include <systemctrl_private.h>
#include <macros.h>
#include <globals.h>
#include <functions.h>

struct DeviceSize {
	u32 maxClusters;
	u32 freeClusters;
	u32 maxSectors;
	u32 sectorSize;
	u32 sectorCount;
};

// this fixes old games that report "not enough space" when more than 2GB are available
static u32 _sceIoDevctl(const char *name, int cmd, u32 argAddr, int argLen, u32 outPtr, int outLen){
    u32 res = sceIoDevctl(name, cmd, argAddr, argLen, outPtr, outLen);

    if (cmd == 0x02425818){

        struct DeviceSize* deviceSize = *(struct DeviceSize**)argAddr;

        u32 sectorSize = deviceSize->sectorSize;
        u32 memStickSectorSize = 32 * 1024;
        u32 sectorCount = memStickSectorSize / sectorSize;
        u32 freeSize = 1 * 1024 * 1024 * 1024; // pretend to have 1GB, enough for any game
        u32 maxClusters = (u32)(freeSize / (sectorSize * sectorCount));

        if (deviceSize->freeClusters > maxClusters){
            deviceSize->maxClusters = maxClusters;
            deviceSize->freeClusters = maxClusters;
            deviceSize->maxSectors = maxClusters;
            deviceSize->sectorCount = sectorCount;
        }
    }

    return res;
}

void patch_ioDevCtl(){
    sctrlHENPatchSyscall(sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0x54F5FB11), _sceIoDevctl);
}