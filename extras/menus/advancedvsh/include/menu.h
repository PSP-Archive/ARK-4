#ifndef _MENU_H
#define _MENU_H


#include "vsh.h"


enum {
	TMENU_ADVANCED_VSH,
	TMENU_RECOVERY_MENU,
	TMENU_SHUTDOWN_DEVICE,
	TMENU_SUSPEND_DEVICE,
	TMENU_RESET_DEVICE,
	TMENU_RESET_VSH,
	TMENU_EXIT,
	TMENU_MAX
};


int menu_draw(void);
const char *get_enable_disable(int opt);
int menu_setup(void);
int menu_ctrl(u32 button_on);
void button_func(vsh_Menu *vsh);


#endif
