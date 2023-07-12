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
extern void copyPSPVram(u32*);

extern u8* rebootbuffer;
extern u32 size_rebootbuffer;
extern int iso_mode;

int (* _KernelLoadExecVSHWithApitype)(int, char *, struct SceKernelLoadExecVSHParam *, int);

u32 sctrlHENFindFunction(char* mod, char* lib, u32 nid){
    return FindFunction(mod, lib, nid);
}

int reboot_thread(int argc, void* argv){

    // launcher reboot
    char menupath[ARK_PATH_SIZE];
    strcpy(menupath, ark_config->arkpath);
    strcat(menupath, ark_config->launcher);
    //char* menupath = "ms0:/PSP/SAVEDATA/ARK_01234/GAME.BIN";

    struct SceKernelLoadExecVSHParam param;
    memset(&param, 0, sizeof(param));
    param.size = sizeof(param);
    param.args = strlen(menupath) + 1;
    param.argp = menupath;
    param.key = "game";

    PRTSTR1("Running Menu at %s", menupath);
    int res = _KernelLoadExecVSHWithApitype(0x141, menupath, &param, 0x10000);
}



void loadstart_pops(){
    int (*LoadModule)() = FindFunction("sceModuleManager", "ModuleMgrForKernel", 0x939E4270);
    int (*StartModule)() = FindFunction("sceModuleManager", "ModuleMgrForKernel", 0x3FF74DF1);

    int modid = LoadModule("flash0:/kd/pops_01g.prx", 0, NULL);
    if (modid < 0){
        cls();
        PRTSTR1("modid: %p", modid);
        _sw(0, 0);
    }
    int res = StartModule(modid, 0, NULL, NULL, NULL);
    if (res < 0){
        cls();
        PRTSTR1("res: %p\n", res);
        _sw(0, 0);
    }

}

int exploitEntry() __attribute__((section(".text.startup")));
int exploitEntry(){
    if (!isKernel()){
        return; // we don't have kernel privilages? better error out than crash
    }

    clearBSS();

    // Switch to Kernel Permission Level
    setK1Kernel();

    loadstart_pops();

    int (*DisplaySetHoldMode)(int) = FindFunction("sceDisplay_Service", "sceDisplay", 0x7ED59BC4);
    DisplaySetHoldMode(0);

    PRTSTR("ARK-X Loader Started");

    scanArkFunctions(g_tbl);

    g_tbl->config = ark_config;

    // make PRTSTR available for payloads
    g_tbl->prtstr = (void *)&PRTSTR11;

    initVitaPopsVram();
    setScreenHandler(&copyPSPVram);
    initScreen(NULL);

    PRTSTR("Scanning kernel functions");
    // get kernel functions
    scanKernelFunctions(k_tbl);

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
    u32 getuserlevel = FindFunction("sceThreadManager", "ThreadManForKernel", 0xF6427665);
    patchLoadExec(loadexec, (u32)LoadReboot, getuserlevel, 3);

    /*
    SceModule2* modman = k_tbl->KernelFindModuleByName("sceModuleManager");
    for (u32 addr=modman->text_addr; addr<modman->text_addr+modman->text_size; addr+=4){
        u32 data = _lw(addr);
        if (data == JUMP(getuserlevel)){
            _sw(JR_RA, addr); // patch sceKernelGetUserLevel stub to make it return 1
            _sw(LI_V0(1), addr + 4);
            break;
        }
        else if (data == 0x37258001){
            u32 call = _lw(addr+16);
            _sw(0x24020000, addr+16); // MODULEMGR_DEVICE_CHECK_1
            int found = 0;
            for (addr+=20; !found; addr+=4){
                if (_lw(addr) == call){
                    _sw(0x24020000, addr); // MODULEMGR_DEVICE_CHECK_2
                    found=1;
                }
            }
        }
        else if (data == 0x34458003 || data == 0x34458006){
            addr = patchDeviceCheck(addr);
        }
    }
    */

    // patch IO checks
    //_sw(JR_RA, loadexec->text_addr + 0x0000222C);
    //_sw(LI_V0(0), loadexec->text_addr + 0x00002230);

    _KernelLoadExecVSHWithApitype = (void *)findFirstJALForFunction("sceLoadExec", "LoadExecForKernel", 0xD8320A28);

    // Invalidate Cache
    k_tbl->KernelDcacheWritebackInvalidateAll();
    k_tbl->KernelIcacheInvalidateAll();

    //int thid = k_tbl->KernelCreateThread("simpleloader", &simpleloader_thread, 0x10, 0x20000, PSP_THREAD_ATTR_VFPU, NULL);
    //k_tbl->KernelStartThread(thid, 0, NULL);

    k_tbl->KernelDelayThread(1000000);

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

void setK1User(void){
    // Set K1 to Kernel Value
    __asm__ (
        "nop\n"
        "lui $k1,0x10\n"
    );
}

void clearBSS(void){
    // BSS Start and End Address from Linkfile
    extern char __bss_start, __bss_end;
    
    // Clear Memory
    memset(&__bss_start, 0, &__bss_end - &__bss_start);
}