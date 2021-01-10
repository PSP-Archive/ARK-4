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

#include <pspsdk.h>
#include <macros.h>
#include <rebootconfig.h>
#include <systemctrl_se.h>
#include <functions.h>
#include "rc4.h"
#include "psid.h"
#include "rebootconfig.h"
#include "ansi_c_functions.h"
#include "rebootbuffer.h"

// Function Prototypes
int loadcoreModuleStart(unsigned int args, void * argp, int (* start)(SceSize, void *));
void flushCache(void);
int _sceReboot(int arg1, int arg2, int arg3, int arg4) __attribute__((section(".text.startup")));
int _pspemuLfatOpen(char **filename, int unk);

// more or less get the end of reboot.prx
u32 getRebootEnd(){
	u32 addr = REBOOT_TEXT;
	while (strcmp("ApplyPspRelSection", (char*)addr)) addr++;
	return (addr & -0x4);
}

void SetMemoryPartitionTablePatched(void *sysmem_config, SceSysmemPartTable *table)
{
	SetMemoryPartitionTable(sysmem_config, table);

	/* Add partition 11 */
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
			/* Patch to add new partition */
			_sw(MAKE_CALL(SetMemoryPartitionTablePatched), text_addr + i + 4);
		} else if ((_lw(text_addr + i) == 0x2405000C) && (_lw(text_addr + i + 8) == 0x00608821)) {
			/* Change attribute to 0xF (user accessable) */
			_sh(0xF, text_addr + i);
		} else if (_lw(text_addr + i) == 0x8C830014) {
			/* Patch to add new partition */
			SetMemoryPartitionTable = (void *)(text_addr + i);
			break;
		}
	}

	/* Patch to add new partition */
	//SetMemoryPartitionTable = (void *)text_addr + 0x11BD8;
	//_sw(MAKE_CALL(SetMemoryPartitionTablePatched), text_addr + 0x10F70);

	/* Change attribute to 0xF (user accessable) */
	//_sh(0xF, text_addr + 0x115F8);

	flushCache();

	return module_bootstart(4, sysmem_config);
}

// Entry Point
int _sceReboot(int arg1, int arg2, int arg3, int arg4)
{
	//12 MB extra ram through p11
	if (!IS_VITA_POPS){
		u32 i;
		for (i = 0; i < 0x4000; i += 4) {
			u32 addr = 0x88600000 + i;

			/* Patch call to SysMem module_bootstart */
			if (_lw(addr) == 0x24040004) {
				_sw(0x02402021, addr); //move $a0, $s2
				_sw(MAKE_CALL(PatchSysMem), addr + 0x64);
				continue;
			}
		}
	}

	// TODO Parse Reboot Buffer Configuration (what to configure yet? lol)

	u32 addr = REBOOT_TEXT;
	u32 reboot_end = getRebootEnd();
	for (; addr<reboot_end; addr+=4){
	
		if ((_lw(addr) & 0x0000FFFF) == 0x8B00){
			// Link Filesystem Buffer to 0x8BA00000
			_sb(0xA0, addr);
		}
		else if ((_lw(addr) == _lw(addr+4)) && (_lw(addr) & 0xFC000000) == 0xAC000000){
			// Patch ~PSP Check ???
			_sw(0xAFA50000, addr+4);
			// Returns size of the buffer on loading whatever modules
			_sw(0x20A30000, addr+8);
		}
		else if (_lw(addr) == 0x00600008){
			// Move LoadCore module_start Address into third Argument
			_sw(0x00603021, addr);
			// Hook LoadCore module_start Call
			_sw(JUMP(loadcoreModuleStart), addr+8);
		}
		else if (_lw(addr) == 0xBD01FFC0)
			_sceRebootDacheWritebackInvalidateAll  = (void*)addr-0x28;
		else if (_lw(addr) == 0xBD140000)
			_sceRebootIcacheInvalidateAll = (void*)addr-0x14;
		else if (_lw(addr) == 0x3A230001)
			pspemuLfatOpen = (void*)addr-0x40;
		else if (_lw(addr) == JAL(pspemuLfatOpen))
			// Hook _pspemuLfatOpen to switch pspbtcnf.bin
			_sw(JAL(_pspemuLfatOpen), addr);
			
	}
	
	// Flush Cache
	flushCache();
	
	// Forward Call
	return sceReboot(arg1, arg2, arg3, arg4);
}

void getPsidHash(unsigned char key[16])
{
	RebootBufferConfiguration * conf = (RebootBufferConfiguration*)REBOOTEX_CONFIG;

	memcpy(key, conf->psidHash, 16);
}

// PRO GZIP Decrypt Support
int PROPRXDecrypt(void * prx, unsigned int size, unsigned int * newsize)
{
	if(isPrxEncrypted(prx, size))
	{
		unsigned int compsize = *(unsigned int*)(prx + 0xB0);
		unsigned char key[16];
		void *rc4;

		getPsidHash(key);
		prxXorKeyMix(key, sizeof(key), key, prx+0x80);
		rc4 = rc4_initstate(key, sizeof(key));
		rc4_process_stream(rc4, prx+0x150, compsize);
		rc4_destroystate(rc4);
		memcpy(prx, prx+0x150, compsize);
		*newsize = compsize;

		return 0;
	}

	// GZIP Packed PRX File
	if(_lb((unsigned)prx + 0x150) == 0x1F && _lb((unsigned)prx + 0x151) == 0x8B)
	{
		// Read GZIP Size
		unsigned int compsize = *(unsigned int *)(prx + 0xB0);
		
		// Return GZIP Size
		*newsize = compsize;
		
		// Remove PRX Header
		memcpy(prx, prx + 0x150, compsize);
		
		// Fake Decrypt Success
		return 0;
	}
	
	// Decrypt Sony PRX Files
	return SonyPRXDecrypt(prx, size, newsize);
}

// Load Core module_start Hook
int loadcoreModuleStart(unsigned int args, void * argp, int (* start)(SceSize, void *))
{

	typedef struct LoadCoreBackup{
		u32 data;
		u32* addrs[2];
	}LoadCoreBackup;
	
	LoadCoreBackup* backup = (LoadCoreBackup*)0x08D20000;

	// Calculate Text Address
	unsigned int text_addr = FindTextAddrByName("sceLoaderCore");

	// Fetch Original Decrypt Function Stub
	SonyPRXDecrypt = (void *)FindImportRange("memlmd", 0xEF73E85B, text_addr, 0x88400000);
	backup->data = JAL(SonyPRXDecrypt);

	// Hook Signcheck Function Calls
	int count = 0;
	u32 addr;
	for (addr = text_addr; count<2; addr+=4){
		u32 data = _lw(addr);
		if (data == backup->data){
			_sw(JAL(PROPRXDecrypt), addr);
			backup->addrs[count] = (u32 *)addr;
			count++;
		}
	}
	
	// Flush Cache
	flushCache();
	
	// Forward
	return start(args, argp);
}

// Invalidate Instruction and Data Cache
void flushCache(void)
{
	// Invalidate Instruction Cache
	_sceRebootIcacheInvalidateAll();
	
	// Invalidate Data Cache
	_sceRebootDacheWritebackInvalidateAll();
}

int _pspemuLfatOpen(char **filename, int unk)
{
	if(filename != NULL && 0 == strcmp(*filename, "pspbtcnf.bin"))
	{
		RebootBufferConfiguration * conf = (RebootBufferConfiguration*)REBOOTEX_CONFIG;
		char *p = *filename;

		if (!IS_PSP)
			p[2] = 'v';

		if (IS_VITA_POPS)
			p[5] = 'x';
		else{
			switch(conf->iso_mode)
			{
				case MODE_MARCH33:
					/* March33 ISO Driver */
					p[5] = 'm';
					break;
				case MODE_UMD:
					/* Homebrew mode, use inferno */
					/* FALL THROUGH */
				case MODE_INFERNO:
					// pspbtinf.bin
					p[5] = 'i';
					break;
				case MODE_NP9660:
					// pspbtnnf.bin
					p[5] = 'n';
					break;
			}
		}
	}

	return pspemuLfatOpen(filename, unk);
}
