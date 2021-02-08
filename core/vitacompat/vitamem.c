#include "vitamem.h"

//12 MB extra ram through p11 on Vita
void SetMemoryPartitionTablePatched(void *sysmem_config, SceSysmemPartTable *table)
{
    // Add partition 11
    SetMemoryPartitionTable(sysmem_config, table);
    table->extVshell.addr = 0x0B000000;
    table->extVshell.size = 12 * 1024 * 1024;
}
// call this from rebootex
int PatchSysMem(void *a0, void *sysmem_config)
{
    int (* module_bootstart)(SceSize args, void *sysmem_config) = (void *)_lw((u32)a0 + 0x28);

    u32 text_addr = 0x88000000;
    int i;

    for (i = 0; i < 0x12000; i += 4) {
        if ((_lw(text_addr + i) == 0x8C860010) && (_lw(text_addr + i + 8) == 0xAE060008)) {
            // Patch to add new partition
            _sw(JAL(SetMemoryPartitionTablePatched), text_addr + i + 4);
        } else if ((_lw(text_addr + i) == 0x2405000C) && (_lw(text_addr + i + 8) == 0x00608821)) {
            // Change attribute to 0xF (user accessable)
            _sh(0xF, text_addr + i);
        } else if (_lw(text_addr + i) == 0x8C830014) {
            // Patch to add new partition
            SetMemoryPartitionTable = (void *)(text_addr + i);
            break;
        }
    }

    // Patch to add new partition
    //SetMemoryPartitionTable = (void *)text_addr + 0x11BD8;
    //_sw(JAL(SetMemoryPartitionTablePatched), text_addr + 0x10F70);

    // Change attribute to 0xF (user accessable)
    //_sh(0xF, text_addr + 0x115F8);

    flushCache();

    return module_bootstart(4, sysmem_config);
}


// unprotect extra RAM for user apps
// call this from systemcontrol
void unprotectVitaMemory(){
    u32 *prot = (u32 *)0xBC000040;
    for (int i = 0; i < 0x10; i++)
        prot[i] = 0xFFFFFFFF;
}

