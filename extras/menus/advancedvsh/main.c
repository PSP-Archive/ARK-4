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
#include "launcher.h"


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
	.status.bc_alpha = 0,
	.status.bc_delta = 5,
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


int TSRThread(SceSize args, void *argp) {
	// change priority - needs to be the first thing executed when main thread started
	sceKernelChangeThreadPriority(0, 8);
	// register eat key function
	vctrlVSHRegisterVshMenu(ui_eat_key);
	
	// init VPL
	vpl_init();
	
	// get psp model
	vsh_menu.psp_model = kuKernelGetModel();

	// ARK Version
	int ver = sctrlHENGetMinorVersion();
 	int major = (ver & 0xFF0000) >> 16;
	int minor = (ver & 0xFF00) >> 8;
	int micro = (ver & 0xFF);

	#ifdef DEBUG
	if (micro > 0) 
		scePaf_snprintf(ark_version, sizeof(ark_version), "    ARK %d.%d.%.2i DEBUG    ", major, minor, micro);
	else 
		scePaf_snprintf(ark_version, sizeof(ark_version), "    ARK %d.%d DEBUG    ", major, minor);
	#else
	if (micro > 0) 
		scePaf_snprintf(vsh_menu.ark_version, sizeof(vsh_menu.ark_version), "    ARK %d.%d.%.2i    ", major, minor, micro);
	else 
		scePaf_snprintf(vsh_menu.ark_version, sizeof(vsh_menu.ark_version), "    ARK %d.%d    ", major, minor); 
	#endif
	
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