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
#include "rebootconfig.h"
#include "libs/graphics/graphics.h"
#include "kermit.h"

extern STMOD_HANDLER previous;

int (* DisplaySetFrameBuf)(void*, int, int, int) = NULL;

//static SceModule2* lastmod = NULL;

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

void OnSystemStatusIdle() {
	SceAdrenaline *adrenaline = (SceAdrenaline *)ADRENALINE_ADDRESS;

	initAdrenalineInfo();

	// Set fake framebuffer so that cwcheat can be displayed
	if (adrenaline->pops_mode) {
		DisplaySetFrameBuf((void *)NATIVE_FRAMEBUFFER, PSP_SCREEN_LINE, PSP_DISPLAY_PIXEL_FORMAT_8888, PSP_DISPLAY_SETBUF_NEXTFRAME);
		memset((void *)NATIVE_FRAMEBUFFER, 0, SCE_PSPEMU_FRAMEBUFFER_SIZE);
	} else {
		SendAdrenalineCmd(ADRENALINE_VITA_CMD_RESUME_POPS);
	}
}

// kermit_peripheral's sub_000007CC clone, called by loadexec + 0x0000299C with a0=8 (was a0=7 for fw <210)
// Returns 0 on success
int (* Kermit_driver_4F75AA05)(void* kermit_packet, u32 cmd_mode, u32 cmd, u32 argc, u32 allow_callback, u64 *resp) = NULL;
u64 kermit_flash_load(int cmd)
{
    u8 buf[128];
    u64 resp;
    void *alignedBuf = (void*)ALIGN_64((int)buf + 63);
    sceKernelDcacheInvalidateRange(alignedBuf, 0x40);
    KermitPacket *packet = (KermitPacket *)KERMIT_PACKET((int)alignedBuf);
    u32 argc = 0;
    Kermit_driver_4F75AA05(packet, KERMIT_MODE_PERIPHERAL, cmd, argc, KERMIT_CALLBACK_DISABLE, &resp);
    return resp;
}

int flashLoadPatch(int cmd)
{
    int ret = kermit_flash_load(cmd);
    // Custom handling on loadFlash mode, else nothing
    if ( cmd == KERMIT_CMD_ERROR_EXIT || cmd == KERMIT_CMD_ERROR_EXIT_2 )
    {
        int linked;
        // Wait for flash to load
        sceKernelDelayThread(10000);
        // Load FLASH0.ARK
		RebootConfigARK* reboot_config = sctrlHENGetRebootexConfig(NULL);
		char archive[ARK_PATH_SIZE];
		strcpy(archive, ark_config->arkpath);
		strcat(archive, FLASH0_ARK);
		int fd = sceIoOpen(archive, PSP_O_RDONLY, 0777);
		sceIoRead(fd, reboot_config->flashfs, MAX_FLASH0_SIZE);
		sceIoClose(fd);

        flushCache();
    }
    return ret;
}

u32 findKermitFlashDriver(){
    u32 nids[] = {0x4F75AA05, 0x36666181};
    for (int i=0; i<sizeof(nids)/sizeof(u32) && Kermit_driver_4F75AA05 == NULL; i++){
        Kermit_driver_4F75AA05 = sctrlHENFindFunction("sceKermit_Driver", "sceKermit_driver", nids[i]);
    }
    return Kermit_driver_4F75AA05;
}

int patchKermitPeripheral()
{
    findKermitFlashDriver();
    // Redirect KERMIT_CMD_ERROR_EXIT loadFlash function
    u32 knownnids[2] = { 0x3943440D, 0x0648E1A3 /* 3.3X */ };
    u32 swaddress = 0;
    u32 i;
    for (i = 0; i < 2; i++)
    {
        swaddress = findFirstJAL(sctrlHENFindFunction("sceKermitPeripheral_Driver", "sceKermitPeripheral_driver", knownnids[i]));
        if (swaddress != 0)
            break;
    }
    _sw(JUMP(flashLoadPatch), swaddress);
    _sw(NOP, swaddress+4);
    
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

int memcmp_patched(const void *b1, const void *b2, size_t len) {
	u32 tag = 0x4C9494F0;

	if (memcmp(&tag, b2, len) == 0) {
		static u8 kernel661_keys[0x10] = { 0x76, 0xF2, 0x6C, 0x0A, 0xCA, 0x3A, 0xBA, 0x4E, 0xAC, 0x76, 0xD2, 0x40, 0xF5, 0xC3, 0xBF, 0xF9 };
		memcpy((void *)0xBFC00220, kernel661_keys, sizeof(kernel661_keys));
		return 0;
	}

	return memcmp(b1, b2, len);
}

void PatchMemlmd() {
	SceModule2 *mod = sceKernelFindModuleByName("sceMemlmd");
	u32 text_addr = mod->text_addr;
	u32 text_size = mod->text_size;

	// Allow 6.61 kernel modules
	/*
	for (u32 addr=text_addr; addr<text_addr+text_size; addr+=4){
		if (_lw(addr) == 0x7C8326C0){
			MAKE_CALL(addr + 84, memcmp_patched);
			break;
		}
	}
	*/
	MAKE_CALL(text_addr + 0x2C8, memcmp_patched);
	
	flushCache();
}

int ReadFile(char *file, void *buf, int size) {
	SceUID fd = sceIoOpen(file, PSP_O_RDONLY, 0);
	if (fd < 0)
		return fd;

	int read = sceIoRead(fd, buf, size);

	sceIoClose(fd);
	return read;
}

int sceResmgrDecryptIndexPatched(void *buf, int size, int *retSize) {
	int k1 = pspSdkSetK1(0);
	*retSize = ReadFile("flash0:/vsh/etc/version.txt", buf, size);
	pspSdkSetK1(k1);
	return 0;
}

void settingsHandler(char* path){
    //int apitype = sceKernelInitApitype();
}

void AdrenalineOnModuleStart(SceModule2 * mod){

    // System fully booted Status
    static int booted = 0;
	
	/*
	if (DisplaySetFrameBuf){
		initScreen(DisplaySetFrameBuf);
    	PRTSTR1("Cur Mod: %s", mod->modname);
	}
	*/

    if(strcmp(mod->modname, "sceDisplay_Service") == 0)
    {
        // can use screen now
        DisplaySetFrameBuf = (void*)sctrlHENFindFunction("sceDisplay_Service", "sceDisplay", 0x289D82FE);
        goto flush;
    }

	// Patch Kermit Peripheral Module to load flash0
    if(strcmp(mod->modname, "sceKermitPeripheral_Driver") == 0)
    {
        patchKermitPeripheral();
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

    if (strcmp(mod->modname, "sceLoadExec") == 0) {
		PatchLoadExec(mod->text_addr, mod->text_size);
        goto flush;
	}

	if (strcmp(mod->modname, "scePower_Service") == 0) {
		PatchPowerService(mod->text_addr);
		PatchPowerService2(mod->text_addr);
        goto flush;
	}
    
	if (strcmp(mod->modname, "sceChkreg") == 0) {
		MAKE_DUMMY_FUNCTION(sctrlHENFindFunction("sceChkreg", "sceChkreg_driver", 0x54495B19), 1);
    	HIJACK_FUNCTION(sctrlHENFindFunction("sceChkreg", "sceChkreg_driver", 0x59F8491D), sceChkregGetPsCodePatched, _sceChkregGetPsCode);
        goto flush;
    }

    if (strcmp(mod->modname, "sceMesgLed") == 0) {
		REDIRECT_FUNCTION(sctrlHENFindFunction("sceMesgLed", "sceResmgr_driver", 0x9DC14891), sceResmgrDecryptIndexPatched);
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

    if (strcmp(mod->modname, "sceUtility_Driver") == 0) {
		PatchUtility();
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

	// load and process settings file
    if(strcmp(mod->modname, "sceMediaSync") == 0)
    {
        //PatchMediaSync(mod->text_addr);
        loadSettings(&settingsHandler);
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

            // patch bug in ePSP volatile mem
            _sceKernelVolatileMemTryLock = (void *)sctrlHENFindFunction("sceSystemMemoryManager", "sceSuspendForUser", 0xA14F40B2);
            sctrlHENPatchSyscall((u32)_sceKernelVolatileMemTryLock, sceKernelVolatileMemTryLockPatched);
            
            // fix sound bug in ePSP (make sceAudioOutput2Release behave like real PSP)
			/*
            _sceAudioOutput2GetRestSample = (void *)sctrlHENFindFunction("sceAudio_Driver", "sceAudio", 0x647CEF33);
            _sceAudioOutput2Release = (void *)sctrlHENFindFunction("sceAudio_Driver", "sceAudio", 0x43196845);
            sctrlHENPatchSyscall((u32)_sceAudioOutput2Release, sceAudioOutput2ReleaseFixed);
			*/
			OnSystemStatusIdle();

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
		if (mod == NULL || modid < 0){
			initScreen(DisplaySetFrameBuf);
    	    PRTSTR1("Cur Mod ID: %p", modid);
			if (lastmod) PRTSTR1("Last mod: %s", lastmod->modname);
			while(1){};
		}
	}
	lastmod = mod;
	*/

    // forward to previous or default StartModule
    if (prev_start) return prev_start(modid, argsize, argp, modstatus, opt);
    return -1;
}

void AdrenalineSysPatch(){
    SceModule2* loadcore = patchLoaderCore();
    PatchIoFileMgr();
    PatchMemlmd();
    initAdrenaline();
	// patch loadexec to use inferno for UMD drive emulation (needed for some homebrews to load)
    patchLoadExecUMDemu();
}
