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

static u32 findGetPartition(){
    int found = 0;
    for (u32 addr = SYSMEM_TEXT; !found; addr+=4){
        if (_lw(addr) == 0x2C85000D){
            return addr-4;
        }
    }
    return 0;
}

void unlockVitaMemory(u32 user_size_mib){

    int apitype = sceKernelInitApitype(); // prevent in pops and vsh
    if (apitype == 0x144 || apitype == 0x155 || apitype == 0x200 || apitype ==  0x210 || apitype ==  0x220 || apitype == 0x300)
        return;

    SysMemPartition *(* GetPartition)(int partition) = findGetPartition();
    if (!GetPartition){
        return;
    }

    u32 user_size = user_size_mib * 1024 * 1024; // new p2 size

    // modify p2
    SysMemPartition *partition = GetPartition(PSP_MEMORY_PARTITION_USER);
    partition->size = user_size;
    partition->data->size = (((user_size >> 8) << 9) | 0xFC);

    // modify p11
    SysMemPartition *partition = GetPartition(11);
    if (partition){
        partition->size = 0;
        partition->address = 0x88800000 + user_size;
		partition->data->size = (((partition->size >> 8) << 9) | 0xFC);
    }

    sctrlHENSetMemory(user_size_mib, 0);
}
