#include <pspsdk.h>
#include <pspdebug.h>
#include <pspiofilemgr.h>
#include <pspctrl.h>
#include <pspinit.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include "globals.h"

PSP_MODULE_INFO("ARKInstaller", 0x800, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VSH | PSP_THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(4096);

#define BUF_SIZE 16*1024
#define KERNELIFY(a) ((u32)a|0x80000000)

struct {
    char* orig;
    char* dest;
} flash_files[] = {
    {IDSREG_PRX, IDSREG_PRX_FLASH},
    {XMBCTRL_PRX, XMBCTRL_PRX_FLASH},
    {USBDEV_PRX, USBDEV_PRX_FLASH},
    {VSH_MENU, VSH_MENU_FLASH},
    {RECOVERY_PRX, RECOVERY_PRX_FLASH},
    {UPDATER_FILE, UPDATER_FILE_FLASH},
};


static const int N_FLASH_FILES = (sizeof(flash_files)/sizeof(flash_files[0]));

void uninstall() {
	
	pspDebugScreenClear();
	open_flash();
	for (int i=0; i<N_FLASH_FILES; i++){
		if(!sceIoRead(flash_files[i].dest, PSP_O_RDONLY, 0777)) {
			return;
		}
		else {
			pspDebugScreenPrintf("Removing %s\n", flash_files[i].dest);
			rm_file(flash_files[i].dest);
		}
    }

    // Kill Main Thread
    sceKernelExitGame();


	return;
}


// Entry Point
int main(int argc, char * argv[])
{
	

    ARKConfig ark_config;

    sctrlHENGetArkConfig(&ark_config);
    
    // Initialize Screen Output
    pspDebugScreenInit();

	SceCtrlData pad;
	pspDebugScreenPrintf("ARK-4 Full Installer\n");
	pspDebugScreenPrintf("Press (X) to install\n");
	pspDebugScreenPrintf("Press (O) to uninstall\n");
	pspDebugScreenPrintf("Press (R Trigger) to quit\n");

	while(1) {
		sceCtrlReadBufferPositive(&pad, 1);
		if(pad.Buttons & PSP_CTRL_CIRCLE) 
			uninstall();
		else if(pad.Buttons & PSP_CTRL_CROSS) 
			break;
		else if(pad.Buttons & PSP_CTRL_RTRIGGER)
			sceKernelExitGame();

	}
	pspDebugScreenClear();
    if (ark_config.magic != ARK_CONFIG_MAGIC){
        pspDebugScreenPrintf("ERROR: not running ARK\n");
        while (1){};
    }

    pspDebugScreenPrintf("ARK Full Installer Started\n");

    u32 my_ver = (ARK_MAJOR_VERSION << 16) | (ARK_MINOR_VERSION << 8) | ARK_MICRO_VERSION;
    u32 cur_ver = sctrlHENGetMinorVersion();
    int major = (cur_ver&0xFF0000)>>16;
	int minor = (cur_ver&0xFF00)>>8;
	int micro = (cur_ver&0xFF);

    pspDebugScreenPrintf("Version %d.%d.%.2i\n", major, minor, micro);

    pspDebugScreenPrintf("Opening flash0 for writing\n");
    open_flash();

    for (int i=0; i<N_FLASH_FILES; i++){
        char path[ARK_PATH_SIZE];
        strcpy(path, ark_config.arkpath);
        strcat(path, flash_files[i].orig);
        pspDebugScreenPrintf("Installing %s to %s\n", flash_files[i].orig, flash_files[i].dest);
        copy_file(path, flash_files[i].dest);
    }

    // Kill Main Thread
    sceKernelExitGame();

    // Exit Function
    return 0;
}

// Exit Point
int module_stop(SceSize args, void * argp)
{
    // Return Success
    return 0;
}

void open_flash(){
    while(sceIoUnassign("flash0:") < 0) {
        sceKernelDelayThread(500000);
    }
    while (sceIoAssign("flash0:", "lflash0:0,0", "flashfat0:", 0, NULL, 0)<0){
        sceKernelDelayThread(500000);
    }
    while(sceIoUnassign("flash1:") < 0) {
        sceKernelDelayThread(500000);
    }
    while (sceIoAssign("flash1:", "lflash0:0,1", "flashfat1:", 0, NULL, 0)<0){
        sceKernelDelayThread(500000);
    }
}

void copy_file(char* orig, char* dest){
    static u8 buf[BUF_SIZE];
    int fdr = sceIoOpen(orig, PSP_O_RDONLY, 0777);
    int fdw = sceIoOpen(dest, PSP_O_WRONLY|PSP_O_CREAT|PSP_O_TRUNC, 0777);
    while (1){
        int read = sceIoRead(fdr, buf, BUF_SIZE);
        if (read <= 0) break;
        sceIoWrite(fdw, buf, read);
    }
    sceIoClose(fdr);
    sceIoClose(fdw);
}

void rm_file(char* dest){
    sceIoRemove(dest);
}
