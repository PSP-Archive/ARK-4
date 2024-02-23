#include <pspsdk.h>
#include <pspsysmem_kernel.h>
#include <pspinit.h>
#include <globals.h>
#include <graphics.h>
#include <macros.h>
#include <module2.h>
#include <pspdisplay_kernel.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <systemctrl_private.h>
#include <pspiofilemgr.h>
#include <pspgu.h>
#include <functions.h>
#include "exitgame.h"
#include "region_free.h"
#include "libs/graphics/graphics.h"

extern u32 psp_model;
extern ARKConfig* ark_config;
extern SEConfig* se_config;
extern STMOD_HANDLER previous;

extern int sceKernelSuspendThreadPatched(SceUID thid);


static int _sceKernelBootFromForUmdMan(void)
{
    return 0x20;
}

void patch_sceUmdMan_driver(SceModule2* mod)
{
    int apitype = sceKernelInitApitype();
    if (apitype == 0x152 || apitype == 0x141) {
        hookImportByNID(mod, "InitForKernel", 0x27932388, _sceKernelBootFromForUmdMan);
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

static void patch_devicename(SceUID modid)
{
	SceModule2 *mod;
	int i;

	mod = (SceModule2*)sceKernelFindModuleByUID(modid);

	if(mod == NULL) {
		return;
	}

	for(i=0; i<mod->nsegment; ++i) {
		u32 addr;
		u32 end;

		end = mod->segmentaddr[i] + mod->segmentsize[i];

		for(addr = mod->segmentaddr[i]; addr < end; addr ++) {
			char *str = (char*)addr;

			if (0 == strncmp(str, "ms0", 3)) {
				str[0] = 'e';
				str[1] = 'f';
			} else if (0 == strncmp(str, "fatms", 5)) {
				str[3] = 'e';
				str[4] = 'f';
			}
		}
	}
	
	u32 start = mod->text_addr+mod->text_size;
	u32 end = start + mod->data_size;
	for (u32 addr=start; addr<end; addr++){
	    char *str = (char*)addr;
		if (0 == strncmp(str, "ms0", 3)) {
			str[0] = 'e';
			str[1] = 'f';
		} else if (0 == strncmp(str, "fatms", 5)) {
			str[3] = 'e';
			str[4] = 'f';
		}
	}

	flushCache();
}

int pause_disabled = 0;
void disable_PauseGame()
{
    if(psp_model == PSP_GO && !pause_disabled) {
        SceModule2* mod = (SceModule2*)sceKernelFindModuleByName("sceImpose_Driver");
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

void processSettings(){
    int apitype = sceKernelInitApitype();

    // USB Charging
    if (se_config->usbcharge){
        usb_charge(); // enable usb charging
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
    if (se_config->noled){
        int (*_sceSysconCtrlLED)(int, int);
        _sceSysconCtrlLED = sctrlHENFindFunction("sceSYSCON_Driver", "sceSyscon_driver", 0x18BFBE65);
        for (int i=0; i<4; i++) _sceSysconCtrlLED(i, 0);
        MAKE_DUMMY_FUNCTION_RETURN_0(_sceSysconCtrlLED);
        flushCache();
    }
    // Enforce extra RAM
    if (se_config->force_high_memory){
        patch_partitions();
        se_config->disable_pause = 1;
    }
    if(!se_config->force_high_memory && (apitype == 0x141 || apitype == 0x152) ){
        int paramsize=4;
        int use_highmem = 0;
        if (sctrlGetInitPARAM("MEMSIZE", NULL, &paramsize, &use_highmem) >= 0 && use_highmem){
            patch_partitions();
            se_config->disable_pause = 1;
            se_config->force_high_memory = 1;
        }
    }
    // Enable Inferno cache
    if (se_config->iso_cache){
        int (*CacheInit)(int, int, int) = sctrlHENFindFunction("PRO_Inferno_Driver", "inferno_driver", 0x8CDE7F95);
        if (CacheInit){
            if (psp_model==PSP_1000){
                se_config->iso_cache_size = 4 * 1024;
                se_config->iso_cache_num = 8;
                CacheInit(4 * 1024, 8, 1); // 32K cache on 1K, allocated in kernel
            }
            else {
                se_config->iso_cache_size = 64 * 1024;
                se_config->iso_cache_num = 128;
                CacheInit(64 * 1024, 128, (se_config->force_high_memory)?2:9); // 8M cache on other models
            }
            se_config->disable_pause = 1; // disable pause feature to maintain stability
        }
        if (se_config->iso_cache == 2){
            int (*CacheSetPolicy)(int) = sctrlHENFindFunction("PRO_Inferno_Driver", "inferno_driver", 0xC0736FD6);
            if (CacheSetPolicy){
                se_config->iso_cache_policy = CACHE_POLICY_RR;
                CacheSetPolicy(CACHE_POLICY_RR);
            }
        }
    }
    // Disable Pause feature on PSP Go
    if (se_config->disable_pause){
        disable_PauseGame();
    }
    // Disable UMD Drive
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

int (*prevPluginHandler)(const char* path, int modid) = NULL;
int pluginHandler(const char* path, int modid){
    if(se_config->oldplugin && psp_model == PSP_GO && path[0] == 'e' && path[1] == 'f') {
		patch_devicename(modid);
	}
	if (prevPluginHandler) return prevPluginHandler(path, modid);
    return 0;
}

void PSPOnModuleStart(SceModule2 * mod){
    // System fully booted Status
    static int booted = 0;

	if (strcmp(mod->modname, "CWCHEATPRX") == 0) {
    	if (sceKernelInitKeyConfig() == PSP_INIT_KEYCONFIG_POPS) {
			hookImportByNID(mod, "ThreadManForKernel", 0x9944F31F, sceKernelSuspendThreadPatched);
			goto flush;
		}
	}

    if(strcmp(mod->modname, "sceUmdMan_driver") == 0) {
        patch_sceUmdMan_driver(mod);
        patch_umd_idslookup(mod);
        goto flush;
    }

    if(strcmp(mod->modname, "sceUmdCache_driver") == 0) {
        patch_umdcache(mod);
        goto flush;
    }

    if(strcmp(mod->modname, "sceWlan_Driver") == 0) {
        patch_sceWlan_Driver(mod);
        patch_Libertas_MAC(mod);
        goto flush;
    }

    if(strcmp(mod->modname, "scePower_Service") == 0) {
        patch_scePower_Service(mod);
        goto flush;
    }
    
    if(strcmp(mod->modname, "sceMediaSync") == 0) {
        processSettings();
        goto flush;
    }
    
    if (strcmp(mod->modname, "sceLoadExec") == 0){
        prepatch_partitions();
        goto flush;
    }

    if(0 == strcmp(mod->modname, "game_plugin_module")) {
		if (se_config->skiplogos) {
		    patch_GameBoot(mod);
	    }
        goto flush;
	}
    
    if (strcmp(mod->modname, "vsh_module") == 0){
        if (se_config->umdregion){
            patch_vsh_region_check(mod);
        }
        if (se_config->skiplogos){
            // patch GameBoot
            hookImportByNID(sceKernelFindModuleByName("sceVshBridge_Driver"), "sceDisplay_driver", 0x3552AB11, 0);
        }
        goto flush;
    }

    if (strcmp(mod->modname, "impose_plugin_module") == 0){
        if (se_config->umdregion)
        {
            SceUID kthreadID = sceKernelCreateThread( "ark_region_change", &patch_umd_thread, 1, 0x20000, PSP_THREAD_ATTR_VFPU, NULL);
            if (kthreadID >= 0){
                // start thread and wait for it to end
                sceKernelStartThread(kthreadID, 0, NULL);
            }
        }
        goto flush;
    }

	if( strcmp(mod->modname, "Legacy_Software_Loader") == 0 )
	{
        // Missing from SDK
        #define PSP_INIT_APITYPE_EF2 0x152
		if( sceKernelInitApitype() == PSP_INIT_APITYPE_EF2 )
		{
			_sw( 0x10000005, mod->text_addr + 0x0000014C );	
			goto flush;
		}
	}

    
    if(booted == 0)
    {
        // Boot is complete
        if(isSystemBooted())
        {

            // handle mscache
            if (se_config->msspeed){
                char* drv = "msstor0p";
                if (psp_model == PSP_GO && sctrlKernelBootFrom()==0x50)
                    drv = "eflash0a0f1p";
                msstorCacheInit(drv);
            }
            // Boot Complete Action done
            booted = 1;
            goto flush;
        }


    }
    
flush:
    flushCache();

    // Forward to previous Handler
    if(previous) previous(mod);

}

int (*prev_start)(int modid, SceSize argsize, void * argp, int * modstatus, SceKernelSMOption * opt) = NULL;
int StartModuleHandler(int modid, SceSize argsize, void * argp, int * modstatus, SceKernelSMOption * opt){

    SceModule2* mod = (SceModule2*) sceKernelFindModuleByUID(modid);

    if (se_config->skiplogos && mod != NULL && ark_config->launcher[0] == 0 && 0 == strcmp(mod->modname, "vsh_module") ) {
		u32* vshmain_args = oe_malloc(1024);

		memset(vshmain_args, 0, 1024);

		if(argp != NULL && argsize != 0 ) {
			memcpy( vshmain_args , argp ,  argsize);
		}

		vshmain_args[0] = 1024;
		vshmain_args[1] = 0x20;
		vshmain_args[16] = 1;

		int ret = sceKernelStartModule(modid, 1024, vshmain_args, modstatus, opt);
		oe_free(vshmain_args);

		return ret;
	}

    // forward to previous or default StartModule
    if (prev_start) return prev_start(modid, argsize, argp, modstatus, opt);
    return -1;
}
