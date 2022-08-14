#include <pspsdk.h>
#include <pspinit.h>
#include <globals.h>
#include <graphics.h>
#include <macros.h>
#include <module2.h>
#include <pspdisplay_kernel.h>
#include <pspsysmem_kernel.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <systemctrl_private.h>
#include <pspiofilemgr.h>
#include <pspgu.h>
#include <functions.h>
#include "high_mem.h"
#include "exitgame.h"
#include "region_free.h"
#include "libs/graphics/graphics.h"

extern u32 psp_fw_version;
extern u32 psp_model;
extern ARKConfig* ark_config;
extern STMOD_HANDLER previous;
extern void SetSpeed(int cpuspd, int busspd);
extern void patch_region();

// Return Boot Status
int isSystemBooted(void)
{

    // Find Function
    int (* _sceKernelGetSystemStatus)(void) = (void*)sctrlHENFindFunction("sceSystemMemoryManager", "SysMemForKernel", 0x452E3696);
    
    // Get System Status
    int result = _sceKernelGetSystemStatus();
        
    // System booted
    if(result == 0x20000) return 1;
    
    // Still booting
    return 0;
}

static u32 fakeDevkitVersion(){
    return FW_660; // Popsloader V3 will check for 6.60, it fails on 6.61 so let's make it think it's on 6.60
}

static unsigned int fakeFindFunction(char * szMod, char * szLib, unsigned int nid){
    if (nid == 0x221400A6 && strcmp(szMod, "SystemControl") == 0)
        return 0; // Popsloader V4 looks for this function to check for ME, let's pretend ARK doesn't have it ;)
    return sctrlHENFindFunction(szMod, szLib, nid);
}

static int fakeUmdActivate(){
    return 0;
}

static int _sceKernelBootFromForUmdMan(void)
{
    return 0x20;
}

void patch_sceUmdMan_driver(SceModule* mod)
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
        MAKE_DUMMY_FUNCTION_RETURN_1(text_addr+0x000009C8);
    }
}

void patch_sceWlan_Driver(SceModule2* mod)
{
    // disable frequency check
    u32 text_addr = mod->text_addr;
    _sw(NOP, text_addr + 0x000026C0);
}

void patch_scePower_Service(SceModule2* mod)
{
    // scePowerGetBacklightMaximum always returns 4
    u32 text_addr = mod->text_addr;
    _sw(NOP, text_addr + 0x00000E68);
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
        SceModule2* mod = sceKernelFindModuleByName("sceImpose_Driver");
        u32 text_addr = mod->text_addr;
        for(int i=0; i<2; i++) {
            _sw(NOP, text_addr + 0x00000574 + i * 4);
        }
        pause_disabled = 1;
    }
}

int is_launcher_mode = 0;
int use_mscache = 0;
int use_highmem = 0;
int oldplugin = 0;
void settingsHandler(char* path){
    int apitype = sceKernelInitApitype();
    if (strcasecmp(path, "overclock") == 0){ // set CPU speed to max
        SetSpeed(333, 166);
    }
    else if (strcasecmp(path, "powersave") == 0){ // underclock to save battery
        if (apitype != 0x144 && apitype != 0x155) // prevent operation in pops
            SetSpeed(133, 66);
    }
    else if (strcasecmp(path, "usbcharge") == 0){
        usb_charge(); // enable usb charging
    }
    else if (strcasecmp(path, "highmem") == 0){ // enable high memory
        use_highmem = 1;
        patch_partitions();
        disable_PauseGame(); // disable pause feature to maintain stability
    }
    else if (strcasecmp(path, "mscache") == 0){
        use_mscache = 1; // enable ms cache for speedup
    }
    else if (strcasecmp(path, "disablepause") == 0){ // disable pause game feature on psp go
        if (apitype != 0x144 && apitype != 0x155 && apitype !=  0x210 && apitype !=  0x220) // prevent in pops and vsh
            disable_PauseGame();
    }
    else if (strcasecmp(path, "launcher") == 0){ // replace XMB with custom launcher
        is_launcher_mode = 1;
    }
    else if (strcasecmp(path, "oldplugin") == 0){ // redirect ms0 to ef0 on psp go
        oldplugin = 1;
    }
    else if (strcasecmp(path, "infernocache") == 0){
        if (apitype == 0x123 || apitype == 0x125){
            void (*CacheSetPolicy)(int) = sctrlHENFindFunction("PRO_Inferno_Driver", "inferno_driver", 0xC0736FD6);
            int (*CacheInit)(int, int, int) = sctrlHENFindFunction("PRO_Inferno_Driver", "inferno_driver", 0x8CDE7F95);
            if (CacheSetPolicy && CacheInit){
                CacheSetPolicy(CACHE_POLICY_LRU);
                if (psp_model==PSP_1000) CacheInit(4 * 1024, 8, 2); // 32K cache on 1K
                else CacheInit(64 * 1024, 128, (use_highmem)?2:9); // 8M cache on other models
                disable_PauseGame(); // disable pause feature to maintain stability
            }
        }
    }
    else if (strcasecmp(path, "region_jp") == 0){
        region_change = REGION_JAPAN;
    }
    else if (strcasecmp(path, "region_us") == 0){
        region_change = REGION_AMERICA;
    }
    else if (strcasecmp(path, "region_eu") == 0){
        region_change = REGION_EUROPE;
    }
}

void processSettings(){
    loadSettings(&settingsHandler);
    if (is_launcher_mode){
        strcpy(ark_config->launcher, ARK_MENU); // set CFW in launcher mode
    }
    else{
        ark_config->launcher[0] = 0; // disable launcher mode
    }
    sctrlHENSetArkConfig(ark_config);

    if (region_change){
        patch_region();
        flushCache();
    }
}

void (*prevPluginHandler)(const char* path, int modid) = NULL;
void pluginHandler(const char* path, int modid){
    if(oldplugin && psp_model == PSP_GO && (strncmp(path, "ef0", 2)==0 || strncmp(path, "EF0", 2)==0)) {
		patch_devicename(modid);
	}
	if (prevPluginHandler) prevPluginHandler(path, modid);
}

void PSPOnModuleStart(SceModule2 * mod){
    // System fully booted Status
    static int booted = 0;
    
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
    
    if (strcmp(mod->modname, "popsloader") == 0 || strcmp(mod->modname, "popscore") == 0){
        // fix for 6.60 check on 6.61
        hookImportByNID(mod, "SysMemForKernel", 0x3FC9AE6A, &fakeDevkitVersion);
        // fix to prevent ME detection
        hookImportByNID(mod, "SystemCtrlForKernel", 0x159AF5CC, &fakeFindFunction);
        goto flush;
    }

    if (strcmp(mod->modname, "MacroFire") == 0){
        // fix for MacroFire (disables sceUmdActivate/Deactivate functions)
        // this is needed because ARK loads plugins when UMD is already active (MediaSync fully loaded and started)
        // while older CFW load plugins a bit earlier (MediaSync loaded but not started)
        hookImportByNID(mod, "sceUmdUser", 0xC6183D47, &fakeUmdActivate);
        hookImportByNID(mod, "sceUmdUser", 0xE83742BA, &fakeUmdActivate);
        goto flush;
    }

    if (strcmp(mod->modname, "DayViewer_User") == 0){
        // fix scePaf imports in DayViewer
        static u32 nids[] = {
            0x2BE8DDBB, 0xE8CCC611, 0xCDDCFFB3, 0x48BB05D5, 0x22FB4177, 0xBC8DC92B, 0xE3D530AE
        };
        for (int i=0; i<NELEMS(nids); i++){
            hookImportByNID(mod, "scePaf", nids[i], sctrlHENFindFunction("scePaf_Module", "scePaf", nids[i]));
        }
        goto flush;
    }

    if (strcmp(mod->modname, "impose_plugin_module") == 0){
        if (region_change)
        {
            SceUID kthreadID = sceKernelCreateThread( "arkflasher", &patch_umd_thread, 1, 0x20000, PSP_THREAD_ATTR_VFPU, NULL);
            if (kthreadID >= 0){
                // start thread and wait for it to end
                sceKernelStartThread(kthreadID, 0, NULL);
            }
        }
        goto flush;
    }
    
    if(booted == 0)
    {
        // Boot is complete
        if(isSystemBooted())
        {
            if (use_mscache){
                if (psp_model == PSP_GO)
                    msstorCacheInit("eflash0a0f1p", 8 * 1024);
                else
                    msstorCacheInit("msstor0p", 16 * 1024);
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

