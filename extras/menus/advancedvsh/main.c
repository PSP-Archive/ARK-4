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

/*
 * vshMenu by neur0n
 * based booster's vshex
 */

/* 
 * avshMenu by krazynez
 * based on PRO vsh, ME vsh, and ultimate vsh, and the Original ARK-4 vshmenu.
 * Plus myself and acid_snake's mentally insane thoughts and awesomeness ;-)
 */

#include <pspkernel.h>
#include <psputility.h>
#include <pspiofilemgr.h>
#include <pspthreadman.h>
#include <pspctrl.h>
#include <pspumd.h>

#include <stdio.h>
#include <time.h>
#include <stdbool.h>


#include "common.h"
#include "systemctrl.h"
#include "systemctrl_se.h"
#include "kubridge.h"
#include "vpl.h"
#include "blit.h"
#include "trans.h"


#include "../arkMenu/include/conf.h"


#include "battery.h"
#include "vsh.h"
#include "config.h"
#include "fonts.h"
#include "menu.h"
#include "advanced.h"
#include "registry.h"


/* Define the module info section */
PSP_MODULE_INFO("VshCtrlSatelite", 0, 2, 2);
/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);



/* Global Variables */
vsh_Menu vsh_menu = {
	.config.p_ark = &vsh_menu.config.ark,
	.status.menu_mode = 0,
	.status.submenu_mode = 0,
	.status.stop_flag = 0,
	.status.sub_stop_flag = 0,
	.status.reset_vsh = 0,
	.buttons.pad.Buttons = 0xFFFFFFFF,
	.buttons.new_buttons_on = 0,
	.thread_id = -1,
};
vsh_Menu *g_vsh_menu = &vsh_menu;
extern char umdvideo_path[256];
UmdVideoList g_umdlist;


/* Extern functions */
extern int scePowerRequestColdReset(int unk);
extern int scePowerRequestStandby(void);
extern int scePowerRequestSuspend(void);


/* Function prototypes */
int module_start(int argc, char *argv[]);
int module_stop(int argc, char *argv[]);



void exec_recovery_menu(vsh_Menu *vsh) {
	char menupath[ARK_PATH_SIZE];
	scePaf_strcpy(menupath, vsh->config.ark.arkpath);
	strcat(menupath, ARK_RECOVERY);
	
	struct SceKernelLoadExecVSHParam param;
	scePaf_memset(&param, 0, sizeof(param));
	param.size = sizeof(param);
	param.args = scePaf_strlen(menupath) + 1;
	param.argp = menupath;
	param.key = "game";
	sctrlKernelLoadExecVSHWithApitype(0x141, menupath, &param);
}

void import_classic_plugins(vsh_Menu *vsh, char* devpath) {
	SceUID game, vsh_id, pops, plugins;
	int i = 0;
	int chunksize = 512;
	int bytesRead;
	char *buf = vpl_alloc(chunksize);
	char *gameChar = "game, ";
	int gameCharLength = scePaf_strlen(gameChar);
	char *vshChar = "vsh, ";
	int vshCharLength = scePaf_strlen(vshChar);
	char *popsChar = "pops, ";
	int popsCharLength = scePaf_strlen(popsChar);
	
	char filename[] = "??0:/seplugins/plugins.txt";
	char gamepath[] = "??0:/seplugins/game.txt";
	char vshpath[] = "??0:/seplugins/vsh.txt";
	char popspath[] = "??0:/seplugins/pops.txt";


	filename[0] = devpath[0];
	gamepath[0] = devpath[0];
	vshpath[0] = devpath[0];
	popspath[0] = devpath[0];
	filename[1] = devpath[1];
	gamepath[1] = devpath[1];
	vshpath[1] = devpath[1];
	popspath[1] = devpath[1];

	game = sceIoOpen(gamepath, PSP_O_RDONLY, 0777);
	vsh_id = sceIoOpen(vshpath, PSP_O_RDONLY, 0777);
	pops = sceIoOpen(popspath, PSP_O_RDONLY, 0777);
	plugins = sceIoOpen(filename, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

	// GAME.txt
	scePaf_memset(buf, 0, chunksize);
	while ((bytesRead = sceIoRead(game, buf, chunksize)) > 0) {
		for(i = 0; i < bytesRead; i++) {
			if (i == 0 || buf[i-1] == '\n' || buf[i-1] == '\0'){
				sceIoWrite(plugins, gameChar, gameCharLength);
			}
			if (buf[i] == ' ' && i != 0)
				sceIoWrite(plugins, ",", 1);
			sceIoWrite(plugins, &buf[i], 1);
		}
	}
	
	sceIoClose(game);


	scePaf_memset(buf, 0, chunksize);

	// VSH.txt
	while ((bytesRead = sceIoRead(vsh_id, buf, chunksize)) > 0) {
		for(i = 0; i < bytesRead; i++) {
			if (i == 0 || buf[i-1] == '\n' || buf[i-1] == '\0'){
				sceIoWrite(plugins, vshChar, vshCharLength);
			}
			if (buf[i] == ' ' && i != 0)
				sceIoWrite(plugins, ",", 1);
			sceIoWrite(plugins, &buf[i], 1);
		}
	}

	sceIoClose(vsh_id);

	scePaf_memset(buf, 0, chunksize);


	// POP.txt
	while ((bytesRead = sceIoRead(pops, buf, chunksize)) > 0) {
		for(i = 0; i < bytesRead; i++) {
			if (i == 0 || buf[i-1] == '\n' || buf[i-1] == '\0'){
				sceIoWrite(plugins, popsChar, popsCharLength);
			}
			if (buf[i] == ' ' && i != 0)
				sceIoWrite(plugins, ",", 1);
			sceIoWrite(plugins, &buf[i], 1);
		}
	}

	sceIoClose(pops);

	sceIoClose(plugins);
	vpl_free(buf);

	vsh->status.reset_vsh = 1;
}

static int get_umdvideo(UmdVideoList *list, char *path) {
	SceIoDirent dir;
	int result = 0, dfd;
	char fullpath[256];

	scePaf_memset(&dir, 0, sizeof(dir));
	dfd = sceIoDopen(path);

	if(dfd < 0) 
		return dfd;

	const char *p;
	while (sceIoDread(dfd, &dir) > 0) {
		p = (const char *)scePaf_strrchr(dir.d_name, '.');

		if (p == NULL)
			p = dir.d_name;

		if (0 == stricmp(p, ".iso") || 0 == stricmp(p, ".cso") || 0 == stricmp(p, ".zso") || 0 == stricmp(p, ".dax") || 0 == stricmp(p, ".jso")) {
			scePaf_sprintf(fullpath, "%s/%s", path, dir.d_name);
			umdvideolist_add(list, fullpath);
		}
	}

	sceIoDclose(dfd);
	return result;
}

typedef struct _pspMsPrivateDirent {
	SceSize size;
	char s_name[16];
	char l_name[1024];
} pspMsPrivateDirent;

void exec_random_game(vsh_Menu *vsh) {
	char iso_dir[128];
	char game[256];
	int num_games = 0;
	scePaf_strcpy(iso_dir, "ms0:/ISO/");

	if(vsh->psp_model == PSP_GO) {
		iso_dir[0] = 'e';
		iso_dir[1] = 'f';
	}

	SceIoDirent isos;
	SceUID iso_path;
	pspMsPrivateDirent* pri_dirent = vpl_alloc(sizeof(pspMsPrivateDirent));

find_random_game:

	iso_path = sceIoDopen(iso_dir);

	scePaf_memset(&isos, 0, sizeof(isos));
	scePaf_memset(pri_dirent, 0, sizeof(*pri_dirent));
	pri_dirent->size = sizeof(*pri_dirent);
	isos.d_private = (void*)pri_dirent;
	while(sceIoDread(iso_path, &isos) > 0) {
		if(isos.d_name[0] != '.' && scePaf_strcmp(isos.d_name, "VIDEO") != 0) {
			num_games++;
		}
	}

	sceIoDclose(iso_path);

	if (num_games == 0){
		vpl_free(pri_dirent);
		return;
	};

	srand(time(NULL));
	int rand_idx = rand() % num_games;
	num_games = 0;

	iso_path = sceIoDopen(iso_dir);

	scePaf_memset(&isos, 0, sizeof(isos));
	scePaf_memset(pri_dirent, 0, sizeof(*pri_dirent));
	pri_dirent->size = sizeof(*pri_dirent);
	isos.d_private = (void*)pri_dirent;
	while(sceIoDread(iso_path, &isos) > 0) {
		if(isos.d_name[0] != '.' && scePaf_strcmp(isos.d_name, "VIDEO") != 0) {
			if (num_games == rand_idx) break;
			else num_games++;
		}
	}

	sceIoDclose(iso_path);

	if (FIO_SO_ISDIR(isos.d_stat.st_attr)){
		strcat(iso_dir, isos.d_name);
		strcat(iso_dir, "/");
		goto find_random_game;
	}

	scePaf_strcpy(game, iso_dir);
	if (pri_dirent->s_name[0])
		strcat(game, pri_dirent->s_name);
	else
		strcat(game, isos.d_name);

	vpl_free(pri_dirent);

	struct SceKernelLoadExecVSHParam param;
	int apitype;
	char* loadexec_file;
	scePaf_memset(&param, 0, sizeof(param));
	param.size = sizeof(param);
	
	//set iso file for reboot
	sctrlSESetUmdFile(game);

	//set iso mode for reboot
	sctrlSESetDiscType(PSP_UMD_TYPE_GAME);
	sctrlSESetBootConfFileIndex(MODE_INFERNO);

	param.key = "umdemu";

	static char pboot_path[256];
	int has_pboot = has_update_file(game, pboot_path);

	if (has_pboot){
		// configure to use dlc/update
		loadexec_file = param.argp = pboot_path;
		param.args = scePaf_strlen(pboot_path) + 1;

		if (vsh->psp_model == PSP_GO && game[0] == 'e' && game[1] == 'f') {
			apitype = 0x126;
		}
		else {
			apitype = 0x124;
		}
	}
	else{
		//reset and configure reboot parameter
		loadexec_file = game;

		if (vsh->psp_model == PSP_GO && game[0] == 'e' && game[1] == 'f') {
			apitype = 0x125;
		}
		else {
			apitype = 0x123;
		}

		if (has_prometheus_module(game)) {
			param.argp = "disc0:/PSP_GAME/SYSDIR/EBOOT.OLD";
		} else {
			param.argp = "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN";
		}
		param.args = 33;
	}

	sctrlKernelLoadExecVSHWithApitype(apitype, game, &param);

}

static void exec_custom_launcher(vsh_Menu *vsh) {
	char menupath[ARK_PATH_SIZE];
	scePaf_strcpy(menupath, vsh->config.ark.arkpath);
	strcat(menupath, ARK_MENU);
	
	struct SceKernelLoadExecVSHParam param;
	scePaf_memset(&param, 0, sizeof(param));
	param.size = sizeof(param);
	param.args = scePaf_strlen(menupath) + 1;
	param.argp = menupath;
	param.key = "game";
	sctrlKernelLoadExecVSHWithApitype(0x141, menupath, &param);
}

static void launch_umdvideo_mount(vsh_Menu *vsh) {
	SceIoStat stat;
	char *path;
	int type;

	if(0 == umdvideo_idx) {
		if(sctrlSEGetBootConfFileIndex() == MODE_VSHUMD) {
			// cancel mount
			sctrlSESetUmdFile("");
			sctrlSESetBootConfFileIndex(MODE_UMD);
			vsh->status.reset_vsh = 1;
		}
		return;
	}

	path = umdvideolist_get(&g_umdlist, (size_t)(umdvideo_idx-1));

	if (path == NULL)
		return;

	if (sceIoGetstat(path, &stat) < 0)
		return;

	type = vshDetectDiscType(path);
	#ifdef DEBUG
	printk("%s: detected disc type 0x%02X for %s\n", __func__, type, path);
	#endif

	if (type < 0)
		return;

	sctrlSESetUmdFile(path);
	sctrlSESetBootConfFileIndex(MODE_VSHUMD);
	sctrlSESetDiscType(type);
	vsh->status.reset_vsh = 1;
}


void clear_language(void) {
	if (g_messages != g_messages_en)
		free_translate_table((char**)g_messages, MSG_END);

	g_messages = g_messages_en;
}

static char ** apply_language(char *translate_file) {
	char **message = NULL;
	int ret;

	ret = load_translate_table(&message, translate_file, MSG_END);

	if(ret >= 0) {
		return message;
	}

	return (char**) g_messages_en;
}

int cur_language = 0;

static void select_language(void) {

	static char *languages[] = { "jp", "en", "fr", "es", "de", "it", "nl", "pt", "ru", "kr", "cht", "chs" };

	int ret, value;
	ret = sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &value);

	if(ret < 0)
		value = PSP_SYSTEMPARAM_LANGUAGE_ENGLISH;

	cur_language = value;
	clear_language();

	char file[64];
	scePaf_sprintf(file, "satelite_%s.txt", languages[value]);
	g_messages = (const char**)apply_language(file);

	if(g_messages == g_messages_en) {
		cur_language = PSP_SYSTEMPARAM_LANGUAGE_ENGLISH;
	}
}



void delete_hibernation(vsh_Menu *vsh) {
	if (vsh->psp_model == PSP_GO) {
		vshCtrlDeleteHibernation();
		vsh->status.reset_vsh = 1;
	}
}

static int activate_codecs(vsh_Menu *vsh) {
	int flash_activated = 0;
	int flash_play = 0;
	int wma_play = 0;

	get_registry_value("/CONFIG/BROWSER", "flash_activated", &flash_activated);
	get_registry_value("/CONFIG/BROWSER", "flash_activated", &flash_play);
	get_registry_value("/CONFIG/MUSIC", "wma_play", &wma_play);

	if (!flash_activated || !flash_play || !wma_play){
		set_registry_value("/CONFIG/BROWSER", "flash_activated", 1);
		set_registry_value("/CONFIG/BROWSER", "flash_play", 1);
		set_registry_value("/CONFIG/MUSIC", "wma_play", 1);
		vsh->status.reset_vsh = 1;
	}
	
	return 0;
}

static int swap_buttons(vsh_Menu *vsh) {
	u32 value;
	get_registry_value("/CONFIG/SYSTEM/XMB", "button_assign", &value);
	value = !value;
	set_registry_value("/CONFIG/SYSTEM/XMB", "button_assign", value);
	
	vsh->status.reset_vsh = 1;
	return 0;
}


int TSRThread(SceSize args, void *argp) {
	// change priority - needs to be the first thing executed when main thread started
	sceKernelChangeThreadPriority(0, 8);
	// register eat key function
	vctrlVSHRegisterVshMenu(ui_eat_key);
	
	// init VPL
	vpl_init();
	
	// get psp model
	vsh_menu.psp_model = kuKernelGetModel();
	
	// load config stuff
	sctrlSEGetConfig(&vsh_menu.config.se);
	sctrlHENGetArkConfig(&vsh_menu.config.ark);
	config_load(&vsh_menu);

	// load font
	font_load(&vsh_menu);
	// select menu language
	select_language();
	
	if (!IS_VITA_ADR(vsh_menu.config.p_ark)) {
		umdvideolist_init(&g_umdlist);
		umdvideolist_clear(&g_umdlist);
		get_umdvideo(&g_umdlist, "ms0:/ISO/VIDEO");
		get_umdvideo(&g_umdlist, "ef0:/ISO/VIDEO");
		kuKernelGetUmdFile(umdvideo_path, sizeof(umdvideo_path));

		if (umdvideo_path[0] == '\0') {
			umdvideo_idx = 0;
			scePaf_strcpy(umdvideo_path, g_messages[MSG_NONE]);
		} else {
			umdvideo_idx = umdvideolist_find(&g_umdlist, umdvideo_path);

			if (umdvideo_idx >= 0) {
				umdvideo_idx++;
			} else {
				umdvideo_idx = 0;
				scePaf_strcpy(umdvideo_path, g_messages[MSG_NONE]);
			}
		}
	}

	scePaf_memcpy(&vsh_menu.config.old_se, &vsh_menu.config.se, sizeof(vsh_menu.config.se));
	scePaf_memcpy(&vsh_menu.config.old_ark_menu, &vsh_menu.config.ark_menu, sizeof(vsh_menu.config.ark_menu));

	
resume:
	while (vsh_menu.status.stop_flag == 0) {
		if (sceDisplayWaitVblankStart() < 0)
			break; // end of VSH ?

		if (vsh_menu.status.menu_mode > 0) {
			menu_setup();
			menu_draw();
		}

		button_func(&vsh_menu);
	}

	config_check(&vsh_menu);

	switch (vsh_menu.status.stop_flag) {
		case 2:
			scePowerRequestColdReset(0);
			break;
		case 3:
			scePowerRequestStandby();
			break;
		case 4:
			vsh_menu.status.reset_vsh = 1;
			break;
		case 5:
			scePowerRequestSuspend();
			break;
		case 7:
			exec_custom_launcher(&vsh_menu);
			break;
		case 8:
			exec_recovery_menu(&vsh_menu);
			break;
		case 15:
			// AVSHMENU START
			while(vsh_menu.status.sub_stop_flag == 0) {
				if( sceDisplayWaitVblankStart() < 0)
					break; // end of VSH ?
				if(vsh_menu.status.submenu_mode > 0) {
					submenu_setup();
					submenu_draw();
				}
				subbutton_func(&vsh_menu);
			}
			config_check(&vsh_menu);
			break;
	}

	switch (vsh_menu.status.sub_stop_flag) {
		case 1:
			vsh_menu.status.stop_flag = 0;
			vsh_menu.status.menu_mode = 0;
			vsh_menu.status.sub_stop_flag = 0;
			vsh_menu.status.submenu_mode = 0;
			goto resume;
		case 6:
			if (IS_VITA_ADR(vsh_menu.config.p_ark)) 
				return -1;
			launch_umdvideo_mount(&vsh_menu);
			break;
		case 9:
			battery_convert(vsh_menu.battery);
			break;
		case 10:
			delete_hibernation(&vsh_menu);
			break;
		case 11:
			activate_codecs(&vsh_menu);
			break;
		case 12:
			swap_buttons(&vsh_menu);
			break;
		case 13:
			import_classic_plugins(&vsh_menu, "ms");
			if (vsh_menu.psp_model == PSP_GO)
				import_classic_plugins(&vsh_menu, "ef");
			break;
		case 14:			
			config_check(&vsh_menu);
			exec_random_game(&vsh_menu);
			break;
	}

	config_check(&vsh_menu);

	if(!IS_VITA_ADR(vsh_menu.config.p_ark))
		umdvideolist_clear(&g_umdlist);
	clear_language();
	vpl_finish();

	vctrlVSHExitVSHMenu(&vsh_menu.config.se, NULL, 0);
	release_font();

	if (vsh_menu.status.reset_vsh) {
		sctrlKernelExitVSH(NULL);
	}

	return sceKernelExitDeleteThread(0);
}

int module_start(int argc, char *argv[]) {
	SceUID thid;
	thid = sceKernelCreateThread("AVshMenu_Thread", TSRThread, 16, 0x1000, 0, NULL);

	vsh_menu.thread_id = thid;

	if (thid >= 0)
		sceKernelStartThread(thid, 0, 0);
	
	return 0;
}

int module_stop(int argc, char *argv[]) {
	int ret;
	SceUInt time = 100*1000;

	g_vsh_menu->status.stop_flag = 1;
	ret = sceKernelWaitThreadEnd(vsh_menu.thread_id, &time);

	if (ret < 0)
		sceKernelTerminateDeleteThread(vsh_menu.thread_id);
	
	return 0;
}



/* not used
int load_start_module(char *path) {
	int ret;
	SceUID modid;

	modid = sceKernelLoadModule(path, 0, NULL);

	if(modid < 0) {
		return modid;
	}

	ret = sceKernelStartModule(modid, scePaf_strlen(path) + 1, path, NULL, NULL);

	return ret;
}
*/