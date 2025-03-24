#include <pspsdk.h>
#include <pspkernel.h>
#include <kubridge.h>

#include <stdio.h>
#include <string.h>
#include <vlf.h>

extern char boot_path[256];
extern int app_main(int argc, char *argv[]);

int start_thread(SceSize args, void *argp)
{
    SceUID mod;
    char *path = (char *)argp;
    int last_trail = -1;
    int i = 0;

    if (path)
    {
        for (i = 0; path[i]; i++)
        {
        	boot_path[i] = path[i];
        	if (path[i] == '/')
        		last_trail = i;
        }
    }
    
    boot_path[i] = 0;

    if (last_trail >= 0)
        boot_path[last_trail] = 0;

    sceIoChdir(boot_path);

    mod = kuKernelLoadModule("iop.prx", 0, NULL);
    mod = sceKernelStartModule(mod, args, argp, NULL, NULL);
    mod = kuKernelLoadModule("intraFont.prx", 0, NULL);
    mod = sceKernelStartModule(mod, args, argp, NULL, NULL);
    mod = kuKernelLoadModule("vlf.prx", 0, NULL);
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

