#ifndef LOADEXEC_PATCH_H
#define LOADEXEC_PATCH_H

#include <sdk.h>
#include <module2.h>
#include <macros.h>
#include <ark.h>

extern void patchLoadExec(SceModule2* loadexec, u32 LoadReboot, u32 GetUserLevel, int k1_checks);

#endif
