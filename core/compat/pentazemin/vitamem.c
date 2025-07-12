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
#include <systemctrl_se.h>
#include "sysmem.h"

extern SEConfig* se_config;

static u32 findGetPartition(){
    for (u32 addr = SYSMEM_TEXT; ; addr+=4){
        if (_lw(addr) == 0x2C85000D){
            return addr-4;
        }
    }
    return 0;
}

int unlockVitaMemory(u32 user_size_mib){

    int apitype = sceKernelInitApitype(); // prevent in pops and vsh
    if (apitype == 0x144 || apitype == 0x155 || apitype >= 0x200)
        return -1;

    SysMemPartition *(* GetPartition)(int partition) = findGetPartition();

    SysMemPartition *partition;
    u32 user_size = user_size_mib * 1024 * 1024; // new p2 size

    // modify p2
    partition = GetPartition(PSP_MEMORY_PARTITION_USER);
    partition->size = user_size;
    partition->data->size = (((user_size >> 8) << 9) | 0xFC);

    // modify p11
    for (int i=8; i<12; i++){
        partition = GetPartition(i);
        if (partition){
            partition->size = 0;
            partition->address = 0x88800000 + user_size;
            partition->data->size = (((partition->size >> 8) << 9) | 0xFC);
        }
    }

    // prevent tampering with pspemu addresses, improves stability
    sceKernelAllocPartitionMemory(2, "SCE_PSPEMU_FLASHFS", PSP_SMEM_Addr, 0x200000, (void*)0x0B000000);

    return 0;
}

int (*_sctrlHENApplyMemory)(u32) = NULL;
int memoryHandlerVita(u32 p2){
    // sanity checks
    if (p2<=24) return -1;

    // the first 16MB are stable and good enough for most use cases
    // but homebrew that require extra ram will be allowed to use (some of) the upper 16MB
    int max_vita_mem = (se_config->force_high_memory == 2)? 52 : 40;

    // call orig function to determine if can unlock
    int res = _sctrlHENApplyMemory(max_vita_mem);
    if (res<0) return res;

    // unlock
    res = unlockVitaMemory(max_vita_mem);
    
    // unlock fail? revert back to 24MB
    if (res<0) _sctrlHENApplyMemory(24);

    return res;
}
