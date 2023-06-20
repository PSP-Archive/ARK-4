#include <pspkernel.h>
#include <kubridge.h>
#include <pspsdk.h>
#include <pspctrl.h>
#include <pspdebug.h>
#include <pspmodulemgr.h>
#include <systemctrl.h>
#include <unistd.h>

PSP_MODULE_INFO("ARKInjector", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER|PSP_THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(4*1024);

void *module_buffer = NULL;
char *module = "/kd/utility.prx";
u32 module_size = 0;

#define printf pspDebugScreenPrintf

int main(int args, char *argv[]) {
	pspDebugScreenInit(); // initialize the debug screen
	pspDebugScreenSetBackColor(0xFFFF0000);  //abgr
	SceUID fd;
	char path[256];
	printf("\nStarting ARK Injector EBOOT\n\n");

	void *buf = malloc(64);
	snprintf(path, sizeof(path), "%s%s", getcwd(buf, 64), "/injector.prx");
	printf("CWD: %s\n", getcwd(buf, 64));
	fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
	if(fd < 0) goto broke;


	printf("Path: %s\n\n", path);
	printf("sceIoOpen: %d\n\n", fd);

	module_size = sceIoLseek(fd, 0, PSP_SEEK_END);
	printf("Module Size: %d\n\n", module_size);
	module_buffer = malloc(sizeof(module_size));

	if(module_buffer == NULL) {
		printf("Failed to malloc(%p)\n", module_buffer);
		SceCtrlData pad;

		while(1) {
			broke:
			printf("YOU DIDN'T SAY THE MAGIC WORD!\n");
			sceKernelDelayThread(120000);
			sceCtrlPeekBufferPositive(&pad, 1);
			if (pad.Buttons & PSP_CTRL_CIRCLE) break;
		}	
		sceIoClose(fd);
	}
	printf("buf -> %p\n\n", module_buffer);
	sceIoLseek(fd, 0, PSP_SEEK_SET);

	sceIoRead(fd, module_buffer, module_size);

	//memcpy(module_buffer, fd, module_size);
	
	// Not working within and EBOOT? 
	sctrlHENLoadModuleOnReboot(module, module_buffer, module_size, 1 | 2 | 4 | 8 | 64); 
	//pspSdkLoadStartModule(path, PSP_MEMORY_PARTITION_KERNEL); // not avaible pre-boot, but does actually load prx
	printf("-- sctrlHENLoadModuleOnReboot triggered!.\n");
	sctrlKernelExitVSH(NULL);


//	sceKernelExitGame();
	//sceKernelExitVSHVSH(NULL);
	//printf("-- sctrlKernelExitVSH() triggered!\n\nReturn status of sctrlKernelExitVSH: %d\n\n\n", ret);

	sceIoClose(fd);

	SceCtrlData pad;

	while(1) {
		sceCtrlPeekBufferPositive(&pad, 1);
		if (pad.Buttons & PSP_CTRL_CIRCLE) break;
	}

	sceKernelExitGame();

	return 0;
}

// Exit Point
int module_stop(SceSize args, void * argp)
{
    // Return Success
    return 0;
}
