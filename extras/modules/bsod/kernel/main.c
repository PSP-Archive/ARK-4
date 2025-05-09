#include <pspsdk.h>
#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspthreadman.h>
#include <systemctrl.h>
#include <macros.h>
#include <stdio.h>
#include <string.h>
#include <ark.h>
#include "graphics.h"

#include "../user/bsod_user.h"

PSP_MODULE_INFO("BlueScreenOfDeath_Kernel", 0x1007, 1, 0);

// for screen debugging
int (* DisplaySetFrameBuf)(void*, int, int, int) = NULL;

void loadstart_usermod(){

    initScreen(DisplaySetFrameBuf);
    PRTSTR("Starting BSoD");

    int uid = sceKernelLoadModuleBuffer(size_bsod_user, bsod_user, 0, NULL);

    if (uid>=0){
        int res = sceKernelStartModule(uid, 0, NULL, NULL, NULL);
        if (res<0){
            PRTSTR1("ERROR starting user module: %p", res);
            sceKernelDelayThread(10000000);
        }
        else {
            PRTSTR("Ok");
        }
    }
    else {
        PRTSTR1("ERROR loading user module: %p", uid);
        sceKernelDelayThread(10000000);
    }
}

int (*prev_start)(int modid, SceSize argsize, void * argp, int * modstatus, SceKernelSMOption * opt) = NULL;
int StartModuleHandler(int modid, SceSize argsize, void * argp, int * modstatus, SceKernelSMOption * opt){

    static int loaded = 0;

    if (!loaded){
        loadstart_usermod();
        loaded = 1;
    }

    // forward to previous or default StartModule
    if (prev_start) return prev_start(modid, argsize, argp, modstatus, opt);
    return -1;
}

int module_start(){

    DisplaySetFrameBuf = (void*)sctrlHENFindFunction("sceDisplay_Service", "sceDisplay", 0x289D82FE);

    // Register custom start module
    prev_start = sctrlSetStartModuleExtra(StartModuleHandler);

    return 0;
}