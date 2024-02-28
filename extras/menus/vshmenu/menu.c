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
	PSP VSH MENU controll
	based Booster's vshex
*/
#include "menu.h"

#include <psputility.h>

#include "common.h"
#include <systemctrl.h>
#include "vsh.h"
#include "ui.h"
#include "scepaf.h"
#include "fonts.h"
#include "blit.h"
#include "color.h"
#include "config.h"


const char **g_messages = g_messages_en;

extern char umd_path[72];


int stop_stock = 0;


char freq_buf[3+3+2] = "";
char freq2_buf[3+3+2] = "";
char device_buf[13] = "";
char umdvideo_path[256] = "";


int item_fcolor[TMENU_MAX];
const char *item_str[TMENU_MAX];

const int xyPoint[] ={0x98, 0x2F, 0xC0, 0xA0, 0x70, 0x08, 0x0E, 0xA8};//data243C=
const int xyPoint2[] ={0xB0, 0x2F, 0xD8, 0xB8, 0x88, 0x08, 0x11, 0xC0};//data2458=

static int menu_sel = TMENU_ADVANCED_VSH;

int menu_draw(void) {
	char msg[128] = {0};
	int max_menu, cur_menu;
	const int *pointer;
	u32 fc,bc;

	vsh_Menu *vsh = vsh_menu_pointer();
	blit_Gfx *gfx = blit_gfx_pointer();
	font_Data *font = font_data_pointer();
	u32 *colors = (u32*)color_data_pointer();
	
	// check & setup video mode
	if(blit_setup() < 0) 
		return -1;

	if(gfx->width == 720)
		pointer = xyPoint;
	else
		pointer = xyPoint2;

	// show menu title & ARK version
	blit_set_color(0xffffff,0x8000ff00);
	scePaf_snprintf(msg, 128, " %s ", g_messages[MSG_ARK_VSH_MENU]);
	blit_string_ctr(pointer[1], msg);
	blit_string_ctr(55, vsh->ark_version);
	fc = 0xffffff;
	
	
	// find widest submenu
	int window_char, window_pixel;
	int width = 0, temp = 0, i;
	for (i = 0; i < TMENU_MAX; i++) {
		temp = scePaf_strlen(g_messages[MSG_ADVANCED_VSH + i]);
		if (temp > width)
			width = temp;
	}
	
	window_char = width;
	// make the window an even value
	if (window_char & 0x1)
		window_char++;
	
	// window pixel = [window_char + leading & trailing space] * font width
	window_pixel = (window_char + 2) * font->width;
	
	// set menu start position
	int menu_start_y = pointer[5] * font->height;
	int menu_start_x = (gfx->width - window_pixel) / 2;
	
	for (max_menu = 0; max_menu < TMENU_MAX; max_menu++) {
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
			if (max_menu == 0){
				blit_set_color(fc, bc);
				blit_rect_fill(menu_start_x, menu_start_y, window_pixel, font->height);
				blit_set_color(0xaf000000, 0xaf000000);
				blit_rect_fill(menu_start_x, menu_start_y-1, window_pixel, 1); // top horizontal outline
				blit_rect_fill(menu_start_x+window_pixel, menu_start_y, 1, 8*(TMENU_MAX+2)); // right vertical outline
				blit_rect_fill(menu_start_x-1, menu_start_y, 1, 8*(TMENU_MAX+2)); // left vertical outline
			
				menu_start_y += font->height;
			}
		}
		
		// if menu is selected, change color
		if (max_menu == menu_sel) {
			bc = (vsh->config.ark_menu.vsh_bg_color < 2 || vsh->config.ark_menu.vsh_bg_color > 28)? 0xff8080:0x0000ff;
			fc = 0xffffff;
			bc |= (((u32)vsh->status.bc_alpha)<<24);
			if (vsh->status.bc_alpha == 0) vsh->status.bc_delta = 5;
			else if (vsh->status.bc_alpha == 255) vsh->status.bc_delta = -5;
			vsh->status.bc_alpha += vsh->status.bc_delta;
		}
		
		blit_set_color(fc, bc);
		
		// display menu
		if (g_messages[MSG_ADVANCED_VSH + max_menu]) {
			int padding = 0, len = 0;
			cur_menu = max_menu;
			
			// center-align menu strings
			len = scePaf_strlen(g_messages[MSG_ADVANCED_VSH + max_menu]);
			
			if (!vsh->config.ark_menu.window_mode) {
				padding = (window_char - len) / 2;
			} else if (vsh->config.ark_menu.window_mode) {
				padding = 0;
			}
			
			scePaf_snprintf(msg, 128, " %*s%s%*s ", padding, "", g_messages[MSG_ADVANCED_VSH + max_menu], padding, "");
			blit_string_ctr(menu_start_y, msg);
			
			if (!vsh->config.ark_menu.window_mode) {
				// add a halfspace after if the length is an odd value
				if (len & 0x1) {
					blit_rect_fill(menu_start_x, menu_start_y, 4, font->height); // front
					int offset = blit_get_string_width(msg);
					blit_rect_fill(menu_start_x + offset + 4, menu_start_y, 4, font->height); // back
				}
			}
			
			menu_start_y += font->height;
		
			// item_str seems to be all NULL values (see menu_setup function)
			// most likely this is not used and can be cleaned up
			/*
			if (item_str[max_menu]) {
				int menu_start_x = pointer[4];
				scePaf_snprintf(msg, 128, "%s", item_str[max_menu]);
				blit_string(menu_start_x, menu_start_y, msg);
			}
			*/
		}
	}
	
	if (!vsh->config.ark_menu.window_mode) {
		// reset colors to default
		bc = colors[vsh->config.ark_menu.vsh_bg_color];
		switch(vsh->config.ark_menu.vsh_fg_color){
			case 0: break;
			case 1: fc = colors[27]; break;
			case 27: fc = colors[1]; break;
			default: fc = colors[vsh->config.ark_menu.vsh_fg_color]; break;
		}

		blit_set_color(fc, bc);
		// add line at the end
		blit_rect_fill(menu_start_x, menu_start_y, window_pixel, font->height);
		blit_set_color(0xaf000000, 0xaf000000);
		blit_rect_fill(menu_start_x, menu_start_y+8, window_pixel, 1); // bottom horizontal outline
	}
	
	blit_set_color(0x00ffffff,0x00000000);
	return 0;
}

const char *get_enable_disable(int opt) {
	if(opt)
		return g_messages[MSG_ENABLE];
	return g_messages[MSG_DISABLE];
}

int menu_setup(void) {
	int i;

	// preset
	for(i=0;i<TMENU_MAX;i++) {
		item_str[i] = NULL;
		item_fcolor[i] = RGB(255,255,255);
	}

	return 0;
}

int menu_ctrl(u32 button_on) {
	int direction;

	if((button_on & PSP_CTRL_SELECT) || (button_on & PSP_CTRL_HOME) || button_decline(button_on)) {
		menu_sel = TMENU_EXIT;
		return 1;
	}

	// change menu select
	direction = 0;

	if (button_on & PSP_CTRL_DOWN) 
		direction++;
	if (button_on & PSP_CTRL_UP) 
		direction--;

	#define ROLL_OVER(val, min, max) ( ((val) < (min)) ? (max): ((val) > (max)) ? (min) : (val) )
	menu_sel = ROLL_OVER(menu_sel + direction, 0, TMENU_MAX - 1);

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

	switch(menu_sel) {
		case TMENU_RECOVERY_MENU:
			return 8; // Recovery menu flag
		case TMENU_ADVANCED_VSH:
			return 15;
		case TMENU_SHUTDOWN_DEVICE:			
			return 3; // SHUTDOWN flag
		case TMENU_RESET_DEVICE:	
			return 2; // RESET flag
		case TMENU_RESET_VSH:	
			return 4; // RESET VSH flag
		case TMENU_SUSPEND_DEVICE:	
			return 5; // SUSPEND flag
		case TMENU_EXIT:
			return 1; // finish
	}

	return 0; // continue
}


void button_func(vsh_Menu *vsh) {
	int res;
	// copy pad from the vsh struct in case it can change during the function
	SceCtrlData pad = vsh->buttons.pad;
	// calculate new_buttons_on from old_pad and pad
	u32 new_buttons_on = ~vsh->buttons.old_pad.Buttons & vsh->buttons.pad.Buttons;
	
	// menu control
	switch(vsh->status.menu_mode) {
		case 0:	
			if ((pad.Buttons & ALL_CTRL) == 0)
				vsh->status.menu_mode = 1;
			break;
		case 1:
			res = menu_ctrl(new_buttons_on);

			if(res != 0) {
				stop_stock = res;
				vsh->status.menu_mode = 2;
			}
			break;
		case 2: // exit waiting 
			// exit menu
			if ((pad.Buttons & ALL_CTRL) == 0)
				vsh->status.stop_flag = stop_stock;
			break;
	}
	// copy pad to oldpad
	scePaf_memcpy(&vsh->buttons.old_pad, &pad, sizeof(SceCtrlData));
}
