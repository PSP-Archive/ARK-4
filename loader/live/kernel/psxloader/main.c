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
    .arkpath = DEFAULT_ARK_PATH, // only ms0 available
    .launcher = ARK_XMENU, // use xMenu
    .exec_mode = PSV_POPS, // set to VitaPops mode
    .exploit_id = "ePSX", // ps1 loader name
    .recovery = 0, // no recovery available
};
ARKConfig* ark_config = &_ark_config;

extern int (* _LoadReboot)(void *, unsigned int, void *, unsigned int);
extern void buildRebootBufferConfig(int rebootBufferSize);
extern int LoadReboot(void * arg1, unsigned int arg2, void * arg3, unsigned int arg4);
extern void copyPSPVram(u32*);

extern u8* rebootbuffer;
extern u32 size_rebootbuffer;
extern int iso_mode;

int (* _KernelLoadExecVSHWithApitype)(int, char *, struct SceKernelLoadExecVSHParam *, int);

u32 sctrlHENFindFunction(char* mod, char* lib, u32 nid){
    return FindFunction(mod, lib, nid);
}

// reboot to launcher
void reboot_launcher(){

    char menupath[ARK_PATH_SIZE];
    strcpy(menupath, ark_config->arkpath);
    strcat(menupath, ark_config->launcher);

    struct SceKernelLoadExecVSHParam param;
    memset(&param, 0, sizeof(param));
    param.size = sizeof(param);
    param.args = strlen(menupath) + 1;
    param.argp = menupath;
    param.key = "game";

    PRTSTR1("Running Menu at %s", menupath);
    int res = _KernelLoadExecVSHWithApitype(0x141, menupath, &param, 0x10000);
}

void reboot_game(char* gamepath){

    struct SceKernelLoadExecVSHParam param;
    memset(&param, 0, sizeof(param));
    param.size = sizeof(param);
    param.args = strlen(gamepath) + 1;
    param.argp = gamepath;
    param.key = "pops";

    PRTSTR1("Running Game at %s", gamepath);
    int res = _KernelLoadExecVSHWithApitype(0x144, gamepath, &param, 0x10000);
}

// autoboot bubble support
int reboot_thread(){
    char game[256];

    char* (*init_filename)() = FindFunction("sceInit", "InitForKernel", 0xA6E71B93);

    if (init_filename == NULL){
        reboot_launcher();
        return 0;
    }

    strcpy(game, init_filename());
    char* p = strrchr(game, '/');
    p[1] = 'V';

    int fd = k_tbl->KernelIOOpen(game, PSP_O_RDONLY, 0777);
    if (fd >= 0){
        k_tbl->KernelIOClose(fd);
        reboot_game(game);
    }
    else {
        reboot_launcher();
    }
    return 0;
}

// load and start pops module
void loadstart_pops(){
    
    int (*LoadModule)() = FindFunction("sceModuleManager", "ModuleMgrForKernel", 0x939E4270);
    int (*StartModule)() = FindFunction("sceModuleManager", "ModuleMgrForKernel", 0x3FF74DF1);

    int modid = LoadModule("flash0:/kd/pops_01g.prx", 0, NULL);
    if (modid < 0){
        cls();
        PRTSTR1("modid: %p", modid);
        _sw(0, 0);
    }
    int res = StartModule(modid, 0, NULL, NULL, NULL);
    if (res < 0){
        cls();
        PRTSTR1("res: %p\n", res);
        _sw(0, 0);
    }

}

// old code from original ARK that I finally found a use for
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

// kill pops before it crashes
void kill_pops(){
    // popsmain, mcworker and cdworker
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

    // resolve some useful functions
    scanKernelFunctions(k_tbl);

    // Extremely nasty solution to get screen working fine
    k_tbl->KernelDelayThread(1000000); // wait for system to finish booting up
    loadstart_pops(); // load and start pops module
    k_tbl->KernelDelayThread(1000000); // wait for pops to set up things
    kill_pops(); // kill pops threads to prevent crash

    // initialize screen
    setScreenHandler(&copyPSPVram);
    initVitaPopsVram();
    initScreen(NULL);

    // now we can draw things!
    PRTSTR("Loading ARK-4 in ePSX mode");

    PRTSTR("Patching FLASH0");
    patchKermitPeripheral(k_tbl);

    // Invalidate Cache
    k_tbl->KernelDcacheWritebackInvalidateAll();
    k_tbl->KernelIcacheInvalidateAll();


    PRTSTR("Patching Loadexec");

    // Find LoadExec Module
    SceModule2 * loadexec = k_tbl->KernelFindModuleByName("sceLoadExec");
    // Find Reboot Loader Function
    _LoadReboot = (void *)loadexec->text_addr;

    rebootbuffer = rebootbuffer_vitapops;
    size_rebootbuffer = size_rebootbuffer_vitapops;

    u32 getuserlevel = FindFunction("sceThreadManager", "ThreadManForKernel", 0xF6427665);
    patchLoadExec(loadexec, (u32)LoadReboot, getuserlevel, 3);

    // Invalidate Cache
    k_tbl->KernelDcacheWritebackInvalidateAll();
    k_tbl->KernelIcacheInvalidateAll();

    PRTSTR("Preparing reboot...");

    _KernelLoadExecVSHWithApitype = (void *)findFirstJALForFunction("sceLoadExec", "LoadExecForKernel", 0xD8320A28);
    
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

void clearBSS(void){
    // BSS Start and End Address from Linkfile
    extern char __bss_start, __bss_end;
    
    // Clear Memory
    memset(&__bss_start, 0, &__bss_end - &__bss_start);
}