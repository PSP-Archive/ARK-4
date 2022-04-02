#include "vitamem.h"

int prevent_highmem(){
    int apitype = sceKernelInitApitype();
    return (apitype == 0x144 || apitype == 0x155 || apitype ==  0x210 || apitype ==  0x220);
}

void unprotectVitaMem(){
    // unprotect
    u32 *prot = (u32 *)0xBC000040;
    for (int i = 0; i < 0x10; i++)
        prot[i] = 0xFFFFFFFF;
}

// unprotect extra RAM for user apps
// call this from systemcontrol/vitacompat
void unlockVitaMemory(){

    _sw(0x44000000, 0xBC800100);

    if (prevent_highmem()) return;

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

    u32 user_size = 40 * 1024 * 1024;
    partition = GetPartition(PSP_MEMORY_PARTITION_USER);
    partition->size = user_size;
    partition->data->size = (((user_size >> 8) << 9) | 0xFC);

    partition = GetPartition(11);
    partition->size = 0;
    partition->address = 0x88800000 + user_size;
    partition->data->size = (((partition->size >> 8) << 9) | 0xFC);
    
    //reset partition length for next reboot
    sctrlHENSetMemory(24, 0);
    
}
