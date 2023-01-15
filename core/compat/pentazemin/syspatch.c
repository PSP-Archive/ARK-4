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
#include "adrenaline_compat.h"
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

int (* _sceChkregGetPsCode)(u8 *pscode);
int sceChkregGetPsCodePatched(u8 *pscode) {
	int res = _sceChkregGetPsCode(pscode);

	pscode[0] = 0x01;
	pscode[1] = 0x00;
	pscode[3] = 0x00;
	pscode[4] = 0x01;
	pscode[5] = 0x00;
	pscode[6] = 0x01;
	pscode[7] = 0x00;

	return res;
}

int sceUmdRegisterUMDCallBackPatched(int cbid) {
	int k1 = pspSdkSetK1(0);
	int res = sceKernelNotifyCallback(cbid, PSP_UMD_NOT_PRESENT);
	pspSdkSetK1(k1);
	return res;
}

int (* sceMeAudio_driver_C300D466)(int codec, int unk, void *info);
int sceMeAudio_driver_C300D466_Patched(int codec, int unk, void *info) {
	int res = sceMeAudio_driver_C300D466(codec, unk, info);

	if (res < 0 && codec == 0x1002 && unk == 2)
		return 0;

	return res;
}

int (* SetIdleCallback)(int flags);
int SetIdleCallbackPatched(int flags) {
	// Only allow idle callback for music player sleep-timer
	if (flags & 8) {
		return SetIdleCallback(flags);
	}

	return 0;
}

int exit_callback(int arg1, int arg2, void *common) {
	sceKernelSuspendAllUserThreads();
	SceAdrenaline *adrenaline = (SceAdrenaline *)ADRENALINE_ADDRESS;
	adrenaline->pops_mode = 0;
	SendAdrenalineCmd(ADRENALINE_VITA_CMD_RESUME_POPS);

	static u32 vshmain_args[0x100];
	memset(vshmain_args, 0, sizeof(vshmain_args));

	vshmain_args[0] = sizeof(vshmain_args);
	vshmain_args[1] = 0x20;
	vshmain_args[16] = 1;

	struct SceKernelLoadExecVSHParam param;

	memset(&param, 0, sizeof(param));
	param.size = sizeof(param);
	param.argp = NULL;
	param.args = 0;
	param.vshmain_args = vshmain_args;
	param.vshmain_args_size = sizeof(vshmain_args);
	param.key = "vsh";

	sctrlKernelExitVSH(&param);

	return 0;
}

int CallbackThread(SceSize args, void *argp) {
	SceUID cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	if (cbid < 0)
		return cbid;

	int (* sceKernelRegisterExitCallback)() = (void *)sctrlHENFindFunction("sceLoadExec", "LoadExecForUser", 0x4AC57943);
	sceKernelRegisterExitCallback(cbid);

	sceKernelSleepThreadCB();

	return 0;
}


SceUID SetupCallbacks() {
	SceUID thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
	if (thid >= 0)
		sceKernelStartThread(thid, 0, 0);
	return thid;
}

int sceKernelWaitEventFlagPatched(int evid, u32 bits, u32 wait, u32 *outBits, SceUInt *timeout) {
	int res = sceKernelWaitEventFlag(evid, bits, wait, outBits, timeout);

	if (*outBits & 0x1) {
		SendAdrenalineCmd(ADRENALINE_VITA_CMD_PAUSE_POPS);
	} else if (*outBits & 0x2) {
		SendAdrenalineCmd(ADRENALINE_VITA_CMD_RESUME_POPS);
	}

	return res;
}

void PatchImposeDriver(u32 text_addr) {
	// Hide volume bar
	_sw(0, text_addr + 0x4AEC);

	HIJACK_FUNCTION(text_addr + 0x381C, SetIdleCallbackPatched, SetIdleCallback);

	if (sceKernelInitKeyConfig() == PSP_INIT_KEYCONFIG_POPS) {
		SetupCallbacks();
		MAKE_DUMMY_FUNCTION(text_addr + 0x91C8, PSP_INIT_KEYCONFIG_GAME);
	}

	REDIRECT_FUNCTION(text_addr + 0x92B0, sceKernelWaitEventFlagPatched);
}

// for screen debugging
int (* DisplaySetFrameBuf)(void*, int, int, int) = NULL;
void AdrenalineOnModuleStart(SceModule2 * mod){

    // System fully booted Status
    static int booted = 0;

    if(strcmp(mod->modname, "sceDisplay_Service") == 0)
    {
        // can use screen now
        DisplaySetFrameBuf = (void*)sctrlHENFindFunction("sceDisplay_Service", "sceDisplay", 0x289D82FE);
        goto flush;
    }

    if (DisplaySetFrameBuf){
        initScreen(DisplaySetFrameBuf);
        PRTSTR(mod->modname);
    }
    
    // load and process settings file
    if(strcmp(mod->modname, "sceMediaSync") == 0)
    {
        loadSettings(&settingsHandler);
        goto flush;
    }

    if (strcmp(mod->modname, "scePower_Service") == 0) {
		PatchPowerService(mod->text_addr);
		PatchPowerService2(mod->text_addr);
        goto flush;
	}

    if (strcmp(mod->modname, "sceUtility_Driver") == 0) {
		PatchUtility();
        goto flush;
	}

    if (strcmp(mod->modname, "sceChkreg") == 0) {
		MAKE_DUMMY_FUNCTION(sctrlHENFindFunction("sceChkreg", "sceChkreg_driver", 0x54495B19), 1);
    	HIJACK_FUNCTION(sctrlHENFindFunction("sceChkreg", "sceChkreg_driver", 0x59F8491D), sceChkregGetPsCodePatched, _sceChkregGetPsCode);
        goto flush;
    }

    if (strcmp(mod->modname, "sceUmd_driver") == 0) {
		REDIRECT_FUNCTION(mod->text_addr + 0xC80, sceUmdRegisterUMDCallBackPatched);
		goto flush;
	}

    if(strcmp(mod->modname, "sceMeCodecWrapper") == 0) {
		HIJACK_FUNCTION(sctrlHENFindFunction(mod->modname, "sceMeAudio_driver", 0xC300D466), sceMeAudio_driver_C300D466_Patched, sceMeAudio_driver_C300D466);
		goto flush;
	}

    if (strcmp(mod->modname, "sceImpose_Driver") == 0) {
		PatchImposeDriver(mod->text_addr);
        goto flush;
	}

    if (strcmp(mod->modname, "sceNpSignupPlugin_Module") == 0) {
		// ImageVersion = 0x10000000
		_sw(0x3C041000, mod->text_addr + 0x38CBC);
		goto flush;
	}

    if (strcmp(mod->modname, "sceVshNpSignin_Module") == 0) {
		// Kill connection error
		_sw(0x10000008, mod->text_addr + 0x6CF4);
		// ImageVersion = 0x10000000
		_sw(0x3C041000, mod->text_addr + 0x96C4);
		goto flush;
	}

    if (strcmp(mod->modname, "sceSAScore") == 0) {
		PatchSasCore();
        goto flush;
	}

    if (strcmp(mod->modname, "sceLowIO_Driver") == 0) {

		// Protect pops memory
		if (sceKernelInitKeyConfig() == PSP_INIT_KEYCONFIG_POPS) {
			sceKernelAllocPartitionMemory661(6, "", PSP_SMEM_Addr, 0x80000, (void *)0x09F40000);
		}

		memset((void *)0x49F40000, 0, 0x80000);
		memset((void *)0xABCD0000, 0, 0x1B0);

		PatchLowIODriver2(mod->text_addr);
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
    PatchIoFileMgr();
    initAdrenaline();
}
