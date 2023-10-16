#include <pspsdk.h>
#include <pspsysmem_kernel.h>
#include <psputilsforkernel.h>
#include <pspinit.h>
#include <pspkernel.h>
#include <pspdisplay.h>
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

extern int sceKernelSuspendThreadPatched(SceUID thid);

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

/*
int (*UtilityNetconfInitStart)(pspUtilityNetconfData *data);
int myUtilityNetconfInitStart(pspUtilityNetconfData *data){
    pspUtilityNetconfData* p = data;
    struct pspUtilityNetconfAdhoc* a = p->adhocparam;

    if (p->action == 0){
        memset(p, 0, sizeof(p));
        p->base.size = 0x44;
        p->base.language = 1;
        p->base.graphicsThread = 0x11;
        p->base.accessThread = 0x13;
        p->base.fontThread = 0x12;
        p->base.soundThread = 0x10;
        p->base.result = 0;
        p->base.reserved[0] = 0;
        p->base.reserved[1] = 0;
        p->base.reserved[2] = 0;
        p->base.reserved[3] = 0;
        p->action = 3;
        p->adhocparam = a;
        p->hotspot = 1;
        p->hotspot_connected = 1;
        p->wifisp = 0;

        if (a){
            memset(a, 0, sizeof(a));
            a->timeout = 10;
        }
    }

    return UtilityNetconfInitStart(data);
}
*/

void ARKVitaOnModuleStart(SceModule2 * mod){

    // System fully booted Status
    static int booted = 0;

    patchFileManagerImports(mod);
    
    patchGameInfoGetter(mod);

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
    if (strcmp(mod->modname, "CWCHEATPRX") == 0) {
    	if (sceKernelInitKeyConfig() == PSP_INIT_KEYCONFIG_POPS) {
        	hookImportByNID(mod, "ThreadManForKernel", 0x9944F31F, sceKernelSuspendThreadPatched);
			goto flush;
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

            // enable inferno cache
            if (se_config->iso_cache){
                int (*CacheInit)(int, int, int) = sctrlHENFindFunction("PRO_Inferno_Driver", "inferno_driver", 0x8CDE7F95);
                if (CacheInit){
                    CacheInit(32 * 1024, 64, 11); // 2MB cache for PS Vita standalone
                }
                if (se_config->iso_cache == 2){
                    int (*CacheSetPolicy)(int) = sctrlHENFindFunction("PRO_Inferno_Driver", "inferno_driver", 0xC0736FD6);
                    if (CacheSetPolicy) CacheSetPolicy(CACHE_POLICY_RR);
                }
            }

			//UtilityNetconfInitStart = sctrlHENFindFunction("sceUtility_Driver", "sceUtility", 0x4DB1E739);
            //if (UtilityNetconfInitStart) sctrlHENPatchSyscall(UtilityNetconfInitStart, myUtilityNetconfInitStart);
            
            // Apply Directory IO PSP Emulation
            patchFileSystemDirSyscall();

            //dumpFlashToMs();

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

    struct {
        char* name;
        char* path;
    } pops_files[] = {
        {"scePops_Manager", "POPSMAN.PRX"},
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

void PROVitaSysPatch(){
    SceModule2* mod = NULL;
    // filesystem patches
    initFileSystem();
    // patch loadexec to use inferno for UMD drive emulation (needed for some homebrews to load)
    patchLoadExecUMDemu();

    // Register custom start module
    prev_start = sctrlSetStartModuleExtra(StartModuleHandler);
}
