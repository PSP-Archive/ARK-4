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
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
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
#include "psputilsforkernel.h"
#include <systemctrl.h>
#include "kubridge.h"
#include "pspcipher.h"

#define KIRK_CMD_DECRYPT_PRIVATE 0x1
#define KIRK_CMD_DECRYPT_IV_0 0x7
#define KIRK_CMD_SHA1_HASH 0xB 
#define KIRK_ECDSA_VERIFY_SIGNATURE 0x11

static u8 g_pubkey_28672[40] = {
    0x77, 0x3F, 0x4B, 0xE1, 0x4C, 0x0A, 0xB4, 0x52, 0x67, 0x2B, 0x67, 0x56, 0x82, 0x4C, 0xCF, 0x42, 
    0xAA, 0x37, 0xFF, 0xC0, 0x89, 0x41, 0xE5, 0x63, 0x5E, 0x84, 0xE9, 0xFB, 0x53, 0xDA, 0x94, 0x9E, 
    0x9B, 0xB7, 0xC2, 0xA4, 0x22, 0x9F, 0xDF, 0x1F
};

static u8 g_pubkey_28712[40] = {
    0x25, 0xDC, 0xFD, 0xE2, 0x12, 0x79, 0x89, 0x54, 0x79, 0x37, 0x13, 0x24, 0xEC, 0x25, 0x08, 0x81, 
    0x57, 0xAA, 0xF1, 0xD0, 0xA4, 0x64, 0x8C, 0x15, 0x42, 0x25, 0xF6, 0x90, 0x3F, 0x44, 0xE3, 0x6A, 
    0xE6, 0x64, 0x12, 0xFC, 0x80, 0x68, 0xBD, 0xC1
};

static u8 g_pubkey_28752[40] = {
    0xE3, 0x5E, 0x4E, 0x7E, 0x2F, 0xA3, 0x20, 0x96, 0x75, 0x43, 0x94, 0xA9, 0x92, 0x01, 0x83, 0xA7, 
    0x85, 0xBD, 0xF6, 0x19, 0x1F, 0x44, 0x8F, 0x95, 0xE0, 0x43, 0x35, 0xA3, 0xF5, 0xE5, 0x05, 0x65, 
    0x5E, 0xD7, 0x59, 0x3F, 0xC6, 0xDB, 0xAF, 0x39
};

static int check_blacklist(u8 *prx, u8 *blacklist, u32 blacklistsize)
{
    u32 i;

    if (blacklistsize / 16 == 0) {
        return 0;
    }

    i = 0;

    while (i < blacklistsize / 16) {
        if (!memcmp(blacklist + i * 16, (prx + 0x140), 0x10)) {
            return 1;
        }

        i++;
    }

    return 0;
}

// sub_00000000
static int kirk7(u8* prx, u32 size, u32 scramble_code, u32 use_polling)
{
    int ret;

    ((u32 *) prx)[0] = 5;
    ((u32 *) prx)[1] = 0;
    ((u32 *) prx)[2] = 0;
    ((u32 *) prx)[3] = scramble_code;
    ((u32 *) prx)[4] = size;

    if (!use_polling) {
        ret = sceUtilsBufferCopyWithRange(prx, size + 20, prx, size + 20, KIRK_CMD_DECRYPT_IV_0);
    } else {
        ret = sceUtilsBufferCopyByPollingWithRange(prx, size + 20, prx, size + 20, KIRK_CMD_DECRYPT_IV_0);
    }

    return ret;
}

static void prx_xor_key_mix(u8 *dstbuf, u32 size, u8 *srcbuf, u8 *xor_key)
{
    u32 i;

    i = 0;

    while (i < size) {
        dstbuf[i] = srcbuf[i] ^ xor_key[i];
        ++i;
    }
}

static void prx_xor_key_round(u8 *buf, u32 size, u8 *xor_key1, u8 *xor_key2)
{
    u32 i;

    i=0;

    while (i < size) {
        if (xor_key1 != NULL) {
            buf[i] ^= xor_key1[i&0xf];
        }

        if (xor_key2 != NULL) {
            buf[i] ^= xor_key2[i&0xf];
        }

        ++i;
    }
}

/*
 * 6.39/01g: 412
 * 6.60/01g: 424
 */
static u8 buf1[0x150] __attribute__((aligned(64)));

/*
 * 6.39/01g: 896
 * 6.60/01g: 960
 */
static u8 buf2[0x150] __attribute__((aligned(64)));

/*
 * 6.39/01g: 748
 * 6.60/01g: 760
 */
static u8 buf3[0x90] __attribute__((aligned(64)));

/**
 * 6.39/01g: 1280
 * 6.60/01g: 1344
 */
static u8 buf4[0xb4] __attribute__((aligned(64)));

/**
 * 6.39/01g: 1536
 * 6.60/01g: 1600
 */
static u8 buf5[0x20] __attribute__((aligned(64)));

/**
 * 6.60/01g: 1632
 */
static u8 buf6[0x28] __attribute__((aligned(32)));

static u8 sig[0x28] __attribute__((aligned(16)));

static u8 sha1buf[0x14] __attribute__((aligned(16)));

int _uprx_decrypt(user_decryptor *pBlock)
{
    if (pBlock == NULL) {
        return -1;
    }

    if (pBlock->prx == NULL || pBlock->newsize == NULL) {
        return -2;
    }

    if (pBlock->size < 0x160) {
        return -202;
    }

    if ((u32)pBlock->prx & 0x3f) {
        return -203;
    }

    if (((0x00220202 >> (((u32)pBlock->prx >> 27) & 0x001F)) & 0x0001) == 0x0000) {
        return -204;
    }

    u32 b_0xd4 = 0;

    memset(buf1, 0, sizeof(buf1));
    memset(buf2, 0, sizeof(buf2));
    memset(buf3, 0, sizeof(buf3));
    memset(buf4, 0, sizeof(buf4));
    memset(buf5, 0, sizeof(buf5));
    memset(buf6, 0, sizeof(buf6));

    // loc_00000190
    memcpy(buf1, pBlock->prx, 0x150);

    // loc_000001C4: tag mismatched
    if (0 != memcmp(buf1 + 0xd0, pBlock->tag, 4)) {
        return -45;
    }

    int ret = -1;

    if (pBlock->type == 3) {
        u8 *p = buf1;
        u32 cnt = 0;

        // loc_00001994
        while (p[0xd4] && cnt < 0x18) {
            cnt++;
            p = buf1 + cnt;
        }

        if (p[0xd4] != 0) {
            return -17;
        }
    } else if (pBlock->type == 2) {
        u8 *p = buf1;
        u32 cnt = 0;

        // loc_00001960
        while (p[0xd4] && cnt < 0x58) {
            cnt++;
            p = buf1 + cnt;
        }

        if (p[0xd4] != 0) {
            return -12;
        }
    } else if (pBlock->type == 5) {
        u8 *p = buf1 + 1;
        u32 cnt = 1;

        // loc_0000192C
        while (p[0xd4] && cnt < 0x58) {
            cnt++;
            p = buf1 + cnt;
        }

        if (p[0xd4] != 0) {
            return -13;
        }

        b_0xd4 = buf1[0xd4];
    } else if (pBlock->type == 6) {
        u8 *p = buf1;
        u32 cnt = 0;

        // loc_000018F8
        while (p[0xd4] && cnt < 0x38) {
            cnt++;
            p = buf1 + cnt;
        }

        if (p[0xd4] != 0) {
            return -302;
        }
    } else if (pBlock->type == 7) {
        u8 *p = buf1 + 1;
        u32 cnt = 1;

        // loc_000018C4
        while (p[0xd4] && cnt < 0x38) {
            cnt++;
            p = buf1 + cnt;
        }

        if (p[0xd4] != 0) {
            return -302;
        }
    } if (pBlock->type == 9) {
        u8 *p = buf1;
        u32 cnt = 0;

        // loc_00001890
        while (p[0xd4] && cnt < 0x30) {
            cnt++;
            p = buf1 + cnt;
        }

        if (p[0xd4] != 0) {
            return -302;
        }
    } else if (pBlock->type == 10) {
        u8 *p = buf1 + 1;
        u32 cnt = 1;

        // loc_00001858
        while (p[0xd4] && cnt < 0x30) {
            cnt++;
            p = buf1 + cnt;
        }

        if (p[0xd4] != 0) {
            return -302;
        }
    }

    // loc_00000244
    if (pBlock->blacklist != NULL && pBlock->blacklistsize != 0) {
        ret = check_blacklist(buf1, pBlock->blacklist, pBlock->blacklistsize);

        if (ret == 1) {
            return -305;
        }
    }

    // loc_00000254
    u32 elf_size_comp =  *(u32*)(buf1+0xb0);
    *pBlock->newsize = elf_size_comp;

    if (pBlock->size - 0x150 < elf_size_comp) {
        return -206;
    }

    if ((pBlock->type >= 2 && pBlock->type <= 7) || pBlock->type == 9 || pBlock->type == 10) {
        // loc_000017E0
        int i;

        for (i=0; i<9; i++) {
            memcpy(buf2+0x14+(i<<4), pBlock->key, 0x10);
            buf2[0x14+(i<<4)] = i;
        }
    } else {
        // loc_000002C8
        memcpy(buf2+0x14, pBlock->key, 0x90);
    }

    // loc_000002E8
    ret = kirk7(buf2, 0x90, pBlock->code, pBlock->use_polling);

    if (ret != 0) {
        if (ret == 0xC) {
            return -101;
        } else {
            return -104;
        }
    }

    // loc_00000314
    if (pBlock->type == 3 || pBlock->type == 5 || pBlock->type == 7 || pBlock->type == 10) {
        // loc_0000034C
        if (pBlock->xor_key2 != NULL) {
            prx_xor_key_round(buf2, 0x90, pBlock->xor_key2, NULL);
        }
    }

    // loc_00000388
    memcpy(buf3, buf2, 0x90);

    if (pBlock->type == 9 || pBlock->type == 10) {
        // loc_000003C4
        memcpy(buf6, pBlock->prx+0x104, sizeof(buf6));
        memset(pBlock->prx+0x104, 0, sizeof(buf6));
        memcpy(sig, buf6, sizeof(sig));

        *(u32*)pBlock->prx = pBlock->size-4;

        if(pBlock->use_polling) {
            ret = sceUtilsBufferCopyByPollingWithRange(pBlock->prx, pBlock->size, pBlock->prx, pBlock->size, KIRK_CMD_SHA1_HASH);
        } else {
            // loc_000017C0
            ret = sceUtilsBufferCopyWithRange(pBlock->prx, pBlock->size, pBlock->prx, pBlock->size, KIRK_CMD_SHA1_HASH);
        }

        // loc_0000045C
        if(ret != 0) {
            return -105;
        }

        // loc_00000468
        memcpy(sha1buf, pBlock->prx, sizeof(sha1buf));
        // loc_00000490
        memcpy(pBlock->prx, buf1, 0x20);

        if (0x16 == ((u8*)(pBlock->tag))[2]) {
            // loc_00001790
            memcpy(buf4, g_pubkey_28752, sizeof(g_pubkey_28752));
        } else if (0x5E == ((u8*)(pBlock->tag))[2]) {
            // loc_00001764
            memcpy(buf4, g_pubkey_28712, sizeof(g_pubkey_28712));
        } else {
            // loc_000004D4
            memcpy(buf4, g_pubkey_28672, sizeof(g_pubkey_28672));
        }

        // loc_000004F4
        memcpy(buf4+0x28, sha1buf, sizeof(sha1buf));
        // loc_0000051C
        memcpy(buf4+0x28+sizeof(sha1buf), sig, sizeof(sig));

        if (pBlock->use_polling) {
            ret = sceUtilsBufferCopyByPollingWithRange(NULL, 0, buf4, 100, KIRK_ECDSA_VERIFY_SIGNATURE);
        } else {
            // loc_00001744
            ret = sceUtilsBufferCopyWithRange(NULL, 0, buf4, 100, KIRK_ECDSA_VERIFY_SIGNATURE);
        }

        // loc_0000055C
        if(ret != 0) {
            return -306;
        }
    }

    // loc_00000568
    if (pBlock->type == 3) {
        u8 *p;

        // loc_0000145C
        p = buf2;
        memcpy(p, buf1+0xec, 0x40);
        p += 0x40;
        memset(p, 0, 0x50);

        buf2[0x60] = 0x03;
        buf2[0x70] = 0x50;

        // loc_000014D0
        p = buf2+0x90;
        memcpy(p, buf1+0x80, 0x30);
        p += 0x30;
        // loc_000014FC
        memcpy(p, buf1+0xc0, 0x10);
        p += 0x10;
        // loc_00001528
        memcpy(p, buf1+0x12c, 0x10);

        // loc_00001550
        prx_xor_key_round(buf2+0x90, 0x50, pBlock->xor_key1, pBlock->xor_key2);
        ret = sceUtilsBufferCopyWithRange(buf4, 0xb4, buf2, 0x150, 3);

        if (ret != 0) {
            return -14;
        }
        
        // loc_000015D4
        p = buf2;
        memcpy(p, buf1+0xd0, 4);
        p += 4;
        // loc_000015F8
        memset(p, 0, 0x58);
        p += 0x58;
        // loc_0000161C
        memcpy(p, buf1+0x140, 0x10);
        p += 0x10;
        // loc_00001648
        memcpy(p, buf1+0x12c, 0x14);
        // loc_00001670, (yes, p no increase)
        memcpy(p, buf4+0x40, 0x10);
        p += 0x14;
        // loc_0000169C
        memcpy(p, buf4, 0x30);
        p += 0x30;
        // loc_000016C8
        memcpy(p, buf4+0x30, 0x10);
        p += 0x10;
        // loc_000016F4
        memcpy(p, buf1+0xb0, 0x10);
        p += 0x10;
        // loc_00001720
        memcpy(p, buf1, 0x80);
    } else if (pBlock->type == 5 || pBlock->type == 7 || pBlock->type == 10) {
        u8 *p;

        // loc_0000113C
        p = buf2+0x14;
        memcpy(p, buf1+0x80, 0x30);
        p += 0x30;
        // loc_00001178
        memcpy(p, buf1+0xc0, 0x10);
        p += 0x10;
        // loc_000011A4
        memcpy(p, buf1+0x12c, 0x10);

        // loc_000011CC
        prx_xor_key_round(buf2+0x14, 0x50, pBlock->xor_key1, pBlock->xor_key2);
        ret = kirk7(buf2, 0x50, pBlock->code, pBlock->use_polling);

        // loc_00001238
        if (ret != 0) {
            if (ret == 0xC) {
                return -101;
            } else {
                return -104;
            }
        }

        // loc_0000124C
        memcpy(buf4, buf2, 0x50);
        // loc_00001284
        p = buf2;
        memcpy(p, buf1+0xd0, 0x4);
        p += 0x4;
        // loc_000012A8
        memset(p, 0, 0x58);
        p += 0x58;
        // loc_000012D4
        memcpy(p, buf1+0x140, 0x10);
        p += 0x10;
        // loc_00001300
        memcpy(p, buf1+0x12c, 0x14);
        // loc_00001328 (yes, p no increase)
        memcpy(p, buf4+0x40, 0x10);
        p += 0x14;
        // loc_00001354
        memcpy(p, buf4, 0x30);
        p += 0x30;
        // loc_00001380
        memcpy(p, buf4+0x30, 0x10);
        p += 0x10;
        // loc_000013AC
        memcpy(p, buf1+0xb0, 0x10);
        p += 0x10;
        // loc_000013D8
        memcpy(p, buf1, 0x80); // 0xd0
    } else if (pBlock->type == 2 || pBlock->type == 4 || pBlock->type == 6 || pBlock->type == 9) {
        u8 *p;

        // loc_000005CC
        p = buf2;
        memcpy(p, buf1+0xd0, 0x5C);
        p += 0x5c;
        // loc_00000608
        memcpy(p, buf1+0x140, 0x10);
        p += 0x10;
        // loc_00000634
        memcpy(p, buf1+0x12c, 0x14);
        p += 0x14;
        // loc_00000660
        memcpy(p, buf1+0x80, 0x30);
        p += 0x30;
        // loc_0000068C
        memcpy(p, buf1+0xc0, 0x10);
        p += 0x10;
        // loc_000006B8
        memcpy(p, buf1+0xb0, 0x10);
        p += 0x10;
        // loc_000006E4
        memcpy(p, buf1, 0x80);
    } else {
        // loc_000010A4
        memcpy(buf2, buf1+0xd0, 0x80);
        // loc_000010E4
        memcpy(buf2+0x80, buf1+0x80, 0x50);
        // loc_00001114
        memcpy(buf2+0xd0, buf1, 0x80);
    }

    // loc_00000710
    if (pBlock->type == 1) {
        // loc_00000FF8
        memcpy(buf4+0x14, buf2+0x10, 0xa0);
        ret = kirk7(buf4, 0xa0, pBlock->code, pBlock->use_polling);

        if (ret != 0) {
            return -15;
        }

        // loc_0000105C
        memcpy(buf2+0x10, buf4, 0xa0);
    } else if ((pBlock->type >= 2 && pBlock->type <= 7) || pBlock->type == 9 || pBlock->type == 10) {
        // loc_00000F0C
        memcpy(buf4+0x14, buf2+0x5c, 0x60);

        if (pBlock->type == 3 || pBlock->type == 5 || pBlock->type == 7 || pBlock->type == 10) {
            // loc_00000F70
            prx_xor_key_round(buf4+0x14, 0x60, pBlock->xor_key1, NULL);
        }

        // loc_00000FA4
        ret = kirk7(buf4, 0x60, pBlock->code, pBlock->use_polling);

        if (ret != 0) {
            return -5;
        }

        // loc_00000FD4
        memcpy(buf2+0x5c, buf4, 0x60);
    }

    // loc_0000073C
    if ((pBlock->type >= 2 && pBlock->type <= 7) || pBlock->type == 9 || pBlock->type == 10) {
        u32 *p;

        // loc_00000D44
        memcpy(buf4, buf2+0x6c, 0x14);

        if (pBlock->type == 4) {
            // loc_00000EE0
            memmove(buf2+0x18, buf2, 0x67);
        } else {
            // loc_00000D84
            memcpy(buf2+0x70, buf2+0x5C, 0x10);

            if (pBlock->type == 6 || pBlock->type == 7) {
                // loc_00000DC8
                memcpy(buf5, buf2+0x3c, 0x20);
                // loc_00000DF8
                memcpy(buf2+0x50, buf5, 0x20);
                // loc_00000E20
                memset(buf2+0x18, 0, 0x38);
            } else {
                // loc_00000EC4
                memset(buf2+0x18, 0, 0x58);
            }

            // loc_00000E38
            if (b_0xd4 == 0x80 ) {
                // loc_00000EB8
                buf2[0x18] = 0x80;
            }
        }

        // loc_00000E48
        memcpy(buf2+0x04, buf2, 4);
        p = (u32*)buf2;
        *p = 0x14C;
        // loc_00000E94
        memcpy(buf2+0x08, buf3, 0x10);    
    } else {
        u32 *p;
        
        // loc_00000770
        memcpy(buf4, buf2+0x4, 0x14);
        p = (u32*)buf2;
        *p = 0x14c;
        // loc_000007B4
        memcpy(buf2+4, buf3, 0x14);
    }

    // loc_000007D0
    if (pBlock->use_polling) {
        ret = sceUtilsBufferCopyByPollingWithRange(buf2, 0x150, buf2, 0x150, KIRK_CMD_SHA1_HASH);
    } else {
        ret = sceUtilsBufferCopyWithRange(buf2, 0x150, buf2, 0x150, KIRK_CMD_SHA1_HASH);
    }

    if (ret != 0) {
        return -6;
    }

    // loc_00000914
    if (0 != memcmp(buf2, buf4, 0x14)) {
        return -8;
    }

    // loc_00000934
    if ((pBlock->type >= 2 && pBlock->type <= 7) || pBlock->type == 9 || pBlock->type == 10) {
        // loc_00000B34
        prx_xor_key_mix(buf2+0x80, 0x40, buf2+0x80, buf3+0x10);
        ret = kirk7(buf2+0x6c, 0x40, pBlock->code, pBlock->use_polling);

        if (ret != 0) {
            return -7;
        }

        // loc_00000B9C
        prx_xor_key_mix(pBlock->prx+0x40, 0x40, buf2+0x6c, buf3+0x50);

        if (pBlock->type == 6 || pBlock->type == 7) {
            // loc_00000BE0
            memcpy(pBlock->prx+0x80, buf5, 0x20);
            // loc_00000C00
            memset(pBlock->prx+0xA0, 0, 0x10);
            ((u8*)pBlock->prx)[0xA4] = 1;
            ((u8*)pBlock->prx)[0xA0] = 1;
        } else {
            // loc_00000C90
            memset(pBlock->prx+0x80, 0, 0x30);
            ((u8*)pBlock->prx)[0xA0] = 1;
        }

        // loc_00000C2C
        memcpy(pBlock->prx+0xB0, buf2+0xc0, 0x10);
        // loc_00000C4C
        memset(pBlock->prx+0xC0, 0, 0x10);
        // loc_00000C6C
        memcpy(pBlock->prx+0xD0, buf2+0xD0, 0x80);
    } else {
        // loc_00000970
        prx_xor_key_mix(buf2+0x40, 0x70, buf2+0x40, buf3+0x14);
        ret = kirk7(buf2+0x2c, 0x70, pBlock->code, pBlock->use_polling);

        // loc_000009B8
        if (ret != 0) {
            return -16;
        }

        // loc_000009CC
        prx_xor_key_mix(pBlock->prx+0x40, 0x70, buf2+0x2C, buf3+0x20);
        // loc_00000A10
        memcpy(pBlock->prx+0xB0, buf2+0xB0, 0xA0);

        if (pBlock->type == 8) {
            // loc_00000B1C
            if (1 != ((u8*)pBlock->prx)[0xA4]) {
                return -303;
            }
        }
    }

    // loc_00000A38
    if (b_0xd4 == 0x80) {
        if (((u8*)pBlock->prx)[0x590]) {
            return -302;
        }

        ((u8*)pBlock->prx)[0x590] |= 0x80;
    }

    // loc_00000A44: The real decryption
    if(pBlock->use_polling) {
        ret = sceUtilsBufferCopyByPollingWithRange(pBlock->prx, pBlock->size, pBlock->prx+0x40, pBlock->size-0x40, KIRK_CMD_DECRYPT_PRIVATE);
    } else {
        ret = sceUtilsBufferCopyWithRange(pBlock->prx, pBlock->size, pBlock->prx+0x40, pBlock->size-0x40, KIRK_CMD_DECRYPT_PRIVATE);
    }

    // loc_00000A60
    if (ret != 0) {
        return -9;
    }

    // loc_00000A98
    if (elf_size_comp < 0x150) {
        // Fill with 0
        memset(pBlock->prx+elf_size_comp, 0, 0x150-elf_size_comp);        
    }

    return 0;
}

int uprx_decrypt(u32 *tag, u8 *key, u32 code, u8 *prx, u32 size, u32 *newsize, u32 use_polling, u8 *blacklist, u32 blacklistsize, u32 type, u8 *xor_key1, u8 *xor_key2)
{
    user_decryptor block;
    int ret;

    block.tag = tag;
    block.key = key;
    block.code = code;
    block.prx = prx;
    block.size = size;
    block.newsize = newsize;
    block.use_polling = use_polling;
    block.blacklist = blacklist;
    block.blacklistsize = blacklistsize;
    block.type = type;
    block.xor_key1 = xor_key1;
    block.xor_key2 = xor_key2;

    ret = _uprx_decrypt(&block);

    if (ret < 0) {
        ret = -301;
    }

    return ret;
}
