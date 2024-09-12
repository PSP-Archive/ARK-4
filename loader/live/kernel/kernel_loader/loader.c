#include "main.h"
#include <functions.h>
#include <loadexec_patch.h>
#include "reboot.h"

#include "core/compat/vita/vitaflash.h"
#include "core/compat/pentazemin/adrenaline_compat.h"

#include "core/compat/psp/rebootex/payload.h"
#include "core/compat/vita/rebootex/payload.h"
#include "core/compat/vitapops/rebootex/payload.h"
#include "core/compat/pentazemin/rebootex/payload.h"

extern char* kbin_path;
extern ARKConfig* ark_config;
extern int extractFlash0Archive();

static int isVitaFile(char* filename){
    return (strstr(filename, "psv")!=NULL // PS Vita btcnf replacement, not used on PSP
            || strstr(filename, "660")!=NULL // PSP 6.60 modules can be used on Vita, not needed for PSP
            || strstr(filename, "vita")!=NULL // Vita modules
    );
}

void flashPatch(){
    char archive[ARK_PATH_SIZE];
    strcpy(archive, ark_config->arkpath);
    strcat(archive, FLASH0_ARK);

    if (IS_VITA_ADR(ark_config)){ // read FLASH0.ARK into RAM
        strcpy(ark_config->exploit_id, "Adrenaline");
        PRTSTR("Reading FLASH0.ARK into RAM");
        int fd = k_tbl->KernelIOOpen(archive, PSP_O_RDONLY, 0777);
        k_tbl->KernelIORead(fd, ARK_FLASH, MAX_FLASH0_SIZE);
        k_tbl->KernelIOClose(fd);
    }
    else if (IS_PSP(ark_config)){ // on PSP, extract FLASH0.ARK into flash0
        PRTSTR("Installing on PSP");
        SceUID kthreadID = k_tbl->KernelCreateThread( "arkflasher", (void*)KERNELIFY(&extractFlash0Archive), 1, 0x20000, PSP_THREAD_ATTR_VFPU, NULL);
        if (kthreadID >= 0){
            // create thread parameters
            void* args[] = {(void*)archive, "flash0:/", (void*)&isVitaFile, (void*)KERNELIFY(&PRTSTR11)};
            // start thread and wait for it to end
            k_tbl->KernelStartThread(kthreadID, sizeof(void*)*4, &args);
            k_tbl->waitThreadEnd(kthreadID, NULL);
            k_tbl->KernelDeleteThread(kthreadID);
            // delete archive on FinalSpeed installs
            if (kbin_path && strstr(kbin_path, "/PSP/APP/")){
                PRTSTR("FinalSpeed Live Install detected, deleting FLASH0.ARK");
                k_tbl->KernelIORemove(archive);
            }
        }
    }
    else{ // Patching flash0 on Vita
        PRTSTR("Installing on PS Vita");
        strcpy(ark_config->exploit_id, "ePSP");
        patchKermitPeripheral(k_tbl);
    }
}

// patch to inject our own rebootex intead of adrenaline's
void patchedmemcpy(void* a1, void* a2, u32 size){
    if ((u32)a1 == 0x88FC0000){ // Rebootex payload
        a2 = rebootbuffer;
        size = size_rebootbuffer;
        if (rebootbuffer[0] == 0x1F && rebootbuffer[1] == 0x8B){ // gzip packed rebootex
            k_tbl->KernelGzipDecompress((unsigned char *)REBOOTEX_TEXT, REBOOTEX_MAX_SIZE, rebootbuffer, NULL);
        }
        else{ // plain rebootex
            memcpy(REBOOTEX_TEXT, rebootbuffer, size_rebootbuffer);
        }
    }
    else if ((u32)a1 == 0x88FB0000){ // Rebootex config
        buildRebootBufferConfig(size_rebootbuffer);
    }
}

// yo dawg, I heard you like patches
// so I made a patch that patches your patch to inject my patch to patch your patches
void patchAdrenalineReboot(SceModule2* loadexec){
    for (u32 addr = loadexec->text_addr; addr < loadexec->text_addr+loadexec->text_size; addr+=4){
        if (_lw(addr) == 0x04400020) {
            // found patch that injects rebootex
            void* DecodeKL4EPatched = K_EXTRACT_CALL(addr-8);
            int calls = 2;
            u32 a = DecodeKL4EPatched;
            while (calls){
                // scan for two JALs (to memcpy) and redirect them
                u32 d = _lw(a);
                if (IS_JAL(d)){
                    _sw(JAL(patchedmemcpy), a);
                    calls--;
                }
                a+=4;
            }
			break;
		}
    }
}

void setupRebootBuffer(){
    if (IS_VITA(ark_config)){
        if (IS_VITA_POPS(ark_config)){
            rebootbuffer = rebootbuffer_vitapops;
            size_rebootbuffer = size_rebootbuffer_vitapops;
        }
        else if (IS_VITA_ADR(ark_config)){
            rebootbuffer = rebootbuffer_pentazemin;
            size_rebootbuffer = size_rebootbuffer_pentazemin;
        }
        else{
            rebootbuffer = rebootbuffer_vita;
            size_rebootbuffer = size_rebootbuffer_vita;
        }
    }
    else{
        rebootbuffer = rebootbuffer_psp;
        size_rebootbuffer = size_rebootbuffer_psp;
    }
}

static void createFolders(char* path){
    char* sep = path;
    while ((sep = strchr(sep, '/')) != NULL){
        sep[0] = '\0';
        k_tbl->KernelIOMkdir(path, 0777);
        sep[0] = '/';
        sep++;
    }
}

void dumpVitaFlash0(){

    int i = 0;
    char path[128];
    VitaFlashBufferFile * f0 = (VitaFlashBufferFile*)FLASH_SONY;
    
    while (f0[i].name != NULL){
        char* filename = f0[i].name;
        void* buf = f0[i].content;
        int buf_size = f0[i].size;
        strcpy(path, "ms0:/flash/0");
        if (filename[0] != '/') strcat(path, "/");
        strcat(path, filename);
        createFolders(path);
        int fd = k_tbl->KernelIOOpen(path, PSP_O_RDONLY, 0777);
        if (fd < 0){
            PRTSTR1("Dumping %s", path);
            fd = k_tbl->KernelIOOpen(path, PSP_O_WRONLY|PSP_O_CREAT|PSP_O_TRUNC, 0777);
            k_tbl->KernelIOWrite(fd, buf, buf_size);
        }
        k_tbl->KernelIOClose(fd);
        i++;
    }
}

void onVitaFlashLoaded(){
    SceIoStat stat;
    if (k_tbl->KernelIOGetStat("ms0:/flash", &stat) < 0){
        k_tbl->KernelIOMkdir("ms0:/flash", 0777);
        k_tbl->KernelIOMkdir("ms0:/flash/0", 0777);
        k_tbl->KernelIOMkdir("ms0:/flash/1", 0777);
        k_tbl->KernelIOMkdir("ms0:/flash/2", 0777);
        k_tbl->KernelIOMkdir("ms0:/flash/3", 0777);
        PRTSTR("Dumping flash0");
        dumpVitaFlash0();
        PRTSTR("Extracting FLASH0.ARK");
        char archive[ARK_PATH_SIZE];
        strcpy(archive, ark_config->arkpath);
        strcat(archive, FLASH0_ARK);
        void* args[] = {(void*)archive, "ms0:/flash/0/", (void*)NULL, (void*)KERNELIFY(&PRTSTR11)};
        // start thread and wait for it to end
        SceUID kthreadID = k_tbl->KernelCreateThread( "arkflasher", (void*)KERNELIFY(&extractFlash0Archive), 1, 0x20000, PSP_THREAD_ATTR_VFPU, NULL);
        k_tbl->KernelStartThread(kthreadID, sizeof(void*)*4, &args);
        k_tbl->waitThreadEnd(kthreadID, NULL);
        k_tbl->KernelDeleteThread(kthreadID);
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
    if (IS_VITA_ADR(ark_config)) patchAdrenalineReboot(loadexec);
    else patchLoadExec(loadexec, (u32)LoadReboot, (u32)FindFunction("sceThreadManager", "ThreadManForKernel", 0xF6427665), 3);
    
    // Invalidate Cache
    k_tbl->KernelDcacheWritebackInvalidateAll();
    k_tbl->KernelIcacheInvalidateAll();

    if (IS_VITA(ark_config) && !IS_VITA_ADR(ark_config)){
        // launcher reboot
        char menupath[ARK_PATH_SIZE];

        strcpy(menupath, ark_config->arkpath);
        if (IS_VITA_POPS(ark_config)){
            strcpy(ark_config->launcher, ARK_XMENU);
        }
        else{
            strcpy(ark_config->launcher, VBOOT_PBP);
        }
        strcat(menupath, ark_config->launcher);

        struct SceKernelLoadExecVSHParam param;
        memset(&param, 0, sizeof(param));
        param.size = sizeof(param);
        param.args = strlen(menupath) + 1;
        param.argp = menupath;
        param.key = "game";

        PRTSTR1("Running Menu at %s", menupath);
        int (* _KernelLoadExecVSHWithApitype)(int, char *, struct SceKernelLoadExecVSHParam *, int);
        _KernelLoadExecVSHWithApitype = (void *)findFirstJALForFunction("sceLoadExec", "LoadExecForKernel", 0xD8320A28);
        _KernelLoadExecVSHWithApitype(0x141, menupath, &param, 0x10000);
    }
    else {
        // vsh reboot
        PRTSTR("Running VSH");
        extern int iso_mode;
        iso_mode = MODE_UMD;
        int (*_KernelExitVSH)(void*) = FindFunction("sceLoadExec", "LoadExecForKernel", 0x08F7166C);
        _KernelExitVSH(NULL);
    }
}
