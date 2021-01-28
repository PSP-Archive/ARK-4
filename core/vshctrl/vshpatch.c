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
static void patch_game_plugin_module(u32 text_addr);
static void patch_vsh_module(SceModule2 * mod);

static void hook_iso_file_io(void);
static void hook_iso_directory_io(void);
//static void patch_sceCtrlReadBufferPositive(void); 
static void patch_Gameboot(SceModule2 *mod); 
static void patch_hibblock(SceModule2 *mod); 
static void patch_msvideo_main_plugin_module(u32 text_addr);
static void patch_htmlviewer_plugin_module(u32 text_addr);
static void patch_htmlviewer_utility_module(u32 text_addr);

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
		patch_game_plugin_module(text_addr);
		sync_cache();
		goto exit;
	}

	if(0 == strcmp(mod->modname, "vsh_module")) {
		patch_vsh_module(mod);
		sync_cache();
		goto exit;
	}

	if(0 == strcmp(mod->modname, "sceVshBridge_Driver")) {
		patch_Gameboot(mod);
        /*
		if(psp_model == PSP_GO && conf.hibblock) {
			patch_hibblock(mod);
		}
        */
		sync_cache();
		goto exit;
	}

	if(0 == strcmp(mod->modname, "msvideo_main_plugin_module")) {
		patch_msvideo_main_plugin_module(text_addr);
		sync_cache();
		goto exit;
	}
    /*
	if( 0 == strcmp(mod->modname, "update_plugin_module")) {
		patch_update_plugin_module((SceModule*)mod);
		sync_cache();
		goto exit;
	}
	*/
    /*
	if(conf.useownupdate && 0 == strcmp(mod->modname, "SceUpdateDL_Library")) {
		patch_SceUpdateDL_Library(text_addr);
		sync_cache();
		goto exit;
	}
	*/

    /*
	if(conf.htmlviewer_custom_save_location && 0 == strcmp(mod->modname, "htmlviewer_plugin_module")) {
		patch_htmlviewer_plugin_module(text_addr);
		sync_cache();
		goto exit;
	}
	
	if(0 == strcmp(mod->modname, "sceVshHVUtility_Module")) {
		patch_htmlviewer_utility_module(text_addr);
		sync_cache();
		goto exit;
	}
	*/

exit:
	if (previous) previous(mod);
}

// sceDisplay_driver_73CA5F45
static int (*sceDisplaySetHoldMode)(int a0) = NULL;

static int _sceDisplaySetHoldMode(int a0)
{
	//if (conf.skipgameboot == 0) {
		return (*sceDisplaySetHoldMode)(a0);
	//}

	//return 0;
}

/*
static void patch_sceCtrlReadBufferPositive(void)
{
	SceModule* mod;

	mod = sceKernelFindModuleByName("sceVshBridge_Driver");
	hookImportByNID(mod, "sceCtrl_driver", 0xBE30CED0, _sceCtrlReadBufferPositive, 0);
	g_sceCtrlReadBufferPositive = (void *) sctrlHENFindFunction("sceController_Service", "sceCtrl", 0x1F803938);
	sctrlHENPatchSyscall(g_sceCtrlReadBufferPositive, _sceCtrlReadBufferPositive);
}
*/

static void patch_Gameboot(SceModule2 *mod)
{
	_sw(JAL(_sceDisplaySetHoldMode), mod->text_addr + 0x00001A34);
	sceDisplaySetHoldMode = (void*)(mod->text_addr + 0x00005630);
}

/*
static void patch_hibblock(SceModule2 *mod)
{
	MAKE_DUMMY_FUNCTION_RETURN_0(mod->text_addr + 0x000051C8);
}
*/

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

static const char *g_ver_checklist[] = {
	"release:",
	"build:",
	"system:",
	"vsh:",
	"target:",
};

static int load_version_txt(void *buf, int size)
{
	SceUID fd;

   	fd = sceIoOpen("ms0:/seplugins/version.txt", PSP_O_RDONLY, 0777);

	if(fd < 0) {
		fd = sceIoOpen("ef0:/seplugins/version.txt", PSP_O_RDONLY, 0777);
	}

	if(fd < 0) {
		return fd;
	}

	size = sceIoRead(fd, buf, size);
	sceIoClose(fd);

	return size;
}

static int check_valid_version_txt(const void *buf, int size)
{
	const char *p;
	int i;

	p = buf;

	if(size < 159) {
		return -1;
	}

	for(i=0; i<NELEMS(g_ver_checklist); ++i) {
		if(p - (const char*)buf >= size) {
			return -2;
		}

		if(0 != strncmp(p, g_ver_checklist[i], strlen(g_ver_checklist[i]))) {
			return -3;
		}

		while(*p != '\n' && p - (const char*)buf < size) {
			p++;
		}

		if(p - (const char*)buf >= size) {
			return -4;
		}

		p++;
	}

	return 0;
}

static void patch_sysconf_plugin_module(SceModule2 *mod)
{
	void *p = NULL;
	char str[30];
	u32 text_addr;
	u32 minor_version;

	text_addr = mod->text_addr;
	//minor_version = sctrlHENGetMinorVersion();

	sprintf(str, "ARK-%d", 4);

    /*
	if(minor_version != 0) {
		sprintf(str+strlen(str), "%d", (uint)minor_version);
	}
	*/
	
	p = (void*)(text_addr + 0x0002A62C);
	ascii2utf16(p, str);
	_sw(0x3C020000 | ((u32)(p) >> 16), text_addr + 0x000192E0); // lui $v0, 
	_sw(0x34420000 | ((u32)(p) & 0xFFFF), text_addr + 0x000192E0 + 4); // or $v0, $v0, 
	
	/*
	for (u32 addr=mod->text_addr; addr<mod->text_addr+mod->text_size; addr+=4){
	    u32 data = _lw(addr);
	    if (data == 0x00402821 && _lw(addr+4) == 0x03A02021 && _lw(addr+8)==0x24620000){
	        p = addr-8;
	        break;
	    }
	}
	if (p!=text_addr + 0x000192E0){
	    colorDebug(0xff);
	    while(1){};
	}
	ascii2utf16(p, str);
	for (u32 addr=mod->text_addr; addr<mod->text_addr+mod->text_size; addr+=4){
	    u32 data = _lw(addr);
	    if (data == 0x02602821 && _lw(addr+12) == 0xAFA00090){
	        _sw(0x3C020000 | ((u32)(p) >> 16), addr); // lui $v0, 
        	_sw(0x34420000 | ((u32)(p) & 0xFFFF), addr + 4); // or $v0, $v0, 
	        break;
	    }
	}
	*/

	hookImportByNID((SceModule*)mod, "IoFileMgrForUser", 0x06A70004, myIoMkdir);

    /*
	if(psp_model == PSP_1000 && conf.slimcolor) {
		u32 patch_addr, value;

		patch_addr = 0x000076EC + text_addr;
		value = *(u32 *)(patch_addr + 4);

		_sw(0x24020001, patch_addr + 4);
		_sw(value,  patch_addr);
	}
	*/
}

int fakeParamInexistance(void)
{
	return 0x80120005;
}

static void patch_game_plugin_module(u32 text_addr)
{
	//disable executable check for normal homebrew
	MAKE_DUMMY_FUNCTION_RETURN_0(text_addr + 0x00020528);

	//kill ps1 eboot check
	MAKE_DUMMY_FUNCTION_RETURN_0(text_addr + 0x00020E6C);

	//kill multi-disc ps1 check
	_sw(NOP, text_addr + 0x00014850);

    /*
	if (conf.hidepic) {
		_sw(0x00601021, text_addr + 0x0001D858);
		_sw(0x00601021, text_addr + 0x0001D864);
	}
	
	if (conf.skipgameboot) {
		_sw(MAKE_CALL(text_addr + 0x000194B0), text_addr + 0x00019130);
		_sw(0x24040002, text_addr + 0x00019130 + 4);
	}
	*/

	// disable check for custom psx eboot restore 
	// rif file check
	_sw(0x00001021, text_addr + 0x0002062C);
	// rif content memcmp check
	_sw(NOP, text_addr + 0x00020654);
	// some type check, branch it
	_sw(0x10000010, text_addr + 0x00020668);
	// fake npdrm call
	_sw(0x00001021, text_addr + 0x000206D0);
}

static void patch_msvideo_main_plugin_module(u32 text_addr)
{
    u32 offsets1[] = {
		0x0003AF24,
		0x0003AFAC,
		0x0003D7EC,
		0x0003DA48,
		0x000441A0,
		0x000745A0,
		0x00088BF0,
	};
	u32 offsets2[] = {
    	0x0003D764,
		0x0003D7AC,
		0x00043248,
	};
	/* Patch resolution limit to (130560) pixels (480x272) */
	for (int i=0; i<sizeof(offsets1)/sizeof(u32); i++){
	    _sh(0xFE00, text_addr + offsets1[i]);
	}
	/* Patch bitrate limit (increase to 16384+2) */
	for (int i=0; i<sizeof(offsets2)/sizeof(u32); i++){
	    _sh(0x4003, text_addr + offsets2[i]);
	}
}

/*
static void patch_htmlviewer_plugin_module(u32 text_addr)
{
	char *p;

	p = (void*)(text_addr + 0x0001C7FC); // "/PSP/COMMON"

	strcpy(p, "/ISO");
	
	p = (void*)(text_addr + 0x0001C2B0); // "http://manuals.playstation.net/document/pspindex.html"
	
	strcpy(p, "http://www.prometheus.uk.to/manual/index.html");
}

static void patch_htmlviewer_utility_module(u32 text_addr)
{
	char *p;

	p = (void*)(text_addr + 0x0000CDBC); // "http://manuals.playstation.net/document/pspindex.html"
	
	strcpy(p, "http://www.prometheus.uk.to/manual/index.html");
}
*/

int umdLoadExec(char * file, struct SceKernelLoadExecVSHParam * param)
{
	//result
	int ret = 0;
	//SEConfig config;

	printk("%s: %s %s\n", __func__, file, param->key);
	printk("%s: %d %s\n", __func__, (int)sctrlSEGetBootConfFileIndex(), sctrlSEGetUmdFile());

	//sctrlSEGetConfig(&config);

	//if(sctrlSEGetBootConfFileIndex() == MODE_VSHUMD) {
		sctrlSESetBootConfFileIndex(MODE_VSHUMD);
		sctrlSESetDiscType(PSP_UMD_TYPE_GAME);
	//}
	
    /*
	//enable high memory on demand
	if(config.retail_high_memory) sctrlHENSetMemory(55, 0);
	*/

	/*if(psp_model == PSP_GO) {
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
		ret = sctrlKernelLoadExecVSHWithApitype(apitype, file, param);
	} else {*/
		ret = sceKernelLoadExecVSHDisc(file, param);
	//}

	return ret;
}

int umdLoadExecUpdater(char * file, struct SceKernelLoadExecVSHParam * param)
{
	//result
	int ret = 0;
	//SEConfig config;

	printk("%s: %s %s\n", __func__, file, param->key);
	printk("%s: %d %s\n", __func__, (int)sctrlSEGetBootConfFileIndex(), sctrlSEGetUmdFile());

	//sctrlSEGetConfig(&config);

	if(sctrlSEGetBootConfFileIndex() == MODE_VSHUMD) {
		sctrlSESetBootConfFileIndex(MODE_UPDATERUMD);
		sctrlSESetDiscType(PSP_UMD_TYPE_GAME);
	}

	ret = sceKernelLoadExecVSHDiscUpdater(file, param);

	return ret;
}

/*
static void patch_vsh_module_for_pspgo_umdvideo(SceModule2 *mod)
{
	u32 text_addr = mod->text_addr, prev, i;
    u32 offsets[] = {
	    0x0000670C,
	    0x0002068C,
	    0x0002D240,
    };
	for(i=0; i<sizeof(offsets)/sizeof(u32); ++i) {
		u32 offset = offsets[i];
		prev = _lw(text_addr + offset + 4);
		_sw(prev, text_addr + offset);
		_sw(0x24020000 | PSP_4000, text_addr + offset + 4);
	}
}
*/

static void patch_vsh_module(SceModule2 * mod)
{
	//enable homebrew boot
	int patches = 2;
	for (u32 addr=mod->text_addr; addr<mod->text_addr+mod->text_size && patches; addr+=4){
	    u32 data = _lw(addr);
	    if (data == 0x8E630000 && _lw(addr+8) == 0x8FA60020){
	        _sw(NOP, addr+4);
	        patches--;
	    }
	    else if (data == 0x2407000D && _lw(addr+8) == 0x83A2006C){
	        _sw(NOP, addr+4);
	        _sw(NOP, addr+12);
	        patches--;
	    }
	}

	hookImportByNID((SceModule *)mod, "sceVshBridge", 0x21D4D038, gameloadexec);
	hookImportByNID((SceModule *)mod, "sceVshBridge", 0xE533E98C, gameloadexec);
	hookImportByNID((SceModule *)mod, "sceVshBridge", 0x63E69956, umdLoadExec);
	hookImportByNID((SceModule *)mod, "sceVshBridge", 0x81682A40, umdLoadExecUpdater);

    SceUID block_id = sceKernelAllocPartitionMemory(PSP_MEMORY_PARTITION_USER, "", PSP_SMEM_High, 2 * sizeof(u32), NULL);
	u32 stub = (u32)sceKernelGetBlockHeadAddr(block_id);
	_sw(JR_RA, stub);
	_sw(SYSCALL(sceKernelQuerySystemCall(fakeParamInexistance)), stub + 4);

    /*
	if(psp_model == PSP_GO && sctrlSEGetBootConfFileIndex() == MODE_VSHUMD) {
		patch_vsh_module_for_pspgo_umdvideo(mod);
	}
	*/
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

	int i; for(i=0; i<NELEMS(hook_list); ++i) {
		void *fp;

		fp = (void*)sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", hook_list[i].nid);

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

	int i; for(i=0; i<NELEMS(hook_list); ++i) {
		void *fp;

		fp = (void*)sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForUser", hook_list[i].nid);

		if(fp != NULL) {
			sctrlHENPatchSyscall(fp, hook_list[i].func);
		}
	}
}

int vshpatch_init(void)
{
	previous = sctrlHENSetStartModuleHandler(&vshpatch_module_chain);
	vpbp_init();
	hook_iso_file_io();
	hook_iso_directory_io();

	return 0;
}
