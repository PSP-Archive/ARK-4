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

#include "rebootconfig.h"

// sceReboot Main Function
int (* sceReboot)(int, int, int, int) = (void *)(REBOOT_TEXT);

// Instruction Cache Invalidator
void (* sceRebootIcacheInvalidateAll)(void) = NULL;

// Data Cache Invalidator
void (* sceRebootDacheWritebackInvalidateAll)(void) = NULL;

// Sony PRX Decrypter Function Pointer
int (* SonyPRXDecrypt)(void *, unsigned int, unsigned int *) = NULL;
int (* origCheckExecFile)(unsigned char * addr, void * arg2) = NULL;

// LfatOpen
void* origLfatOpen = NULL;
int (*pspemuLfatOpen)(char **filename, int unk) = NULL;
int (*pspLfatOpen)(char*) = NULL;

// UnpackBootConfig
int (* UnpackBootConfig)(char * buffer, int length) = NULL;

//
ARKConfig* ark_conf = ark_conf_backup;

// more or less get the end of reboot.prx
u32 getRebootEnd(){
	u32 addr = REBOOT_TEXT;
	while (strcmp("ApplyPspRelSection", (char*)addr)) addr++;
	return (addr & -0x4);
}

// PRO GZIP Decrypt Support
int PROPRXDecrypt(void * prx, unsigned int size, unsigned int * newsize)
{

	// GZIP Packed PRX File
	if ( (_lb((unsigned)prx + 0x150) == 0x1F && _lb((unsigned)prx + 0x151) == 0x8B)
	        || (*(unsigned int *)(prx + 0x130) == 0xC01DB15D) )
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


int _sceKernelCheckExecFile(unsigned char * addr, void * arg2)
{
	//scan structure
	//6.31 kernel modules use type 3 PRX... 0xd4~0x10C is zero padded
	int pos = 0; for(; pos < 0x38; pos++)
	{
		//nonzero byte encountered
		if(addr[pos + 212])
		{
			//forward to unsign function?
			return origCheckExecFile(addr, arg2);
		}
	}

	//return success
	return 0;
}

static void loadCoreModuleStartCommon(int is_psp){

    // Calculate Text Address
	unsigned int text_addr = FindTextAddrByName("sceLoaderCore");
	// Fetch Original Decrypt Function Stub
	SonyPRXDecrypt = (void *)FindImportRange("memlmd", 0xEF73E85B, text_addr, 0x88400000);
	origCheckExecFile = (void *)FindImportRange("memlmd", 0x6192F715, text_addr, 0x88400000);

    // save this configuration to restore loadcore later on
    RebootexFunctions* rex_funcs = REBOOTEX_FUNCTIONS;
    rex_funcs->rebootex_decrypt = &PROPRXDecrypt;
    rex_funcs->rebootex_checkexec = &_sceKernelCheckExecFile;
    rex_funcs->orig_decrypt = SonyPRXDecrypt;
    rex_funcs->orig_checkexec = origCheckExecFile;

	u32 decrypt_call = JAL(SonyPRXDecrypt);
	u32 check_call = JAL(origCheckExecFile);

	// Hook Signcheck Function Calls
	int count = 0;
	u32 addr;
    u32 top_addr = text_addr+0x8000; // read 32KB at most (more than enough to scan loadcore)
	for (addr = text_addr; addr<top_addr; addr+=4){
		u32 data = _lw(addr);
		if (data == decrypt_call){
			_sw(JAL(PROPRXDecrypt), addr);
		}
		else if (data == check_call && is_psp){
		    _sw(JAL(_sceKernelCheckExecFile), addr);
		}
	}
}

int loadcoreModuleStartPSP(void * arg1, void * arg2, void * arg3, int (* start)(void *, void *, void *)){
    loadCoreModuleStartCommon(1);
    flushCache();
    return start(arg1, arg2, arg3);
}

// Load Core module_start Hook
int loadcoreModuleStartVita(unsigned int args, void * argp, int (* start)(SceSize, void *))
{
	loadCoreModuleStartCommon(0);
	flushCache();
	return start(args, argp);
}

// Invalidate Instruction and Data Cache
void flushCache(void)
{
	// Invalidate Data Cache
	sceRebootDacheWritebackInvalidateAll();
	// Invalidate Instruction Cache
	sceRebootIcacheInvalidateAll();
}

int _pspemuLfatOpen(char **filename, int unk)
{
    if (filename != NULL && 0 == strcmp(*filename, "pspbtcnf.bin")){
		RebootBufferConfiguration * conf = (RebootBufferConfiguration*)REBOOTEX_CONFIG;
		char *p = filename;

		p[2] = 'v'; // custom btcnf for PS Vita

		if (IS_VITA_POPS(ark_conf->exec_mode)){
			p[5] = 'x'; // Vita PSX emulator uses custom setup for POPS
		}
		else{
			switch(conf->iso_mode)
			{
				case MODE_UMD:
				case MODE_NP9660:
					// psvbtnnf.bin
					p[5] = 'n';
					break;
			    case MODE_MARCH33:
			    case MODE_INFERNO:
			    default:
					// psvbtinf.bin
					p[5] = 'i';
					break;
			}
		}
    }
	return pspemuLfatOpen(filename, unk);
}

int _pspLfatOpen(char* filename){
    /*
    if (filename != NULL && strncmp(filename, "/kd/pspbtcnf", sizeof("/kd/pspbtcnf")-2)==0){
        filename[9] = 'i';
    }
    */
    return pspLfatOpen(filename);
}

/*
int _UnpackBootConfig(char **p_buffer, int length)
{
	int res = UnpackBootConfig(*p_buffer, length);
	
	if (res<=0){
        return length;
	}
	
	return res;
}
*/

void findRebootFunctions(u32 reboot_start, u32 reboot_end){

    register void (* Icache)(void) = NULL;
    register void (* Dcache)(void) = NULL;
	int wanted = 4; //(IS_PSP(ark_conf->exec_mode))?3:4; // lfatopen not needed for psp
	// find functions
	for (u32 addr = reboot_start; addr<reboot_end && wanted; addr+=4){
	    u32 data = _lw(addr);
	    if (data == 0xBD01FFC0){ // works on PSP and Vita
	        u32 a = addr;
            do {a-=4;} while (_lw(a) != 0x40088000);
			Dcache = (void*)a;
			wanted--;
		}
		else if (data == 0xBD14FFC0){ // works on PSP and Vita
			u32 a = addr;
            do {a-=4;} while (_lw(a) != 0x40088000);
            Icache = (void*)a;
            wanted--;
	    }
	    else if (data == 0x3A230001 || data == 0x30C30002 /*&& IS_VITA(ark_conf->exec_mode)*/ ){ // only appears inside lFatOpen
	        u32 a = addr;
            do {a-=4;} while (_lw(a) != 0x27BDFFF0);
            origLfatOpen = (void*)a;
            wanted--;
            /*
            colorDebug(0xff);
            _sw(0x44000000, 0xBC800100);
            while(1){};
            */
	    }
	    else if (data == 0x3466507E && IS_PSP(ark_conf->exec_mode)){
	        UnpackBootConfig = addr-12;
	        wanted--;
	    }
	}
	
	origLfatOpen = reboot_start+0x0000B6C0;
	
	sceRebootIcacheInvalidateAll = Icache;
	sceRebootDacheWritebackInvalidateAll = Dcache;
	Icache();
	Dcache();
	
}

// patch reboot
void patchRebootBufferPSP(u32 reboot_start, u32 reboot_end){
    // due to cache inconsistency, we must do these three patches first and also make sure we read as little ram as possible
	u32 UnpackBootConfigCall = JAL(UnpackBootConfig);
	pspLfatOpen = origLfatOpen;
	u32 lFatOpenCall = JAL(pspLfatOpen);
	int patches = 3;
	for (u32 addr = (u32)UnpackBootConfig; addr<reboot_end && patches; addr+=4){
	    u32 data = _lw(addr);
	    if (data == UnpackBootConfigCall){
            _sw(JAL(_UnpackBootConfig), addr); // Hook UnpackBootConfig
            patches--;
        }
        else if (data == 0x8FA50008){ // UnpackBootConfigBufferAddress
            _sw(0x27A40004, addr+8); // addiu $a0, $sp, 4
            patches--;
        }
        else if (data == 0x24D90001){  // rebootexcheck5
            u32 a = addr;
            do {a-=4;} while (_lw(a) != NOP);
            _sw(NOP, a-4); // Killing Branch Check bltz ...
            patches--;
        }
	}
	patches = 6;
	for (u32 addr = reboot_start; addr<reboot_end && patches; addr+=4){
	    u32 data = _lw(addr);
	    if (data == 0x02A0E821){ // found loadcore jump on PSP
            _sw(0x00113821, addr-4); // move a3, s1
            _sw(JUMP(loadcoreModuleStartPSP), addr);
            _sw(0x02A0E821, addr + 4);
            patches--;
        }
        else if (data == 0x2C860040){ // kdebug patch
            _sw(0x03E00008, addr-4); // make it return 1
        	_sw(0x24020001, addr); // rebootexcheck1
        	patches--;
        }
        else if (data == 0x34650001){ // rebootexcheck2
            _sw(NOP, addr-4); // Killing Branch Check bltz ...
            patches--;
        }
        else if (data == 0x00903021 && _lw(addr+4) == 0x00D6282B){ // rebootexcheck3 and rebootexcheck4
            u32 a = addr;
            do {a-=4;} while (_lw(a) != NOP);
            _sw(NOP, a-4); // Killing Branch Check beqz
            _sw(NOP, addr+8); // Killing Branch Check bltz ...
            patches--;
        }
        else if ((data == _lw(addr+4)) && (data & 0xFC000000) == 0xAC000000){ // Patch ~PSP header check
            // Returns size of the buffer on loading whatever modules
            _sw(0xAFA50000, addr+4); // sw a1, 0(sp)
            _sw(0x20A30000, addr+8); // move v1, a1
            patches--;
        }
        else if (data == lFatOpenCall){
            _sw(JAL(_pspLfatOpen), addr); // Hook pspLfatOpen
            patches--;
        }
	}
}

void patchRebootBufferVita(u32 reboot_start, u32 reboot_end){
    pspemuLfatOpen = origLfatOpen;
	u32 lFatOpenCall = JAL(pspemuLfatOpen);
	for (u32 addr = reboot_start; addr<reboot_end; addr+=4){
	    u32 data = _lw(addr);
        if (data == lFatOpenCall){
            _sw(JAL(_pspemuLfatOpen), addr); // Hook pspLfatOpen
        }
        else if (data == 0x00600008){ // found loadcore jump on Vita
            // Move LoadCore module_start Address into third Argument
            _sw(0x00603021, addr); // move a2, v1
            // Hook LoadCore module_start Call
            _sw(JUMP(loadcoreModuleStartVita), addr+8);
        }
        else if ((data == _lw(addr+4)) && (data & 0xFC000000) == 0xAC000000){ // Patch ~PSP header check
            // Returns size of the buffer on loading whatever modules
            _sw(0xAFA50000, addr+4); // sw a1, 0(sp)
            _sw(0x20A30000, addr+8); // move v1, a1
        }
        else if ((data & 0x0000FFFF) == 0x8B00){
	        _sb(0xA0, addr); // Link Filesystem Buffer to 0x8BA00000
        }
	}
	//setBreakpoint(addr);
}

// Entry Point
int _arkReboot(int arg1, int arg2, int arg3, int arg4) __attribute__((section(".text.startup")));
int _arkReboot(int arg1, int arg2, int arg3, int arg4)
{

    // NOTE: ARKConfig must be properly setup in user ram for reboot to function

	// TODO Parse Reboot Buffer Configuration (what to configure yet? lol)
	
	u32 reboot_start = REBOOT_TEXT;
	u32 reboot_end = getRebootEnd;
	findRebootFunctions(reboot_start, reboot_end); // scan for reboot functions
	
	// patch reboot buffer
	if (IS_PSP(ark_conf->exec_mode)) patchRebootBufferPSP(reboot_start, reboot_end);
	else patchRebootBufferVita(reboot_start, reboot_end);
	
	// Flush Cache
	flushCache();
	
	// Forward Call
	return sceReboot(arg1, arg2, arg3, arg4);
}
