/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

#include <globals.h>

// sceReboot Main Function
int (* sceReboot)(int, int, int, int) = (void *)(REBOOT_TEXT);

// Instruction Cache Invalidator
void (* _sceRebootIcacheInvalidateAll)(void) = NULL;

// Data Cache Invalidator
void (* _sceRebootDacheWritebackInvalidateAll)(void) = NULL;

// Sony PRX Decrypter Function Pointer
int (* SonyPRXDecrypt)(void *, unsigned int, unsigned int *) = NULL;

int (*pspemuLfatOpen)(char **filename, int unk) = NULL;

typedef struct {
	u32 addr;
	u32 size;
} SceSysmemPartInfo;

typedef struct {
	u32 memSize;
	u32 unk4;
	u32 unk8;
	SceSysmemPartInfo other1; // 12
	SceSysmemPartInfo other2; // 20
	SceSysmemPartInfo vshell; // 28
	SceSysmemPartInfo scUser; // 36
	SceSysmemPartInfo meUser; // 44
	SceSysmemPartInfo extSc2Kernel; // 52
	SceSysmemPartInfo extScKernel; // 60
	SceSysmemPartInfo extMeKernel; // 68
	SceSysmemPartInfo extVshell; // 76
} SceSysmemPartTable;

void (* SetMemoryPartitionTable)(void *sysmem_config, SceSysmemPartTable *table);
