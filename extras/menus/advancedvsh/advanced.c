#include "common.h"
#include <psputility.h>
#include <pspctrl.h>
#include <time.h>

#include <systemctrl.h>
#include "clock.h"

#include "vsh.h"
#include "ui.h"
#include "scepaf.h"
#include "fonts.h"
#include "advanced.h"
#include "blit.h"
#include "color.h"
#include "config.h"


extern char umd_path[72];

extern char device_buf[13];
extern char umdvideo_path[256];

extern int xyPoint[];
extern int xyPoint2[];


int sub_stop_stock = 0;


int item_fcolor[SUBMENU_MAX];
const char *subitem_str[SUBMENU_MAX];
static int submenu_sel = SUBMENU_USB_DEVICE;


int submenu_draw(void) {
	char msg[128] = {0};
	int submax_menu, subcur_menu;
	const int *pointer;
	u32 fc, bc;
	
	vsh_Menu *vsh = vsh_menu_pointer();
	blit_Gfx *gfx = blit_gfx_pointer();
	font_Data *font = font_data_pointer();
	u32 *colors = (u32*)color_data_pointer();

	// check & setup video mode
	if (blit_setup() < 0) 
		return -1;
	
	if (gfx->width == 720)
		pointer = xyPoint;
	else
		pointer = xyPoint2;

	// show menu title & ARK version
	blit_set_color(0xFFFFFF,0x8000FF00);
	scePaf_snprintf(msg, 128, " %s ", g_messages[MSG_ADVANCED_VSH]);
	blit_string_ctr(pointer[1], msg);
	blit_string_ctr(55, vsh->ark_version);
	fc = 0xFFFFFF;
	
	int submenu_start_x, submenu_start_y;
	int window_char, window_pixel;
	int width = 0, temp = 0, i;
	// find widest submenu up until the UMD region option
	for (i = SUBMENU_USB_DEVICE; i <= SUBMENU_UMD_REGION_MODE; i++){
		temp = scePaf_strlen(g_messages[MSG_USB_DEVICE + i]);
		if (temp > width)
			width = temp;
	}
	
	window_char = width + submenu_find_longest_string() + 3;
	if (window_char & 0x1)
		window_char++;
	window_pixel = window_char * font->width;
	// submenu width + leading & trailing space + subitem space + subitem width
	submenu_start_x = (gfx->width - window_pixel) / 2;
	submenu_start_y = pointer[5] * font->height;

	int drawn = 0;
	for (submax_menu = 0; submax_menu < SUBMENU_MAX; submax_menu++){
		if (g_messages[MSG_USB_DEVICE + submax_menu] && !vsh->config.ark_menu.avm_hidden[submax_menu]) {
			drawn++;
		}
	}

	for (submax_menu = 0; submax_menu < SUBMENU_MAX; submax_menu++) {
		// set default colors
		bc = colors[vsh->config.ark_menu.vsh_bg_color];
		switch(vsh->config.ark_menu.vsh_fg_color){
			case 0: break;
			case 1: fc = colors[27]; break;
			case 27: fc = colors[1]; break;
			default: fc = colors[vsh->config.ark_menu.vsh_fg_color]; break;
		}
		
		if (!vsh->config.ark_menu.window_mode) {
			// add line at the top
			if (submax_menu == 0){
				blit_set_color(fc, bc);
				blit_rect_fill(submenu_start_x, submenu_start_y, window_pixel, font->height);
				blit_set_color(0xaf000000, 0xaf000000);
				blit_rect_fill(submenu_start_x, submenu_start_y-1, window_pixel, 1); // top horizontal outline
				blit_rect_fill(submenu_start_x+window_pixel, submenu_start_y, 1, 8*(drawn+2)); // right vertical outline
				blit_rect_fill(submenu_start_x-1, submenu_start_y, 1, 8*(drawn+2)); // left vertical outline
				// set the y position
				submenu_start_y += font->height;
			}
		}
		
		// if menu is selected, change color
		if (submax_menu==submenu_sel){
			bc = (vsh->config.ark_menu.vsh_bg_color < 2 || vsh->config.ark_menu.vsh_bg_color > 28)? 0xff8080:0x0000ff;
			fc = 0xffffff;
			bc |= (((u32)vsh->status.bc_alpha)<<24);
			if (vsh->status.bc_alpha == 0) vsh->status.bc_delta = 5;
			else if (vsh->status.bc_alpha == 255) vsh->status.bc_delta = -5;
			vsh->status.bc_alpha += vsh->status.bc_delta;
		}
		
		blit_set_color(fc, bc);

		temp = 0;
		int submenu_width = 0;
		// find widest submenu up until the UMD region option
		for (i = SUBMENU_USB_DEVICE; i <= SUBMENU_UMD_REGION_MODE; i++){
			temp = scePaf_strlen(g_messages[MSG_USB_DEVICE + i]);
			if (temp > submenu_width)
				submenu_width = temp;
		}

		// display menu
		if (g_messages[MSG_USB_DEVICE + submax_menu] && !vsh->config.ark_menu.avm_hidden[submax_menu]) {
			int len = 0, offset = 0, padding = 0;
			subcur_menu = submax_menu;
			drawn++;
			// submenus between USB_DEVICE and UMD_REGION are the only ones with subitems
			if (submax_menu >= SUBMENU_USB_DEVICE && submax_menu <= SUBMENU_UMD_REGION_MODE) {
				int subitem_start_x = 0;
				int space = 3;
				
				if (!vsh->config.ark_menu.window_mode) {
					// left justify
					scePaf_snprintf(msg, 128, " %-*s", submenu_width, g_messages[MSG_USB_DEVICE + submax_menu]);
					subitem_start_x = blit_string(submenu_start_x, submenu_start_y, msg);
				} else if (vsh->config.ark_menu.window_mode) {
					// right justify
					scePaf_snprintf(msg, 128, " %s", g_messages[MSG_USB_DEVICE + submax_menu]);
					subitem_start_x = blit_string(submenu_start_x + (submenu_width - scePaf_strlen(msg)) * font->width , submenu_start_y, msg);
				}
				
				
				if(subitem_str[submax_menu]) {
					char *subitem_p = 0;
					// check if PSP Go or PSVita because UMD Region mode is unsupported on them
					if ((vsh->psp_model == PSP_GO || IS_VITA_ADR(vsh->config.p_ark)) && submax_menu == SUBMENU_UMD_REGION_MODE) {
						subitem_p = g_messages[MSG_UNSUPPORTED];
					} else {
						subitem_p = subitem_str[submax_menu];
					}
					
					// write subitem or unsupported message
					if (!vsh->config.ark_menu.window_mode) {
						scePaf_snprintf(msg, 128, "%-*s", window_char - space - submenu_width, subitem_p);
					} else if (vsh->config.ark_menu.window_mode) {
						scePaf_snprintf(msg, 128, "%s ", subitem_p);
					}
				}
				
				if (!vsh->config.ark_menu.window_mode) {
					// fill space between submenu and it's subitem
					blit_rect_fill(subitem_start_x, submenu_start_y, font->width * (space - 1), font->height);
					subitem_start_x += font->width * (space - 1);
				} else if (vsh->config.ark_menu.window_mode) {
					subitem_start_x += font->width * (space - 1);
				}
				
				blit_string(subitem_start_x, submenu_start_y, msg);
			// for all other submenus (ie those with no subitems)
			} else {
				// center-justify submenu options
				if (vsh->psp_model != PSP_GO && submax_menu == SUBMENU_DELETE_HIBERNATION) {
					// hibernation mode unsupported if model is not PSP Go
					len = scePaf_strlen(g_messages[MSG_NO_HIBERNATION]);
			
					if (!vsh->config.ark_menu.window_mode) {
						padding = (window_char - len) / 2;
					} else if (vsh->config.ark_menu.window_mode) {
						padding = 1;
					}
					
					scePaf_snprintf(msg, 128, "%*s%s%*s", padding, "", g_messages[MSG_NO_HIBERNATION], padding, "");
				} else {
					len = scePaf_strlen(g_messages[MSG_USB_DEVICE + submax_menu]);
					
					if (!vsh->config.ark_menu.window_mode) {
						padding = (window_char - len) / 2;
					} else if (vsh->config.ark_menu.window_mode) {
						padding = 1;
					}
					
					scePaf_snprintf(msg, 128, "%*s%s%*s", padding, "", g_messages[MSG_USB_DEVICE + submax_menu], padding, "");
				}
				
				blit_string_ctr(submenu_start_y, msg);
			
				if (!vsh->config.ark_menu.window_mode) {
					// add a halfspace after if the length is an odd value
					if (len & 0x1) {
						blit_rect_fill(submenu_start_x, submenu_start_y, 4, font->height);
						offset = blit_get_string_width(msg);
						blit_rect_fill(submenu_start_x + offset + 4, submenu_start_y, 4, font->height);
					}
				}
			}
			
			// set the y position
			submenu_start_y += font->height;
		}
	}
	
	if (!vsh->config.ark_menu.window_mode) {
	// set default colors
	bc = colors[vsh->config.ark_menu.vsh_bg_color];
		switch(vsh->config.ark_menu.vsh_fg_color){
			case 0: break;
			case 1: fc = colors[27]; break;
			case 27: fc = colors[1]; break;
			default: fc = colors[vsh->config.ark_menu.vsh_fg_color]; break;
		}
		blit_set_color(fc, bc);
		// add line at the end
		blit_rect_fill(submenu_start_x, submenu_start_y, window_pixel, font->height);
		blit_set_color(0xaf000000, 0xaf000000);
		blit_rect_fill(submenu_start_x, submenu_start_y+font->height, window_pixel, 1); // bottom horizontal outline
		
	}
	
	blit_set_color(0x00ffffff,0x00000000);
	return 0;
}

int submenu_find_longest_string(void){
	int width = 0, temp = 0, i;
	temp = scePaf_strlen(g_messages[SUBITEM_DEFAULT]);
	if (temp > width)
		width = temp;
	
	for (i = SUBITEM_REGION; i <= SUBITEM_REGION_END; i++) {
		temp = scePaf_strlen(g_messages[i]);
		if (temp > width)
			width = temp;
	}
	
	for (i = SUBITEM_USBREADONLY; i <= SUBITEM_USBREADONLY_END; i++) {
		temp = scePaf_strlen(g_messages[i]);
		if (temp > width)
			width = temp;
	}
	
	for (i = SUBITEM_SWAPXO; i <= SUBITEM_SWAPXO_END; i++) {
		temp = scePaf_strlen(g_messages[i]);
		if (temp > width)
			width = temp;
	}
	
	for (i = SUBITEM_PANDORA; i <= SUBITEM_PANDORA_END; i++) {
		temp = scePaf_strlen(g_messages[i]);
		if (temp > width)
			width = temp;
	}
	
	temp = scePaf_strlen(g_messages[SUBITEM_UNSUPPORTED]);
	if (temp > width)
		width = temp;
	
	for (i = SUBITEM_USBDEVICE; i <= SUBITEM_USBDEVICE_END; i++) {
		temp = scePaf_strlen(g_messages[i]);
		if (temp > width)
			width = temp;
	}
	
	temp = scePaf_strlen(g_messages[SUBITEM_NONE]);
	if (temp > width)
		width = temp;
	
	for (i = SUBITEM_COLOR; i <= SUBITEM_COLOR_END; i++) {
		temp = scePaf_strlen(g_messages[i]);
		if (temp > width)
			width = temp;
	}
	
	return width;
}


int submenu_setup(void) {
	int i;
	const char *bridge;
	const char *umdvideo_disp;
	
	vsh_Menu *vsh = vsh_menu_pointer();

	// preset
	for (i = 0; i < SUBMENU_MAX; i++) {
		subitem_str[i] = NULL;
		item_fcolor[i] = RGB(255,255,255);
	}

	if (vsh->battery == 2){
		vsh->config.ark_menu.avm_hidden[SUBMENU_CONVERT_BATTERY] = 1;
	}

	if (IS_VITA_ADR(vsh->config.p_ark)){
		vsh->config.ark_menu.avm_hidden[SUBMENU_USB_DEVICE] = 1;
		vsh->config.ark_menu.avm_hidden[SUBMENU_USB_READONLY] = 1;
		vsh->config.ark_menu.avm_hidden[SUBMENU_UMD_VIDEO] = 1;
		vsh->config.ark_menu.avm_hidden[SUBMENU_UMD_REGION_MODE] = 1;
	}

	if (vsh->psp_model == PSP_GO){
		vsh->config.ark_menu.avm_hidden[SUBMENU_UMD_REGION_MODE] = 1;
	}
	else {
		vsh->config.ark_menu.avm_hidden[SUBMENU_DELETE_HIBERNATION] = 1;
	}

	if (vsh->codecs){
		vsh->config.ark_menu.avm_hidden[SUBMENU_ACTIVATE_FLASH_WMA] = 1;
	}

	if (vsh->config.ark_menu.avm_hidden[submenu_sel]){
		for (i = 0; i < SUBMENU_MAX; i++){
			if (!vsh->config.ark_menu.avm_hidden[i]){
				submenu_sel = i;
				break;
			}
		}
	}

	//usb device
	if ((vsh->config.se.usbdevice > 0) && (vsh->config.se.usbdevice < 5)) {
		scePaf_sprintf(device_buf, "%s %d", g_messages[MSG_FLASH], vsh->config.se.usbdevice - 1);
		bridge = device_buf;
	} else if (IS_VITA_ADR(vsh->config.p_ark)) {
		scePaf_sprintf(device_buf, "%s", g_messages[MSG_USE_ADRENALINE_SETTINGS]);
		bridge = device_buf;
	} else {
		const char *device;
		if(vsh->config.se.usbdevice==5)
			device= g_messages[MSG_UMD_DISC];
		else if(vsh->psp_model == PSP_GO)
			device = g_messages[MSG_INTERNAL_STORAGE];
		else
			device = g_messages[MSG_MEMORY_STICK];

		bridge = device;
	}
	subitem_str[SUBMENU_USB_DEVICE] = bridge;

	umdvideo_disp = (const char*)scePaf_strrchr(umdvideo_path, '/');

	if (umdvideo_disp == NULL)
		umdvideo_disp = umdvideo_path;
	else
		umdvideo_disp++;

	if (IS_VITA_ADR(vsh->config.p_ark))
		subitem_str[SUBMENU_UMD_VIDEO] = g_messages[MSG_UNSUPPORTED];
	else
		subitem_str[SUBMENU_UMD_VIDEO] = umdvideo_disp;

	if (vsh->config.ark_menu.vsh_font)
		subitem_str[SUBMENU_FONT] = font_list()[vsh->config.ark_menu.vsh_font - 1];
	else
		subitem_str[SUBMENU_FONT] = g_messages[MSG_DEFAULT];

	switch (vsh->config.ark_menu.window_mode) {
		case 1:
			subitem_str[SUBMENU_MENU_DESIGN] = g_messages[MSG_CLASSIC];
			break;
		case 0:
			subitem_str[SUBMENU_MENU_DESIGN] = g_messages[MSG_NEW];
			break;
	}
	switch (vsh->config.ark_menu.advanced_vsh) {
		case 1:
			subitem_str[SUBMENU_MAIN_MENU] = g_messages[MSG_ADVANCED];
			break;
		case 0:
			subitem_str[SUBMENU_MAIN_MENU] = g_messages[MSG_SIMPLE];
			break;
	}

	switch (vsh->config.se.usbdevice_rdonly) {
		case 0:
			subitem_str[SUBMENU_USB_READONLY] = g_messages[MSG_DISABLE];
			break;
		case 1:
			subitem_str[SUBMENU_USB_READONLY] = g_messages[MSG_ENABLE];
			break;
		case 2:
			subitem_str[SUBMENU_USB_READONLY] = g_messages[MSG_UNSUPPORTED];
			break;
		default:
			subitem_str[SUBMENU_USB_READONLY] = g_messages[MSG_ENABLE];
	}

	subitem_str[SUBMENU_SWAP_XO_BUTTONS] = g_messages[MSG_O_PRIM-vsh->status.swap_xo];

	subitem_str[SUBMENU_CONVERT_BATTERY] = (vsh->battery<2)? g_messages[MSG_NORMAL_TO_PANDORA+vsh->battery] : g_messages[MSG_UNSUPPORTED];

	if (vsh->config.se.vshregion < 14){
		subitem_str[SUBMENU_REGION_MODE] = g_messages[vsh->config.se.vshregion];
	}
	else {
		subitem_str[SUBMENU_REGION_MODE] = g_messages[MSG_DISABLE];
	}

	if (vsh->config.se.umdregion < 4){
		subitem_str[SUBMENU_UMD_REGION_MODE] = g_messages[vsh->config.se.umdregion];
	}
	else {
		subitem_str[SUBMENU_UMD_REGION_MODE] = g_messages[MSG_DEFAULT];
	}
	
	if (vsh->config.ark_menu.vsh_fg_color < 29){
		switch(vsh->config.ark_menu.vsh_fg_color){
			case 1: subitem_str[SUBMENU_FG_COLORS] = g_messages[MSG_WHITE]; break;
			case 27: subitem_str[SUBMENU_FG_COLORS] = g_messages[MSG_RED]; break;
			default: subitem_str[SUBMENU_FG_COLORS] = g_messages[MSG_RANDOM+vsh->config.ark_menu.vsh_fg_color]; break;
		}
		
	}
	else {
		subitem_str[SUBMENU_BG_COLORS] = g_messages[MSG_RED];
	}

	if (vsh->config.ark_menu.vsh_bg_color < 29){
		subitem_str[SUBMENU_BG_COLORS] = g_messages[MSG_RANDOM+vsh->config.ark_menu.vsh_bg_color];
	}
	else {
		subitem_str[SUBMENU_BG_COLORS] = g_messages[MSG_RED];
	}

	return 0;
}


int submenu_ctrl(u32 button_on) {
	int direction;
	vsh_Menu *vsh = vsh_menu_pointer();


	if ((button_on & PSP_CTRL_SELECT) || (button_on & PSP_CTRL_HOME) || button_decline(button_on)) {
		submenu_sel = SUBMENU_GO_BACK;
		return 1;
	}

	if (button_on & PSP_CTRL_LTRIGGER && submenu_sel != SUBMENU_GO_BACK){
		vsh->config.ark_menu.avm_hidden[submenu_sel] = 1; // hide entry
		button_on |= PSP_CTRL_DOWN; // go to next available entry
	}

	if (button_on & PSP_CTRL_RTRIGGER){
		// unhide all entries
		memset(vsh->config.ark_menu.avm_hidden, 0, sizeof(vsh->config.ark_menu.avm_hidden));
		return 0;
	}

	// change menu select
	do {
		direction = 0;
		if (button_on & PSP_CTRL_DOWN) 
			direction++;
		if (button_on & PSP_CTRL_UP) 
			direction--;

		#define ROLL_OVER(val, min, max) ( ((val) < (min)) ? (max): ((val) > (max)) ? (min) : (val) )
		submenu_sel = ROLL_OVER(submenu_sel + direction, 0, SUBMENU_MAX - 1);
	} while (vsh->config.ark_menu.avm_hidden[submenu_sel]);

	// LEFT & RIGHT
	direction = -2;

	if(button_on & PSP_CTRL_LEFT)
		direction = -1;
	if(button_accept(button_on))
		direction = 0;
	if(button_on & PSP_CTRL_RIGHT) 
		direction = 1;

	if(direction <= -2)
		return 0;

	switch(submenu_sel) {
		case SUBMENU_USB_DEVICE:
			if (IS_VITA_ADR(vsh->config.p_ark)) 
				break;
			if (direction) 
				change_usb(direction);
			break;
		case SUBMENU_USB_READONLY:
			if (IS_VITA_ADR(vsh->config.p_ark)) 
				break;
			if (direction) 
				swap_readonly(direction);
			break;
		case SUBMENU_UMD_VIDEO:
			if (IS_VITA_ADR(vsh->config.p_ark)) 
				break;
			if (direction) {
			   	change_umd_mount_idx(direction);

				if(vsh->status.umdvideo_idx != 0) {
					char *umdpath;
					umdpath = umdvideolist_get(&vsh->umdlist, vsh->status.umdvideo_idx-1);

					if(umdpath != NULL) {
						scePaf_strncpy(umdvideo_path, umdpath, sizeof(umdvideo_path));
						umdvideo_path[sizeof(umdvideo_path)-1] = '\0';
					} else
						goto none;
				} else {
none:
					scePaf_strcpy(umdvideo_path, g_messages[MSG_NONE]);
				}
			} else
				return 6; // Mount UMDVideo ISO flag
			break;
		case SUBMENU_IMPORT_CLASSIC_PLUGINS:
			return 13; // Import Classic Plugins flag 
		case SUBMENU_DELETE_HIBERNATION:
			return 10; // Delete Hibernation flag 
		case SUBMENU_RANDOM_GAME:
			return 14; // Random Game flag 
		case SUBMENU_ACTIVATE_FLASH_WMA:
			return 11; // Activate Flash/WMA flag 
		case SUBMENU_REGION_MODE:
			if (direction) 
				change_region(direction, 13);
			break;
		case SUBMENU_UMD_REGION_MODE:
			if (vsh->psp_model == PSP_GO || IS_VITA_ADR(vsh->config.p_ark)) 
				break;
			if (direction) 
				change_umd_region(direction, 3);
			break;
		case SUBMENU_SWAP_XO_BUTTONS:
			return 12; // Swap X/O Buttons flag  
		case SUBMENU_CONVERT_BATTERY:
			return 9; // Convert Battery flag
		case SUBMENU_FG_COLORS:
			// This will be where I will be adding to set the color
			if (direction)
				change_fg_color(direction);
			break;
		case SUBMENU_BG_COLORS:
			// This will be where I will be adding to set the color
			if(direction) 
				change_bg_color(direction);
			break;
		case SUBMENU_FONT:
			if (direction) {
				change_font(direction);
				release_font();
				font_load(vsh);
			}
			break;
		case SUBMENU_MENU_DESIGN:
			if(direction)
				change_design(direction);
			break;
		case SUBMENU_MAIN_MENU:
			if(direction)
				change_menu(direction);
			break;
		case SUBMENU_GO_BACK:
			return 1; // finish
	}

	return 0; // continue
}


void subbutton_func(vsh_Menu *vsh) {
	int res;
	// copy pad from the vsh struct in case it can change during the function
	SceCtrlData pad = vsh->buttons.pad;
	// calculate new_buttons_on from old_pad and pad
	u32 new_buttons_on = ~vsh->buttons.old_pad.Buttons & vsh->buttons.pad.Buttons;
	
	// submenu control
	switch(vsh->status.submenu_mode) {
		case 0:	
			if ((pad.Buttons & ALL_CTRL) == 0)
				vsh->status.submenu_mode = 1;
			break;
		case 1:
			res = submenu_ctrl(new_buttons_on);

			if (res != 0) {
				sub_stop_stock = res;
				vsh->status.submenu_mode = 2;
			}
			break;
		case 2: // exit waiting 
			// exit submenu
			if ((pad.Buttons & ALL_CTRL) == 0)
				vsh->status.sub_stop_flag = sub_stop_stock;
			break;
	}
	// copy pad to oldpad
	scePaf_memcpy(&vsh->buttons.old_pad, &pad, sizeof(SceCtrlData));
}
