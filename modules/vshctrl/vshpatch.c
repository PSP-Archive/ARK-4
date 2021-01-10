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
#include "utils.h"
#include "libs.h"
#include "systemctrl.h"
#include "printk.h"
#include "xmbiso.h"
#include "systemctrl.h"
#include "systemctrl_se.h"
#include "systemctrl_private.h"
#include "main.h"
#include "virtual_pbp.h"
#include "strsafe.h"
#include "vshctrl_patch_offset.h"

extern int _sceCtrlReadBufferPositive(SceCtrlData *ctrl, int count);
extern void patch_sceUSB_Driver(void);

extern int (*g_sceCtrlReadBufferPositive) (SceCtrlData *, int);

typedef struct _HookUserFunctions {
	u32 nid;
	void *func;
} HookUserFunctions;

static STMOD_HANDLER previous;
SEConfig conf;

static void patch_sysconf_plugin_module(SceModule2 *mod);
static void patch_game_plugin_module(u32 text_addr);
static void patch_vsh_module(SceModule2 * mod);

static void hook_iso_file_io(void);
static void hook_iso_directory_io(void);
static void patch_sceCtrlReadBufferPositive(void); 
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
		patch_sceCtrlReadBufferPositive();
		sync_cache();
		goto exit;
	}

	if(0 == strcmp(mod->modname, "sceVshBridge_Driver")) {
		patch_Gameboot(mod);

		if(psp_model == PSP_GO && conf.hibblock) {
			patch_hibblock(mod);
		}

		sync_cache();
		goto exit;
	}

	if(0 == strcmp(mod->modname, "msvideo_main_plugin_module")) {
		patch_msvideo_main_plugin_module(text_addr);
		sync_cache();
		goto exit;
	}

	if( 0 == strcmp(mod->modname, "update_plugin_module")) {
		patch_update_plugin_module((SceModule*)mod);
		sync_cache();
		goto exit;
	}

	if(conf.useownupdate && 0 == strcmp(mod->modname, "SceUpdateDL_Library")) {
		patch_SceUpdateDL_Library(text_addr);
		sync_cache();
		goto exit;
	}

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

exit:
	if (previous)
		return (*previous)(mod);

	return 0;
}

// sceDisplay_driver_73CA5F45
static int (*sceDisplaySetHoldMode)(int a0) = NULL;

static int _sceDisplaySetHoldMode(int a0)
{
	if (conf.skipgameboot == 0) {
		return (*sceDisplaySetHoldMode)(a0);
	}

	return 0;
}

static void patch_sceCtrlReadBufferPositive(void)
{
	SceModule* mod;

	mod = sceKernelFindModuleByName("sceVshBridge_Driver");
	hook_import_bynid(mod, "sceCtrl_driver", g_offs->vshbridge_patch.sceCtrlReadBufferPositiveNID, _sceCtrlReadBufferPositive, 0);
	g_sceCtrlReadBufferPositive = (void *) sctrlHENFindFunction("sceController_Service", "sceCtrl", 0x1F803938);
	sctrlHENPatchSyscall(g_sceCtrlReadBufferPositive, _sceCtrlReadBufferPositive);
}

static void patch_Gameboot(SceModule2 *mod)
{
	_sw(MAKE_CALL(_sceDisplaySetHoldMode), mod->text_addr + g_offs->vshbridge_patch.sceDisplaySetHoldModeCall);
	sceDisplaySetHoldMode = (void*)(mod->text_addr + g_offs->vshbridge_patch.sceDisplaySetHoldMode);
}

static void patch_hibblock(SceModule2 *mod)
{
	MAKE_DUMMY_FUNCTION_RETURN_0(mod->text_addr + g_offs->vshbridge_patch.HibBlockCheck);
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
	void *p;
	char str[30];
	u32 text_addr;
	u32 minor_version;

	text_addr = mod->text_addr;
	minor_version = sctrlHENGetMinorVersion();

	sprintf(str, g_offs->sysconf_plugin_patch.SystemVersionMessage, 'A'+(sctrlHENGetVersion()&0xF)-1);

	if(minor_version != 0) {
		sprintf(str+strlen(str), "%d", (uint)minor_version);
	}

#ifdef NIGHTLY
	strcpy(str, "PRO NIGHTLY");
#endif
	
	p = (void*)(text_addr + g_offs->sysconf_plugin_patch.SystemVersionStr);
	ascii2utf16(p, str);

	_sw(0x3C020000 | ((u32)(p) >> 16), text_addr + g_offs->sysconf_plugin_patch.SystemVersion); // lui $v0, 
	_sw(0x34420000 | ((u32)(p) & 0xFFFF), text_addr + g_offs->sysconf_plugin_patch.SystemVersion + 4); // or $v0, $v0, 

	if (conf.machidden) {
		p = (void*)(text_addr + g_offs->sysconf_plugin_patch.MacAddressStr);
		
		if(conf.useversion) {
			char *tmpbuf;
			int tmpsize;

			tmpsize = 164;
			tmpbuf = oe_malloc(tmpsize);

			if(tmpbuf == NULL) {
				goto out;
			}

			tmpsize = load_version_txt(tmpbuf, tmpsize);

			if(tmpsize > 0 && check_valid_version_txt(tmpbuf, tmpsize) == 0) {
				sprintf(str, "[ Model: 0%dg Fake: %.4s ]", (int)psp_model+1, tmpbuf + sizeof("release:") - 1);
			} else {
out:
				sprintf(str, "[ Model: 0%dg ]", (int)psp_model+1);
			}

			if(tmpbuf != NULL) {
				oe_free(tmpbuf);
			}
		} else {
			sprintf(str, "[ Model: 0%dg ]", (int)psp_model+1);
		}

#ifdef NIGHTLY
		strcpy(str, NIGHTLY);
#endif

		ascii2utf16(p, str);
	}

	hook_import_bynid((SceModule*)mod, "IoFileMgrForUser", 0x06A70004, myIoMkdir, 1);

	if(psp_model == PSP_1000 && conf.slimcolor) {
		u32 patch_addr, value;

		patch_addr = g_offs->sysconf_plugin_patch.SlimColor + text_addr;
		value = *(u32 *)(patch_addr + 4);

		_sw(0x24020001, patch_addr + 4);
		_sw(value,  patch_addr);
	}
}

int fakeParamInexistance(void)
{
	return 0x80120005;
}

static void patch_game_plugin_module(u32 text_addr)
{
	//disable executable check for normal homebrew
	MAKE_DUMMY_FUNCTION_RETURN_0(text_addr + g_offs->game_plugin_patch.HomebrewCheck);

	//kill ps1 eboot check
	MAKE_DUMMY_FUNCTION_RETURN_0(text_addr + g_offs->game_plugin_patch.PopsCheck);

	//kill multi-disc ps1 check
	_sw(NOP, text_addr + g_offs->game_plugin_patch.MultiDiscPopsCheck);

	if (conf.hidepic) {
		_sw(0x00601021, text_addr + g_offs->game_plugin_patch.HidePicCheck1);
		_sw(0x00601021, text_addr + g_offs->game_plugin_patch.HidePicCheck2);
	}
	
	if (conf.skipgameboot) {
		_sw(MAKE_CALL(text_addr + g_offs->game_plugin_patch.SkipGameBootSubroute), text_addr + g_offs->game_plugin_patch.SkipGameBoot);
		_sw(0x24040002, text_addr + g_offs->game_plugin_patch.SkipGameBoot + 4);
	}

	// disable check for custom psx eboot restore 
	// rif file check
	_sw(0x00001021, text_addr + g_offs->game_plugin_patch.RifFileCheck);
	// rif content memcmp check
	_sw(NOP, text_addr + g_offs->game_plugin_patch.RifCompareCheck);
	// some type check, branch it
	_sw(0x10000010, text_addr + g_offs->game_plugin_patch.RifTypeCheck);
	// fake npdrm call
	_sw(0x00001021, text_addr + g_offs->game_plugin_patch.RifNpDRMCheck);
}

static void patch_msvideo_main_plugin_module(u32 text_addr)
{
	/* Patch resolution limit to (130560) pixels (480x272) */
	_sh(0xFE00, text_addr + g_offs->msvideo_main_plugin_patch.checks[0]);
	_sh(0xFE00, text_addr + g_offs->msvideo_main_plugin_patch.checks[1]);
	_sh(0xFE00, text_addr + g_offs->msvideo_main_plugin_patch.checks[2]);
	_sh(0xFE00, text_addr + g_offs->msvideo_main_plugin_patch.checks[3]);

	_sh(0xFE00, text_addr + g_offs->msvideo_main_plugin_patch.checks[4]);
	_sh(0xFE00, text_addr + g_offs->msvideo_main_plugin_patch.checks[5]);
	_sh(0xFE00, text_addr + g_offs->msvideo_main_plugin_patch.checks[6]);

	/* Patch bitrate limit (increase to 16384+2) */
	_sh(0x4003, text_addr + g_offs->msvideo_main_plugin_patch.checks[7]);
	_sh(0x4003, text_addr + g_offs->msvideo_main_plugin_patch.checks[8]);
	_sh(0x4003, text_addr + g_offs->msvideo_main_plugin_patch.checks[9]);
}

static void patch_htmlviewer_plugin_module(u32 text_addr)
{
	char *p;

	p = (void*)(text_addr + g_offs->htmlviewer_plugin_patch.htmlviewer_save_location); // "/PSP/COMMON"

	strcpy(p, "/ISO");
	
	p = (void*)(text_addr + g_offs->htmlviewer_plugin_patch.htmlviewer_manual_location); // "http://manuals.playstation.net/document/pspindex.html"
	
	strcpy(p, "http://www.prometheus.uk.to/manual/index.html");
}

static void patch_htmlviewer_utility_module(u32 text_addr)
{
	char *p;

	p = (void*)(text_addr + g_offs->htmlviewer_utility_patch.htmlviewer_manual_location); // "http://manuals.playstation.net/document/pspindex.html"
	
	strcpy(p, "http://www.prometheus.uk.to/manual/index.html");
}

int umdLoadExec(char * file, struct SceKernelLoadExecVSHParam * param)
{
	//result
	int ret = 0;
	SEConfig config;

	printk("%s: %s %s\n", __func__, file, param->key);
	printk("%s: %d %s\n", __func__, (int)sctrlSEGetBootConfFileIndex(), sctrlSEGetUmdFile());

	sctrlSEGetConfig(&config);

	if(sctrlSEGetBootConfFileIndex() == MODE_VSHUMD) {
		sctrlSESetBootConfFileIndex(config.umdmode);
		sctrlSESetDiscType(PSP_UMD_TYPE_GAME);
	}

	//enable high memory on demand
	if(config.retail_high_memory) sctrlHENSetMemory(55, 0);

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
		ret = sctrlKernelLoadExecVSHWithApitype(apitype, file, param);
	} else {
		ret = sctrlKernelLoadExecVSHDisc(file, param);
	}

	return ret;
}

int umdLoadExecUpdater(char * file, struct SceKernelLoadExecVSHParam * param)
{
	//result
	int ret = 0;
	SEConfig config;

	printk("%s: %s %s\n", __func__, file, param->key);
	printk("%s: %d %s\n", __func__, (int)sctrlSEGetBootConfFileIndex(), sctrlSEGetUmdFile());

	sctrlSEGetConfig(&config);

	if(sctrlSEGetBootConfFileIndex() == MODE_VSHUMD) {
		sctrlSESetBootConfFileIndex(MODE_UPDATERUMD);
		sctrlSESetDiscType(PSP_UMD_TYPE_GAME);
	}

	ret = sctrlKernelLoadExecVSHDiscUpdater(file, param);

	return ret;
}

static void patch_vsh_module_for_pspgo_umdvideo(SceModule2 *mod)
{
	u32 text_addr = mod->text_addr, prev, i;

	/*
	 * vshbridge_get_model_call:
	 *  6.35: sceVshBridge_AD90BEE5
	 *  6.20: sceVshBridge_63E40313
	 */

	for(i=0; i<NELEMS(g_offs->vsh_module_patch.vshbridge_get_model_call); ++i) {
		u32 offset;

		offset = g_offs->vsh_module_patch.vshbridge_get_model_call[i];
		prev = _lw(text_addr + offset + 4);
		_sw(prev, text_addr + offset);
		_sw(0x24020000 | PSP_4000, text_addr + offset + 4);
	}
}

static void patch_vsh_module(SceModule2 * mod)
{
	//enable homebrew boot
	_sw(NOP, mod->text_addr + g_offs->vsh_module_patch.checks[0]);
	_sw(NOP, mod->text_addr + g_offs->vsh_module_patch.checks[1]);
	_sw(NOP, mod->text_addr + g_offs->vsh_module_patch.checks[2]);

	hook_import_bynid((SceModule *)mod, "sceVshBridge", g_offs->vsh_module_patch.loadexecNID1, gameloadexec, 1);
	hook_import_bynid((SceModule *)mod, "sceVshBridge", g_offs->vsh_module_patch.loadexecNID2, gameloadexec, 1);
	hook_import_bynid((SceModule *)mod, "sceVshBridge", g_offs->vsh_module_patch.loadexecDisc, umdLoadExec, 1);
	hook_import_bynid((SceModule *)mod, "sceVshBridge", g_offs->vsh_module_patch.loadexecDiscUpdater, umdLoadExecUpdater, 1);

	int i = 0; for(; i < NELEMS(g_offs->vsh_module_patch.PBPFWCheck); i++) {
		_sw(MAKE_SYSCALL(sctrlKernelQuerySystemCall(fakeParamInexistance)), mod->text_addr + g_offs->vsh_module_patch.PBPFWCheck[i]);
	}

	if(psp_model == PSP_GO && sctrlSEGetBootConfFileIndex() == MODE_VSHUMD) {
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
	sctrlSEGetConfig(&conf);
	previous = sctrlHENSetStartModuleHandler(&vshpatch_module_chain);
	patch_sceUSB_Driver();
	vpbp_init();
	hook_iso_file_io();
	hook_iso_directory_io();

	return 0;
}
