#include "main.h"
#include <functions.h>
#include <loadexec_patch.h>
#include "reboot.h"

extern u8 rebootbuffer[REBOOTEX_MAX_SIZE];

// Sony Reboot Buffer Loader
int (* _LoadReboot)(void *, unsigned int, void *, unsigned int) = NULL;

// LoadExecVSHWithApitype Direct Call
int (* _KernelLoadExecVSHWithApitype)(int, char *, struct SceKernelLoadExecVSHParam *, int) = NULL;

static int isVitaFile(char* filename){
    return (strstr(filename, "psv")!=NULL // PS Vita btcnf replacement, not used on PSP
            || strstr(filename, "660")!=NULL // PSP 6.60 modules can be used on Vita, not needed for PSP
            || strstr(filename, "vita")!=NULL // Vita modules
            || strcmp(filename, "/fake.cso")==0 // fake.cso used on Vita to simulate UMD drive when no ISO available
    );
}

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
            void* args[3] = {(void*)archive, (void*)&isVitaFile, (void*)KERNELIFY(&PRTSTR11)};
            // start thread and wait for it to end
            k_tbl->KernelStartThread(kthreadID, sizeof(void*)*3, &args);
            k_tbl->waitThreadEnd(kthreadID, NULL);
            k_tbl->KernelDeleteThread(kthreadID);
            // delete archive
            //k_tbl->KernelIORemove(archive);
        }
    }
    else{ // Patching flash0 on Vita
        PRTSTR("Installing on PS Vita");
        patchKermitPeripheral(k_tbl);
    }
}

void setupRebootBuffer(){
    char path[ARK_PATH_SIZE];
    strcpy(path, ark_config->arkpath);
    strcat(path, "REBOOT.BIN");
    
    int fd = k_tbl->KernelIOOpen(path, PSP_O_RDONLY, 0777);
    if (fd >= 0){ // read external rebootex
        k_tbl->KernelIORead(fd, rebootbuffer, REBOOTEX_MAX_SIZE);
        k_tbl->KernelIOClose(fd);
    }
    else{ // error
        PRTSTR("ERROR: Could not read REBOOT.BIN");
        while (1);
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

    PRTSTR("Preparing reboot.");
    // Find LoadExec Module
    SceModule2 * loadexec = k_tbl->KernelFindModuleByName("sceLoadExec");
    
    
    // Find Reboot Loader Function
    _LoadReboot = (void *)loadexec->text_addr;    
    
    setupRebootBuffer();
    
    // make the common loadexec patches
    patchLoadExec(loadexec, (u32)LoadReboot, (u32)FindFunction("sceThreadManager", "ThreadManForKernel", 0xF6427665), 3);
    
    // Invalidate Cache
    k_tbl->KernelDcacheWritebackInvalidateAll();
    k_tbl->KernelIcacheInvalidateAll();
    
    if (IS_VITA(ark_config)){
        // Prepare Homebrew Reboot
        char menupath[ARK_PATH_SIZE];
        strcpy(menupath, ark_config->arkpath);
        if (IS_VITA_POPS(ark_config)){
            strcpy(ark_config->launcher, ARK_XMENU);
            strcat(menupath, ark_config->launcher);
        }
        else if (ark_config->recovery){
            strcat(menupath, ARK_RECOVERY);
            ark_config->recovery = 0;
        }
        else
            strcat(menupath, ARK_MENU);
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
