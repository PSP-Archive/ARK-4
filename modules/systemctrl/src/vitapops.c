#include <pspsdk.h>
#include <globals.h>
#include <graphics.h>
#include <macros.h>
#include <module2.h>
#include <pspdisplay_kernel.h>
#include <pspiofilemgr.h>
#include <pspgu.h>
#include <functions.h>
#include "custom_png.h"
#include "filesystem.h"
#include "vitapops.h"

int (* _sceDisplaySetFrameBufferInternal)(int pri, void *topaddr, int width, int format, int sync) = 0;
int (* SetKeys)(char *filename, void *keys, void *keys2) = 0;
int (* scePopsSetKeys)(int size, void *keys, void *keys2) = 0;

void* sctrlHENSetPSXVramHandler(void (*handler)(u32* psp_vram, u16* ps1_vram)){
	int k1 = pspSdkSetK1(0);
	void* prev = registerPSXVramHandler(handler);
	pspSdkSetK1(k1);
	return prev;
}

int sceDisplaySetFrameBufferInternalHook(int pri, void *topaddr,
		int width, int format, int sync){
	copyPSPVram(topaddr);
	return _sceDisplaySetFrameBufferInternal(pri, topaddr, width, format, sync); 
}

// TODO: Create a PSX Vram handler for hardware copy (can this be done anyways?)
void psxVramHardwareHandler(u32* psp_vram, u16* psx_vram){
}

void patchVitaPopsDisplay(){
	
	u32 display_func = FindFunction("sceDisplay_Service", "sceDisplay_driver", 0x3E17FE8D);
	
	if (display_func != 0){
		HIJACK_FUNCTION(display_func, sceDisplaySetFrameBufferInternalHook,
			_sceDisplaySetFrameBufferInternal);
	}
	
	// TODO: register a Vram Handler for Hardware drawing
	//registerPSXVramHandler(&psxVramHardwareHandler);
}
