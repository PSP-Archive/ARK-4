#include "vita_mem.h"

void (* SetMemoryPartitionTable)(void *sysmem_config, SceSysmemPartTable *table);

void SetMemoryPartitionTablePatched(void *sysmem_config, SceSysmemPartTable *table)
{
	SetMemoryPartitionTable(sysmem_config, table);

	// Add partition 11
	table->extVshell.addr = 0x0B000000;
	table->extVshell.size = 12 * 1024 * 1024;
}

int PatchSysMem(void *a0, void *sysmem_config)
{
	int (* module_bootstart)(SceSize args, void *sysmem_config) = (void *)_lw((u32)a0 + 0x28);

	u32 text_addr = 0x88000000;
	int i;

	for (i = 0; i < 0x12000; i += 4) {
		if ((_lw(text_addr + i) == 0x8C860010) && (_lw(text_addr + i + 8) == 0xAE060008)) {
			// Patch to add new partition
			_sw(MAKE_CALL(SetMemoryPartitionTablePatched), text_addr + i + 4);
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
	//_sw(MAKE_CALL(SetMemoryPartitionTablePatched), text_addr + 0x10F70);

	// Change attribute to 0xF (user accessable)
	//_sh(0xF, text_addr + 0x115F8);

	flushCache();

	return module_bootstart(4, sysmem_config);
}

void patchVitaMem(){
	//12 MB extra ram through p11 on Vita (not available in higher firmwares anymore?)
	if (IS_VITA && !IS_VITA_POPS){
		u32 i;
		for (i = 0; i < 0x4000; i += 4) {
			u32 addr = 0x88600000 + i;

			// Patch call to SysMem module_bootstart
			if (_lw(addr) == 0x24040004) {
				_sw(0x02402021, addr); //move $a0, $s2
				_sw(MAKE_CALL(PatchSysMem), addr + 0x64);
				continue;
			}
		}
	}
}
