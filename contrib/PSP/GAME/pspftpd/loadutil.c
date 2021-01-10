#include <psptypes.h>
#include <pspkernel.h>
#include <pspctrl.h>
#include <pspdisplay.h>

#include <pspiofilemgr.h>
#include <pspiofilemgr_fcntl.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "loadutil.h"

////////////////////////////////////////////////////////////////////
// SLIME NOTE: Module library linkage trickery
//  we need to be in special kernel mode to load flash0: libraries
//   (created kernel thread + kernel memory module flag set)
//  for everything else program runs in user mode
//  the system does not patch the stubs properly, so we do it ourselves

SceUID LoadAndStartAndPatch(SceModuleInfo* modInfoPtr, const char* szFile)
    // return oid or error code
{
    SceUID oid;
    int err;
    int startStatus; // ignored

    oid = sceKernelLoadModule(szFile, 0, NULL);
    if (oid & 0x80000000)
        return oid; // error code

    err = sceKernelStartModule(oid, 0, 0, &startStatus, 0);
    if (err != oid)
        return err;

    return oid;
}


extern void sceKernelIcacheInvalidateAll();

void FlushCaches()
{
    // not sure if these are necessary, but to be extra safe
    sceKernelDcacheWritebackAll();
    // sceKernelIcacheInvalidateAll();
}
