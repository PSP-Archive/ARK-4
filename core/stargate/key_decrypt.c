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

/*
 * This file is part of stargate.
 *
 * Copyright (C) 2008 hrimfaxi (outmatch@gmail.com)
 */

#include <pspsdk.h>
#include <pspkernel.h>
#include <pspsysmem_kernel.h>
#include <pspthreadman_kernel.h>
#include <pspcrypt.h>
#include <pspdebug.h>
#include <pspinit.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "pspmodulemgr_kernel.h"
#include "psputilsforkernel.h"
#include "systemctrl.h"
#include "systemctrl_se.h"
#include "kubridge.h"
#include "pspcipher.h"
#include "macros.h"
#include "functions.h"

static u8 g_key_d91609f0[16] = {
    0xD0, 0x36, 0x12, 0x75, 0x80, 0x56, 0x20, 0x43,
       0xC4, 0x30, 0x94, 0x3E, 0x1C, 0x75, 0xD1, 0xBF,
};

static u8 g_key_d91628f0[16] = {
    0x49, 0xA4, 0xFC, 0x66, 0xDC, 0xE7, 0x62, 0x21,
    0xDB, 0x18, 0xA7, 0x50, 0xD6, 0xA8, 0xC1, 0xB6,
};

static u8 g_key_d91612f0[16] = {
    0x9e, 0x20, 0xe1, 0xcd, 0xd7, 0x88, 0xde, 0xc0, 
    0x31, 0x9b, 0x10, 0xaf, 0xc5, 0xb8, 0x73, 0x23,
};

static u8 g_key_d91613f0[16] = {
    0xEB, 0xFF, 0x40, 0xD8, 0xb4, 0x1a, 0xe1, 0x66, 
    0x91, 0x3b, 0x8f, 0x64, 0xb6, 0xfc, 0xb7, 0x12,
};

static u8 g_key_d91614f0[16] = {
    0xFD, 0xF7, 0xB7, 0x3C, 0x9F, 0xD1, 0x33, 0x95, 
    0x11, 0xB8, 0xB5, 0xBB, 0x54, 0x23, 0x73, 0x85, 
};

static u8 g_key_d91615f0[16] = {
    0xC8, 0x03, 0xE3, 0x44, 0x50, 0xF1, 0xE7, 0x2A, 
    0x6A, 0x0D, 0xC3, 0x61, 0xB6, 0x8E, 0x5F, 0x51,
};

static u8 g_key_d91616f0[16] = {
    0x53, 0x03, 0xB8, 0x6A, 0x10, 0x19, 0x98, 0x49,
    0x1C, 0xAF, 0x30, 0xE4, 0x25, 0x1B, 0x6B, 0x28,
};

static u8 g_key_d91617f0[16] = {
    0x02, 0xFA, 0x48, 0x73, 0x75, 0xAF, 0xAE, 0x0A,
       0x67, 0x89, 0x2B, 0x95, 0x4B, 0x09, 0x87, 0xA3,
};

static u8 g_key_d91618f0[16] = {
    0x96, 0x96, 0x7C, 0xC3, 0xF7, 0x12, 0xDA, 0x62,
    0x1B, 0xF6, 0x9A, 0x9A, 0x44, 0x44, 0xBC, 0x48, 
};

static u8 g_key_d91619f0[16] = {
    0xE0, 0x32, 0xA7, 0x08, 0x6B, 0x2B, 0x29, 0x2C,
    0xD1, 0x4D, 0x5B, 0xEE, 0xA8, 0xC8, 0xB4, 0xE9,
};

static u8 g_key_d9161af0[16] = {
    0x27, 0xE5, 0xA7, 0x49, 0x52, 0xE1, 0x94, 0x67,
    0x35, 0x66, 0x91, 0x0C, 0xE8, 0x9A, 0x25, 0x24,
};

static u8 g_key_d91620f0[16] = {
    0x52, 0x1C, 0xB4, 0x5F, 0x40, 0x3B, 0x9A, 0xDD,
    0xAC, 0xFC, 0xEA, 0x92, 0xFD, 0xDD, 0xF5, 0x90,
};

static u8 g_key_d91621f0[16] = {
    0xD1, 0x91, 0x2E, 0xA6, 0x21, 0x14, 0x29, 0x62,
    0xF6, 0xED, 0xAE, 0xCB, 0xDD, 0xA3, 0xBA, 0xFE,
};

static u8 g_key_d91622f0[16] = {
    0x59, 0x5D, 0x78, 0x4D, 0x21, 0xB2, 0x01, 0x17,
    0x6C, 0x9A, 0xB5, 0x1B, 0xDA, 0xB7, 0xF9, 0xE6,
};

static u8 g_key_d91623f0[16] = {
    0xAA, 0x45, 0xEB, 0x4F, 0x62, 0xFB, 0xD1, 0x0D, 
    0x71, 0xD5, 0x62, 0xD2, 0xF5, 0xBF, 0xA5, 0x2F,
};

static u8 g_key_d91624f0[16] = {
    0x61, 0xB7, 0x26, 0xAF, 0x8B, 0xF1, 0x41, 0x58,
    0x83, 0x6A, 0xC4, 0x92, 0x12, 0xCB, 0xB1, 0xE9,
};

static u8 g_key_d9160bf0[16] = {
    0x83, 0x83, 0xF1, 0x37, 0x53, 0xD0, 0xBE, 0xFC,
    0x8D, 0xA7, 0x32, 0x52, 0x46, 0x0A, 0xC2, 0xC2,
};

static u8 g_key_d9160af0[16] = {
    0x10, 0xA9, 0xAC, 0x16, 0xAE, 0x19, 0xC0, 0x7E,
    0x3B, 0x60, 0x77, 0x86, 0x01, 0x6F, 0xF2, 0x63,
};

static u8 g_key_d91611f0[16] = {
    0x61, 0xB0, 0xC0, 0x58, 0x71, 0x57, 0xD9, 0xFA,
    0x74, 0x67, 0x0E, 0x5C, 0x7E, 0x6E, 0x95, 0xB9,
};

// 6.30 game key
static u8 g_key_d91680f0[16] = {
    0x2C, 0x22, 0x9B, 0x12, 0x36, 0x74, 0x11, 0x67,
    0x49, 0xD1, 0xD1, 0x88, 0x92, 0xF6, 0xA1, 0xD8,
};

// 6.36 game key
static u8 g_key_d91681f0[16] = {
    0x52, 0xB6, 0x36, 0x6C, 0x8C, 0x46, 0x7F, 0x7A, 
    0xCC, 0x11, 0x62, 0x99, 0xC1, 0x99, 0xBE, 0x98, 
};

/* 6.30 phat kernel-2 */
static u8 g_key_457b80f0[16] = {
    0xd4, 0x35, 0x18, 0x02, 0x29, 0x68, 0xfb, 0xa0, 
    0x6a, 0xa9, 0xa5, 0xed, 0x78, 0xfd, 0x2e, 0x9d,
};

/* 6.30 game key-2 */
static u8 g_key_0b2b80f0[16] = {
    0x57, 0xb4, 0xa6, 0x5c, 0x75, 0x2d, 0xb9, 0x4d, 
    0xe1, 0x67, 0xe3, 0x31, 0xbf, 0x4d, 0x70, 0xf8
};

typedef struct {
    u32 tag;
    u8 *key;
    u32 code;
    u32 type;
} GameCipher;

static GameCipher g_cipher[] = {
    { 0xd91609f0, g_key_d91609f0, 0x5d, 2},
    { 0xd9160af0, g_key_d9160af0, 0x5d, 2},
    { 0xd9160bf0, g_key_d9160bf0, 0x5d, 2},
    { 0xd91611f0, g_key_d91611f0, 0x5d, 2},
    { 0xd91612f0, g_key_d91612f0, 0x5d, 2},
    { 0xd91613f0, g_key_d91613f0, 0x5d, 2},
    { 0xd91614f0, g_key_d91614f0, 0x5d, 2},
    { 0xd91615f0, g_key_d91615f0, 0x5d, 2},
    { 0xd91616f0, g_key_d91616f0, 0x5d, 2},
    { 0xd91617f0, g_key_d91617f0, 0x5d, 2},
    { 0xd91618f0, g_key_d91618f0, 0x5d, 2},
    { 0xd91619f0, g_key_d91619f0, 0x5d, 2},
    { 0xd9161af0, g_key_d9161af0, 0x5d, 2},
    { 0xd91620f0, g_key_d91620f0, 0x5d, 2},
    { 0xd91621f0, g_key_d91621f0, 0x5d, 2},
    { 0xd91622f0, g_key_d91622f0, 0x5d, 2},
    { 0xd91623f0, g_key_d91623f0, 0x5d, 2},
    { 0xd91624f0, g_key_d91624f0, 0x5d, 2},
    { 0xd91628f0, g_key_d91628f0, 0x5d, 2},
    { 0xd91680f0, g_key_d91680f0, 0x5d, 6},
    { 0xd91681f0, g_key_d91681f0, 0x5d, 6},
    { 0x0b2b80f0, g_key_0b2b80f0, 0x5c, 6},
    { 0x457b80f0, g_key_457b80f0, 0x5b, 6},
};

static GameCipher *get_game_cipher(u32 tag)
{
    int i;

    for(i=0; i<NELEMS(g_cipher); ++i) {
        if (g_cipher[i].tag == tag)
            return &g_cipher[i];
    }

    return NULL;
}

int (*mesgled_decrypt)(u32 *tag, u8 *key, u32 code, u8 *prx, u32 size, u32 *newsize, u32 use_polling, u8 *blacklist, u32 blacklistsize, u32 type, u8 *xor_key1, u8 *xor_key2) = NULL;

static int _mesgled_decrypt(u32 *tag, u8 *key, u32 code, u8 *prx, u32 size, u32 *newsize, u32 use_polling, u8 *blacklist, u32 blacklistsize, u32 type, u8 *xor_key1, u8 *xor_key2)
{
    int ret;
    u32 keytag;
    GameCipher *cipher = NULL;
    
    // try default decrypt
    ret = (*mesgled_decrypt)(tag, key, code, prx, size, newsize, use_polling, blacklist, blacklistsize, type, xor_key1, xor_key2);

    if (ret == 0) {
        return 0;
    }
    
    // try game key
    keytag = *((u32*)(prx+0xd0));
    cipher = get_game_cipher(keytag);

    if (cipher != NULL) {
        ret = uprx_decrypt(&cipher->tag, cipher->key, cipher->code, prx, size, newsize, use_polling, blacklist, blacklistsize, cipher->type, NULL, NULL);

        if (ret < 0) {
            ret = (*mesgled_decrypt)(&cipher->tag, cipher->key, cipher->code, prx, size, newsize, use_polling, blacklist, blacklistsize, cipher->type, NULL, NULL);
        }
        
        if (ret == 0) {
            #ifdef DEBUG
            printk("%s: tag=0x%08X type=%d decrypt OK\n", __func__, (uint)cipher->tag, (int)cipher->type);
            #endif

            return ret;
        }
    }
    
    return -301;
}

void patch_sceMesgLed()
{
    SceModule2 *mod;
    u32 intr, text_addr, topaddr;

    mod = (SceModule2*)sceKernelFindModuleByName("sceMesgLed");

    if (mod == NULL) {
        return;
    }
    
    intr = JAL(_mesgled_decrypt);
    text_addr = mod->text_addr;
    topaddr = text_addr+mod->text_size;
    
    for (u32 addr = text_addr; addr<topaddr; addr+=4){
        u32 data = _lw(addr);
        if (data == 0x2CE30001){
            mesgled_decrypt = addr; // Save Original Decrypt Function Pointer
            break;
        }
    }
    
    u32 call = JAL(mesgled_decrypt);
    u32 func1 = (void*)sctrlHENFindFunction("sceMesgLed", "sceMesgLed_driver", 0x792A6126);
    u32 func2 = (void*)sctrlHENFindFunction("sceMesgLed", "sceMesgLed_driver", 0x337D0DD3);
    
    _sw(intr, func1+140); // patch the first call to mesgled_decrypt in sceMesgLed_driver_792A6126
    _sw(intr, func2+920); // patch the second call to mesgled_decrypt in sceMesgLed_driver_337D0DD3
}
