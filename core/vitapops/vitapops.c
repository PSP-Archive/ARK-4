#include <pspsdk.h>
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
#include "vitapops.h"
#include "libs/graphics/graphics.h"

PSP_MODULE_INFO("VitaPops", 0x3007, 1, 0);

// Previous Module Start Handler
STMOD_HANDLER previous = NULL;

static ARKConfig _ark_conf;
ARKConfig* ark_config = &_ark_conf;

KernelFunctions _ktbl = {
    .KernelDcacheInvalidateRange = &sceKernelDcacheInvalidateRange,
    .KernelIcacheInvalidateAll = &sceKernelIcacheInvalidateAll,
    .KernelDcacheWritebackInvalidateAll = &sceKernelDcacheWritebackInvalidateAll,
    .KernelIOOpen = &sceIoOpen,
    .KernelIORead = &sceIoRead,
    .KernelIOClose = &sceIoClose,
    .KernelDelayThread = &sceKernelDelayThread,
}; // for vita flash patcher

// Internal function
int (* _sceDisplaySetFrameBufferInternal)(int pri, void *topaddr, int width, int format, int sync) = 0;

// Vram address and config
u16* pops_vram = (u16*)0x490C0000;
POPSVramConfigList* vram_config = (POPSVramConfigList*)0x49FE0000;

// Registered Vram handler
void (*_psxVramHandler)(u32* psp_vram, u16* ps1_vram) = (void*)NULL;

// Flush Instruction and Data Cache
void flushCache()
{
	// Flush Instruction Cache
	sceKernelIcacheInvalidateAll();
	
	// Flush Data Cache
	sceKernelDcacheWritebackInvalidateAll();
}

static void initVitaPopsVram(){
	memset((void *)pops_vram, 0, 0x3C0000);
	vram_config->counter = 0;
	vram_config->configs[0].x = 0x1F6;
	vram_config->configs[0].y = 0;
	vram_config->configs[0].width = 640*4;
	vram_config->configs[0].height = 240;
	vram_config->configs[0].color_width = sizeof(u16);
	vram_config->configs[0].cur_buffer = 0;
}

static u16 RGBA8888_to_RGBA5551(u32 color)
{
	int r, g, b, a;
	a = (color >> 24) ? 0x8000 : 0;
	b = (color >> 19) & 0x1F;
	g = (color >> 11) & 0x1F;
	r = (color >> 3) & 0x1F;
	return a | r | (g << 5) | (b << 10);
}

static u32 GetPopsVramAddr(u32 framebuffer, int x, int y)
{
	return framebuffer + x * 2 + y * 640 * 4;
}

static u32 GetPspVramAddr(u32 framebuffer, int x, int y)
{
	return framebuffer + x * 4 + y * 512 * 4;
}

static void RelocateVram(u32* psp_vram, u16* ps1_vram)
{
	if(psp_vram)
	{
		int y;
		for(y = 0; y < 272; y++)
		{
			int x;
			for(x = 0; x < 480; x++)
			{
				u32 color = *(u32 *)GetPspVramAddr((u32)psp_vram, x, y);
				*(u16 *)GetPopsVramAddr((u32)pops_vram, x, y) = RGBA8888_to_RGBA5551(color);
			}
		}
	}
}

static int sceDisplaySetFrameBufferInternalHook(int pri, void *topaddr,
		int width, int format, int sync){
    if (_psxVramHandler) _psxVramHandler(topaddr, pops_vram);
	return _sceDisplaySetFrameBufferInternal(pri, topaddr, width, format, sync); 
}

static void* registerPSXVramHandler(void (*handler)(u32* psp_vram, u16* ps1_vram)){
	void* prev = _psxVramHandler;
	if (handler) _psxVramHandler = handler;
	return prev;
}

void* sctrlARKSetPSXVramHandler(void (*handler)(u32* psp_vram, u16* ps1_vram)){
	int k1 = pspSdkSetK1(0);
	void* prev = registerPSXVramHandler(handler);
	pspSdkSetK1(k1);
	return prev;
}

void patchVitaPopsDisplay(SceModule2* mod){
	u32 display_func = FindFunction("sceDisplay_Service", "sceDisplay_driver", 0x3E17FE8D);
	if (display_func){
	    // protect vita pops vram
	    sceKernelAllocPartitionMemory(6, "POPS VRAM CONFIG", 2, 0x1B0, (void *)0x09FE0000);
		sceKernelAllocPartitionMemory(6, "POPS VRAM", 2, 0x3C0000, (void *)0x090C0000);
		memset((void *)0x49FE0000, 0, 0x1B0);
    	memset((void *)0x490C0000, 0, 0x3C0000);
		// register default screen handler
    	registerPSXVramHandler(&RelocateVram);
    	// initially screen configuration
	    initVitaPopsVram();
	    // patch display function
		HIJACK_FUNCTION(display_func, sceDisplaySetFrameBufferInternalHook,
			_sceDisplaySetFrameBufferInternal);
	}
}

// Return Boot Status
static int isSystemBooted(void)
{

	// Find Function
	int (* _sceKernelGetSystemStatus)(void) = (void *)sctrlHENFindFunction("sceSystemMemoryManager", "SysMemForKernel", 0x452E3696);
	
	// Get System Status
	int result = _sceKernelGetSystemStatus();
		
	// System booted
	if(result == 0x20000) return 1;
	
	// Still booting
	return 0;
}

static void ARKVitaPopsOnModuleStart(SceModule2 * mod){

    static int booted = 0;

    // Patch display in PSX exploits
	if(strcmp(mod->modname, "sceDisplay_Service") == 0) {
	    patchVitaPopsDisplay(mod);
		goto flush;
	}
	
	// Kermit Peripheral Patches
	if(strcmp(mod->modname, "sceKermitPeripheral_Driver") == 0)
	{
		// Patch Kermit Peripheral Module to load flash0
		patchKermitPeripheral(&_ktbl);
		// Exit Handler
		goto flush;
	}
	
	// Boot Complete Action not done yet
	if(booted == 0)
	{
		// Boot is complete
		if(isSystemBooted())
		{
			// Allow exiting through key combo
			patchExitGame();
			// Boot Complete Action done
			booted = 1;
			goto flush;
		}
	}

flush:
    flushCache();

exit:

	// Forward to previous Handler
	if(previous) previous(mod);
}

// Boot Time Entry Point
int module_start(SceSize args, void * argp)
{
    // copy configuration
    memcpy(ark_config, ark_conf_backup, sizeof(ARKConfig));
	// Register Module Start Handler
	previous = sctrlHENSetStartModuleHandler(ARKVitaPopsOnModuleStart);
	// Return Success
	return 0;
}
