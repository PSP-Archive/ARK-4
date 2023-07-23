#include <pspsdk.h>
#include <pspsysmem_kernel.h>
#include <psputilsforkernel.h>
#include <pspinit.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <systemctrl_private.h>
#include <globals.h> 
#include "functions.h"
#include "macros.h"
#include "exitgame.h"
#include "popspatch.h"
#include "libs/graphics/graphics.h"

extern STMOD_HANDLER previous;

extern void exitLauncher();

extern SEConfig* se_config;

int (* DisplaySetFrameBuf)(void*, int, int, int) = NULL;

KernelFunctions _ktbl = { // for vita flash patcher
    .KernelDcacheInvalidateRange = &sceKernelDcacheInvalidateRange,
    .KernelIcacheInvalidateAll = &sceKernelIcacheInvalidateAll,
    .KernelDcacheWritebackInvalidateAll = &sceKernelDcacheWritebackInvalidateAll,
    .KernelIOOpen = &sceIoOpen,
    .KernelIORead = &sceIoRead,
    .KernelIOWrite = &sceIoWrite,
    .KernelIOClose = &sceIoClose,
    .KernelIOMkdir = &sceIoMkdir,
    .KernelDelayThread = &sceKernelDelayThread,
};

// Return Game Product ID of currently running Game
int sctrlARKGetGameID(char gameid[GAME_ID_MINIMUM_BUFFER_SIZE])
{
    // Invalid Arguments
    if(gameid == NULL) return -1;
    
    // Elevate Permission Level
    unsigned int k1 = pspSdkSetK1(0);
    
    // Fetch Game Information Structure
    void * gameinfo = SysMemForKernel_EF29061C_Fixed();
    
    // Restore Permission Level
    pspSdkSetK1(k1);
    
    // Game Information unavailable
    if(gameinfo == NULL) return -3;
    
    // Copy Product Code
    memcpy(gameid, gameinfo + 0x44, GAME_ID_MINIMUM_BUFFER_SIZE - 1);
    
    // Terminate Product Code
    gameid[GAME_ID_MINIMUM_BUFFER_SIZE - 1] = 0;
    
    // Return Success
    return 0;
}

// This patch injects Inferno with no ISO to simulate an empty UMD drive on homebrew
int sctrlKernelLoadExecVSHWithApitypeWithUMDemu(int apitype, const char * file, struct SceKernelLoadExecVSHParam * param)
{
    // Elevate Permission Level
    unsigned int k1 = pspSdkSetK1(0);
    
    if (apitype == 0x141){ // homebrew API
        sctrlSESetBootConfFileIndex(MODE_INFERNO); // force inferno to simulate UMD drive
        sctrlSESetUmdFile(""); // empty UMD drive (makes sceUmdCheckMedium return false)
    }
    
    // Find Target Function
    int (* _LoadExecVSHWithApitype)(int, const char*, struct SceKernelLoadExecVSHParam*, unsigned int)
        = (void *)findFirstJAL(sctrlHENFindFunction("sceLoadExec", "LoadExecForKernel", 0xD8320A28));

    // Load Execute Module
    int result = _LoadExecVSHWithApitype(apitype, file, param, 0x10000);
    
    // Restore Permission Level on Failure
    pspSdkSetK1(k1);
    
    // Return Error Code
    return result;
}

void patchLoadExecUMDemu(){
    // highjack SystemControl
    u32 func = K_EXTRACT_IMPORT(&sctrlKernelLoadExecVSHWithApitype);
    _sw(JUMP(sctrlKernelLoadExecVSHWithApitypeWithUMDemu), func);
    _sw(NOP, func+4);
    flushCache();
}

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

int (*_sceKernelVolatileMemTryLock)(int unk, void **ptr, int *size);
int sceKernelVolatileMemTryLockPatched(int unk, void **ptr, int *size) {
	int res = 0;

	int i;
	for (i = 0; i < 0x10; i++) {
		res = _sceKernelVolatileMemTryLock(unk, ptr, size);
		if (res >= 0)
			break;

		sceKernelDelayThread(100);
	}

	return res;
}

// this patch makes sceAudioOutput2Release behave like on real PSP (audio is not released if there are still samples left)
int (*_sceAudioOutput2Release)(void);
int (*_sceAudioOutput2GetRestSample)();
int sceAudioOutput2ReleaseFixed(){
    if (_sceAudioOutput2GetRestSample() > 0) return -1;
	return _sceAudioOutput2Release();
}

static void breakPoint(){
    _sw(0x44000000, 0xBC800100);
    colorDebug(0xFF00);
    _sw(0, 0);
}

void ARKVitaOnModuleStart(SceModule2 * mod){

    // System fully booted Status
    static int booted = 0;

    patchFileManagerImports(mod);
    
    patchGameInfoGetter(mod);

    if (strcmp(mod->modname, "sceThreadMan") == 0){
        _sw(JAL(breakPoint), mod->text_addr+0x00017930);
        goto flush;
    }

    if(strcmp(mod->modname, "sceDisplay_Service") == 0) {
        DisplaySetFrameBuf = (void*)sctrlHENFindFunction("sceDisplay_Service", "sceDisplay", 0x289D82FE);
        goto flush;
    }

    // Patch sceKernelExitGame Syscalls
    if(strcmp(mod->modname, "sceLoadExec") == 0)
    {
        REDIRECT_FUNCTION(sctrlHENFindFunction(mod->modname, "LoadExecForUser", 0x05572A5F), K_EXTRACT_IMPORT(exitLauncher));
        REDIRECT_FUNCTION(sctrlHENFindFunction(mod->modname, "LoadExecForUser", 0x2AC9954B), K_EXTRACT_IMPORT(exitLauncher));
        //REDIRECT_FUNCTION(sctrlHENFindFunction(mod->modname, "LoadExecForKernel", 0x08F7166C), K_EXTRACT_IMPORT(exitLauncher));
        goto flush;
    }
    
    // Patch Kermit Peripheral Module to load flash0
    if(strcmp(mod->modname, "sceKermitPeripheral_Driver") == 0)
    {
        patchKermitPeripheral(&_ktbl);
        goto flush;
    }
    
    // Patch PSP Popsman
    if (strcmp(mod->modname, "scePops_Manager") == 0){
        patchPspPopsman(mod);
        // Hook scePopsManExitVSHKernel
        //sctrlHENPatchSyscall((void *)sctrlHENFindFunction("scePops_Manager", "scePopsMan", 0x0090B2C8), K_EXTRACT_IMPORT(exitLauncher));
        goto flush;
    }
    
    // Patch PSP POPS SPU
    if (strcmp(mod->modname, "pops") == 0)
    {
        //breakPoint();
        patchPspPopsSpu(mod);
        goto flush;
    }

    // VLF Module Patches
    if(strcmp(mod->modname, "VLF_Module") == 0)
    {
        // Patch VLF Module
        patchVLF(mod);
        // Exit Handler
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

            // enable inferno cache
            if (se_config->iso_cache){
                int (*CacheInit)(int, int, int) = sctrlHENFindFunction("PRO_Inferno_Driver", "inferno_driver", 0x8CDE7F95);
                if (CacheInit){
                    CacheInit(32 * 1024, 64, 11); // 2MB cache for PS Vita standalone
                }
            }
            
            // Apply Directory IO PSP Emulation
            patchFileSystemDirSyscall();

            // patch bug in ePSP volatile mem
            _sceKernelVolatileMemTryLock = (void *)sctrlHENFindFunction("sceSystemMemoryManager", "sceSuspendForUser", 0xA14F40B2);
            sctrlHENPatchSyscall((u32)_sceKernelVolatileMemTryLock, sceKernelVolatileMemTryLockPatched);
            
            // fix sound bug in ePSP (make sceAudioOutput2Release behave like real PSP)
            _sceAudioOutput2GetRestSample = (void *)sctrlHENFindFunction("sceAudio_Driver", "sceAudio", 0x647CEF33);
            _sceAudioOutput2Release = (void *)sctrlHENFindFunction("sceAudio_Driver", "sceAudio", 0x43196845);
            sctrlHENPatchSyscall((u32)_sceAudioOutput2Release, sceAudioOutput2ReleaseFixed);

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
    if (DisplaySetFrameBuf){
        static int screen_init = 0;
        if (!screen_init){
            initScreen(DisplaySetFrameBuf);
            screen_init = 1;
        }
        cls();
        PRTSTR1("mod: %s", mod->modname);
    }
    */

    struct {
        char* name;
        char* path;
    } pops_files[] = {
        {"scePops_Manager", "POPSMAN.PRX"},
        {"sceMediaSync", "MEDIASYNC.PRX"},
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

void PROVitaSysPatch(){
    SceModule2* mod = NULL;
    // filesystem patches
    initFileSystem();
    // patch loadexec to use inferno for UMD drive emulation (needed for some homebrews to load)
    patchLoadExecUMDemu();

    // Register custom start module
    prev_start = sctrlSetStartModuleExtra(StartModuleHandler);
}
