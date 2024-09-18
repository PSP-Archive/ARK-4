#include <stdio.h>
#include <pspsdk.h>
#include <pspiofilemgr.h>
#include <psploadexec.h>
#include <psploadexec_kernel.h>
#include <psputility_modules.h>
#include <pspumd.h>
#include <module2.h>
#include <macros.h>
#include <rebootconfig.h>
#include <systemctrl_se.h>
#include <string.h>
#include <ark.h>
#include "functions.h"

// attempt to free as much memory as possible, some kernel/user exploits need this, others don't
void freeMem(){
    register u32 (* sceKernelGetBlockHeadAddr_)(SceUID) = (void *)RelocImport("SysMemUserForUser", 0x9D9A5BA1, 0);
    register int (* sceKernelDeleteFpl_)(SceUID) = (void *)RelocImport("ThreadManForUser", 0xED1410E0, 0);
    register int (* sceKernelTerminateThread_)(SceUID) = (void *)RelocImport("ThreadManForUser", 0x616403BA, 0);
    register int (* sceKernelSuspendThread_)(SceUID) = (void *)RelocImport("ThreadManForUser", 0x9944F31F, 0);

    register u32 addr;

    for (addr = 0x08800000; addr < 0x0A000000; addr += 4) {
        if (IsUID(*(SceUID*)addr)) {
            if ((sceKernelGetBlockHeadAddr_) && (sceKernelGetBlockHeadAddr_(*(SceUID*)addr) > 0))
                g_tbl->KernelFreePartitionMemory(*(SceUID*)addr);

            if (sceKernelTerminateThread_)
                sceKernelTerminateThread_(*(SceUID*)addr);
            else if (sceKernelSuspendThread_)
                sceKernelSuspendThread_(*(SceUID*)addr);
            
            if (sceKernelDeleteFpl_)
                sceKernelDeleteFpl_(*(SceUID*)addr);
        }
    }
}

void unloadUtilities(){
    u16 utilities[] = { PSP_MODULE_NET_ADHOC, PSP_MODULE_USB_PSPCM, PSP_MODULE_USB_MIC, PSP_MODULE_USB_CAM, \
        PSP_MODULE_USB_GPS, PSP_MODULE_AV_AVCODEC, PSP_MODULE_AV_SASCORE, PSP_MODULE_AV_ATRAC3PLUS, \
        PSP_MODULE_AV_MPEGBASE, PSP_MODULE_AV_MP3, PSP_MODULE_AV_VAUDIO, PSP_MODULE_AV_AAC, PSP_MODULE_AV_G729, \
        PSP_MODULE_NP_COMMON, PSP_MODULE_NP_SERVICE, PSP_MODULE_NP_MATCHING2, PSP_MODULE_NP_DRM
    };
    
    for (int i=0; i<NELEMS(utilities); i++){
        g_tbl->UtilityUnloadModule(utilities[i]);
    }
}

