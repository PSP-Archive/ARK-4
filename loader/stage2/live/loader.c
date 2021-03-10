#include "main.h"
#include <functions.h>

// Sony Reboot Buffer Loader
int (* _LoadReboot)(void *, unsigned int, void *, unsigned int) = NULL;

// LoadExecVSHWithApitype Direct Call
int (* _KernelLoadExecVSHWithApitype)(int, char *, struct SceKernelLoadExecVSHParam *, int) = NULL;

void flashPatch(){
    extern ARKConfig* ark_config;
    extern int extractFlash0Archive();
    if (IS_PSP(ark_config)){ // on PSP, extract FLASH0.ARK into flash0
        PRTSTR("Installing on PSP");
        SceUID kthreadID = k_tbl->KernelCreateThread( "arkflasher", (void*)KERNELIFY(&extractFlash0Archive), 1, 0x20000, PSP_THREAD_ATTR_VFPU, NULL);
        if (kthreadID >= 0){
            // create thread parameters
            char archive[ARK_PATH_SIZE];
            strcpy(archive, ark_config->arkpath);
            strcat(archive, FLASH0_ARK);
            void* args[2] = {(void*)archive, (void*)KERNELIFY(&PRTSTR11)};
            // start thread and wait for it to end
            k_tbl->KernelStartThread(kthreadID, sizeof(void*)*2, &args);
            k_tbl->waitThreadEnd(kthreadID, NULL);
            k_tbl->KernelDeleteThread(kthreadID);
        }
    }
    else{ // Patching flash0 on Vita
        PRTSTR("Installing on PS Vita");
        patchKermitPeripheral(k_tbl);
    }
}

void loadKernelArk(){
     // Install flash0 files
    PRTSTR("Installing "FLASH0_ARK);
    flashPatch();
    
    // Invalidate Cache
    k_tbl->KernelDcacheWritebackInvalidateAll();
    k_tbl->KernelIcacheInvalidateAll();

    // check for recovery flag (R1 button)
    int (*CtrlPeekBufferPositive)(SceCtrlData *, int) = (void *)FindFunction("sceController_Service", "sceCtrl", 0x3A622550);
    if (CtrlPeekBufferPositive){
        SceCtrlData data;
        memset(&data, 0, sizeof(data));
        CtrlPeekBufferPositive(&data, 1);
        if((data.Buttons & PSP_CTRL_RTRIGGER) == PSP_CTRL_RTRIGGER){
            ark_config->recovery = 1;
        }
    }

    PRTSTR("Patching loadexec");
    // Find LoadExec Module
    SceModule2 * loadexec = k_tbl->KernelFindModuleByName("sceLoadExec");
    
    // Find Reboot Loader Function
    _LoadReboot = (void *)loadexec->text_addr;    
    
    // make the common loadexec patches
    int k1_patches = 2;
    patchLoadExecCommon(loadexec, (u32)LoadReboot, k1_patches);
    
    // Invalidate Cache
    k_tbl->KernelDcacheWritebackInvalidateAll();
    k_tbl->KernelIcacheInvalidateAll();
    
    if (IS_VITA(ark_config)){
        // Prepare Homebrew Reboot
        char menupath[ARK_PATH_SIZE];
        strcpy(menupath, ark_config->arkpath);
        strcat(menupath, (ark_config->recovery)? ARK_RECOVERY : ARK_MENU);
        struct SceKernelLoadExecVSHParam param;
        memset(&param, 0, sizeof(param));
        param.size = sizeof(param);
        param.args = strlen(menupath) + 1;
        param.argp = menupath;
        param.key = "game";
        PRTSTR1("Running Menu at %s", menupath);
        _KernelLoadExecVSHWithApitype = (void *)findFirstJALForFunction("sceLoadExec", "LoadExecForKernel", 0xD8320A28);
        _KernelLoadExecVSHWithApitype(0x141, menupath, &param, 0x10000);
    }
    else {
        PRTSTR("Running VSH");
        int (*_KernelExitVSH)(void*) = FindFunction("sceLoadExec", "LoadExecForKernel", 0x08F7166C);
        _KernelExitVSH(NULL);
    }
}
