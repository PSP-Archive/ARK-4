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

int (* DisplaySetFrameBuf)(void*, int, int, int) = NULL;

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

static int debuglog(char* text){
    int fd = sceIoOpen("ms0:/ps1log.txt", PSP_O_WRONLY|PSP_O_APPEND|PSP_O_CREAT, 0777);
    sceIoWrite(fd, text, strlen(text));
    sceIoClose(fd);
}

static struct {
    char* lib;
    u32 nid;
} pops_imports[] = {
    {"pspvmc",0x0820B291},
    {"pspvmc",0x38A87A12},
    {"sceLibFont_HV",0x67F17ED7},
    {"sceLibFont_HV",0x574B6FBC},
    {"sceLibFont_HV",0x099EF33C},
    {"sceLibFont_HV",0xA834319D},
    {"sceLibFont_HV",0xE4606649},
    {"sceLibFont_HV",0x3AEA8CB6},
    {"sceLibFont_HV",0x0DA7535E},
    {"sceLibFont_HV",0xDCC80C2F},
    {"sceLibFont_HV",0x980F4895},
    {"sceLibFont_HV",0xAA3DE7B5},
    {"sceLibFont_HV",0x568BE516},
    {"scePaf",0x003B3F87},
    {"scePaf",0x0C2CD696},
    {"scePaf",0x0FCDFA1E},
    {"scePaf",0x1F02DD65},
    {"scePaf",0x20BEF384},
    {"scePaf",0x22420CC7},
    {"scePaf",0x3C4BC2CD},
    {"scePaf",0x3E564415},
    {"scePaf",0x43759F51},
    {"scePaf",0x4E43A742},
    {"scePaf",0x5A12583F},
    {"scePaf",0x65169E51},
    {"scePaf",0x66CCA794},
    {"scePaf",0x6D55F3F5},
    {"scePaf",0x706ABBFF},
    {"scePaf",0x71B92320},
    {"scePaf",0x7B7133D5},
    {"scePaf",0x8166CA82},
    {"scePaf",0x8C5CC663},
    {"scePaf",0x8E805192},
    {"scePaf",0x9A418CCC},
    {"scePaf",0x9CD6C5F4},
    {"scePaf",0x9D854500},
    {"scePaf",0xA138A376},
    {"scePaf",0xA631AC8B},
    {"scePaf",0xB05D9677},
    {"scePaf",0xB110AF46},
    {"scePaf",0xB4652CFE},
    {"scePaf",0xBB89C9EA},
    {"scePaf",0xD229572C},
    {"scePaf",0xD7DCB972},
    {"scePaf",0xD9E2D6E1},
    {"scePaf",0xE0B32AE8},
    {"scePaf",0xE49835DC},
    {"scePaf",0xFF9C876B},
    {"sceVshCommonUtil",0x649C3568},
    {"sceVshCommonUtil",0x91838DED},
    {"sceVshCommonUtil",0x91B254B2},
    {"sceVshCommonUtil",0x937E715D},
    {"sceVshCommonUtil",0xD3A016E1},
    {"sceVshCommonUtil",0xF9E12DAA},
    {"sceDmac", 0x617F3FE6},
    {"sceDisplay",0x289D82FE},
    {"sceDisplay",0x46F186C3},
    {"sceDisplay",0x7ED59BC4},
    {"sceDisplay",0x984C27E7},
    {"sceDisplay",0x9C6EAAD7},
    {"sceGe_user",0x03444EB4},
    {"sceGe_user",0x0BF608FB},
    {"sceGe_user",0x1F6752AD},
    {"sceGe_user",0x438A385A},
    {"sceGe_user",0xAB49E76A},
    {"sceGe_user",0xB287BD61},
    {"sceGe_user",0xB77905EA},
    {"sceGe_user",0xE0D68148},
    {"sceUtility",0x50C4CD57},
    {"sceUtility",0x8874DBE0},
    {"sceUtility",0x9790B33C},
    {"sceUtility",0xD4B95FFB},
    {"sceRtc", 0x57726BC1},
    {"sceRtc", 0xE7C27D1B},
    {"scePower",0x04B7766E},
    {"scePower",0x469989AD},
    {"scePower",0x78A1A796},
    {"sceCtrl",0x1F4011E6},
    {"sceCtrl",0x3A622550},
    {"sceCtrl",0x6A2774F3},
    {"sceCtrl",0xA7144800},
    {"sceCtrl",0xC152080A},
    {"sceImpose", 0x24FD7BCF},
    {"sceImpose", 0x9BA61B49},
    {"sceMeAudio",0x0BABD960},
    {"sceMeAudio",0x0FA28FE6},
    {"sceMeAudio",0x14447BA0},
    {"sceMeAudio",0x1A23C094},
    {"sceMeAudio",0x2AB4FE43},
    {"sceMeAudio",0x2AC64C3F},
    {"sceMeAudio",0x30BE34E4},
    {"sceMeAudio",0x3771229C},
    {"sceMeAudio",0x42F0EA37},
    {"sceMeAudio",0x4F5B6D82},
    {"sceMeAudio",0x528266FA},
    {"sceMeAudio",0x54F2AE52},
    {"sceMeAudio",0x68C55F4C},
    {"sceMeAudio",0x69C4BCCB},
    {"sceMeAudio",0x7014C540},
    {"sceMeAudio",0x805D1205},
    {"sceMeAudio",0x83378E12},
    {"sceMeAudio",0x8A8DFE17},
    {"sceMeAudio",0x8D5A07D2},
    {"sceMeAudio",0x9B4AAF7D},
    {"sceMeAudio",0xA6EDDF16},
    {"sceMeAudio",0xAE5AC375},
    {"sceMeAudio",0xBD5F7689},
    {"sceMeAudio",0xC93C56F8},
    {"sceMeAudio",0xD4F17F54},
    {"sceMeAudio",0xDE630CD2},
    {"sceMeAudio",0xE7F06E2B},
    {"sceMeAudio",0xE907AE69},
    {"sceMeAudio",0xF6637A72},
    {"scePopsMan", 0x0090B2C8},
    {"sceOpenPSID", 0xC69BEBCE},
    {"InterruptManager",0xCA04A2B9},
    {"InterruptManager",0xD61E6961},
    {"InterruptManager",0xFB8E22EC},
    {"IoFileMgrForUser",0x06A70004},
    {"IoFileMgrForUser",0x42EC03AC},
    {"IoFileMgrForUser",0x54F5FB11},
    {"IoFileMgrForUser",0x63632449},
    {"IoFileMgrForUser",0x6A638D83},
    {"IoFileMgrForUser",0x810C4BC3},
    {"IoFileMgrForUser",0xACE946E8},
    {"IoFileMgrForUser",0xB29DDF9C},
    {"IoFileMgrForUser",0xE3EB004C},
    {"IoFileMgrForUser",0xEB092469},
    {"IoFileMgrForUser",0xF27A9C51},
    {"IoFileMgrForUser",0x109F50BC},
    {"IoFileMgrForUser",0x1117C65F},
    {"IoFileMgrForUser",0x27EB27B8},
    {"Kernel_Library", 0xA089ECA4},
    {"Kernel_Library", 0x1839852A},
    {"LoadExecForUser", 0x05572A5F},
    {"SysMemUserForUser",0x13A5ABEF},
    {"SysMemUserForUser",0xF77D77CB},
    {"SysMemUserForUser",0x358CA1BB},
    {"ThreadManForUser",0xCEADEB47},
    {"ThreadManForUser",0xD6DA4BA1},
    {"ThreadManForUser",0x1FB15A32},
    {"ThreadManForUser",0xE81CAF8F},
    {"ThreadManForUser",0xF475845D},
    {"ThreadManForUser",0x328C546A},
    {"ThreadManForUser",0x369ED59D},
    {"ThreadManForUser",0x3F53E640},
    {"ThreadManForUser",0x446D8DE6},
    {"ThreadManForUser",0x4E3A1105},
    {"ThreadManForUser",0x55C20A00},
    {"ThreadManForUser",0x6652B8CA},
    {"ThreadManForUser",0x68DA9E36},
    {"ThreadManForUser",0x71BC9871},
    {"ThreadManForUser",0x7E65B999},
    {"UtilsForUser",0x79D1C3FA},
    {"UtilsForUser",0xB435DEC5},
    {"UtilsForUser",0xBFA98062},
    {"UtilsForUser",0x34B9FA9E},
    {"UtilsForUser",0x3EE30821},
};

static u32 find_import(char* lib, u32 nid){
    SceModule2* mod = sceKernelFindModuleByName("sceSystemMemoryManager");
    while (mod){
        u32 f = sctrlHENFindFunction(mod->modname, lib, nid);
        if (f) return f;
        mod = mod->next;
    }
    return 0;
}

int myPopsManLoadModule(u32 a0, u32 a1){
    char msg[128];

    int k1 = pspSdkSetK1(0);
    sprintf(msg, "popsman load module: %p, %p\n", a0, a1);
    debuglog(msg);

    int (*load_pops)(u32, u32) = sctrlHENFindFunction("scePops_Manager", "scePopsMan", 0x29B3FB24);

    pspSdkSetK1(k1);
    int res = load_pops(a0, a1);
    k1 = pspSdkSetK1(0);

    sprintf(msg, "load pops res: %p\n", res);
    debuglog(msg);

    while (res < 0){
        debuglog("checking imports\n");
        for (int i=0; i<NELEMS(pops_imports); i++){
            u32 f = find_import(pops_imports[i].lib, pops_imports[i].nid);
            if (f == 0){
                sprintf(msg, "ERROR import: %s, %p\n", pops_imports[i].lib, pops_imports[i].nid);
                debuglog(msg);
            }
        }
        debuglog("imports checked\n");
        sceKernelDelayThread(10000000);
    }

    pspSdkSetK1(k1);    
    return res;
}

void PSPOnModuleStart(SceModule2 * mod){
    // System fully booted Status
    static int booted = 0;

    if (strcmp(mod->modname, "simple") == 0){
        u32 counter = 0;
        for (u32 addr=mod->text_addr; addr < mod->text_addr+mod->text_size; addr+=4){
            if (_lw(addr) == 0x34840004){
                _sw((0x34840000|counter++), addr);
            }
        }
        hookImportByNID(mod, "scePopsMan", 0x29B3FB24, myPopsManLoadModule);
        goto flush;
    }

    if(strcmp(mod->modname, "sceDisplay_Service") == 0) {
        DisplaySetFrameBuf = (void*)sctrlHENFindFunction("sceDisplay_Service", "sceDisplay", 0x289D82FE);
        goto flush;
    }

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

    if (DisplaySetFrameBuf) {
        initScreen(DisplaySetFrameBuf);
        cls();
        PRTSTR1("modid: %p", modid);
        if (mod) PRTSTR1("mod: %s", mod->modname);
    }

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
