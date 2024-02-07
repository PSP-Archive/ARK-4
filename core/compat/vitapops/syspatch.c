#include <pspsdk.h>
#include <globals.h>
#include <macros.h>
#include <module2.h>
#include <pspdisplay_kernel.h>
#include <pspsysmem_kernel.h>
#include <psputilsforkernel.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <systemctrl_private.h>
#include <pspiofilemgr.h>
#include <pspgu.h>
#include <pspinit.h>
#include <functions.h>
#include <graphics.h>
#include "popsdisplay.h"

extern ARKConfig* ark_config;
extern STMOD_HANDLER previous;

static int draw_thread = -1;
static volatile int do_draw = 0;
static u32* fake_vram = (u32*)0x0A400000; // use extra RAM for fake framebuffer
int (* DisplaySetFrameBuf)(void*, int, int, int) = NULL;
int (* DisplayGetFrameBuf)(void**, int*, int*, int) = NULL;
int (*DisplayWaitVblankStart)() = NULL;

extern SEConfig* se_config;

#define ALIGN(x, align) (((x) + ((align) - 1)) & ~((align) - 1))

KernelFunctions _ktbl = {
    .KernelDcacheInvalidateRange = &sceKernelDcacheInvalidateRange,
    .KernelIcacheInvalidateAll = &sceKernelIcacheInvalidateAll,
    .KernelDcacheWritebackInvalidateAll = &sceKernelDcacheWritebackInvalidateAll,
    .KernelIOOpen = &sceIoOpen,
    .KernelIORead = &sceIoRead,
    .KernelIOClose = &sceIoClose,
    .KernelDelayThread = &sceKernelDelayThread,
}; // for vita flash patcher


// hooked function to copy framebuffer
int (* _sceDisplaySetFrameBufferInternal)(int pri, void *topaddr, int width, int format, int sync) = NULL;
int sceDisplaySetFrameBufferInternalHook(int pri, void *topaddr,
        int width, int format, int sync){
    copyPSPVram(topaddr);
    if (_sceDisplaySetFrameBufferInternal) // passthrough
        return _sceDisplaySetFrameBufferInternal(pri, topaddr, width, format, sync); 
    return -1;
}

// patch pops display to set up our own screen handler
void patchVitaPopsDisplay(SceModule2* mod){
    u32 display_func = sctrlHENFindFunction("sceDisplay_Service", "sceDisplay_driver", 0x3E17FE8D);
    if (display_func){
        // protect vita pops vram
        sceKernelAllocPartitionMemory(6, "POPS VRAM CONFIG", PSP_SMEM_Addr, 0x1B0, (void *)0x09FE0000);
        sceKernelAllocPartitionMemory(6, "POPS VRAM", PSP_SMEM_Addr, 0x3C0000, (void *)0x090C0000);
        memset((void *)0x49FE0000, 0, 0x1B0);
        memset((void *)0x490C0000, 0, 0x3C0000);
        // register default screen handler
        registerPSXVramHandler(&SoftRelocateVram);
        // initialize screen configuration
        initVitaPopsVram();
        // patch display function
        HIJACK_FUNCTION(display_func, sceDisplaySetFrameBufferInternalHook,
            _sceDisplaySetFrameBufferInternal);
    }
}

int sceDisplayGetFrameBufPatched(void** p, int* w, int* f, int t){
    if (p) *p = fake_vram;
    if (w) *w = PSP_SCREEN_LINE;
    if (f) *f = PSP_DISPLAY_PIXEL_FORMAT_8888;
    return 0;
}

// background thread to relocate screen
int pops_draw_thread(int argc, void* argp){
    while (do_draw){
        void* framebuf = NULL;
        int pixelformat, bufferwidth;
        DisplayGetFrameBuf(&framebuf, &bufferwidth, &pixelformat, 0);
        if (!framebuf){
            framebuf = fake_vram;
        }
        copyPSPVram(framebuf);
        DisplayWaitVblankStart();
    }
    return 0;
}

int sceKernelSuspendThreadPatched(SceUID thid) {
	SceKernelThreadInfo info;
	info.size = sizeof(SceKernelThreadInfo);
	if (sceKernelReferThreadStatus(thid, &info) == 0) {
		if (strcmp(info.name, "popsmain") == 0) {
            if (draw_thread < 0){
                do_draw = 1;
			    draw_thread = sceKernelCreateThread("psxloader", &pops_draw_thread, 0x10, 0x10000, PSP_THREAD_ATTR_VFPU, NULL);
                sceKernelStartThread(draw_thread, 0, NULL);
            }
		}
	}

	return sceKernelSuspendThread(thid);
}

int sceKernelResumeThreadPatched(SceUID thid) {
	SceKernelThreadInfo info;
	info.size = sizeof(SceKernelThreadInfo);
	if (sceKernelReferThreadStatus(thid, &info) == 0) {
		if (strcmp(info.name, "popsmain") == 0) {
			if (draw_thread >= 0){
                do_draw = 0;
                sceKernelWaitThreadEnd(draw_thread, NULL);
                sceKernelDeleteThread(draw_thread);
                draw_thread = -1;
            }
		}
	}

	return sceKernelResumeThread(thid);
}

int popsExit(){
    int k1 = pspSdkSetK1(0);
    // attempt to exit via ps1cfw_enabler
    sceIoOpen("ms0:/__popsexit__", 0, 0);
    pspSdkSetK1(k1);
    // fallback to regular exit
    return sctrlKernelExitVSH(NULL);
}

void ARKVitaPopsOnModuleStart(SceModule2 * mod){

    static int booted = 0;
    
    // Patch display for homebrew
    if(strcmp(mod->modname, "sceDisplay_Service") == 0) {
        DisplaySetFrameBuf = (void*)sctrlHENFindFunction("sceDisplay_Service", "sceDisplay", 0x289D82FE);
        DisplayGetFrameBuf = (void*)sctrlHENFindFunction("sceDisplay_Service", "sceDisplay", 0xEEDA2E54);
        DisplayWaitVblankStart = (void*)sctrlHENFindFunction("sceDisplay_Service", "sceDisplay", 0x984C27E7);
        if (sceKernelInitApitype() != 0x144){
            patchVitaPopsDisplay(mod);
        }
        goto flush;
    }

    // Patch sceKernelExitGame Syscalls
    if(strcmp(mod->modname, "sceLoadExec") == 0)
    {
        REDIRECT_FUNCTION(sctrlHENFindFunction(mod->modname, "LoadExecForUser", 0x05572A5F), popsExit);
        REDIRECT_FUNCTION(sctrlHENFindFunction(mod->modname, "LoadExecForUser", 0x2AC9954B), popsExit);
        goto flush;
    }

    if (strcmp(mod->modname, "sceLowIO_Driver") == 0) {
		// Protect pops memory
        sceKernelAllocPartitionMemory(6, "", PSP_SMEM_Addr, 0x80000, (void *)0x09F40000);
        memset((void *)0x49F40000, 0, 0x80000);

        goto flush;
    }
    
    // Patch Kermit Peripheral Module to load flash0
    if(strcmp(mod->modname, "sceKermitPeripheral_Driver") == 0)
    {
        patchKermitPeripheral(&_ktbl);
        goto flush;
    }

    // patch to display plugins
    if (isLoadingPlugins() && sceKernelInitKeyConfig() == PSP_INIT_KEYCONFIG_POPS) {
        if (mod->text_addr&0x80000000){
            hookImportByNID(mod, "ThreadManForKernel", 0x9944F31F, sceKernelSuspendThreadPatched);
            hookImportByNID(mod, "ThreadManForKernel", 0x75156E8F, sceKernelResumeThreadPatched);
        }
        else {
            hookImportByNID(mod, "ThreadManForUser", 0x9944F31F, sceKernelSuspendThreadPatched);
            hookImportByNID(mod, "ThreadManForUser", 0x75156E8F, sceKernelResumeThreadPatched);
        }
        hookImportByNID(mod, "sceDisplay", 0x289D82FE, 0);
        hookImportByNID(mod, "sceDisplay", 0xEEDA2E54, sceDisplayGetFrameBufPatched);
        goto flush;
    }

    if (strcmp(mod->modname, "sceKernelLibrary") == 0){
        // Configure ps1cfw_enabler
        if (sceKernelInitKeyConfig() == PSP_INIT_KEYCONFIG_POPS){
            // send current game information (ID and path)
            char* path = sceKernelInitFileName();
            char title[20]; int n; sctrlGetInitPARAM("DISC_ID", NULL, &n, title);
            char* config = oe_malloc(300);
            sprintf(config, "ms0:/__popsconfig__/%s%s", title, strchr(path, '/'));
            sceIoOpen(config, 0, 0);
            oe_free(config);
        }
        else {
            // clear config
            sceIoOpen("ms0:/__popsclear__", 0, 0);
        }
        goto flush;
    }

    // Boot Complete Action not done yet
    if(booted == 0)
    {
        // Boot is complete
        if(isSystemBooted())
        {
            // Initialize Memory Stick Speedup Cache
            if (se_config->msspeed) msstorCacheInit("ms", 8 * 1024);

            // Set fake framebuffer so that plugins can be displayed
            if (sceKernelInitKeyConfig() == PSP_INIT_KEYCONFIG_POPS){
                DisplaySetFrameBuf((void *)fake_vram, PSP_SCREEN_LINE, PSP_DISPLAY_PIXEL_FORMAT_8888, PSP_DISPLAY_SETBUF_NEXTFRAME);
                memset((void *)fake_vram, 0, SCE_PSPEMU_FRAMEBUFFER_SIZE);
            }

            // notify ps1cfw_enabler that boot is complete
            sceIoOpen("ms0:/__popsbooted__", 0, 0);

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

