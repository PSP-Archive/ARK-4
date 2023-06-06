/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

#include <pspsdk.h>
#include <pspsysmem_kernel.h>
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <pspsysevent.h>
#include <pspiofilemgr.h>
#include <pspctrl.h>
#include <stdio.h>
#include <string.h>
#include <pspumd.h>
#include "systemctrl.h"
#include "xmbiso.h"
#include "systemctrl.h"
#include "systemctrl_se.h"
#include "systemctrl_private.h"
#include "main.h"
#include "virtual_pbp.h"
#include "macros.h"
#include "strsafe.h"
#include "globals.h"

extern int _sceCtrlReadBufferPositive(SceCtrlData *ctrl, int count);
extern void patch_sceUSB_Driver(void);

extern int (*g_sceCtrlReadBufferPositive) (SceCtrlData *, int);

typedef struct _HookUserFunctions {
    u32 nid;
    void *func;
} HookUserFunctions;

static STMOD_HANDLER previous;
//SEConfig conf;

static void patch_sysconf_plugin_module(SceModule2 *mod);
static void patch_game_plugin_module(SceModule2 *mod);
static void patch_vsh_module(SceModule2 * mod);

static void hook_iso_file_io(void);
static void hook_iso_directory_io(void);
static void patch_sceCtrlReadBufferPositive(void); 
static void patch_Gameboot(SceModule2 *mod); 
static void patch_hibblock(SceModule2 *mod); 
static void patch_msvideo_main_plugin_module(SceModule2* mod);
static void patch_htmlviewer_plugin_module(u32 text_addr);
static void patch_htmlviewer_utility_module(u32 text_addr);

extern SEConfig* se_config;

extern int has_umd_iso;

static int vshpatch_module_chain(SceModule2 *mod)
{
    u32 text_addr;

    text_addr = mod->text_addr;

    if(0 == strcmp(mod->modname, "sysconf_plugin_module")) {
        patch_sysconf_plugin_module(mod);
        sync_cache();
        goto exit;
    }

    if(0 == strcmp(mod->modname, "game_plugin_module")) {
        patch_game_plugin_module(mod);
        sync_cache();
        goto exit;
    }

    if(0 == strcmp(mod->modname, "vsh_module")) {
        patch_vsh_module(mod);
        patch_sceCtrlReadBufferPositive();
        sync_cache();
        goto exit;
    }

    if(0 == strcmp(mod->modname, "msvideo_main_plugin_module")) {
        patch_msvideo_main_plugin_module(mod);
        sync_cache();
        goto exit;
    }

    if( 0 == strcmp(mod->modname, "update_plugin_module")) {
		patch_update_plugin_module(mod);
		sync_cache();
		goto exit;
	}

	if(0 == strcmp(mod->modname, "SceUpdateDL_Library")) {
		patch_SceUpdateDL_Library(mod);
		sync_cache();
		goto exit;
	}

    if(0 == strcmp(mod->modname, "sceVshBridge_Driver")) {
        
        if (se_config->skiplogos){
		    patch_Gameboot(mod);
        }

		if (psp_model == PSP_GO && se_config->hibblock) {
			patch_hibblock(mod);
		}

		sync_cache();
		goto exit;
	}

exit:
    if (previous) previous(mod);
}

static void patch_sceCtrlReadBufferPositive(void)
{
    SceModule* mod;

    mod = sceKernelFindModuleByName("sceVshBridge_Driver");
    hookImportByNID(mod, "sceCtrl_driver", 0xBE30CED0, _sceCtrlReadBufferPositive);
    g_sceCtrlReadBufferPositive = (void *) sctrlHENFindFunction("sceController_Service", "sceCtrl", 0x1F803938);
    sctrlHENPatchSyscall(g_sceCtrlReadBufferPositive, _sceCtrlReadBufferPositive);
}

static void patch_Gameboot(SceModule2 *mod)
{
    hookImportByNID(mod, "sceDisplay_driver", 0x3552AB11, 0);
}

static void patch_hibblock(SceModule2 *mod)
{
    u32 text_addr = mod->text_addr;
    u32 top_addr = text_addr + mod->text_size;
    for (u32 addr=text_addr; addr<top_addr; addr+=4){
        if (_lw(addr) == 0x7C022804){
            MAKE_DUMMY_FUNCTION_RETURN_0(addr - 8);
            break;
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

static const char *g_cfw_dirs[] = {
    "/SEPLUGINS",
    "/ISO",
    "/ISO/VIDEO",
};

int myIoMkdir(const char *dir, SceMode mode)
{
    int ret, i;
    u32 k1;
    if(0 == strcmp(dir, "ms0:/PSP/GAME") || 
            0 == strcmp(dir, "ef0:/PSP/GAME")) {
        k1 = pspSdkSetK1(0);
        for(i=0; i<NELEMS(g_cfw_dirs); ++i) {
            char path[40];
            get_device_name(path, sizeof(path), dir);
            STRCAT_S(path, g_cfw_dirs[i]);
            sceIoMkdir(path, mode);
        }
        pspSdkSetK1(k1);
    }
    ret = sceIoMkdir(dir, mode);
    return ret;
}

static void patch_sysconf_plugin_module(SceModule2 *mod)
{
    u32 text_addr = mod->text_addr;
    u32 top_addr = text_addr+mod->text_size;
    u32 p = 0;
    u32 a = 0;
    u32 addr;
    char str[50];

    SceIoStat stat; int test_fd = sceIoGetstat("flash0:/vsh/resource/13-27.bmp", &stat);

    int patches = (psp_model==PSP_1000 && test_fd >= 0)? 2 : 1;
    for (addr=text_addr; addr<top_addr && patches; addr+=4){
        if (_lw(addr) == 0x34C600C9 && _lw(addr+8) == NOP){
            a = addr+20;
            patches--;
        }
        else if (psp_model == PSP_1000 && test_fd >= 0 && _lw(addr) == 0x26530008 && _lw(addr-4) == 0x24040018){
            // allow slim colors in PSP 1K
            u32 patch_addr, value;

            patch_addr = addr-28;
            value = *(u32 *)(patch_addr + 4);

            _sw(0x24020001, patch_addr + 4);
            _sw(value,  patch_addr);

            patches--;
        }
    }
    
    patches = (se_config->hidemac)? 2:1;
    for (; addr < top_addr && patches; addr++){
        if (strcmp(addr, "sysconf_plugin_module") == 0){
            p = addr;
            patches--;
        }
        else if (se_config->hidemac
                && ((u8*)addr)[0] == 0x25
                && ((u8*)addr)[1] == 0
                && ((u8*)addr)[2] == 0x30
                && ((u8*)addr)[3] == 0
                && ((u8*)addr)[4] == 0x32
                && ((u8*)addr)[5] == 0
                && ((u8*)addr)[6] == 0x58
                && ((u8*)addr)[7] == 0 )
        {
            static const char* format = " [ FW: %d.%d%d Model: %s ] ";
            u32 fw = sceKernelDevkitVersion();
            u32 major = fw>>24;
            u32 minor = (fw>>16)&0xF;
            u32 micro = (fw>>8)&0xF;
            char model[10];
            if (IS_VITA_ADR(ark_config)){
                model[0]='v'; model[1]='P'; model[2]='S'; model[3]='P'; model[4]=0;
            }
            else{
                sprintf(model, "%02dg", (int)psp_model+1);
            }
            sprintf(str, format, major, minor, micro, model);
            ascii2utf16(addr, str);
            patches--;
        }
    }
    

    #if ARK_MICRO_VERSION > 0
    sprintf(str, "ARK %d.%d.%.2i %s", ARK_MAJOR_VERSION, ARK_MINOR_VERSION, ARK_MICRO_VERSION, ark_config->exploit_id);
    #else
    sprintf(str, "ARK %d.%d %s", ARK_MAJOR_VERSION, ARK_MINOR_VERSION, ark_config->exploit_id);
    #endif
    ascii2utf16(p, str);
    
    _sw(0x3C020000 | ((u32)(p) >> 16), a); // lui $v0, 
    _sw(0x34420000 | ((u32)(p) & 0xFFFF), a + 4); // or $v0, $v0, 

    hookImportByNID((SceModule*)mod, "IoFileMgrForUser", 0x06A70004, myIoMkdir);
}

int fakeParamInexistance(void)
{
    return 0x80120005;
}

static void patch_game_plugin_module(SceModule2* mod)
{
    u32 text_addr = mod->text_addr;
    u32 top_addr = text_addr + mod->text_size;
    int patches = 5;
    if (se_config->hidepics) patches++;
    for (u32 addr=text_addr; addr<top_addr && patches; addr+=4){
        u32 data = _lw(addr);
        if (data == 0x8FA400A4){
            //disable executable check for normal homebrew
            u32 p = U_EXTRACT_CALL(addr+40);
            MAKE_DUMMY_FUNCTION_RETURN_0(p);
            patches--;
        }
        else if (data == 0xAFB000E0){
            //kill ps1 eboot check
            MAKE_DUMMY_FUNCTION_RETURN_0(addr-4);
            patches--;
        }
        else if (data == 0x27A701C8){
            //kill multi-disc ps1 check
            _sw(NOP, addr+16);
            patches--;
        }
        else if (data == 0x27BD10F0){
            // disable check for custom psx eboot restore 
            // rif file check
            _sw(0x00001021, addr + 4);
            if (_lw(addr+48) == 0x3463850E){
                // rif content memcmp check
                _sw(NOP, addr+44);
                // some type check, branch it
                _sw(0x10000010, addr + 64);
            }
            patches--;
        }
        else if (data == 0x3C028000){
            _sw(0x00001021, addr-24);
            patches--;
        }
        else if (data == 0x0062A023 && se_config->hidepics){
            _sw(0x00601021, addr+36);
            _sw(0x00601021, addr+48);
            patches--;
        }
    }
}

static void patch_msvideo_main_plugin_module(SceModule2* mod)
{
    u32 text_addr = mod->text_addr;
    u32 top_addr = text_addr + mod->text_size;
    int patches = 10;

    for (u32 addr=text_addr; addr<top_addr && patches; addr+=4){
        u32 data = _lw(addr);
        if ((data && 0xFF00FFFF) == 0x34002C00){
            _sh(0xFE00, addr);
            patches--;
        }
        else if (data == 0x2C420303 || data == 0x2C420FA1){
            _sh(0x4003, addr);
            patches--;
        }
    }
}

int umdLoadExec(char * file, struct SceKernelLoadExecVSHParam * param)
{
    //result
    int ret = 0;

    sctrlSESetDiscType(PSP_UMD_TYPE_GAME);

    if(psp_model == PSP_GO) {
        char devicename[20];
        int apitype;

        file = sctrlSEGetUmdFile();
        ret = get_device_name(devicename, sizeof(devicename), file);

        if(ret == 0 && 0 == stricmp(devicename, "ef0:")) {
            apitype = 0x125;
        } else {
            apitype = 0x123;
        }

        param->key = "umdemu";
        sctrlSESetBootConfFileIndex(MODE_INFERNO);
        ret = sctrlKernelLoadExecVSHWithApitype(apitype, file, param);
    } else {
        sctrlSESetBootConfFileIndex(MODE_UMD);
        sctrlSESetUmdFile("");
        ret = sceKernelLoadExecVSHDisc(file, param);
    }

    return ret;
}

int umdLoadExecUpdater(char * file, struct SceKernelLoadExecVSHParam * param)
{
    //result
    int ret = 0;
    sctrlSESetBootConfFileIndex(MODE_UPDATERUMD);
    sctrlSESetDiscType(PSP_UMD_TYPE_GAME);
    ret = sceKernelLoadExecVSHDiscUpdater(file, param);
    return ret;
}

static void do_pspgo_umdvideo_patch(u32 addr){
    u32 prev = _lw(addr + 4);
    _sw(prev, addr);
    _sw(0x24020000 | PSP_4000, addr + 4);
}

static void patch_vsh_module_for_pspgo_umdvideo(SceModule2 *mod)
{
    if (!has_umd_iso) return;
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

static void patch_vsh_module(SceModule2 * mod)
{
    //enable homebrew boot
    u32 top_addr = mod->text_addr+mod->text_size;
    u32 fakeparam = SYSCALL(sctrlKernelQuerySystemCall(fakeParamInexistance));
    int patches = 6;
    for (u32 addr=mod->text_addr; addr<top_addr && patches; addr+=4){
        u32 data = _lw(addr);
        if (data == 0x27A602EC){
            _sw(NOP, addr+16);
            patches--;
        }
        else if (data == 0x83A2006C && _lw(addr-8) == 0x2407000D){
            _sw(NOP, addr-4);
            _sw(NOP, addr+4);
            patches--;
        }
        else if (data == 0x93A40060){
            _sw(fakeparam, addr-76);
            patches--;
        }
        else if (data == 0x27B60080){
            _sw(fakeparam, addr+16);
            patches--;
        }
        else if (data == 0x8FA30068){
            _sw(fakeparam, addr-72);
            patches--;
        }
        else if (data == 0xAFA000AC){
            _sw(fakeparam, addr-60);
            patches--;
        }
        else if (data == 0x2C430004 && psp_model == PSP_GO){
            // allow PSP Go to use Type 1 Updaters
			_sw( 0x24030002 , addr - 8 ); //addiu      $v1, $zr, 2
        }
        
    }
    
    hookImportByNID((SceModule *)mod, "sceVshBridge", 0x21D4D038, gameloadexec);
    hookImportByNID((SceModule *)mod, "sceVshBridge", 0xE533E98C, gameloadexec);
    hookImportByNID((SceModule *)mod, "sceVshBridge", 0x63E69956, umdLoadExec);
    hookImportByNID((SceModule *)mod, "sceVshBridge", 0x81682A40, umdLoadExecUpdater);
    if(psp_model == PSP_GO && has_umd_iso) {
        patch_vsh_module_for_pspgo_umdvideo(mod);
    }
}

static void hook_iso_file_io(void)
{
    HookUserFunctions hook_list[] = {
        { 0x109F50BC, gameopen,    },
        { 0x6A638D83, gameread,    },
        { 0x810C4BC3, gameclose,   },
        { 0x27EB27B8, gamelseek,   },
        { 0xACE946E8, gamegetstat, },
        { 0xF27A9C51, gameremove,  },
        { 0x1117C65F, gamermdir,   },
        { 0x779103A0, gamerename,  },
        { 0xB8A740F4, gamechstat,  },
    };
    for(int i=0; i<NELEMS(hook_list); ++i) {
        void *fp = (void*)sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", hook_list[i].nid);
        if(fp != NULL) {
            sctrlHENPatchSyscall(fp, hook_list[i].func);
        }
    }
}

static void hook_iso_directory_io(void)
{
    HookUserFunctions hook_list[] = {
        { 0xB29DDF9C, gamedopen  }, 
        { 0xE3EB004C, gamedread  }, 
        { 0xEB092469, gamedclose },
    };
    for(int i=0; i<NELEMS(hook_list); ++i) {
        void *fp = (void*)sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", hook_list[i].nid);
        if(fp != NULL) {
            sctrlHENPatchSyscall(fp, hook_list[i].func);
        }
    }
}

int vshpatch_init(void)
{
    previous = sctrlHENSetStartModuleHandler(&vshpatch_module_chain);
    patch_sceUSB_Driver();
    vpbp_init();
    hook_iso_file_io();
    hook_iso_directory_io();
    return 0;
}
