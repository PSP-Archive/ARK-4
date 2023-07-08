#include <sdk.h>
#include <psploadexec.h>
#include <psploadexec_kernel.h>
#include <psputility_modules.h>
#include <module2.h>
#include <macros.h>
#include <systemctrl_se.h>
#include <string.h>
#include <functions.h>
#include "libs/graphics/graphics.h"
#include <rebootconfig.h>

#include "core/compat/vitapops/rebootex/payload.h"

ARKConfig _ark_config = {
    .magic = ARK_CONFIG_MAGIC,
    .arkpath = DEFAULT_ARK_PATH, // only ms0 available anyways
    .launcher = ARK_XMENU, // use xMenu
    .exec_mode = PSV_POPS, // set to VitaPops mode
    .exploit_id = "ePSX", // ps1 loader name
    .recovery = 0,
};
ARKConfig* ark_config = &_ark_config;

extern int (* _LoadReboot)(void *, unsigned int, void *, unsigned int);
extern void buildRebootBufferConfig(int rebootBufferSize);
extern int LoadReboot(void * arg1, unsigned int arg2, void * arg3, unsigned int arg4);

extern u8* rebootbuffer;
extern u32 size_rebootbuffer;
extern int iso_mode;

u32 sctrlHENFindFunction(char* mod, char* lib, u32 nid){
    return FindFunction(mod, lib, nid);
}

int reboot_thread(int argc, void* argv){

    // launcher reboot
    char menupath[ARK_PATH_SIZE];
    strcpy(menupath, ark_config->arkpath);
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

void dumpbuf(char* path, void* buf, int size){
    int fd = k_tbl->KernelIOOpen(path, PSP_O_WRONLY|PSP_O_CREAT|PSP_O_TRUNC, 0777);
    k_tbl->KernelIOWrite(fd, buf, size);
    k_tbl->KernelIOClose(fd);
}

int exploitEntry() __attribute__((section(".text.startup")));
int exploitEntry(){
    if (!isKernel()){
        return; // we don't have kernel privilages? better error out than crash
    }

    clearBSS();

    // Switch to Kernel Permission Level
    setK1Kernel();

    PRTSTR("ARK-X Loader Started");

    scanArkFunctions(g_tbl);

    g_tbl->config = ark_config;

    // make PRTSTR available for payloads
    g_tbl->prtstr = (void *)&PRTSTR11;

    initScreen(NULL);
    initVitaPopsVram();
    setScreenHandler(&copyPSPVram);

    PRTSTR("Scanning kernel functions");
    // get kernel functions
    scanKernelFunctions(k_tbl);

    //k_tbl->KernelIOClose(500);

    //dumpbuf("ms0:/IoRead.bin", 0x8805769c, sizeof(void*)); // 0x001B32C0 -> 0x00003021
    //dumpbuf("ms0:/IoWrite.bin", 0x880577b0, sizeof(void*)); // 0x001B32C0 -> 0x00003021

    PRTSTR("Patching FLASH0");
    patchKermitPeripheral(k_tbl);

    // Invalidate Cache
    k_tbl->KernelDcacheWritebackInvalidateAll();
    k_tbl->KernelIcacheInvalidateAll();

    PRTSTR("Preparing reboot.");
    // Find LoadExec Module
    SceModule2 * loadexec = k_tbl->KernelFindModuleByName("sceLoadExec");
    // Find Reboot Loader Function
    _LoadReboot = (void *)loadexec->text_addr;

    rebootbuffer = rebootbuffer_vitapops;
    size_rebootbuffer = size_rebootbuffer_vitapops;

    PRTSTR("Patching Loadexec");
    patchLoadExec(loadexec, (u32)LoadReboot, (u32)FindFunction("sceThreadManager", "ThreadManForKernel", 0xF6427665), 3);

    // Invalidate Cache
    k_tbl->KernelDcacheWritebackInvalidateAll();
    k_tbl->KernelIcacheInvalidateAll();

    SceUID kthreadID = k_tbl->KernelCreateThread( "ark-x-loader", &reboot_thread, 1, 0x20000, PSP_THREAD_ATTR_VFPU, NULL);
    k_tbl->KernelStartThread(kthreadID, 0, NULL);
    k_tbl->waitThreadEnd(kthreadID, NULL);

    return 0;

}

// Fake K1 Kernel Setting
void setK1Kernel(void){
    // Set K1 to Kernel Value
    __asm__ (
        "nop\n"
        "lui $k1,0x0\n"
    );
}

void clearBSS(void){
    // BSS Start and End Address from Linkfile
    extern char __bss_start, __bss_end;
    
    // Clear Memory
    memset(&__bss_start, 0, &__bss_end - &__bss_start);
}