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
static int do_draw = 0;
static u32* fake_vram = (u32*)0x44000000; // might wanna use extra ram: 0x0BC00000 or so
int (* DisplaySetFrameBuf)(void*, int, int, int) = NULL;
int (*DisplayWaitVblankStart)() = NULL;
int (*DisplaySetHoldMode)(int) = NULL;

extern SEConfig* se_config;

typedef struct {
	uint32_t cmd; //0x0
	SceUID sema_id; //0x4
	uint64_t *response; //0x8
	uint32_t padding; //0xC
	uint64_t args[14]; // 0x10
} SceKermitRequest; //0x80

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

// patch pops display to set up our own screen handler
void patchVitaPopsDisplay(SceModule2* mod){
    u32 display_func = sctrlHENFindFunction("sceDisplay_Service", "sceDisplay_driver", 0x3E17FE8D);
    if (display_func){
        //if (sceKernelInitApitype() != 0x144){
            // protect vita pops vram
            sceKernelAllocPartitionMemory(6, "POPS VRAM CONFIG", PSP_SMEM_Addr, 0x1B0, (void *)0x09FE0000);
            sceKernelAllocPartitionMemory(6, "POPS VRAM", PSP_SMEM_Addr, 0x3C0000, (void *)0x090C0000);
        //}
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

int pops_draw_thread(int argc, void* argp){
    
    while (do_draw){
        SoftRelocateVram(fake_vram, NULL);
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

int (* sceMeAudio_driver_C300D466)(int codec, int unk, void *info);
int sceMeAudio_driver_C300D466_Patched(int codec, int unk, void *info) {
	int res = sceMeAudio_driver_C300D466(codec, unk, info);

	if (res < 0 && codec == 0x1002 && unk == 2)
		return 0;

	return res;
}

typedef struct {
	void *sasCore;
	int grainSamples;
	int maxVoices;
	int outMode;
	int sampleRate;
} SasInitArguments;

SasInitArguments sas_args;
int sas_inited = 0;

int (* sceSasCoreInit)();
int (* sceSasCoreExit)();

int (* __sceSasInit)(void *sasCore, int grainSamples, int maxVoices, int outMode, int sampleRate);

int __sceSasInitPatched(void *sasCore, int grainSamples, int maxVoices, int outMode, int sampleRate) {
	sas_args.sasCore = sasCore;
	sas_args.grainSamples = grainSamples;
	sas_args.maxVoices = maxVoices;
	sas_args.outMode = outMode;
	sas_args.sampleRate = sampleRate;

	sas_inited = 1;

	return __sceSasInit(sasCore, grainSamples, maxVoices, outMode, sampleRate);
}

void PatchSasCore() {
	sceSasCoreInit = (void *)sctrlHENFindFunction("sceSAScore", "sceSasCore_driver", 0xB0F9F98F);
	sceSasCoreExit = (void *)sctrlHENFindFunction("sceSAScore", "sceSasCore_driver", 0xE143A1EA);

	HIJACK_FUNCTION(sctrlHENFindFunction("sceSAScore", "sceSasCore", 0x42778A9F), __sceSasInitPatched, __sceSasInit);

	ClearCaches();
}

void ARKVitaPopsOnModuleStart(SceModule2 * mod){

    static int booted = 0;
    
    // Patch display in PSX exploits
    if(strcmp(mod->modname, "sceDisplay_Service") == 0) {
        DisplaySetFrameBuf = (void*)sctrlHENFindFunction("sceDisplay_Service", "sceDisplay", 0x289D82FE);
        DisplayWaitVblankStart = (void*)sctrlHENFindFunction("sceDisplay_Service", "sceDisplay", 0x984C27E7);
        DisplaySetHoldMode = sctrlHENFindFunction("sceDisplay_Service", "sceDisplay", 0x7ED59BC4);
        if (sceKernelInitApitype() != 0x144){
            patchVitaPopsDisplay(mod);
        }
        goto flush;
    }

    if(strcmp(mod->modname, "sceLoadExec") == 0)
    {
        PatchLoadExec(mod->text_addr, mod->text_size);
        goto flush;
    }

    if (strcmp(mod->modname, "sceLowIO_Driver") == 0) {

		// Protect pops memory
		if (sceKernelInitKeyConfig() == PSP_INIT_KEYCONFIG_POPS) {
			sceKernelAllocPartitionMemory(6, "", PSP_SMEM_Addr, 0x80000, (void *)0x09F40000);
			memset((void *)0x49F40000, 0, 0x80000);
		}

        goto flush;
    }

    if(strcmp(mod->modname, "sceMeCodecWrapper") == 0) {
		HIJACK_FUNCTION(sctrlHENFindFunction(mod->modname, "sceMeAudio_driver", 0xC300D466), sceMeAudio_driver_C300D466_Patched, sceMeAudio_driver_C300D466);
		goto flush;
	}

    if (strcmp(mod->modname, "sceSAScore") == 0) {
		PatchSasCore();
        goto flush;
	}
    
    // Patch Kermit Peripheral Module to load flash0
    if(strcmp(mod->modname, "sceKermitPeripheral_Driver") == 0)
    {
        patchKermitPeripheral(&_ktbl);
        goto flush;
    }

    /*
    if (strcmp(mod->modname, "CWCHEATPRX") == 0) {
		if (sceKernelInitKeyConfig() == PSP_INIT_KEYCONFIG_POPS) {
			hookImportByNID(mod, "ThreadManForKernel", 0x9944F31F, sceKernelSuspendThreadPatched);
			hookImportByNID(mod, "ThreadManForKernel", 0x75156E8F, sceKernelResumeThreadPatched);
			goto flush;
		}
	}
    */

    if (strcmp(mod->modname, "sceKernelLibrary") == 0){
        int keyconfig = sceKernelInitKeyConfig();
        if (keyconfig == PSP_INIT_KEYCONFIG_POPS){
            char* path = sceKernelInitFileName();
            char title[20]; int n; sctrlGetInitPARAM("DISC_ID", NULL, &n, title);
            char* config = oe_malloc(300);
            sprintf(config, "ms0:/__popsconfig__/%s%s", title, strchr(path, '/'));
            int res = sceIoOpen(config, 0, 0);
            oe_free(config);
            if (res == -401){
                colorDebug(0xFF00);
            }
        }
        else {
            int res = sceIoOpen("ms0:/__popsclear__", 0, 0);
            if (res == -402){
                colorDebug(0xFF00);
            }
        }
    }   

    // Boot Complete Action not done yet
    if(booted == 0)
    {
        // Boot is complete
        if(isSystemBooted())
        {

            // Initialize Memory Stick Speedup Cache
            if (se_config->msspeed) msstorCacheInit("ms", 8 * 1024);

            // Set fake framebuffer so that cwcheat can be displayed
            //DisplaySetFrameBuf((void *)fake_vram, PSP_SCREEN_LINE, PSP_DISPLAY_PIXEL_FORMAT_8888, PSP_DISPLAY_SETBUF_NEXTFRAME);
            //memset((void *)fake_vram, 0, SCE_PSPEMU_FRAMEBUFFER_SIZE);

            // Start control poller thread so we can exit via combo on PS1 games
            //startControlPoller();

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

int (*prev_start)(int modid, SceSize argsize, void * argp, int * modstatus, SceKernelSMOption * opt) = NULL;
int StartModuleHandler(int modid, SceSize argsize, void * argp, int * modstatus, SceKernelSMOption * opt){

    SceModule2* mod = (SceModule2*) sceKernelFindModuleByUID(modid);

    if (DisplaySetFrameBuf && sceKernelFindModuleByName("sceKernelLibrary") == NULL){
        static int screen_init = 0;
        if (!screen_init){
            setScreenHandler(&copyPSPVram);
            initVitaPopsVram();
            initScreen(NULL);
            screen_init = 1;
        }
        cls();
        PRTSTR1("mod: %s", mod->modname);
    }

    struct {
        char* name;
        char* path;
    } pops_files[] = {
        {"sceMediaSync", "MEDIASYN.PRX"},
    };

    for (int i=0; i < sizeof(pops_files)/sizeof(pops_files[0]); i++){
        if (strcmp(mod->modname, pops_files[i].name) == 0){
            char path[ARK_PATH_SIZE];
            strcpy(path, ark_config->arkpath);
            strcat(path, pops_files[i].path);
            SceIoStat stat;
            int res = sceIoGetstat(path, &stat);
            if (res>=0){
                sceKernelUnloadModule(modid);
                modid = sceKernelLoadModule(path, 0, NULL);
                return sceKernelStartModule(modid, argsize, argp, modstatus, opt);
            }
        }
    }

    // forward to previous or default StartModule
    if (prev_start) return prev_start(modid, argsize, argp, modstatus, opt);
    return -1;
}