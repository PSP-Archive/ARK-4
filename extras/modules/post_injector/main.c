#include <pspkernel.h>
#include <pspsdk.h>
#include <stdio.h>

int TSRThread(SceSize args, void *argp);

/* Define the module info section */
PSP_MODULE_INFO("POST ARK Injector", 0, 2, 2);

/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

int module_start(int argc, char *argv[])
{
	SceUID thid;

	thid = sceKernelCreateThread("postInjector_Thread", TSRThread, 16 , 0x1000 ,0 ,0);

	if (thid>=0) {
		sceKernelStartThread(thid, 0, 0);
	}
	
	return 0;
}

int module_stop(int argc, char *argv[])
{
	return 0;
}


int TSRThread(SceSize args, void *argp)
{
	sceKernelChangeThreadPriority(0, 8);
	void *buf = NULL;
	int mod_size, fd = 0;
	char *module = "/kd/usersystemlib.prx";
	char *path;
	snprintf(path, sizeof(path), "%s", "ms0:/proshell.prx");
	
	fd = sceIoOpen(path, PSP_O_RDONLY, 0);

	mod_size = sceIoLseek(fd, 0, PSP_SEEK_END);

	buf =  malloc(mod_size);

	sceIoLseek(fd, 0, PSP_SEEK_SET);

	sceIoRead(fd, buf, mod_size);

	sctrlHENLoadModuleOnReboot(module, buf, mod_size, 1 | 2 | 3 | 4 | 8 | 64);

	sctrlKernelExitVSH(NULL);

	return sceKernelExitDeleteThread(0);
}
