#include "vitamem.h"

void unprotectVitaMem(){
    // unprotect from user access
    u32 *prot = (u32 *)0xBC000040;
    for (int i = 0; i < 0x10; i++)
        prot[i] = 0xFFFFFFFF;
}

// use flash0 RAM for user apps
void unlockVitaMemory(){

    int apitype = sceKernelInitApitype(); // prevent in pops and vsh(?)
    if (apitype == 0x144 || apitype == 0x155 || apitype ==  0x210 || apitype ==  0x220) return;

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


    u32 user_size = USER_SIZE + VITA_FLASH_SIZE;
    partition = GetPartition(PSP_MEMORY_PARTITION_USER);
    partition->size = user_size;
    partition->data->size = (((user_size >> 8) << 9) | 0xFC);

    partition = GetPartition(11);
    partition->size = 0;
    partition->address = 0x88800000 + user_size;
    partition->data->size = 0xFC;
    
    sctrlHENSetMemory(40, 0);
}
