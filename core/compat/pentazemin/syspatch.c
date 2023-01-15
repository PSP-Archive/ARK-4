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
#include "libs/graphics/graphics.h"

typedef struct {
    char *name;
    void *buf;
    int size;
    int unk_12;
    int attr;
    int unk_20;
    int argSize;
    int argPartId;
} SceLoadCoreBootModuleInfo;

extern STMOD_HANDLER previous;

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

void settingsHandler(char* path){
    //int apitype = sceKernelInitApitype();
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

SceUID sceKernelLoadModuleBufferBootInitBtcnfPatched(SceLoadCoreBootModuleInfo *info, void *buf, int flags, SceKernelLMOption *option) {

	char path[64];
	sprintf(path, "ms0:/__ADRENALINE__/flash0%s", info->name); //not use flash0 cause of cxmb

	SceUID mod = sceKernelLoadModule661(path, 0, NULL);
	if (mod >= 0)
		return mod;

	return sceKernelLoadModuleBufferBootInitBtcnf661(info->size, buf, flags, option);
}

SceUID (* LoadModuleBufferAnchorInBtcnf)(void *buf, int a1);
SceUID LoadModuleBufferAnchorInBtcnfPatched(void *buf, SceLoadCoreBootModuleInfo *info) {
	char path[64];
	sprintf(path, "ms0:/__ADRENALINE__/flash0%s", info->name);

	SceUID mod = sceKernelLoadModule661(path, 0, NULL);
	if (mod >= 0)
		return mod;

	return LoadModuleBufferAnchorInBtcnf(buf, (info->attr >> 8) & 1);
}

int (*ARKPatchInit)(int (* module_bootstart)(SceSize, void *), void *argp) = NULL;
int AdrenalinePatchInit(int (* module_bootstart)(SceSize, void *), void *argp) {
	u32 init_addr = ((u32)module_bootstart) - 0x1A54;

	// Ignore StopInit
	_sw(0, init_addr + 0x18EC);

	// Redirect load functions to load from MS
	LoadModuleBufferAnchorInBtcnf = (void *)init_addr + 0x1038;
	MAKE_CALL(init_addr + 0x17E4, LoadModuleBufferAnchorInBtcnfPatched);
	_sw(0x02402821, init_addr + 0x17E8); //move $a1, $s2

	_sw(0x02402021, init_addr + 0x1868); //move $a0, $s2
	MAKE_CALL(init_addr + 0x1878, sceKernelLoadModuleBufferBootInitBtcnfPatched);

	flushCache();

	return ARKPatchInit(module_bootstart, argp);
}

// Patch Loader Core Module
SceModule2* patchLoaderCore(void)
{
    // Find Module
    SceModule2* mod = (SceModule2 *)sceKernelFindModuleByName("sceLoaderCore");

    // Fetch Text Address
    u32 start_addr = mod->text_addr;
    u32 topaddr = mod->text_addr+mod->text_size;

    int found = 0;
    for (u32 addr = start_addr; addr<topaddr&&!found; addr+=4){
        u32 data = _lw(addr);
        if (data == 0x02E02021){
            ARKPatchInit = K_EXTRACT_CALL(addr-4);
            _sw(JAL(AdrenalinePatchInit), addr-4);
            found = 1;
        }
    }

    return mod;
}

void AdrenalineOnModuleStart(SceModule2 * mod){

    // System fully booted Status
    static int booted = 0;
    
    // load and process settings file
    if(strcmp(mod->modname, "sceMediaSync") == 0)
    {
        loadSettings(&settingsHandler);
        goto flush;
    }
    
    if (strcmp(mod->modname, "Legacy_Software_Loader") == 0){
        // Remove patch of sceKernelGetUserLevel on sceLFatFs_Driver
        _sw(NOP, mod->text_addr + 0x1140);
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
            //if (use_mscache) msstorCacheInit("ms", 8 * 1024);
            
            // Apply Directory IO PSP Emulation
            //patchFileSystemDirSyscall();

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

void AdrenalineSysPatch(){
    SceModule2* loadcore = patchLoaderCore();

}
