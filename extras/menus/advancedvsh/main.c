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
#include <pspdisplay.h>
#include <pspctrl.h>
#include <pspumd.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <kubridge.h>

#include <stdio.h>
#include <time.h>
#include <stdbool.h>


#include "common.h"
#include "kubridge.h"
#include "vpl.h"
#include "vsh.h"
#include "scepaf.h"
#include "blit.h"
#include "trans.h"

#include "../arkMenu/include/conf.h"

#include "ui.h"
#include "battery.h"
#include "config.h"
#include "fonts.h"
#include "menu.h"
#include "advanced.h"
#include "registry.h"
#include "launcher.h"
#include "umdvideo_list.h"


/* Define the module info section */
PSP_MODULE_INFO("VshCtrlSatelite", 0, 2, 2);
/* Define the main thread's attribute value (optional) */
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);


extern char umdvideo_path[256];


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
	
	vsh_Menu *vsh = vsh_menu_pointer();
	
	// get psp model
	vsh->psp_model = kuKernelGetModel();

	// ARK Version
	u32 ver = sctrlHENGetVersion(); // ARK's full version number
    u32 major = (ver&0xFF000000)>>24;
    u32 minor = (ver&0xFF0000)>>16;
    u32 micro = (ver&0xFF00)>>8;
	u32 rev   = sctrlHENGetMinorVersion();

	#ifdef DEBUG
	if (micro > 0 && !rev) 
		scePaf_snprintf(vsh->ark_version, sizeof(vsh->ark_version), "    ARK %d.%d.%.2i DEBUG    ", major, minor, micro);
	else if (rev)
		scePaf_snprintf(vsh->ark_version, sizeof(vsh->ark_version), " ARK %d.%d.%.2i r%i DEBUG ", major, minor, micro, rev);
	else 
		scePaf_snprintf(vsh->ark_version, sizeof(vsh->ark_version), "    ARK %d.%d DEBUG    ", major, minor);
	#else
	if (micro > 0 && !rev) 
		scePaf_snprintf(vsh->ark_version, sizeof(vsh->ark_version), "    ARK %d.%d.%.2i    ", major, minor, micro);
	else if (rev)
		scePaf_snprintf(vsh->ark_version, sizeof(vsh->ark_version), " ARK %d.%d.%.2i r%i ", major, minor, micro, rev);
	else 
		scePaf_snprintf(vsh->ark_version, sizeof(vsh->ark_version), "    ARK %d.%d    ", major, minor); 
	#endif
	
	// load config stuff
	sctrlSEGetConfig(&vsh->config.se);
	sctrlHENGetArkConfig(&vsh->config.ark);
	config_load(vsh);
	if(vsh->config.ark_menu.advanced_vsh)
		vsh->status.stop_flag = 15;

	// load font
	font_load(vsh);
	// select menu language
	select_language();
	
	if (!IS_VITA_ADR(vsh->config.p_ark)) {
		umdvideolist_init(&vsh->umdlist);
		umdvideolist_clear(&vsh->umdlist);
		get_umdvideo(&vsh->umdlist, "ms0:/ISO/VIDEO");
		get_umdvideo(&vsh->umdlist, "ef0:/ISO/VIDEO");
		kuKernelGetUmdFile(umdvideo_path, sizeof(umdvideo_path));

		if (umdvideo_path[0] == '\0') {
			vsh->status.umdvideo_idx = 0;
			scePaf_strcpy(umdvideo_path, g_messages[MSG_NONE]);
		} else {
			vsh->status.umdvideo_idx = umdvideolist_find(&vsh->umdlist, umdvideo_path);

			if (vsh->status.umdvideo_idx >= 0) {
				vsh->status.umdvideo_idx++;
			} else {
				vsh->status.umdvideo_idx = 0;
				scePaf_strcpy(umdvideo_path, g_messages[MSG_NONE]);
			}
		}
	}

	scePaf_memcpy(&vsh->config.old_se, &vsh->config.se, sizeof(vsh->config.se));
	scePaf_memcpy(&vsh->config.old_ark_menu, &vsh->config.ark_menu, sizeof(vsh->config.ark_menu));

	
resume:
	while (vsh->status.stop_flag == 0) {
		if (sceDisplayWaitVblankStart() < 0)
			break; // end of VSH ?

		if (vsh->status.menu_mode > 0) {
			menu_setup();
			menu_draw();
		}

		button_func(vsh);
	}

	config_check(vsh);

	switch (vsh->status.stop_flag) {
		case 2:
			scePowerRequestColdReset(0);
			break;
		case 3:
			scePowerRequestStandby();
			break;
		case 4:
			vsh->status.reset_vsh = 1;
			break;
		case 5:
			scePowerRequestSuspend();
			break;
		case 8:
			exec_recovery_menu(vsh);
			break;
		case 15:
			// AVSHMENU START
			while(vsh->status.sub_stop_flag == 0) {
				if( sceDisplayWaitVblankStart() < 0)
					break; // end of VSH ?
				if(vsh->status.submenu_mode > 0) {
					submenu_setup();
					submenu_draw();
				}
				subbutton_func(vsh);
			}
			config_check(vsh);
			break;
	}

	switch (vsh->status.sub_stop_flag) {
		case 1:
			vsh->status.stop_flag = 0;
			vsh->status.menu_mode = 0;
			vsh->status.sub_stop_flag = 0;
			vsh->status.submenu_mode = 0;
			goto resume;
		case 6:
			if (IS_VITA_ADR(vsh->config.p_ark)) 
				return -1;
			launch_umdvideo_mount(vsh);
			break;
		case 9:
			battery_convert(vsh->battery);
			break;
		case 10:
			delete_hibernation(vsh);
			break;
		case 11:
			activate_codecs(vsh);
			break;
		case 12:
			swap_buttons(vsh);
			break;
		case 13:
			import_classic_plugins(vsh, DEVPATH_MS0);
			if (vsh->psp_model == PSP_GO)
				import_classic_plugins(vsh, DEVPATH_EF0);
			break;
		case 14:			
			config_check(vsh);
			exec_random_game(vsh);
			break;
	}

	config_check(vsh);

	if(!IS_VITA_ADR(vsh->config.p_ark))
		umdvideolist_clear(&vsh->umdlist);
	clear_language();
	vpl_finish();

	vctrlVSHExitVSHMenu(&vsh->config.se, NULL, 0);
	release_font();

	if (vsh->status.reset_vsh) {
		sctrlKernelExitVSH(NULL);
	}

	return sceKernelExitDeleteThread(0);
}

int module_start(int argc, char *argv[]) {
	SceUID thid;
	vsh_Menu *vsh = vsh_menu_pointer();
	thid = sceKernelCreateThread("AVshMenu_Thread", TSRThread, 16, 0x1000, 0, NULL);

	vsh->thread_id = thid;

	if (thid >= 0)
		sceKernelStartThread(thid, 0, 0);
	
	return 0;
}

int module_stop(int argc, char *argv[]) {
	int ret;
	vsh_Menu *vsh = vsh_menu_pointer();
	SceUInt time = 100*1000;

	vsh->status.stop_flag = 1;
	ret = sceKernelWaitThreadEnd(vsh->thread_id, &time);

	if (ret < 0)
		sceKernelTerminateDeleteThread(vsh->thread_id);
	
	return 0;
}
