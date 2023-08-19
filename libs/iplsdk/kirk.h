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

void kirk_hwreset(void);

int kirk1(void *dst, const void *src);
int kirk4(void *dst, const void *src);
int kirk7(void *dst, const void *src);
int kirkE(void *dst);
int kirkF(void *dst);

#ifdef __cplusplus
}
#endif //__cplusplus
