#include <pspsdk.h>
#include <pspinit.h>
#include <globals.h>
#include <graphics.h>
#include <macros.h>
#include <module2.h>
#include <pspdisplay_kernel.h>
#include <pspsysmem_kernel.h>
#include <systemctrl.h>
#include <systemctrl_private.h>
#include <pspiofilemgr.h>
#include <pspgu.h>
#include <functions.h>
#include "high_mem.h"
#include "libs/graphics/graphics.h"

PSP_MODULE_INFO("PSPCompat", 0x3007, 1, 0);

// Previous Module Start Handler
STMOD_HANDLER previous = NULL;

// for some model specific patches
u32 psp_model = 0;

extern void SetSpeed(int cpuspd, int busspd);

// Flush Instruction and Data Cache
void flushCache()
{
	// Flush Instruction Cache
	sceKernelIcacheInvalidateAll();
	
	// Flush Data Cache
	sceKernelDcacheWritebackInvalidateAll();
}

static int _sceKernelBootFromForUmdMan(void)
{
	return 0x20;
}

static void patch_sceUmdMan_driver(SceModule* mod)
{
    int apitype = sceKernelInitApitype();
    if (apitype == 0x152 || apitype == 0x141) {
        hookImportByNID(mod, "InitForKernel", 0x27932388, _sceKernelBootFromForUmdMan);
	}
}

//prevent umd-cache in homebrew, so we can drain the cache partition.
static void patch_umdcache(SceModule2* mod)
{
	int apitype = sceKernelInitApitype();
    if (apitype == 0x152 || apitype == 0x141)
		return;

	int ret = sceKernelBootFrom();

	if (ret != 0x40 && ret != 0x50)
		return;
	
	//kill module start
	u32 text_addr = mod->text_addr;
	_sw(JR_RA, text_addr+0x000009C8);
	_sw(LI_V0(1), text_addr+0x000009C8+4);
}

static void patch_sceWlan_Driver(SceModule2* mod)
{
    u32 text_addr = mod->text_addr;
	_sw(NOP, text_addr + 0x000026C0);
}

static void patch_scePower_Service(SceModule2* mod)
{
    u32 text_addr = mod->text_addr;
	// scePowerGetBacklightMaximum always returns 4
	_sw(NOP, text_addr + 0x00000E68);
}

static void patch_tekken(SceModule2* mod){
    u32 func = sctrlHENFindImport("tekken", "scePower", 0x34F9C463);
	_sw(JR_RA, func);
	_sw(LI_V0(222), func+4);
}

static void PSPOnModuleStart(SceModule2 * mod){

    /*
	if (strcmp(mod->modname, "tekken") == 0) { // fixes Tekken 6 with any CPU speed
		patch_tekken(mod);
		goto flush;
    }
    */
    
	if(strcmp(mod->modname, "sceUmdMan_driver") == 0) {
		patch_sceUmdMan_driver((SceModule*)mod);
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
	
	if (strcmp(mod->modname, "sceLoadExec") == 0){
		if (psp_model > PSP_1000 && sceKernelApplicationType() == PSP_INIT_KEYCONFIG_GAME) {
			prepatch_partitions();
			goto flush;
		}
	}
	
	if (strcmp(mod->modname, "sceMediaSync") == 0){
		SetSpeed(333, 166); // overclock
		if (psp_model > PSP_1000 && sceKernelApplicationType() == PSP_INIT_KEYCONFIG_GAME) {
			patch_partitions();
			goto flush;
		}
	}
	
flush:
    flushCache();

	// Forward to previous Handler
	if(previous) previous(mod);
}

// Boot Time Entry Point
int module_start(SceSize args, void * argp)
{
    // set LCD framebuffer in hardware reg so we can do color debbuging
    _sw(0x44000000, 0xBC800100);
    // get psp model
    psp_model = sceKernelGetModel();
	// Register Module Start Handler
	previous = sctrlHENSetStartModuleHandler(PSPOnModuleStart);
	// Return Success
	return 0;
}
