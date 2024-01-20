#ifndef _VSH_H
#define _VSH_H


#include <psptypes.h>
#include <pspctrl.h>
#include <pspkernel.h>

#include "globals.h"
#include <systemctrl_se.h>
#include "umdvideo_list.h"

#include "../arkMenu/include/conf.h"

// Config stuff go here
typedef struct _vsh_Config{
	// SE config
	SEConfig se;
	SEConfig old_se;
	
	// ARK menu config
	t_conf ark_menu;
	t_conf old_ark_menu;
	
	// ARK config
	ARKConfig ark;
	ARKConfig *p_ark;
}vsh_Config;

// Button stuff go here
typedef struct{
	SceCtrlData pad;
	SceCtrlData old_pad;
	u32 new_buttons_on;
}vsh_Buttons;

// Status stuff go here
typedef struct _vsh_Status{
	u32 swap_xo;
	
	int menu_mode;
	int submenu_mode;
	int stop_flag;
	int sub_stop_flag;
	
	int reset_vsh;

	u8 bc_alpha;
	u8 bc_delta;

	int umdvideo_idx;
}vsh_Status;

// VSH Menu struct
typedef struct _vsh_Menu{
	char ark_version[24];
	SceUID thread_id;
	
	vsh_Config config;
	vsh_Buttons buttons;
	vsh_Status status;
	
	UmdVideoList umdlist;
	
	u32 psp_model;
	
	int battery;
	int codecs;
}vsh_Menu;


vsh_Menu* vsh_menu_pointer(void);


#endif
