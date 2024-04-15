#pragma once

#include "sysreg.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

typedef struct
{
    unsigned int mode;
    unsigned int unk_4;
    unsigned int unk_8;
    unsigned int keyslot;
    unsigned int size;
} Kirk47Header;

typedef struct
{
    unsigned int mode;
    unsigned int unk_4;
    unsigned int unk_8;
    unsigned int keyslot;
    unsigned int size;
} Kirk58Header;

#define KIRK_MODE_ENCRYPT_CBC 4
#define KIRK_MODE_DECRYPT_CBC 5

void kirk_hwreset(void);

int kirk1(void *dst, const void *src);
int kirk4(void *dst, const void *src);
int kirk5(void *dst, const void *src);
int kirk7(void *dst, const void *src);
int kirkE(void *dst);
int kirkF(void *dst);
int kirk_decrypt_aes(unsigned char *out, unsigned char *data, unsigned int size, unsigned char key_idx);

#ifdef __cplusplus
}
#endif //__cplusplus
