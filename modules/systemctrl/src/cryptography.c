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
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <systemctrl.h>
#include <systemctrl_private.h>
#include <string.h>
#include <macros.h>
#include <module2.h>
#include <globals.h>
#include <functions.h>
#include "imports.h"
#include "elf.h"
#include "rc4.h"
#include "rebootconfig.h"
#include "psid.h"

// Memlmd Decrypt Function
int (*memlmd_unsigner)(u8 *prx, u32 size, u32 use_polling);
int (* memlmdDecrypt)(unsigned char * prx, unsigned int size, unsigned int * newsize, unsigned int use_polling) = NULL;
int (* mesgledDecrypt)(unsigned int * tag, unsigned char * key, unsigned int code, unsigned char * prx, unsigned int size, unsigned int * newsize, unsigned int use_polling, unsigned char * blacklist, unsigned int blacklistsize, unsigned int type, unsigned char * xor_key1, unsigned char * xor_key2);
static int (*sceMemlmdInitializeScrambleKey)(u32 unk, void *hash_addr);

// sub_334
int _memlmd_unsigner(u8 *prx, u32 size, u32 use_polling)
{
	if (prx != NULL && *(u32*)prx == 0x5053507E) { // ~PSP
		u32 i;
		int result = 0;

		// 6.60 kernel modules use type 9 PRX... 0xd4~0x104 is zero padded
		for(i=0; i<0x30; ++i) {
			if (prx[i+0xd4]) {
				result = 1;
				break;
			}
		}

		if (result == 0) {
			return 0;
		}

		result = 0;

		// updater prx ... 0xB8~0x18 is zero padded
		for(i=0; i<0x18; ++i) {
			if (prx[i+0xB8]) {
				result = 1;
				break;
			}
		}

		if (result == 0) {
			return 0;
		}
	}

	return (*memlmd_unsigner)(prx, size, use_polling);
}

// Known GZIP Compression Tags
unsigned int compressTag[] = {
	0x28796DAA,
	0x7316308C,
	0x3EAD0AEE,
	0x8555ABF2,
	0xC6BA41D3,
	0x55668D96,
	0xC01DB15D,
};

// PRX Tag Compression Check
int isTagCompressed(unsigned int tag)
{
	// Iterate Supported GZIP Tags
	int i = 0; for(; i < NELEMS(compressTag); ++i)
	{
		// Matching Tag found
		if(compressTag[i] == tag)
		{
			// GZIP PRX
			return 1;
		}
	}
	
	// Encrypted PRX
	return 0;
}

// PRX Compression Check
int isPrxCompressed(unsigned char * prx, unsigned int size)
{
	// Minimum PRX Size Check
	if(size < 0x160) return 0;
	
	// GZIP Magic detected
	if(*(unsigned short *)(prx + 0x150) == 0x8B1F)
	{
		// Supported Compression Tag
		if(isTagCompressed(*(unsigned int *)(prx + 0x130)))
		{
			// GZIP PRX
			return 1;
		}
	}
	
	// Encrypted PRX
	return 0;
}
/*
int psidHash(const unsigned char psid[16], unsigned char out[16])
{
	static unsigned char salt[20] __attribute__((aligned(64)));
	int err;

	_sw(PSID_SALT_MAGIC, (unsigned int)salt);
	memcpy(salt+4, psid, 16);
	err = sceKernelUtilsSha1Digest(salt, sizeof(salt), salt);

	if(err >= 0)
	{
		memcpy(out, salt, 16);
	}

	return err;
}

int getPsid(unsigned char psid[16])
{
	int (*_sceOpenPSIDGetOpenPSID)(unsigned char psid[0x10]), err;

	_sceOpenPSIDGetOpenPSID = (void*)sctrlHENFindFunction("sceOpenPSID_Service", "sceOpenPSID_driver", 0xC69BEBCE);
	memset(psid, 0, 16);
	err = (*_sceOpenPSIDGetOpenPSID)(psid);

	return err;
}

void getPsidHashFromReboot(unsigned char key[16])
{
	memcpy(key, reboot_config.psidHash, 16);
}
*/

// Memlmd Decrypt Function Hook
int _memlmdDecrypt(unsigned char * prx, unsigned int size, unsigned int * newsize, unsigned int use_polling)
{
	// Valid Parameters
	if(prx != NULL && newsize != NULL)
	{
		// Read GZIP Payload Size
		unsigned int compsize = *(unsigned int*)(prx + 0xB0);

        /*
		if(isPrxEncrypted(prx, size))
		{
			unsigned char key[16];
			void *rc4;

			getPsidHashFromReboot(key);
			prxXorKeyMix(key, sizeof(key), key, prx+0x80);

			rc4 = rc4_initstate(key, sizeof(key));
			rc4_process_stream(rc4, prx+0x150, compsize);
			rc4_destroystate(rc4);
			goto unzip;
		}
		*/
		
		// GZIP Compressed PRX
		if(isPrxCompressed(prx, size))
		{
unzip:
			// Remove PRX Header
			memcpy(prx, prx + 0x150, compsize);
			
			// Write GZIP Payload Size
			*newsize = compsize;
			
			// Return Decrypt Success
			return 0;
		}
	}
	
	// Passthrough
	int ret = memlmdDecrypt(prx, size, newsize, use_polling);

	if (ret >= 0 || IS_VITA(ark_config->exec_mode)) {
		return ret;
	}

	// re-calculate key with xor seed
	if ((*sceMemlmdInitializeScrambleKey)(0, (void*)0xBFC00200) < 0)
		return ret;

	if (_memlmd_unsigner(prx, size, use_polling) < 0) {
		return ret;
	}

	return (*memlmdDecrypt)(prx, size, newsize, use_polling);
	
}

// Mesgled Decrypt Function Hook
int _mesgledDecrypt(unsigned int * tag, unsigned char * key, unsigned int code, unsigned char * prx, unsigned int size, unsigned int * newsize, unsigned int use_polling, unsigned char * blacklist, unsigned int blacklistsize, unsigned int type, unsigned char * xor_key1, unsigned char * xor_key2)
{
	// Valid Parameters
	if(prx != NULL && newsize != NULL)
	{
		// Read GZIP Payload Size from PRX Header
		unsigned int compsize = *(unsigned int *)(prx + 0xB0);

        /*
		if(isPrxEncrypted(prx, size))
		{
			unsigned char key[16];
			void *rc4;

			getPsidHashFromReboot(key);
			prxXorKeyMix(key, sizeof(key), key, prx+0x80);

			rc4 = rc4_initstate(key, sizeof(key));
			rc4_process_stream(rc4, prx+0x150, compsize);
			rc4_destroystate(rc4);
			goto unzip;
		}*/
		
		// GZIP Compressed PRX
		if(isPrxCompressed(prx, size))
		{
unzip:
			// Remove PRX Header
			memcpy(prx, prx + 0x150, compsize);
			
			// Write GZIP Payload Size
			*newsize = compsize;
			
			// Return Decrypt Success
			return 0;
		}
	}
	
	// Passthrough
	return mesgledDecrypt(tag, key, code, prx, size, newsize, use_polling, blacklist, blacklistsize, type, xor_key1, xor_key2);
}

// Patch Memlmd Cryptography
SceModule2* patchMemlmd(void)
{
	// Find Module
	SceModule2* mod = (SceModule2*)sceKernelFindModuleByName("sceMemlmd");
	
	// Fetch Text Address
	unsigned int text_addr = mod->text_addr;
	
	if (IS_PSP(ark_config->exec_mode)){
	    //This patch allow to load packed usermode module.
	    _sh( 0xF005 , mod->text_addr + 0x000013F0 );
	    //patches
	    memlmd_unsigner = (void*)mod->text_addr + 0x00001070; // inner function which unsigns a PRX module 
	    sceMemlmdInitializeScrambleKey = (void*)mod->text_addr + 0x000012B8;
	    _sw(JAL(_memlmd_unsigner), mod->text_addr + 0x00001238); // the offset where memlmd_3F2AC9C6 call memlmd_unsigner
	    _sw(JAL(_memlmd_unsigner), mod->text_addr + 0x0000128C); // the offset where memlmd_97DA82BC call memlmd_unsigner
	    _sw(JAL(_memlmdDecrypt), mod->text_addr + 0x00000F70); // the offset where memlmd_E42AFE2E call memlmd_decrypt
	    _sw(JAL(_memlmdDecrypt), mod->text_addr + 0x00000FD4); // the offset where memlmd_D56F8AEC call memlmd_decrypt
	    memlmdDecrypt = (void*)mod->text_addr + 0x0000020C; // inner function which decrypt a PRX module
	}
	else{
	    // Backup Decrypt Function Pointer
	    memlmdDecrypt = (void*)findJALReverseForFunction("sceMemlmd", "memlmd", 0xCF03556B, 1);
	    // Hook Decrypt Function Calls
	    u32 addr;
	    u32 topaddr = mod->text_addr + mod->text_size;
	    for (addr = text_addr; addr<topaddr; addr+=4){
		    u32 data = _lw(addr);
		    if (data == JAL(memlmdDecrypt))
			    _sw(JAL(_memlmdDecrypt), addr);
	    }
	}
	// Flush Cache
	flushCache();
	return mod;
}

// Patch MesgLed Cryptography
void patchMesgLed(SceModule2 * mod)
{
    if (IS_PSP(ark_config->exec_mode)){
        u32 text_addr, offset, i;
	    text_addr = mod->text_addr;
	    mesgledDecrypt = (void*)(text_addr + 0x000000E0);
	    u32 calls[] = { 0x00003160, 0x00003FD8, 0x00004DD8, 0x00002354, 0x00002BE0 };
	    for(i=0; i<sizeof(calls)/sizeof(u32); ++i) {
		    offset = calls[i];
			_sw(JAL(_mesgledDecrypt), text_addr + offset);
	    }
    }
    else{
        // Save Original Decrypt Function Pointer
	    mesgledDecrypt = (void*)findFirstJALReverseForFunction("sceMesgLed", "sceDbsvr_driver", 0x94561901);
	    // Hook Decrypt Function Calls
	    u32 addr;
	    u32 topaddr = mod->text_addr + mod->text_size;
	    for (addr = mod->text_addr; addr<topaddr; addr+=4){
		    u32 data = _lw(addr);
		    if (data == JAL(mesgledDecrypt))
			    _sw(JAL(_mesgledDecrypt), addr);
	    }
    }
    // Flush Cache
	flushCache();
}
