#include <pspsdk.h>
#include <pspkernel.h>
#include <pspiofilemgr_kernel.h>
#include <pspumd.h>
#include <systemctrl.h>
#include <macros.h>

#include <stdio.h>
#include <string.h>

PSP_MODULE_INFO("DeadEfDriver", 0x3007, 1, 0);

static void do_pspgo_umdvideo_patch(u32 addr){
    u32 prev = _lw(addr + 4);
    _sw(prev, addr);
    _sw(0x24020000 | PSP_4000, addr + 4);
}

static void patch_vsh_module_for_pspgo_umdvideo(SceModule2 *mod)
{
    u32 text_addr = mod->text_addr;
    u32 top_addr = text_addr + mod->text_size;
    int patches = 3;
    for (u32 addr=text_addr; addr<top_addr && patches; addr+=4){
        u32 data = _lw(addr);
        if (data == 0x38840001){
            do_pspgo_umdvideo_patch(addr+40);
            patches--;
        }
        else if (data == 0x3C0500FF){
            do_pspgo_umdvideo_patch(addr-48);
            patches--;
        }
        else if (data == 0x02821007){
            do_pspgo_umdvideo_patch(addr-56);
            patches--;
        }
    }
}

STMOD_HANDLER prev = NULL;
int OnModuleStart(SceModule2* mod){

	if (strcmp(mod->modname, "sceEFlash_driver") == 0){
		MAKE_DUMMY_FUNCTION_RETURN_0(mod->text_addr);
		flushCache();
	}

	if (strcmp(mod->modname, "vsh_module") == 0) {
        patch_vsh_module_for_pspgo_umdvideo(mod);
        flushCache();
    }

	if (prev) return prev(mod);
	return 0;
}

int (* _sceIoAddDrv)(PspIoDrv *drv);
int sceIoAddDrvHook(PspIoDrv *drv){
	if (strcmp(drv->name, "fatef")==0 || strcmp(drv->name, "fateh")==0){
		return 0;
	}
	return _sceIoAddDrv(drv);
}

int (* _sceIoAssign)(const char *dev1, const char *dev2, const char *dev3, int mode, void* unk1, long unk2);
int sceIoAssignHook(const char *dev1, const char *dev2, const char *dev3, int mode, void* unk1, long unk2){
	if (strcmp(dev1, "ef0")==0 || strcmp(dev1, "eh0")==0 || strcmp(dev1, "fatef0")==0 || strcmp(dev1, "fateh0")==0 || strncmp(dev1, "eflash", 6)==0){
		return 0;
	}
	return _sceIoAssign(dev1, dev2, dev3, mode, unk1, unk2);
}

int module_start(){

	prev = sctrlHENSetStartModuleHandler(OnModuleStart);

	u32 IoAddDrv = sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForKernel", 0x8E982A74);
	u32 IoAssign = sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForKernel", 0xB2A628C1);

	HIJACK_FUNCTION(IoAddDrv, sceIoAddDrvHook, _sceIoAddDrv);
	HIJACK_FUNCTION(IoAssign, sceIoAssignHook, _sceIoAssign);

	return 0;
}