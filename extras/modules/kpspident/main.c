#include <pspsdk.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <psploadcore.h>
#include <pspiofilemgr.h>
#include <pspdisplay.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

PSP_MODULE_INFO("kpspident", 0x1006, 1, 0);
PSP_MAIN_THREAD_ATTR(0);

u32 sceSyscon_driver_7EC5A957(u32 *baryon);
u32 sceSyscon_driver_E7E87741(u32 *pommel);
u32 sceSysreg_driver_E2A5D1EE(void);
int sceSysregKirkBusClockEnable(void);
int sceSysregAtaBusClockEnable(void);

u32 sceSysconGetBaryonVersion(u32 *baryon) {
    int k1 = pspSdkSetK1(0);
    int err = sceSyscon_driver_7EC5A957(baryon);
    pspSdkSetK1(k1);
    return err;
}

u32 sceSysconGetPommelVersion(u32 *pommel) {
    int k1 = pspSdkSetK1(0);
    int err = sceSyscon_driver_E7E87741(pommel);
    pspSdkSetK1(k1);
    return err;
}

u32 sceSysregGetTachyonVersion(void) {
    int k1 = pspSdkSetK1(0);
    int err = sceSysreg_driver_E2A5D1EE();
    pspSdkSetK1(k1);
    return err;
}

u32 pspGetKirkVersion(void) {
    int k1 = pspSdkSetK1(0);
    sceSysregKirkBusClockEnable();
    sceKernelDelayThread(1000);
    int err = *(u32 *)0xBDE00004;
    pspSdkSetK1(k1);
    return err;
}

u32 pspGetSpockVersion(void) {
    int k1 = pspSdkSetK1(0);
    sceSysregAtaBusClockEnable();
    sceKernelDelayThread(1000);
    int err = *(u32 *)0xBDF00004;
    pspSdkSetK1(k1);
    return err;
}

int module_start(SceSize args, void *argp)
{
    return 0;
}

int module_stop(void)
{
    return 0;
}
