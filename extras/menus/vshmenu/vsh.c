#include "vsh.h"

/* Global var */
vsh_Menu vsh_menu = {
	.config.p_ark = &vsh_menu.config.ark,
	.status.menu_mode = 0,
	.status.submenu_mode = 0,
	.status.stop_flag = 0,
	.status.sub_stop_flag = 0,
	.status.reset_vsh = 0,
	.status.umdvideo_idx = 0,
	.buttons.pad.Buttons = 0xFFFFFFFF,
	.buttons.old_pad.Buttons = 0xFFFFFFFF,
	.buttons.new_buttons_on = 0,
	.thread_id = -1,
};


vsh_Menu* vsh_menu_pointer(void) {
	return (vsh_Menu*)&vsh_menu;
}
