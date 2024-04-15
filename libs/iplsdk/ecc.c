#include "ecc.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#define ECC_ERR_BIT_SIZE    (6)
#define ECC_ERR_BIT_MASK    (0x3F)

static uint32_t parity(uint64_t value)
{
    value ^= value >> 32;
    value ^= value >> 16;
    value ^= value >> 8;
    value ^= value >> 4;
    return (0x6996 >> (value & 0xF)) & 1U;
}

static uint32_t calc_ecc_64(uint64_t data)
{
    uint32_t ecc = 0;

    ecc = (ecc << 1) | parity(data & 0xFFFFFFFF00000000);
    ecc = (ecc << 1) | parity(data & 0xFFFF0000FFFF0000);
    ecc = (ecc << 1) | parity(data & 0xFF00FF00FF00FF00);
    ecc = (ecc << 1) | parity(data & 0xF0F0F0F0F0F0F0F0);
    ecc = (ecc << 1) | parity(data & 0xCCCCCCCCCCCCCCCC);
    ecc = (ecc << 1) | parity(data & 0xAAAAAAAAAAAAAAAA);
    ecc = (ecc << 1) | parity(data & 0x00000000FFFFFFFF);
    ecc = (ecc << 1) | parity(data & 0x0000FFFF0000FFFF);
    ecc = (ecc << 1) | parity(data & 0x00FF00FF00FF00FF);
    ecc = (ecc << 1) | parity(data & 0x0F0F0F0F0F0F0F0F);
    ecc = (ecc << 1) | parity(data & 0x3333333333333333);
    ecc = (ecc << 1) | parity(data & 0x5555555555555555);

    return ecc;
}

static enum EccError ecc_verify_correct_common(uint8_t *data, uint32_t ecc, int do_correction)
{
    uint64_t data64;
    memcpy(&data64, data, sizeof(data64));

    uint32_t calc_ecc = calc_ecc_64(data64);

    if (ecc != calc_ecc) {
        // shrink the error down to 6 bits
        uint32_t error = ecc ^ calc_ecc;
        error = ((error >> ECC_ERR_BIT_SIZE) ^ error) & ECC_ERR_BIT_MASK;

        // check for max error
        if (error == ECC_ERR_BIT_MASK) {
            if (do_correction) {
                data[7] ^= 0x80;
            }

            return ECC_ERR_UNK1;
        }

        // count number of modified bits
        size_t numbits = 0;
        for (size_t i = 0; i < 6; ++i) {
            numbits += (error >> i) & 1;
        }

        if (numbits == 1) {
            return ECC_ERR_CORRECTABLE;
        }

        return ECC_ERR_UNCORRECTABLE;
    }

    return ECC_ERR_NONE;
}

uint32_t calculate_ecc(void *data)
{
    uint64_t data64;
    memcpy(&data64, data, sizeof(data64));
    return calc_ecc_64(data64);
}

enum EccError correct_ecc(void *data, uint32_t ecc)
{
    return ecc_verify_correct_common((uint8_t *)data, ecc, 1);
}
