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
    u32 top_addr = text_addr+0x14000;
    int patches = 3;
    for (u32 addr = text_addr; addr<top_addr && patches; addr += 4) {
        u32 data = _lw(addr);
        if (data == 0x8C860010 && (_lw(addr + 8) == 0xAE060008)) {
            // Patch to add new partition
            _sw(JAL(SetMemoryPartitionTablePatched), addr + 4);
            patches--;
        } else if (data == 0x2405000C && (_lw(addr + 8) == 0x00608821)) {
            // Change attribute to 0xF (user accessable)
            _sh(0xF, addr);
            patches--;
        } else if (data == 0x8C830014) {
            // Patch to add new partition
            SetMemoryPartitionTable = (void *)addr;
            patches--;
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
// call this from systemcontrol/vitacompat
void unlockVitaMemory(){
    // unprotect
    u32 *prot = (u32 *)0xBC000040;
    for (int i = 0; i < 0x10; i++)
        prot[i] = 0xFFFFFFFF;
    
    // apply partition info
    SysMemPartition *(* GetPartition)(int partition) = NULL;
	SysMemPartition *partition;
	u32 user_size;

	u32 i;
	for (i = 0; i < 0x4000; i += 4) {
		u32 addr = 0x88000000 + i;
		if (_lw(addr) == 0x2C85000D) {
			GetPartition = (void *)(addr - 4);
			break;
		}
	}

	if (!GetPartition)
		return;

	user_size = 36;
	partition = GetPartition(PSP_MEMORY_PARTITION_USER);
	partition->size = user_size;
	partition->data->size = (((user_size >> 8) << 9) | 0xFC);

	partition = GetPartition(11);
	partition->size = 0;
	partition->address = 0x88800000 + user_size;
	partition->data->size = (((partition->size >> 8) << 9) | 0xFC);
    
}
