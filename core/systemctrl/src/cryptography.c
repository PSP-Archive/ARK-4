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
#include <ark.h>
#include <functions.h>
#include "imports.h"
#include "elf.h"
#include "rebootconfig.h"

// custom user crypto functions
int (*ExtendDecryption)() = NULL;
int (*MesgLedDecryptEX)() = NULL;

// Memlmd Decrypt Function
int (*memlmd_unsigner)(u8 *prx, u32 size, u32 use_polling) = NULL;
int (* memlmdDecrypt)(unsigned char * prx, unsigned int size, unsigned int * newsize, unsigned int use_polling) = NULL;
int (* mesgledDecrypt)(unsigned int * tag, unsigned char * key,
    unsigned int code, unsigned char * prx, unsigned int size,
    unsigned int * newsize, unsigned int use_polling, unsigned char * blacklist,
    unsigned int blacklistsize, unsigned int type, unsigned char * xor_key1, unsigned char * xor_key2) = NULL;
static int (*sceMemlmdInitializeScrambleKey)(u32 unk, void *hash_addr) = NULL;

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
    // Supported Compression Tag
    if ( (*(unsigned short *)(prx + 0x150) == 0x8B1F) && isTagCompressed(*(unsigned int *)(prx + 0x130)) )
    {
        // GZIP PRX
        return 1;
    }
    // Encrypted PRX
    return 0;
}

// Memlmd Decrypt Function Hook
int _memlmdDecrypt(unsigned char * prx, unsigned int size, unsigned int * newsize, unsigned int use_polling)
{
    if(ExtendDecryption)
    {
        int res;
        if( ( res = ExtendDecryption(prx, size, newsize, use_polling)) >=0)
            return res;
    }

    // Valid Parameters
    if(prx != NULL && newsize != NULL)
    {
        // Read GZIP Payload Size
        unsigned int compsize = *(unsigned int*)(prx + 0xB0);
        
        // GZIP Compressed PRX
        if(isPrxCompressed(prx, size))
        {
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

    if (ret >= 0) {
        return ret;
    }

    // re-calculate key with xor seed
    if (sceMemlmdInitializeScrambleKey!= NULL && sceMemlmdInitializeScrambleKey(0, (void*)0xBFC00200) < 0)
        return ret;

    if (memlmd_unsigner != NULL && _memlmd_unsigner(prx, size, use_polling) < 0) {
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
    
        if( MesgLedDecryptEX )
        {
            int ret = MesgLedDecryptEX(prx, size, newsize, use_polling);
            if( ret >= 0 )
                return ret;
        }
    
        // Read GZIP Payload Size from PRX Header
        unsigned int compsize = *(unsigned int *)(prx + 0xB0);
        
        // GZIP Compressed PRX
        if(isPrxCompressed(prx, size))
        {
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

    u32 topaddr = mod->text_addr + mod->text_size;
    
    int patches = 5;
    for (u32 addr = text_addr; addr<topaddr && patches; addr+=4){
        u32 data = _lw(addr);
        if (data == JAL(memlmdDecrypt)){
            _sw(JAL(_memlmdDecrypt), addr);
            patches--; // 2
        }
        else if (data == JAL(memlmd_unsigner)){
            _sw(JAL(_memlmd_unsigner), addr);
            patches--; // 2
        }
        else if (data == 0x3C02F009){
            _sh(0xF005, addr); //This patch allow to load packed usermode module.
            patches--; // 1
        }
        else if (data == 0x3222003F){
            u32 a = addr;
            do {a-=4;} while (_lw(a)!=0x27BDFFF0);
            memlmd_unsigner = (void*)a; // inner function which unsigns a PRX module 
        }
        else if (data == 0x27BDFF80){
            memlmdDecrypt = addr-8; // Backup Decrypt Function Pointer
        }
        else if (data == 0x2403FF31 && sceMemlmdInitializeScrambleKey==NULL){
            sceMemlmdInitializeScrambleKey = addr-8;
        }
    }
    // Flush Cache
    flushCache();
    
    return mod;
}

// Patch MesgLed Cryptography
void patchMesgLed(SceModule2 * mod)
{
    u32 addr;
    u32 topaddr = mod->text_addr + mod->text_size;

    for (addr = mod->text_addr; addr<topaddr; addr+=4){
        u32 data = _lw(addr);
        if (data == 0x2CE30001){
            mesgledDecrypt = addr; // Save Original Decrypt Function Pointer
        }
        else if (data == JAL(mesgledDecrypt)){
            _sw(JAL(_mesgledDecrypt), addr);
        }
    }
    // Flush Cache
    flushCache();
}
