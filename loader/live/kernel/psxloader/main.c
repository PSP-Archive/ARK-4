#include <sdk.h>
#include <psploadexec.h>
#include <psploadexec_kernel.h>
#include <psputility_modules.h>
#include <module2.h>
#include <macros.h>
#include <systemctrl_se.h>
#include <string.h>
#include <functions.h>
#include "libs/graphics/graphics.h"
#include <rebootconfig.h>

#include "core/compat/vitapops/rebootex/payload.h"

ARKConfig _ark_config = {
    .magic = ARK_CONFIG_MAGIC,
    .arkpath = DEFAULT_ARK_PATH, // only ms0 available anyways
    .launcher = ARK_XMENU, // use xMenu
    .exec_mode = PSV_POPS, // set to VitaPops mode
    .exploit_id = "ePSX", // ps1 loader name
    .recovery = 0,
};
ARKConfig* ark_config = &_ark_config;

extern int (* _LoadReboot)(void *, unsigned int, void *, unsigned int);
extern void buildRebootBufferConfig(int rebootBufferSize);
extern int LoadReboot(void * arg1, unsigned int arg2, void * arg3, unsigned int arg4);
extern void copyPSPVram(u32*);

extern u8* rebootbuffer;
extern u32 size_rebootbuffer;
extern int iso_mode;

typedef struct {
	uint32_t cmd; //0x0
	SceUID sema_id; //0x4
	uint64_t *response; //0x8
	uint32_t padding; //0xC
	uint64_t args[14]; // 0x10
} SceKermitRequest; //0x80

#define ALIGN(x, align) (((x) + ((align) - 1)) & ~((align) - 1))

int (* _KernelLoadExecVSHWithApitype)(int, char *, struct SceKernelLoadExecVSHParam *, int);

u32 sctrlHENFindFunction(char* mod, char* lib, u32 nid){
    return FindFunction(mod, lib, nid);
}

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
	k_tbl->KernelDcacheInvalidateRange(request_aligned, sizeof(SceKermitRequest));

	u8 resp[128];
	int res = sceKermitSendRequest(request_uncached, 8, cmd, 0, 0, resp);
    if (res < 0){
        cls();
        PRTSTR2("%d=%p", cmd, res);
        _sw(0,0);
    }
    return res;
}

void sync_vita(){

    // sceKermit_driver_36666181: 3844, 3846, 3842, 3843, 3845
    //int cmds[] = { 3842, 3843, 3844, 3845 };
    for (int i=0; i<10; i++){
        SendKermitCmd(3840+i);
    }

    /*
    int (*sceDisplay_driver_03F16FD4)() = FindFunction("sceDisplay_Service", "sceDisplay_driver", 0x03F16FD4);
    sceDisplay_driver_03F16FD4(0, 0x10);

    int (*powerlock)() = FindFunction("sceSystemMemoryManager", "sceSuspendForKernel", 0xEADB1BD7);
    powerlock(0);

    int (*sceDisplay_driver_E38CA615)() = FindFunction("sceDisplay_Service", "sceDisplay_driver", 0xE38CA615);
    sceDisplay_driver_E38CA615();
    */

    //int (*sceMeAudio_EB52DFE0)() = FindFunction("scePops_Manager", "sceMeAudio", 0xEB52DFE0);
    //sceMeAudio_EB52DFE0();
    _sb(0, 0x49FE00A0);

    /*
    char buf[sizeof(SceKermitRequest) + 0x40];
	SceKermitRequest *request_aligned = (SceKermitRequest *)ALIGN((u32)buf, 0x40);
	SceKermitRequest *request_uncached = (SceKermitRequest *)((u32)request_aligned | 0x20000000);
	k_tbl->KernelDcacheInvalidateRange(request_aligned, sizeof(SceKermitRequest));

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

int reboot_thread(int argc, void* argv){

    /*
    SceModule2* kermit_peripheral = k_tbl->KernelFindModuleByName("sceKermitPeripheral_Driver");
    int (*sceKermitPeripheral_C0EBC631)() = kermit_peripheral->text_addr + 0x00000828;
    sceKermitPeripheral_C0EBC631();
    */

    // launcher reboot
    char menupath[ARK_PATH_SIZE];
    strcpy(menupath, ark_config->arkpath);
    strcat(menupath, ark_config->launcher);
    //char* menupath = "ms0:/PSP/GAME/GTA 2/EBOOT.PBP";

    struct SceKernelLoadExecVSHParam param;
    memset(&param, 0, sizeof(param));
    param.size = sizeof(param);
    param.args = strlen(menupath) + 1;
    param.argp = menupath;
    param.key = "game";

    PRTSTR1("Running Menu at %s", menupath);
    int res = _KernelLoadExecVSHWithApitype(0x141, menupath, &param, 0x10000);
}



void loadstart_pops(){

    //int (*sceKermitPeripheral_C0EBC631)() = FindFunction("sceKermitPeripheral_Driver", "sceKermitPeripheral", 0xC0EBC631);
    //sceKermitPeripheral_C0EBC631();
    
    int (*LoadModule)() = FindFunction("sceModuleManager", "ModuleMgrForKernel", 0x939E4270);
    int (*StartModule)() = FindFunction("sceModuleManager", "ModuleMgrForKernel", 0x3FF74DF1);

    int modid = LoadModule("flash0:/kd/pops_01g.prx", 0, NULL);
    if (modid < 0){
        cls();
        PRTSTR1("modid: %p", modid);
        _sw(0, 0);
    }
    SceModule2* mod = k_tbl->KernelFindModuleByName("pops");
    
    /*
    MAKE_JUMP(mod->text_addr + 0x0001AE8C, mod->text_addr + 0x0001AE8C);
    _sw(NOP, mod->text_addr + 0x0001AE8C + 4);
    */
    //
    int res = StartModule(modid, 0, NULL, NULL, NULL);
    if (res < 0){
        cls();
        PRTSTR1("res: %p\n", res);
        _sw(0, 0);
    }
    /*
    MAKE_DUMMY_FUNCTION_RETURN_0(mod->text_addr + 0x0001AE8C);
    k_tbl->KernelDcacheWritebackInvalidateAll();
    k_tbl->KernelIcacheInvalidateAll();
    */

}

/*
void sync_vita(){
    u32 nids[] = {
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

    for (int i=0; i<14; i++){
        int (*meaudio)(int a0, int a1, int a2, int a3, int t0, int t1, int t2, int t3) = FindFunction("scePops_Manager", "sceMeAudio", nids[i]);
        meaudio(0, 0, 0, 0, 0, 0, 0, 0);
    }
}
*/

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

    int (*KernelGetThreadmanIdList)() = FindFunction("sceThreadManager", "ThreadManForUser", 0x94416130);
    int (*KernelReferThreadStatus)() = FindFunction("sceThreadManager", "ThreadManForUser", 0x17C1684E);
	
	// Get Thread UIDs
	if(KernelGetThreadmanIdList(SCE_KERNEL_TMID_Thread, ids, NELEMS(ids), &count) >= 0)
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
			if(KernelReferThreadStatus(ids[i], &info) == 0)
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

void patch_popsman(){
    /*
    u32 nids[] = {0x9FBE4AD3, 0x10DABACD, 0x7AA3E14B, 0xC47D3670, 0x13099620};
    for (int i=0; i<5; i++){
        u32 addr = FindFunction("scePops_Manager", "sceMeAudio", nids[i]);
        MAKE_DUMMY_FUNCTION_RETURN_0(addr);
    }
    */
}

void kill_pops(){
    // popsmain
    int (*KernelTerminateDeleteThread)(int) = FindFunction("sceThreadManager", "ThreadManForUser", 0x383F7BCC);
    
    int popsmain = sctrlGetThreadUIDByName("popsmain");
    KernelTerminateDeleteThread(popsmain);

    int mcworker = sctrlGetThreadUIDByName("mcworker");
    KernelTerminateDeleteThread(mcworker);

    int cdworker = sctrlGetThreadUIDByName("cdworker");
    KernelTerminateDeleteThread(cdworker);
}

int exploitEntry() __attribute__((section(".text.startup")));
int exploitEntry(){

    clearBSS();

    // Switch to Kernel Permission Level
    setK1Kernel();

    //int (*sceKermitPeripheral_D27C5E03)() = FindFunction("sceKermitPeripheral_Driver", "sceKermitPeripheral", 0xD27C5E03);
    //sceKermitPeripheral_D27C5E03(0);

    //patch_popsman();


    scanKernelFunctions(k_tbl);
    scanArkFunctions(g_tbl);

    loadstart_pops();
    k_tbl->KernelDelayThread(1000000);
    kill_pops();

    g_tbl->config = ark_config;

    // make PRTSTR available for payloads
    g_tbl->prtstr = (void *)&PRTSTR11;

    setScreenHandler(&copyPSPVram);
    initScreen(NULL);
    initVitaPopsVram();

    PRTSTR("Loading ARK-4 in ePSX mode");

    PRTSTR("Patching FLASH0");
    patchKermitPeripheral(k_tbl);

    // Invalidate Cache
    k_tbl->KernelDcacheWritebackInvalidateAll();
    k_tbl->KernelIcacheInvalidateAll();

    PRTSTR("Preparing reboot.");
    // Find LoadExec Module
    SceModule2 * loadexec = k_tbl->KernelFindModuleByName("sceLoadExec");
    // Find Reboot Loader Function
    _LoadReboot = (void *)loadexec->text_addr;

    rebootbuffer = rebootbuffer_vitapops;
    size_rebootbuffer = size_rebootbuffer_vitapops;

    PRTSTR("Patching Loadexec");
    u32 getuserlevel = FindFunction("sceThreadManager", "ThreadManForKernel", 0xF6427665);
    patchLoadExec(loadexec, (u32)LoadReboot, getuserlevel, 3);

    /*
    SceModule2* modman = k_tbl->KernelFindModuleByName("sceModuleManager");
    for (u32 addr=modman->text_addr; addr<modman->text_addr+modman->text_size; addr+=4){
        u32 data = _lw(addr);
        if (data == JUMP(getuserlevel)){
            _sw(JR_RA, addr); // patch sceKernelGetUserLevel stub to make it return 1
            _sw(LI_V0(1), addr + 4);
            break;
        }
        else if (data == 0x37258001){
            u32 call = _lw(addr+16);
            _sw(0x24020000, addr+16); // MODULEMGR_DEVICE_CHECK_1
            int found = 0;
            for (addr+=20; !found; addr+=4){
                if (_lw(addr) == call){
                    _sw(0x24020000, addr); // MODULEMGR_DEVICE_CHECK_2
                    found=1;
                }
            }
        }
        else if (data == 0x34458003 || data == 0x34458006){
            addr = patchDeviceCheck(addr);
        }
    }
    */

    // patch IO checks
    //_sw(JR_RA, loadexec->text_addr + 0x0000222C);
    //_sw(LI_V0(0), loadexec->text_addr + 0x00002230);

    _KernelLoadExecVSHWithApitype = (void *)findFirstJALForFunction("sceLoadExec", "LoadExecForKernel", 0xD8320A28);

    // Invalidate Cache
    k_tbl->KernelDcacheWritebackInvalidateAll();
    k_tbl->KernelIcacheInvalidateAll();

    //int thid = k_tbl->KernelCreateThread("simpleloader", &simpleloader_thread, 0x10, 0x20000, PSP_THREAD_ATTR_VFPU, NULL);
    //k_tbl->KernelStartThread(thid, 0, NULL);

    //sync_vita();
    
    SceUID kthreadID = k_tbl->KernelCreateThread( "ark-x-loader", &reboot_thread, 1, 0x20000, PSP_THREAD_ATTR_VFPU, NULL);
    k_tbl->KernelStartThread(kthreadID, 0, NULL);
    k_tbl->waitThreadEnd(kthreadID, NULL);

    return 0;

}

// Fake K1 Kernel Setting
void setK1Kernel(void){
    // Set K1 to Kernel Value
    __asm__ (
        "nop\n"
        "lui $k1,0x0\n"
    );
}

void setK1User(void){
    // Set K1 to Kernel Value
    __asm__ (
        "nop\n"
        "lui $k1,0x10\n"
    );
}

void clearBSS(void){
    // BSS Start and End Address from Linkfile
    extern char __bss_start, __bss_end;
    
    // Clear Memory
    memset(&__bss_start, 0, &__bss_end - &__bss_start);
}