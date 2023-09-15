#include <pspsdk.h>
#include <pspdebug.h>
#include <pspiofilemgr.h>
#include <pspctrl.h>
#include <pspinit.h>
#include <pspdisplay.h>
#include <pspkernel.h>
#include <psputility.h>
#include <pspsuspend.h>
#include <psputilsforkernel.h>
#include <psppower.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>

#include "globals.h"

PSP_MODULE_INFO("ARK VitaPOPS Loader", 0, 1, 0);

int psxloader_thread(int argc, void* argv){

    // trigger virtual exploit in ps1cfw_enabler
    int res = sceIoOpen("ms0:/__dokxploit__", 0, 0);

    if (res < 0){
        sceKernelExitGame();
        return 0;
    }

    // open ARKX binloader
    SceUID fd = sceIoOpen(DEFAULT_ARK_PATH ARKX_BIN, PSP_O_RDONLY, 0);

    if (fd < 0){
        sceKernelExitGame();
        return 0;
    }

    // read binloader into KRAM at 0x88380000
    sceIoRead(fd, (void *)0x88380000, 0x80000);
    sceIoClose(fd);
    sceKernelDcacheWritebackAll();

    // execute main function
    int (*libctime)(u32, u32) = &sceKernelLibcTime;
    libctime(0x08800000, 0x88380000);

    return 0;
}

int module_start(SceSize args, void* argp)
{

    // loader needs to be in a thread
    int thid = sceKernelCreateThread("psxloader", &psxloader_thread, 0x10, 0x20000, PSP_THREAD_ATTR_USER|PSP_THREAD_ATTR_VFPU, NULL);
    sceKernelStartThread(thid, 0, NULL);

    return 0;
}