#include <pspsdk.h>
#include <pspkernel.h>
#include <kubridge.h>

#include <stdio.h>
#include <vlf.h>

extern int app_main(int argc, char *argv[]);

int start_thread(SceSize args, void *argp)
{
    SceUID mod;    
    
    mod = sceKernelLoadModule("flash0:/vsh/module/intraFont.prx", 0, NULL);
    mod = sceKernelStartModule(mod, args, argp, NULL, NULL);
    mod = sceKernelLoadModule("flash0:/vsh/module/vlf.prx", 0, NULL);
    mod = sceKernelStartModule(mod, args, argp, NULL, NULL);
    vlfGuiInit(-1, app_main);
    
    return sceKernelExitDeleteThread(0);
}

int module_start(SceSize args, void *argp)
{
    SceUID thid = sceKernelCreateThread("start_thread", start_thread, 0x10, 0x4000, 0, NULL);
    if (thid < 0)
        return thid;

    sceKernelStartThread(thid, args, argp);
    
    return 0;
}

