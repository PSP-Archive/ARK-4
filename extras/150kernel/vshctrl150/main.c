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
extern int (*g_sceCtrlReadBufferPositive) (SceCtrlData *, int count);

// Flush Instruction and Data Cache
void sync_cache()
{
    // Flush Instruction Cache
    sceKernelIcacheInvalidateAll();
    
    // Flush Data Cache
    sceKernelDcacheWritebackInvalidateAll();
}

static void patch_sceCtrlReadBufferPositive(void)
{
    SceModule* mod;

	mod = sceKernelFindModuleByName("sceVshBridge_Driver");
    hookImportByNID(mod, "sceCtrl_driver", 0x1F803938, _sceCtrlReadBufferPositive);
    g_sceCtrlReadBufferPositive = (void*)sctrlHENFindFunction("sceController_Service", "sceCtrl", 0x1F803938);
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

static inline void ascii2utf16(char *dest, const char *src)
{
    while(*src != '\0') {
        *dest++ = *src;
        *dest++ = '\0';
        src++;
    }
    *dest++ = '\0';
    *dest++ = '\0';
}


/*static void patch_sysconf_plugin_module(SceModule2 *mod) {
	u32 p = 0;
	u32 a = 0;
	u32 addr;
	u32 text_addr = mod->text_addr;
	u32 top_addr = text_addr+mod->text_size;
	char str[] = "1.50 ARK-4 CFW";
	for(addr=text_addr; addr<top_addr; addr += 4) {
		if(_lw(addr) == 0x34C600C9 && _lw(addr+8) == 0) {
			a = addr+20;
		}
	}
	for(; addr < top_addr; addr++) {
		if (strcmp(addr, "sysconf_plugin_module") == 0){ 
            p = addr;
        }
	}

	sprintf(str, "1.50 ARK-4 CFW");
    ascii2utf16(p, str);

    _sw(0x3C020000 | ((u32)(p) >> 16), a); // lui $v0,
    _sw(0x34420000 | ((u32)(p) & 0xFFFF), a + 4); // or $v0, $v0,
}
*/




static int vshpatch_module_chain(SceModule2 *mod)
{
    u32 text_addr = mod->text_addr;

    if(0 == strcmp(mod->modname, "vsh_module")) {
        patch_sceCtrlReadBufferPositive();
        goto exit;
    }
	/*
	if(0 == strcmp(mod->modname, "sysconf_plugin_module")) {
        patch_sysconf_plugin_module(mod);
        goto exit;
    }
	*/

  exit:
    sync_cache();
    if (previous) previous(mod);
}


int module_start(SceSize args, void* argp)
{

    previous = sctrlHENSetStartModuleHandler(vshpatch_module_chain);
    hook_directory_io();

    return 0;
}
