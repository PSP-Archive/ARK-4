#include "ui.h"

#include <pspctrl.h>
#include <psptypes.h>

#include "scepaf.h"
#include "vsh.h"


int ui_eat_key(SceCtrlData *pad_data, int count) {
	int i;
	u32 old_buttons;
	
	vsh_Menu *vsh = vsh_menu_pointer();

	// copy old value of buttons
	old_buttons = vsh->buttons.pad.Buttons;
	// copy new value
	scePaf_memcpy(&vsh->buttons.pad, pad_data, sizeof(SceCtrlData));
	// get only the new buttons pressed (compared to old value)
	vsh->buttons.new_buttons_on = ~old_buttons & vsh->buttons.pad.Buttons;

	// mask buttons for LOCK VSH controll
	for(i = 0; i < count; i++) {
		pad_data[i].Buttons &= ~(
			PSP_CTRL_SELECT		|	PSP_CTRL_START		|
			PSP_CTRL_UP			|	PSP_CTRL_RIGHT		|	PSP_CTRL_DOWN	|	PSP_CTRL_LEFT	|
			PSP_CTRL_LTRIGGER	|	PSP_CTRL_RTRIGGER	|
			PSP_CTRL_TRIANGLE	|	PSP_CTRL_CIRCLE		|	PSP_CTRL_CROSS	|	PSP_CTRL_SQUARE	|
			PSP_CTRL_HOME		|	PSP_CTRL_NOTE
		);
	}

	return 0;
}