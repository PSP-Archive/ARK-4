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

#ifndef DECRYPTBLOCK_H
#define DECRYPTBLOCK_H

/*
 * This file is part of pspcipher.
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

typedef struct _user_decryptor {
    u32 *tag; // key tag addr
    u8 *key;  // 16 bytes key
    u32 code; // scramble code
    u8 *prx;  // prx addr
    u32 size; // prx size
    u32 *newsize; // pointer of prx new size after decryption
    u32 use_polling; // use sceUtilsBufferCopyByPollingWithRange when 1 is set, pass 0
    u8 *blacklist; // module blacklist, pass NULL
    u32 blacklistsize; // module blacklist size in byte, pass 0
    u32 type; // prx type 2 for game, 5 for game patch etc, look up loadcore.prx if you are unsure
    u8 *xor_key1; // optional xor key, when decrypting prx type 3/5 this key is essential, otherwise can be NULL
       u8 *xor_key2; // optional xor key, when decrypting DRMed module this key is essential, otherwise can be NULL
} user_decryptor;

extern int _uprx_decrypt(user_decryptor *pBlock);

/**
 * Decrypt user PRX module such as game, game-patch etc.
 * It has the same behavior with sub_000000e0 in mesg_led_01g.prx from FW 6.30
 */
extern int uprx_decrypt(u32 *tag, u8 *key, u32 code, u8 *prx, u32 size, u32 *newsize, u32 use_polling, u8 *blacklist, u32 blacklistsize, u32 type, u8 *xor_key1, u8 *xor_key2);

#endif
