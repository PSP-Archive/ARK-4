#include <pspsdk.h>
#include <pspdebug.h>
#include <pspiofilemgr.h>
#include <pspctrl.h>
#include <pspinit.h>
#include <pspdisplay.h>
#include <pspkernel.h>
#include <psputility.h>
#include <pspsuspend.h>
#include <psppower.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>

#include "globals.h"
#include "rebootconfig.h"
#include "functions.h"
#include "graphics.h"

PSP_MODULE_INFO("ARK PRO Updater", 0x0800, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VFPU);

#define FW_661 0x06060110
#define FW_660 0x06060010

extern void extractFlash0Archive(SceSize args, void** argp);

char* arkpath = "ms0:/PSP/SAVEDATA/ARK_01234/";

// for flash installer
static KernelFunctions _k_tbl = {
    .KernelDelayThread = sceKernelDelayThread,
    .IoAssign = sceIoAssign,
    .IoUnassign = sceIoUnassign,
    .KernelIOOpen = sceIoOpen,
    .KernelIORead = sceIoRead,
    .KernelIOWrite = sceIoWrite,
    .KernelIOLSeek = sceIoLseek,
    .KernelIOClose = sceIoClose,
    .KernelExitThread = sceKernelExitGame,
};
KernelFunctions* k_tbl = &_k_tbl;

static int fileRellocator(char* filename){
    // redirect ARK files to PRO's
    if (strcmp(filename, "/kd/ark_systemctrl.prx")==0){
        strcpy(filename, "/kd/_systemctrl.prx");
        return 0;
    }
    else if (strcmp(filename, "/kd/ark_vshctrl.prx")==0){
        strcpy(filename, "/kd/_vshctrl.prx");
        return 0;
    }
    else if (strcmp(filename, "/kd/ark_inferno.prx")==0){
        strcpy(filename, "/kd/_inferno.prx");
        return 0;
    }
    else if (strcmp(filename, "/kd/ark_stargate.prx")==0){
        strcpy(filename, "/kd/_stargate.prx");
        return 0;
    }
    else if (strcmp(filename, "/kd/ark_popcorn.prx")==0){
        strcpy(filename, "/kd/_popcorn.prx");
        return 0;
    }
    return 1; // skip file
}

int main(int argc, char** argv){

    initScreen(&sceDisplaySetFrameBuf);
    
    PRTSTR("ARK PRO Updater");

    // set install path device (makes it compatible with ef0)    
    char* cwd = argv[0];
    arkpath[0] = cwd[0];
    arkpath[1] = cwd[1];
    
    u32 psp_fw_version = sceKernelDevkitVersion();
    if((psp_fw_version == FW_660) || (psp_fw_version == FW_661)) {
		PRTSTR("Installing FLASH0.ARK");
        // create parameters
        char archive[ARK_PATH_SIZE];
        strcpy(archive, arkpath);
        strcat(archive, FLASH0_ARK);
        void* args[3] = {(void*)archive, (void*)&fileRellocator, (void*)&PRTSTR11};
        // start thread and wait for it to end
        extractFlash0Archive(sizeof(void*)*3, args);
	}
	else{
	    PRTSTR("Error: only for firmware 6.60 or 6.61");
	}

    sceKernelExitGame();
    return 0;
}
