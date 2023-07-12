#include <pspsdk.h>
#include <globals.h>
#include <macros.h>
#include <module2.h>
#include <pspdisplay_kernel.h>
#include <pspsysmem_kernel.h>
#include <psputilsforkernel.h>
#include <systemctrl.h>
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
static u32* fake_vram = (u32*)0x44000000;
int (* DisplaySetFrameBuf)(void*, int, int, int) = NULL;
int (*DisplayWaitVblankStart)() = NULL;
int (*DisplaySetHoldMode)(int) = NULL;

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
                draw_thread = -1;
            }
		}
	}

	return sceKernelResumeThread(thid);
}

/*
int loadstart_pops(int argc, void* argv){

    int (*LoadModule)() = sctrlHENFindFunction("sceModuleManager", "ModuleMgrForKernel", 0x939E4270);
    int (*StartModule)() = sctrlHENFindFunction("sceModuleManager", "ModuleMgrForKernel", 0x3FF74DF1);

    {
    int modid = LoadModule("flash0:/kd/popsman.prx", 0, NULL);
    if (modid < 0) return 0;
    int res = StartModule(modid, 0, NULL, NULL, NULL);
    }

    {
    int modid = LoadModule("flash0:/kd/pops_01g.prx", 0, NULL);
    if (modid < 0) return 0;
    int res = StartModule(modid, 0, NULL, NULL, NULL);
    }

    return 0;
}
*/

void logtext(char* text){
    int k1 = pspSdkSetK1(0);
    int fd = sceIoOpen("ms0:/pops.log", PSP_O_WRONLY|PSP_O_APPEND|PSP_O_CREAT, 0777);
    sceIoWrite(fd, text, strlen(text));
    sceIoClose(fd);
    pspSdkSetK1(k1);
}

int logmeaudio_10DABACD(int a0, int a1, int a2, int a3, int t0, int t1, int t2, int t3){
    int (*meaudio)() = sctrlHENFindFunction("scePops_Manager", "sceMeAudio", 0x10DABACD);
    logtext("logmeaudio_10DABACD\n");
    return meaudio(a0, a1, a2, a3, t0, t1, t2, t3);
}

int logmeaudio_34F2548F(int a0, int a1, int a2, int a3, int t0, int t1, int t2, int t3){
    int (*meaudio)() = sctrlHENFindFunction("scePops_Manager", "sceMeAudio", 0x34F2548F);
    logtext("logmeaudio_34F2548F\n");
    return meaudio(a0, a1, a2, a3, t0, t1, t2, t3);
}

int logmeaudio_59759606(int a0, int a1, int a2, int a3, int t0, int t1, int t2, int t3){
    int (*meaudio)() = sctrlHENFindFunction("scePops_Manager", "sceMeAudio", 0x59759606);
    logtext("logmeaudio_59759606\n");
    return meaudio(a0, a1, a2, a3, t0, t1, t2, t3);
}

int logmeaudio_6FCDD82D(int a0, int a1, int a2, int a3, int t0, int t1, int t2, int t3){
    int (*meaudio)() = sctrlHENFindFunction("scePops_Manager", "sceMeAudio", 0x6FCDD82D);
    logtext("logmeaudio_6FCDD82D\n");
    return meaudio(a0, a1, a2, a3, t0, t1, t2, t3);
}

int logmeaudio_83609AC9(int a0, int a1, int a2, int a3, int t0, int t1, int t2, int t3){
    int (*meaudio)() = sctrlHENFindFunction("scePops_Manager", "sceMeAudio", 0x83609AC9);
    logtext("logmeaudio_83609AC9\n");
    return meaudio(a0, a1, a2, a3, t0, t1, t2, t3);
}

int logmeaudio_899A3BC0(int a0, int a1, int a2, int a3, int t0, int t1, int t2, int t3){
    int (*meaudio)() = sctrlHENFindFunction("scePops_Manager", "sceMeAudio", 0x899A3BC0);
    logtext("logmeaudio_899A3BC0\n");
    return meaudio(a0, a1, a2, a3, t0, t1, t2, t3);
}

int logmeaudio_9FBE4AD3(int a0, int a1, int a2, int a3, int t0, int t1, int t2, int t3){
    int (*meaudio)() = sctrlHENFindFunction("scePops_Manager", "sceMeAudio", 0x9FBE4AD3);
    logtext("logmeaudio_9FBE4AD3\n");
    return meaudio(a0, a1, a2, a3, t0, t1, t2, t3);
}

int logmeaudio_A651EA7B(int a0, int a1, int a2, int a3, int t0, int t1, int t2, int t3){
    int (*meaudio)() = sctrlHENFindFunction("scePops_Manager", "sceMeAudio", 0xA651EA7B);
    logtext("logmeaudio_A651EA7B\n");
    return meaudio(a0, a1, a2, a3, t0, t1, t2, t3);
}

int logmeaudio_A8176E49(int a0, int a1, int a2, int a3, int t0, int t1, int t2, int t3){
    int (*meaudio)() = sctrlHENFindFunction("scePops_Manager", "sceMeAudio", 0xA8176E49);
    logtext("logmeaudio_A8176E49\n");
    return meaudio(a0, a1, a2, a3, t0, t1, t2, t3);
}

int logmeaudio_C4169D0F(int a0, int a1, int a2, int a3, int t0, int t1, int t2, int t3){
    int (*meaudio)() = sctrlHENFindFunction("scePops_Manager", "sceMeAudio", 0xC4169D0F);
    logtext("logmeaudio_C4169D0F\n");
    return meaudio(a0, a1, a2, a3, t0, t1, t2, t3);
}

int logmeaudio_C47D3670(int a0, int a1, int a2, int a3, int t0, int t1, int t2, int t3){
    int (*meaudio)() = sctrlHENFindFunction("scePops_Manager", "sceMeAudio", 0xC47D3670);
    logtext("logmeaudio_C47D3670\n");
    return meaudio(a0, a1, a2, a3, t0, t1, t2, t3);
}

int logmeaudio_D96DC042(int a0, int a1, int a2, int a3, int t0, int t1, int t2, int t3){
    int (*meaudio)() = sctrlHENFindFunction("scePops_Manager", "sceMeAudio", 0xD96DC042);
    logtext("logmeaudio_D96DC042\n");
    return meaudio(a0, a1, a2, a3, t0, t1, t2, t3);
}

int logmeaudio_EB52DFE0(int a0, int a1, int a2, int a3, int t0, int t1, int t2, int t3){
    int (*meaudio)() = sctrlHENFindFunction("scePops_Manager", "sceMeAudio", 0xEB52DFE0);
    logtext("logmeaudio_EB52DFE0\n");
    return meaudio(a0, a1, a2, a3, t0, t1, t2, t3);
}

int logmeaudio_F1502A62(int a0, int a1, int a2, int a3, int t0, int t1, int t2, int t3){
    int (*meaudio)() = sctrlHENFindFunction("scePops_Manager", "sceMeAudio", 0xF1502A62);
    logtext("logmeaudio_F1502A62\n");
    return meaudio(a0, a1, a2, a3, t0, t1, t2, t3);
}

int logmeaudio_F7CD1362(int a0, int a1, int a2, int a3, int t0, int t1, int t2, int t3){
    int (*meaudio)() = sctrlHENFindFunction("scePops_Manager", "sceMeAudio", 0xF7CD1362);
    logtext("logmeaudio_F7CD1362\n");
    return meaudio(a0, a1, a2, a3, t0, t1, t2, t3);
}

void* meaudio_loggers[] = {
    &logmeaudio_10DABACD,
    &logmeaudio_34F2548F,
    &logmeaudio_59759606,
    &logmeaudio_6FCDD82D,
    &logmeaudio_83609AC9,
    &logmeaudio_899A3BC0,
    &logmeaudio_9FBE4AD3,
    &logmeaudio_A651EA7B,
    &logmeaudio_A8176E49,
    &logmeaudio_C4169D0F,
    &logmeaudio_C47D3670,
    &logmeaudio_D96DC042,
    &logmeaudio_EB52DFE0,
    &logmeaudio_F1502A62,
    &logmeaudio_F7CD1362
};

u32 meaudio_nids[] = {
    0x10DABACD,
    0x34F2548F,
    0x59759606,
    0x6FCDD82D,
    0x83609AC9,
    0x899A3BC0,
    0x9FBE4AD3,
    0xA651EA7B,
    0xA8176E49,
    0xC4169D0F,
    0xC47D3670,
    0xD96DC042,
    0xEB52DFE0,
    0xF1502A62,
    0xF7CD1362
};

void doNastyBreakpoint(){
    colorDebug(0xFF0000);
    _sw(0,0);
}

void ARKVitaPopsOnModuleStart(SceModule2 * mod){

    static int booted = 0;
    
    // Patch display in PSX exploits
    if(strcmp(mod->modname, "sceDisplay_Service") == 0) {
        DisplaySetFrameBuf = (void*)sctrlHENFindFunction("sceDisplay_Service", "sceDisplay", 0x289D82FE);
        DisplayWaitVblankStart = (void*)sctrlHENFindFunction("sceDisplay_Service", "sceDisplay", 0x984C27E7);
        DisplaySetHoldMode = sctrlHENFindFunction("sceDisplay_Service", "sceDisplay", 0x7ED59BC4);
        patchVitaPopsDisplay(mod);
        goto flush;
    }
    
    // Patch Kermit Peripheral Module to load flash0
    if(strcmp(mod->modname, "sceKermitPeripheral_Driver") == 0)
    {
        patchKermitPeripheral(&_ktbl);
        goto flush;
    }
    /*
    if (strcmp(mod->modname, "sceIOFileManager") == 0){
        // remove k1 checks -> move $a2, 0
        _sw(0x00003021, 0x8805769c); // IoRead
        _sw(0x00003021, 0x880577b0); // IoWrite
        goto flush;
    }

    if (strcmp(mod->modname, "sceLoadExec") == 0){
        // patch IO checks
        _sw(JR_RA, mod->text_addr + 0x0000222C);
        _sw(LI_V0(0), mod->text_addr + 0x00002230);
        goto flush;
    }
    */

    /*
    if (strcmp(mod->modname, "scePops_Manager") == 0){
        patchPopsMan(mod);
    }

    if (strcmp(mod->modname, "pops") == 0) {
		// Use different pops register location
		patchPops(mod);
        goto flush;
	}
    */

    if (strcmp(mod->modname, "simple") == 0) {
        /*
        for (int i=0; i<14; i++){
            hookImportByNID(mod, "sceMeAudio", meaudio_nids[i], meaudio_loggers[i]);
        }
        */
        REDIRECT_SYSCALL(mod->text_addr + 0x00000124, doNastyBreakpoint);
        //_sw(JUMP(doNastyBreakpoint), mod->text_addr + 0x00014EA0);
        //_sw(NOP, mod->text_addr + 0x00014EA0 + 4);
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

    // Boot Complete Action not done yet
    if(booted == 0)
    {
        // Boot is complete
        if(isSystemBooted())
        {
            // Set fake framebuffer so that cwcheat can be displayed
            //DisplaySetFrameBuf((void *)fake_vram, PSP_SCREEN_LINE, PSP_DISPLAY_PIXEL_FORMAT_8888, PSP_DISPLAY_SETBUF_NEXTFRAME);
            //memset((void *)fake_vram, 0, SCE_PSPEMU_FRAMEBUFFER_SIZE);

            // Start control poller thread so we can exit via combo on PS1 games
            if (sceKernelInitApitype() == 0x144){
                //startControlPoller();
            }
            else {
                //SceUID kthreadID = sceKernelCreateThread( "ark-x-loader", &loadstart_pops, 1, 0x20000, PSP_THREAD_ATTR_VFPU, NULL);
                //sceKernelStartThread(kthreadID, 0, NULL);
            }

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

    if (DisplaySetFrameBuf){
        static int screen_init = 0;
        if (!screen_init){
            setScreenHandler(&copyPSPVram);
            initVitaPopsVram();
            initScreen(DisplaySetFrameBuf);
            screen_init = 1;
        }
        cls();
        PRTSTR1("mod: %s", mod->modname);
    }

    /*
    static SceModule2* popsman = NULL;

    if (mod){

        if (strcmp(mod->modname, "scePops_Manager") == 0){
            //patchPopsMan(mod);
            popsman = mod;
        }

        else if (strcmp(mod->modname, "sceKernelLibrary") == 0 && popsman){
            int modid = sceKernelLoadModule("flash0:/kd/ark_popcorn.prx", 0, 0);
            sceKernelStartModule(modid, 0, 0, 0, 0);
        }

    }
    */

    // forward to previous or default StartModule
    if (prev_start) return prev_start(modid, argsize, argp, modstatus, opt);
    return -1;
}