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

/*
int sctrlGetThreadUIDByName(const char * name)
{
	// Invalid Arguments
	if(name == NULL) return -1;
	
	// Thread UID List
	int ids[100];
	
	// Clear Memory
	memset(ids, 0, sizeof(ids));
	
	// Thread Counter
	int count = 0;
	
	// Get Thread UIDs
	if(sceKernelGetThreadmanIdList(SCE_KERNEL_TMID_Thread, ids, NELEMS(ids), &count) >= 0)
	{
		// Iterate Results
		int i = 0; for(; i < count; i++)
		{
			// Thread Information
			SceKernelThreadInfo info;
			
			// Clear Memory
			memset(&info, 0, sizeof(info));
			
			// Initialize Structure Size
			info.size = sizeof(info);
			
			// Fetch Thread Status
			if(sceKernelReferThreadStatus(ids[i], &info) == 0)
			{
				// Matching Name
				if(strcmp(info.name, name) == 0)
				{
					
					// Return Thread UID
					return ids[i];
				}
			}
		}
	}
	
	// Thread not found
	return -2;
}

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

    sceKernelDelayThread(1000000);
    
    int popsmain = sctrlGetThreadUIDByName("popsmain");
    sceKernelTerminateDeleteThread(popsmain);

    int mcworker = sctrlGetThreadUIDByName("mcworker");
    sceKernelTerminateDeleteThread(mcworker);

    int cdworker = sctrlGetThreadUIDByName("cdworker");
    sceKernelTerminateDeleteThread(cdworker);

    return 0;
}
*/

// sceKermit_driver_36666181
int sceKermitSendRequest(void* a0, int a1, int a2, int a3, int a4, void* a5){
    static int (*orig)(void*, int, int, int, int, void*) = NULL;

    if (orig == NULL){
        orig = sctrlHENFindFunction("sceKermit_Driver", "sceKermit_driver",0x36666181);
    }

    return orig(a0, a1, a2, a3, a4, a5);
}

int SendKermitCmd(int cmd) {

	char buf[sizeof(SceKermitRequest) + 0x40];
	SceKermitRequest *request_aligned = (SceKermitRequest *)ALIGN((u32)buf, 0x40);
	SceKermitRequest *request_uncached = (SceKermitRequest *)((u32)request_aligned | 0x20000000);
	sceKernelDcacheInvalidateRange(request_aligned, sizeof(SceKermitRequest));

	u8 resp[128];
	int res = sceKermitSendRequest(request_uncached, 9, cmd, 0, 0, resp);
    if (res < 0){
        cls();
        PRTSTR2("%d=%p", cmd, res);
        _sw(0,0);
    }
    return res;
}

void sync_vita(){
    /*
    SendKermitCmd(1047);
    int (*sceKermitMemory_driver_80E1240A)() = sctrlHENFindFunction("sceLowIO_Driver", "sceKermitMemory_driver", 0x80E1240A);
    sceKermitMemory_driver_80E1240A(0x13F80, 128);
    SendKermitCmd(1056);
    */
    //int (* sceKermitPeripheralInitPops)() = (void *)sctrlHENFindFunction("sceKermitPeripheral_Driver", "sceKermitPeripheral", 0xC0EBC631);
	//sceKermitPeripheralInitPops();

    /*
    // sceKermit_driver_36666181: 3844, 3846, 3842, 3843, 3845
    //int cmds[] = { 3842, 3843, 3844, 3845 };
    for (int i=2; i<6; i++){
        SendKermitCmd(3840+i);
    }

    int (*sceDisplay_driver_03F16FD4)() = sctrlHENFindFunction("sceDisplay_Service", "sceDisplay_driver", 0x03F16FD4);
    sceDisplay_driver_03F16FD4(0, 480, 272);

    //int (*powerlock)() = FindFunction("sceSystemMemoryManager", "sceSuspendForKernel", 0xEADB1BD7);
    //powerlock(0);

    int (*sceDisplay_driver_E38CA615)() = sctrlHENFindFunction("sceDisplay_Service", "sceDisplay_driver", 0xE38CA615);
    sceDisplay_driver_E38CA615();

    //int (*sceMeAudio_EB52DFE0)() = FindFunction("scePops_Manager", "sceMeAudio", 0xEB52DFE0);
    //sceMeAudio_EB52DFE0();
    //_sb(0, 0x49FE00A0);

    char buf[sizeof(SceKermitRequest) + 0x40];
	SceKermitRequest *request_aligned = (SceKermitRequest *)ALIGN((u32)buf, 0x40);
	SceKermitRequest *request_uncached = (SceKermitRequest *)((u32)request_aligned | 0x20000000);
	sceKernelDcacheInvalidateRange(request_aligned, sizeof(SceKermitRequest));

    memset(request_uncached, 0, sizeof(SceKermitRequest));
    request_uncached->cmd = 1045;
    request_uncached->args[0] = 7;
    request_uncached->args[1] = (u64)0x00001AF0; //0x00001B3C; // 0x00001AF0

	u8 resp[128];
	int res = sceKermitSendRequest(request_uncached, 9, 1045, 2, 0, resp);

    if (res < 0){
        cls();
        PRTSTR1("%p", res);
        _sw(0,0);
    }
    */
}

int dummythread(int argc, void* argp){
    sceKernelDelayThread(1000000);
    sceKernelExitDeleteThread(0);
    return 0;
}

int kermitSendRequestLog(void* a0, int a1, int a2, int a3, int a4, void* a5){

    static volatile int logging = 0;
    static char tmp[64];

    //if (a1 == 9 && a2 == 1042) return 0;

    if (!logging){
        logging = 1;
        sprintf(tmp, "sceKermitSendRequest - mode: %d, cmd: %d\n", a1, a2);
        int fd = sceIoOpen("ms0:/kermit.log", PSP_O_WRONLY|PSP_O_APPEND|PSP_O_CREAT, 0777);
        sceIoWrite(fd, tmp, strlen(tmp));
        sceIoClose(fd);
        logging = 0;
    }

    return sceKermitSendRequest(a0, a1, a2, a3, a4, a5);
}

int (*sceKermitPeripheral_driver_8C7903E7)() = NULL;
int sceKermitPeripheral_driver_log(void* a0, int a1, int a2, int a3, int a4, void* a5, u32 a6, u32 a7){

    static volatile int logging = 0;
    static char tmp[64];

    //if (a1 == 9 && a2 == 1042) return 0;

    if (!logging){
        logging = 1;
        sprintf(tmp, "sceKermitPeripheral_driver_8C7903E7 - mode: %d, cmd: %d\n", a1, a2);
        int fd = sceIoOpen("ms0:/kermit.log", PSP_O_WRONLY|PSP_O_APPEND|PSP_O_CREAT, 0777);
        sceIoWrite(fd, tmp, strlen(tmp));
        sceIoClose(fd);
        logging = 0;
    }

    return sceKermitPeripheral_driver_8C7903E7(a0, a1, a2, a3, a4, a5, a6, a7);
}

void ARKVitaPopsOnModuleStart(SceModule2 * mod){

    static int booted = 0;

    /*
    if (strcmp(mod->modname, "sceIOFileManager") != 0 && strcmp(mod->modname, "sceKermitMsfs_driver") != 0){
        
    }
    */
    
    // Patch display in PSX exploits
    if(strcmp(mod->modname, "sceDisplay_Service") == 0) {
        DisplaySetFrameBuf = (void*)sctrlHENFindFunction("sceDisplay_Service", "sceDisplay", 0x289D82FE);
        DisplayWaitVblankStart = (void*)sctrlHENFindFunction("sceDisplay_Service", "sceDisplay", 0x984C27E7);
        DisplaySetHoldMode = sctrlHENFindFunction("sceDisplay_Service", "sceDisplay", 0x7ED59BC4);
        //if (sceKernelInitApitype() != 0x144){
            patchVitaPopsDisplay(mod);
        //}
        goto flush;
    }
    
    // Patch Kermit Peripheral Module to load flash0
    if(strcmp(mod->modname, "sceKermitPeripheral_Driver") == 0)
    {
        patchKermitPeripheral(&_ktbl);
        //int (* sceKermitPeripheralInitPops)() = (void *)sctrlHENFindFunction("sceKermitPeripheral_Driver", "sceKermitPeripheral", 0xC0EBC631);
        //sceKermitPeripheralInitPops();
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

    if (strcmp(mod->modname, "scePops_Manager") == 0){
        sceKermitPeripheral_driver_8C7903E7 = sctrlHENFindFunction("sceKermitPeripheral_Driver", "sceKermitPeripheral_driver", 0x8C7903E7);
        hookImportByNID(mod, "sceKermitPeripheral_driver", 0x36666181, sceKermitPeripheral_driver_log);
        hookImportByNID(mod, "sceKermit_driver", 0x36666181, kermitSendRequestLog);
        patchPopsMan(mod);
        goto flush;
    }

    if (strcmp(mod->modname, "pops") == 0) {
		// Use different pops register location
		patchPops(mod);
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

            // Initialize Memory Stick Speedup Cache
            if (se_config->msspeed) msstorCacheInit("ms", 8 * 1024);

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
                /*
                {
                SceUID kthreadID = sceKernelCreateThread( "popsmain", &dummythread, 1, 0x20000, PSP_THREAD_ATTR_VFPU, NULL);
                sceKernelStartThread(kthreadID, 0, NULL);
                }
                {
                SceUID kthreadID = sceKernelCreateThread( "mcworker", &dummythread, 1, 0x20000, PSP_THREAD_ATTR_VFPU, NULL);
                sceKernelStartThread(kthreadID, 0, NULL);
                }
                {
                SceUID kthreadID = sceKernelCreateThread( "cdworker", &dummythread, 1, 0x20000, PSP_THREAD_ATTR_VFPU, NULL);
                sceKernelStartThread(kthreadID, 0, NULL);
                }
                */
                //sync_vita();
                {
                //SceUID kthreadID = sceKernelCreateThread( "popsmain", &sync_vita, 1, 0x20000, PSP_THREAD_ATTR_VFPU|PSP_THREAD_ATTR_NO_FILLSTACK, NULL);
                //sceKernelStartThread(kthreadID, 0, NULL);
                }
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

    /*
    int fd = sceIoOpen("ms0:/vitapops.log", PSP_O_WRONLY|PSP_O_APPEND|PSP_O_CREAT, 0777);
    sceIoWrite(fd, mod->modname, strlen(mod->modname));
    sceIoWrite(fd, "\n", 1);
    sceIoClose(fd);
    */

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

    // forward to previous or default StartModule
    if (prev_start) return prev_start(modid, argsize, argp, modstatus, opt);
    return -1;
}