#include <pspsdk.h>
#include <pspsysmem_kernel.h>
#include <pspinit.h>
#include <ark.h>
#include <graphics.h>
#include <macros.h>
#include <module2.h>
#include <pspdisplay_kernel.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <systemctrl_private.h>
#include <pspiofilemgr.h>
#include <pspgu.h>
#include <pspsysevent.h>
#include <functions.h>
#include "region_free.h"
#include "libs/graphics/graphics.h"

extern u32 psp_model;
extern ARKConfig* ark_config;
extern SEConfig* se_config;

// Previous Module Start Handler
STMOD_HANDLER previous = NULL;

extern int sceKernelSuspendThreadPatched(SceUID thid);

extern int (*_sctrlHENApplyMemory)(u32);
extern int memoryHandlerPSP(u32 p2);

static int _sceKernelBootFromForUmdMan(void)
{
    return 0x20;
}

void patch_sceUmdMan_driver(SceModule2* mod)
{
    int apitype = sceKernelInitApitype();
    if (apitype == 0x152 || apitype == 0x141) {
        sctrlHookImportByNID(mod, "InitForKernel", 0x27932388, _sceKernelBootFromForUmdMan);
    }
}

//prevent umd-cache in homebrew, so we can drain the cache partition.
void patch_umdcache(SceModule2* mod)
{
    int apitype = sceKernelInitApitype();
    if (apitype == 0x152 || apitype == 0x141){
        //kill module start
        u32 text_addr = mod->text_addr;
        u32 top_addr = text_addr + mod->text_size;
        for (u32 addr=text_addr; addr<top_addr; addr+=4){
            if (_lw(addr) == 0x34440D40){
                MAKE_DUMMY_FUNCTION_RETURN_1(addr+4);
                break;
            }
        }
    }
}

void patch_sceWlan_Driver(SceModule2* mod)
{
    // disable frequency check
    u32 text_addr = mod->text_addr;
    u32 top_addr = text_addr + mod->text_size;
    for (int addr=text_addr; addr<top_addr; addr+=4){
        if (_lw(addr) == 0x35070080){
            _sw(NOP, addr-16);
            break;
        }
    }
}

void patch_scePower_Service(SceModule2* mod)
{
    // scePowerGetBacklightMaximum always returns 4
    u32 text_addr = mod->text_addr;
    u32 top_addr = text_addr + mod->text_size;
    for (u32 addr=text_addr; addr<top_addr; addr+=4){
        if (_lw(addr) == 0x0067280B){
            _sw(NOP, addr-16);
            break;
        }
    }
}

void patch_GameBoot(SceModule2* mod){
    u32 p1 = 0;
    u32 p2 = 0;
    int patches = 2;
    for (u32 addr=mod->text_addr; addr<mod->text_addr+mod->text_size && patches; addr+=4){
        u32 data = _lw(addr);
        if (data == 0x2C43000D){
            p1 = addr-36;
            patches--;
        }
        else if (data == 0x27BDFF20 && _lw(addr-4) == 0x27BD0040){
            p2 = addr-24;
            patches--;
        }
    }
    _sw(JAL(p1), p2);
    _sw(0x24040002, p2 + 4);
}

void disable_PauseGame(SceModule2* mod)
{
    static int pause_disabled = 0;
    if(psp_model == PSP_GO && !pause_disabled) {
        u32 text_addr = mod->text_addr;
        for(int i=0; i<2; i++) {
            _sw(NOP, text_addr + 0x00000574 + i * 4);
        }
        pause_disabled = 1;
    }
}

int sceUmdRegisterUMDCallBackPatched(int cbid) {
    int k1 = pspSdkSetK1(0);
    int res = sceKernelNotifyCallback(cbid, PSP_UMD_NOT_PRESENT);
    pspSdkSetK1(k1);
    return res;
}

static int sceGpioPortReadPatched(void) {
    int GPRValue = *((int *) 0xBE240004);
    GPRValue = GPRValue & 0xFBFFFFFF;
    return GPRValue;
}

int (*_sceSysconCtrlLEDOrig)(int, int);
void disableLEDs(){
    if (se_config->noled){
        int (*_sceSysconCtrlLED)(int, int);
        _sceSysconCtrlLED = sctrlHENFindFunction("sceSYSCON_Driver", "sceSyscon_driver", 0x18BFBE65);
        for (int i=0; i<4; i++) _sceSysconCtrlLED(i, 0);
        static u32 dummy[2] = {JR_RA, LI_V0(0)};
        HIJACK_FUNCTION(_sceSysconCtrlLED, dummy, _sceSysconCtrlLEDOrig);
        sctrlFlushCache();
    }
}

void disableUMD(){
    if (se_config->noumd && psp_model != PSP_GO){
        // disable UMD drive by software, only do this if not running an ISO driver
        if (sceKernelFindModuleByName("PRO_Inferno_Driver")==NULL && sceKernelFindModuleByName("sceNp9660_driver")==NULL){
            // redirect UMD callback to DISC_NOT_PRESENT
            u32 f = sctrlHENFindFunction("sceUmd_driver", "sceUmdUser", 0xAEE7404D);
            if (f){
                REDIRECT_FUNCTION(f, sceUmdRegisterUMDCallBackPatched);
            }
            // remove umd driver
            sceIoDelDrv("umd");
            // force UMD check medium to always return 0 (no medium)
            u32 CheckMedium = sctrlHENFindFunction("sceUmd_driver", "sceUmdUser", 0x46EBB729);
            if (CheckMedium){
                MAKE_DUMMY_FUNCTION_RETURN_0(CheckMedium);
            }
        }
        // patch GPIO to disable UMD drive electrically
        u32 sceGpioPortRead = (void*)sctrlHENFindFunction("sceLowIO_Driver", "sceGpio_driver", 0x4250D44A);
        REDIRECT_FUNCTION(sceGpioPortRead, sceGpioPortReadPatched);
    }
}

void processSettings(){
    int apitype = sceKernelInitApitype();

    // USB Charging
    if (se_config->usbcharge){
        usb_charge(5000000); // enable usb charging
    }
    
    // check launcher mode
    if (se_config->launcher_mode){
        strcpy(ark_config->launcher, VBOOT_PBP); // set CFW in launcher mode
    }
    else{
        if (strcmp(ark_config->launcher, "PROSHELL") != 0)
            ark_config->launcher[0] = 0; // disable launcher mode
    }

    // VSH region
    if (se_config->vshregion) patch_sceChkreg();

    // Disable LED
    disableLEDs();

    // Disable UMD Drive
    disableUMD();
}

void PSPOnModuleStart(SceModule2 * mod){
    // System fully booted Status
    static int booted = 0;

    if (strcmp(mod->modname, "CWCHEATPRX") == 0) {
        if (sceKernelInitKeyConfig() == PSP_INIT_KEYCONFIG_POPS) {
        	sctrlHookImportByNID(mod, "ThreadManForKernel", 0x9944F31F, sceKernelSuspendThreadPatched);
        	goto flush;
        }
    }

    if (strcmp(mod->modname, "sceUmdMan_driver") == 0) {
        patch_sceUmdMan_driver(mod);
        patch_umd_idslookup(mod);
        goto flush;
    }

    if (strcmp(mod->modname, "sceUmdCache_driver") == 0) {
        patch_umdcache(mod);
        goto flush;
    }

    if (strcmp(mod->modname, "sceWlan_Driver") == 0) {
        patch_sceWlan_Driver(mod);
        patch_Libertas_MAC(mod);
        goto flush;
    }

    if (strcmp(mod->modname, "scePower_Service") == 0) {
        patch_scePower_Service(mod);
        goto flush;
    }

    if (strcmp(mod->modname, "sceMediaSync") == 0) {
        // Handle some settings
        processSettings();
        goto flush;
    }

    if (strcmp(mod->modname, "sceImpose_Driver") == 0){
        // Handle Inferno cache setting
        if (psp_model>PSP_1000) { // 8M cache on other models
            se_config->iso_cache_size = 64 * 1024;
            se_config->iso_cache_num = 128;
            se_config->iso_cache_partition = (se_config->force_high_memory)? 2 : 9;
        }
        // Disable Pause feature on PSP Go
        if (se_config->disable_pause){
            disable_PauseGame(mod);
        }
        goto flush;
    }
    
    if (strcmp(mod->modname, "sceLoadExec") == 0){
        prepatch_partitions();
        goto flush;
    }

    if (strcmp(mod->modname, "game_plugin_module") == 0) {
        if (se_config->skiplogos == 1 || se_config->skiplogos == 2) {
            patch_GameBoot(mod);
        }
        goto flush;
    }

    if (strcmp(mod->modname, "sceUSBCam_Driver") == 0 || strcmp(mod->modname, "sceUSBMic_Driver") == 0){
        extern int is_usb_charging;
        extern int usb_charge_break;
        if (is_usb_charging){
            usb_charge_break = 1;
            usb_charge(100);
            sceKernelDelayThread(2000);
        }
        goto flush;
    }

    if (strcmp(mod->modname, "sceNetApctl_Library") == 0) {
        if (se_config->wpa2){
            patchSceNetWpa2(mod);
        }
        goto flush;
    }
    
    if (strcmp(mod->modname, "vsh_module") == 0){
        if (se_config->umdregion){
            patch_vsh_region_check(mod);
        }
        goto flush;
    }
    
    if (strcmp(mod->modname, "Legacy_Software_Loader") == 0 )
    {
        // Missing from SDK
        #define PSP_INIT_APITYPE_EF2 0x152
        if( sceKernelInitApitype() == PSP_INIT_APITYPE_EF2 )
        {
        	_sw( 0x10000005, mod->text_addr + 0x0000014C );	
        	goto flush;
        }
    }
    
    if (booted == 0)
    {
        // Boot is complete
        if (isSystemBooted())
        {

            // handle mscache
            if (se_config->msspeed){
                char* drv = 
                    (psp_model == PSP_GO && sctrlKernelBootFrom()==0x50)?
                    "eflash0a0f1p" : "msstor0p";
                msstorCacheInit(drv);
            }

            // fix pops on toolkits
            if (sctrlHENIsToolKit() && sceKernelInitKeyConfig() == PSP_INIT_KEYCONFIG_POPS){
                patchPops4Tool();
            }

            // Boot Complete Action done
            booted = 1;
            goto flush;
        }
    }
    
flush:
    sctrlFlushCache();

    // Forward to previous Handler
    if(previous) previous(mod);

}

int (*prev_start)(int modid, SceSize argsize, void * argp, int * modstatus, SceKernelSMOption * opt) = NULL;
int StartModuleHandler(int modid, SceSize argsize, void * argp, int * modstatus, SceKernelSMOption * opt){

    SceModule2* mod = (SceModule2*) sceKernelFindModuleByUID(modid);
    if (mod && strcmp(mod->modname, "sceMediaSync") == 0 && se_config->umdregion){
        sctrlArkReplaceUmdKeys();
    }

    // forward to previous or default StartModule
    if (prev_start) return prev_start(modid, argsize, argp, modstatus, opt);
    return -1;
}

static int power_event_handler(int ev_id, char *ev_name, void *param, int *result){
    if( ev_id == 0x400000) { // resume complete
        if (se_config->noled && _sceSysconCtrlLEDOrig){
            for (int i=0; i<4; i++) _sceSysconCtrlLEDOrig(i, 0);
        }
    }
    return 0;
}

PspSysEventHandler g_power_event = {
    .size = sizeof(g_power_event),
    .name = "pspSysEvent",
    .type_mask = 0x00FFFF00, // both suspend / resume
    .handler = &power_event_handler,
};

void PSPSyspatchStart(){
    // Register Module Start Handler
    previous = sctrlHENSetStartModuleHandler(PSPOnModuleStart);
    
    // Register custom start module
    prev_start = sctrlSetStartModuleExtra(StartModuleHandler);

    // Register Power Event Handler
    sceKernelRegisterSysEventHandler(&g_power_event);

    // Implement extra memory unlock
    HIJACK_FUNCTION(K_EXTRACT_IMPORT(sctrlHENApplyMemory), memoryHandlerPSP, _sctrlHENApplyMemory);
}
