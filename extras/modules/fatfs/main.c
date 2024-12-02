#include <pspkernel.h>
#include <pspreg.h>
#include <stdio.h>
#include <string.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <pspsysmem_kernel.h>
#include <pspsysevent.h>
#include <pspumd.h>
#include <psprtc.h>
#include <pspinit.h>
#include "systemctrl_private.h"
#include <ark.h>
#include "macros.h"

PSP_MODULE_INFO("ARK_fatfs", 0x1000, 2, 1);

extern PspIoDrvFuncs g_drv_funcs;
extern int power_event_handler(int ev_id, char *ev_name, void *param, int *result);

// 0x00002444
PspIoDrv ms_drv = { "ms", 0x4, 0x200, 0, &g_drv_funcs };
PspIoDrv fatms_drv = {
    .name = "fatms",
    .dev_type = 0x1E0010,
    .unk2 = 1,
    .name2 = "fatfs",
    .funcs = &g_drv_funcs,
};

PspSysEventHandler g_power_event = {
    .size = sizeof(g_power_event),
    .name = "fatfsSysEvent",
    .type_mask = 0x00FFFF00, // both suspend / resume
    .handler = &power_event_handler,
};

unsigned int cpu_suspend_interrupts(void){
    return sceKernelCpuSuspendIntr();
}

void cpu_resume_interrupts(unsigned int mask){
    sceKernelCpuResumeIntr(mask);
}

void _fs_lock(){

}

void _fs_unlock(){

}

void get_fattime(){
    time_t t;
    return sceKernelLibcTime(&t);
}

void* malloc(size_t size){
    return oe_malloc(size);
}

void free(void* ptr){
    oe_free(ptr);
}

int module_start(SceSize args, void* argp)
{
    int ret;

    ret = sceKernelRegisterSysEventHandler(&g_power_event);
    if (ret < 0) return ret;

    ret = sceIoAddDrv(&ms_drv);
    if (ret < 0) return ret;

    ret = sceIoAddDrv(&fatms_drv);
    if (ret < 0) return ret;

    return 0;
}

// 0x0000006C
int module_stop(SceSize args, void *argp)
{
    sceIoDelDrv("ms");
    sceIoDelDrv("fatms");
    sceKernelUnregisterSysEventHandler(&g_power_event);

    return 0;
}