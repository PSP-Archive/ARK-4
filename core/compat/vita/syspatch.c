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
int (*_sctrlKernelLoadExecVSHWithApitype)(int apitype, const char * file, struct SceKernelLoadExecVSHParam * param) = NULL;
int sctrlKernelLoadExecVSHWithApitypeWithUMDemu(int apitype, const char * file, struct SceKernelLoadExecVSHParam * param)
{
    if (apitype == 0x141){ // homebrew API
        sctrlSESetBootConfFileIndex(MODE_INFERNO); // force inferno to simulate UMD drive
        sctrlSESetUmdFile(""); // empty UMD drive (makes sceUmdCheckMedium return false)
    }
    return _sctrlKernelLoadExecVSHWithApitype(apitype, file, param);
}

// patch to remove Adrenaline check in camera_patch_lite plugin
#define FAKE_UID_CAMERA_LITE 0x0B00B1E5
int ioOpenForCameraLite(const char* path, int mode, int flags){
    if (strcmp(path, "flash1:/config.adrenaline") == 0){
        return FAKE_UID_CAMERA_LITE;
    }
    return sceIoOpen(path, mode, flags);
}
int ioCloseForCameraLite(int uid){
    if (uid == FAKE_UID_CAMERA_LITE){
        return 0;
    }
    return sceIoClose(uid);
}

// patch to fix volatile mem issue
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

void ARKVitaOnModuleStart(SceModule2 * mod){

    // System fully booted Status
    static int booted = 0;

    patchFileManagerImports(mod);
    
    patchGameInfoGetter(mod);
    
    // Patch Kermit Peripheral Module to load flash0
    if(strcmp(mod->modname, "sceKermitPeripheral_Driver") == 0)
    {
        patchKermitPeripheral(&_ktbl);
        goto flush;
    }
    
    // Patch PSP Popsman
    if (strcmp(mod->modname, "scePops_Manager") == 0){
        patchPspPopsman(mod);
        goto flush;
    }
    
    // Patch PSP POPS to replace SPU code
    if (strcmp(mod->modname, "pops") == 0)
    {
        patchPspPops(mod);
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
	
    if (strcmp(mod->modname, "camera_patch_lite") == 0) {
        hookImportByNID(mod, "IoFileMgrForKernel", 0x109F50BC, ioOpenForCameraLite);
        hookImportByNID(mod, "IoFileMgrForKernel", 0x810C4BC3, ioCloseForCameraLite);
        goto flush;
	}

    // Boot Complete Action not done yet
    if(booted == 0)
    {
        // Boot is complete
        if(isSystemBooted())
        {
            // Initialize Memory Stick Speedup Cache
            if (se_config->msspeed)
                msstorCacheInit("ms");

            // enable inferno cache
            if (se_config->iso_cache){
                int (*CacheInit)(int, int, int) = sctrlHENFindFunction("PRO_Inferno_Driver", "inferno_driver", 0x8CDE7F95);
                if (CacheInit){
                    se_config->iso_cache_size = 4 * 1024;
                    se_config->iso_cache_num = 16;
                    CacheInit(4 * 1024, 16, 1); // 64KB cache for PS Vita standalone, in kernel
                }
                if (se_config->iso_cache == 2){
                    int (*CacheSetPolicy)(int) = sctrlHENFindFunction("PRO_Inferno_Driver", "inferno_driver", 0xC0736FD6);
                    if (CacheSetPolicy){
                        se_config->iso_cache_policy = CACHE_POLICY_RR;
                        CacheSetPolicy(CACHE_POLICY_RR);
                    }
                }
            }

            // Patch sceKernelExitGame Syscalls
            REDIRECT_FUNCTION(sctrlHENFindFunction("sceLoadExec", "LoadExecForUser", 0x05572A5F), K_EXTRACT_IMPORT(exitLauncher));
            REDIRECT_FUNCTION(sctrlHENFindFunction("sceLoadExec", "LoadExecForUser", 0x2AC9954B), K_EXTRACT_IMPORT(exitLauncher));
            
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

    struct {
        char* name;
        char* path;
    } pops_files[] = {
        {"scePops_Manager", "POPSMAN.PRX"},
        {"sceMediaSync", "MEDIASYN.PRX"},
    };

    // replace files with 6.60 version for PSP POPS
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
    
    // filesystem patches
    initFileSystem();

    // patch loadexec to use inferno for UMD drive emulation (needed for some homebrews to load)
    HIJACK_FUNCTION(K_EXTRACT_IMPORT(sctrlKernelLoadExecVSHWithApitype), sctrlKernelLoadExecVSHWithApitypeWithUMDemu, _sctrlKernelLoadExecVSHWithApitype);

    // Register custom start module
    prev_start = sctrlSetStartModuleExtra(StartModuleHandler);
}
