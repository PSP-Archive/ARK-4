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
#include <psptypes.h>
#include <ark.h>
#include <macros.h>
#include "functions.h"

PSP_MODULE_INFO("VshCtrl", 0x1007, 1, 2);

#define GAME150_PATCH "__150"
static STMOD_HANDLER previous;

extern int _sceCtrlReadBufferPositive(SceCtrlData *ctrl, int count);
extern int (*g_sceCtrlReadBufferPositive) (SceCtrlData *, int count);

typedef struct _HookUserFunctions {
    u32 nid;
    void *func;
} HookUserFunctions;

// Flush Instruction and Data Cache
void sync_cache()
{
    // Flush Instruction Cache
    sceKernelIcacheInvalidateAll();
    
    // Flush Data Cache
    sceKernelDcacheWritebackInvalidateAll();
}

static void Fix150Path(const char *file)
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

static void CorruptIconPatch(SceIoDirent * dir){
    int k1 = pspSdkSetK1(0);

    SceIoStat stat;

    char path[256] = {0};

    if(strchr(dir->d_name, '%') == NULL) {
        sprintf(path, "ms0:/PSP/GAME150/%s%%/EBOOT.PBP", dir->d_name);
        memset(&stat, 0, sizeof(stat));
        if(sceIoGetstat(path, &stat) >= 0) {
            strcpy(dir->d_name, "__SCE"); // hide icon
        }
    }
    pspSdkSetK1(k1);

}

SceUID gamedread(SceUID fd, SceIoDirent * dir) {

    int result = sceIoDread(fd, dir);

    CorruptIconPatch(dir);

    return result;

}

// NEED WORK
/*SceUID gamedopen(const char * dirname)
{

    u32 k1 = pspSdkSetK1(0);

    Fix150Path(dirname);

    SceUID result = sceIoDopen(dirname);

    if(result >= 0) {
        if (stricmp(dirname, "ms0:/PSP/GAME") == 0) {
            sceIoDclose(result);
            result = sceIoDopen("ms0:/PSP/GAME150");
        }
    }

    pspSdkSetK1(k1);
    return result;
}
*/


// GAME150 redirect patch
static void patch_game_plugin_module(SceModule2 *mod) {
    u32 text_addr = mod->text_addr;
    u32 offsets[] = { 0x90dc, 0x9230, 0x92c8, 0x9304, 0x99b8, 0x9de8 };

    strcpy((char *)(text_addr + 0x126ec), "ms0:/PSP/GAME150");

    for (int i = 0; i < sizeof(offsets)/sizeof(u32); i++) {
        u32 addr = text_addr+offsets[i];
        _sw(_lw(addr)-0x10, addr);
    }
}

static void hook_directory_io(){
    HookUserFunctions hook_list[] = {
        //{ 0xB29DDF9C, gamedopen  }, // NEEDS SOME HELP
        { 0xE3EB004C, gamedread  },
        //{ 0xEB092469, gamedclose },
    };
    for(int i = 0; i<sizeof(hook_list)/sizeof(hook_list[0]);i++) {
        void *fp = (void*)sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", hook_list[i].nid);
        if(fp != NULL) {
            sctrlHENPatchSyscall(fp, hook_list[i].func);
        }
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

// Version info patch
static void patch_sysconf_plugin_module(SceModule2 *mod) {
    u32 addrhigh, addrlow;
    u32 text_addr = mod->text_addr;

    //alloc memory for version string
    SceUID uid = sceKernelAllocPartitionMemory(PSP_MEMORY_PARTITION_USER, "", PSP_SMEM_Low, 64, NULL);
    if(uid >= 0)
    {
        char *p = (char *)sceKernelGetBlockHeadAddr(uid);
        // Version info patch

        char verinfo[] = "1.50 ARK-4 CFW";
        ascii2utf16((char *)p, verinfo);

        addrhigh = (u32)p >> 16;
        addrlow = (u32)p & 0xFFFF;

        // lui v0, addrhigh
        _sw(0x3C020000 | addrhigh, text_addr+0x872C);
        // ori v0, v0, addrlow
        _sw(0x34420000 | addrlow, text_addr+0x8730);
    }
}

static void patch_sceCtrlReadBufferPositive(void)
{


    SceModule *mod = sceKernelFindModuleByName("sceVshBridge_Driver");
    hookImportByNID(mod, "sceCtrl_driver", 0x1F803938, _sceCtrlReadBufferPositive);
    g_sceCtrlReadBufferPositive = (void*)sctrlHENFindFunction("sceController_Service", "sceCtrl", 0x1F803938);
    sctrlHENPatchSyscall(g_sceCtrlReadBufferPositive, _sceCtrlReadBufferPositive);
}

static int vshpatch_module_chain(SceModule2 *mod)
{
    u32 text_addr = mod->text_addr;
    
    if(0 == strcmp(mod->modname, "vsh_module")) {
        patch_sceCtrlReadBufferPositive();
        hook_directory_io();
        goto exit;
    }
    if(0 == strcmp(mod->modname, "sysconf_plugin_module")) {
        patch_sysconf_plugin_module(mod);
        goto exit;
    }
	if(0 == strcmp(mod->modname, "game_plugin_module")) {
        patch_game_plugin_module(mod);
        goto exit;
    }

  exit:
    sync_cache();
    if (previous) previous(mod);
}


int module_start(SceSize args, void* argp)
{
    previous = sctrlHENSetStartModuleHandler(vshpatch_module_chain);
    return 0;
}
