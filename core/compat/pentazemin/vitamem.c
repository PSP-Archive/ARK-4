#include <pspsdk.h>
#include <pspsysmem_kernel.h>
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <pspsysevent.h>
#include <pspiofilemgr.h>
#include <stdio.h>
#include <string.h>
#include <ark.h>
#include <systemctrl.h>
#include "sysmem.h"

void unlockVitaMemory(u32 user_size_mib){

    int apitype = sceKernelInitApitype(); // prevent in pops and vsh
    if (apitype == 0x144 || apitype == 0x155 || apitype == 0x200 || apitype ==  0x210 || apitype ==  0x220 || apitype == 0x300)
        return;

    // apply partition info
    SysMemPartition *(* GetPartition)(int partition) = NULL;
    SysMemPartition *partition;

    u32 i;
    for (i = 0; i < 0x4000; i += 4) {
        u32 addr = 0x88000000 + i;
        if (_lw(addr) == 0x2C85000D) {
            GetPartition = (void *)(addr - 4);
            break;
        }
    }

    if (!GetPartition){
        return;
    }


    u32 kernel_size = 0; // EXTRA_RAM_SIZE - extra_user_ram; // p11 size
    u32 user_size = user_size_mib * 1024 * 1024; // new p2 size

    // modify p2
    partition = GetPartition(PSP_MEMORY_PARTITION_USER);
    partition->size = user_size;
    partition->data->size = (((user_size >> 8) << 9) | 0xFC);

    // modify p11
    partition = GetPartition(11);
    partition->size = kernel_size;
    partition->address = 0x88800000 + user_size;
    partition->data->size = (((kernel_size >> 8) << 9) | 0xFC);

    sctrlHENSetMemory(user_size_mib, 0);
}
