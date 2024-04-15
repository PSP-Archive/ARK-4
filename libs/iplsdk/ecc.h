#pragma once

#include <stdint.h>

enum EccError {
    ECC_ERR_NONE = 0,
    ECC_ERR_UNK1 = -1, // uncorrectable?
    ECC_ERR_CORRECTABLE = -2,
    ECC_ERR_UNCORRECTABLE = -3,
};

uint32_t calculate_ecc(void *data);
enum EccError correct_ecc(void *data, uint32_t ecc);
