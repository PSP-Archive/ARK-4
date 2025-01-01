#include <pspsdk.h>
#include <pspsysmem_kernel.h>
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <pspsysevent.h>
#include <pspiofilemgr.h>
#include <stdio.h>
#include <string.h>
#include <pspumd.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include "macros.h"
#include <ark.h>
#include "functions.h"

PSP_MODULE_INFO("VshCtrl", 0x1007, 1, 2);

#define GAME150_PATCH "__150"

static STMOD_HANDLER previous;

extern int _sceCtrlReadBufferPositive(SceCtrlData *ctrl, int count);
extern int (*g_sceCtrlReadBufferPositive) (SceCtrlData *, int);

// Flush Instruction and Data Cache
void sync_cache()
{
    // Flush Instruction Cache
    sceKernelIcacheInvalidateAll();
    
    // Flush Data Cache
    sceKernelDcacheWritebackInvalidateAll();
}

static void patch_vsh_module(SceModule2* vshmain)
{
    SceModule* mod;

    mod = sceKernelFindModuleByName("sceVshBridge_Driver");
    hookImportByNID(mod, "sceCtrl_driver", 0x1F803938, _sceCtrlReadBufferPositive);
    g_sceCtrlReadBufferPositive = (void *) sctrlHENFindFunction("sceController_Service", "sceCtrl", 0x1F803938);
    sctrlHENPatchSyscall(g_sceCtrlReadBufferPositive, _sceCtrlReadBufferPositive);
}


void Fix150Path(const char *file)
{
	char str[256];

	if (strstr(file, "ms0:/PSP/GAME/") == file) {
		strcpy(str, (char *)file);

		char *p = strstr(str, GAME150_PATCH);
		
		if (p) {
			strcpy((char *)file+13, "150/");
			strncpy((char *)file+17, str+14, p-(str+14));
			strcpy((char *)file+17+(p-(str+14)), p+5);		
		}
	}
}

static inline int is_game_dir(const char *dirname)
{
    const char *p;
    char path[256];
    SceIoStat stat;

    p = strchr(dirname, '/');

    if (p == NULL) {
        return 0;
    }

    if (0 != strnicmp(p, "/PSP/GAME", sizeof("/PSP/GAME")-1)) {
        return 0;
    }

    if (0 == strnicmp(p, "/PSP/GAME/_DEL_", sizeof("/PSP/GAME/_DEL_")-1)) {
        return 0;
    }

    strcpy(path, dirname);
    strcat(path, "/EBOOT.PBP");

    if(0 == sceIoGetstat(path, &stat)) {
        return 0;
    }

    strcpy(path, dirname);
    strcat(path, "/PARAM.PBP");

    if(0 == sceIoGetstat(path, &stat)) {
        return 0;
    }

    return 1;
}

SceUID gamedopen(const char * dirname)
{
    SceUID result;
    u32 k1;

    Fix150Path(dirname);

    result = sceIoDopen(dirname);

    if (result >= 0){
        if(is_game_dir(dirname)) {

            if (strcmp(dirname, "ms0:/PSP/GAME") == 0) {
                k1 = pspSdkSetK1(0);
                sceIoDclose(result);
                result = sceIoDopen("ms0:/PSP/GAME150");
                pspSdkSetK1(k1);
            }

        }
    }

    return result;
}

static void hook_directory_io(){
    void *fp = (void*)sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", 0xB29DDF9C);
    if(fp != NULL) {
        sctrlHENPatchSyscall(fp, gamedopen);
    }
}

static int vshpatch_module_chain(SceModule2 *mod)
{
    u32 text_addr = mod->text_addr;

    if(0 == strcmp(mod->modname, "vsh_module")) {
        patch_vsh_module(mod);
        goto exit;
    }

exit:
    sync_cache();
    if (previous) previous(mod);
}


int module_start(SceSize args, void* argp)
{

    previous = sctrlHENSetStartModuleHandler(&vshpatch_module_chain);
    hook_directory_io();

    return 0;
}