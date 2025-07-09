/* Copyright 2022 the unarr project authors (see AUTHORS file).
   License: LGPLv3 */
#include "unarr-imp.h"

#if !defined HAVE_ZLIB || !defined USE_ZLIB_CRC

/*
   crc32 calculation based on Intel slice-by-8 algorithm with lookup-table generation code
   adapted from https://gnunet.org/svn/gnunet/src/util/crypto_crc.c (public domain)        */

static inline uint32_t uint32le(const uint8_t *data) { return data[0] | data[1] << 8 | data[2] << 16 | (uint32_t)data[3] << 24; }

static bool crc_table_ready = false;
static uint32_t crc_table[8][256];

uint32_t ar_crc32(uint32_t crc32, const uint8_t * data, size_t data_len)
{
    if (!crc_table_ready) {

        static const uint32_t crc_poly = 0xEDB88320;

        uint32_t h = 1;
        crc_table[0][0] = 0;

        for (unsigned int i = 128; i; i >>= 1) {
            h = (h >> 1) ^ ((h & 1) ? crc_poly : 0);
            for (unsigned int j = 0; j < 256; j += 2 * i) {
                crc_table[0][i+j] = crc_table[0][j] ^ h;
            }
        }

        for (unsigned int i = 0; i < 256; i++) {
            for (unsigned int j = 1; j < 8; j++) {
                crc_table[j][i] = (crc_table[j-1][i] >> 8) ^ crc_table[0][crc_table[j-1][i] & 0xFF];
            }
        }

        crc_table_ready = true;
    }

    crc32 ^= 0xFFFFFFFF;

    while (data_len >= 8) {

        uint32_t tmp = crc32 ^ uint32le(data);

        crc32 = crc_table[7][ tmp        & 0xFF ] ^
                crc_table[6][(tmp >>  8) & 0xFF ] ^
                crc_table[5][(tmp >> 16) & 0xFF ] ^
                crc_table[4][ tmp >> 24         ];

        tmp = uint32le(data + 4);

        crc32 ^= crc_table[3][tmp        & 0xFF] ^
                crc_table[2][(tmp >>  8) & 0xFF] ^
                crc_table[1][(tmp >> 16) & 0xFF] ^
                crc_table[0][ tmp >> 24        ];

        data += 8;
        data_len -= 8;
    }

    while (data_len-- > 0) {
        crc32 = (crc32 >> 8) ^ crc_table[0][(crc32 ^ *data++) & 0xFF];
    }

    return crc32 ^ 0xFFFFFFFF;
}

#else

#include <zlib.h>

uint32_t ar_crc32(uint32_t crc, const unsigned char *data, size_t data_len)
{
#if SIZE_MAX > UINT32_MAX
    while (data_len > UINT32_MAX) {
        crc = crc32(crc, data, UINT32_MAX);
        data += UINT32_MAX;
        data_len -= UINT32_MAX;
    }
#endif
    return crc32(crc, data, (uint32_t)data_len);
}

#endif
