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

#include "common.h"
#include <psputility.h>

const char **g_messages = g_messages_en;

void change_clock(int dir, int a);

extern int pwidth;
extern char umd_path[72];
extern SEConfig cnf;
extern t_conf config;

char freq_buf[3+3+2] = "";
char freq2_buf[3+3+2] = "";
char device_buf[13] = "";
char umdvideo_path[256] = "";

#define TMENU_MAX 8

enum {
	TMENU_CUSTOM_LAUNCHER,
	TMENU_RECOVERY_MENU,
	TMENU_ADVANCED_VSH,
	TMENU_SHUTDOWN_DEVICE,
	TMENU_SUSPEND_DEVICE,
	TMENU_RESET_DEVICE,
	TMENU_RESET_VSH,
	TMENU_EXIT,
};

int item_fcolor[TMENU_MAX];
const char *item_str[TMENU_MAX];

static int menu_sel = TMENU_CUSTOM_LAUNCHER;

const int xyPoint[] ={0x98, 0x30, 0xC0, 0xA0, 0x70, 0x08, 0x0E, 0xA8};//data243C=
const int xyPoint2[] ={0xB0, 0x30, 0xD8, 0xB8, 0x88, 0x08, 0x11, 0xC0};//data2458=

int colors_dir = 0;

int menu_draw(void)
{
	u32 fc,bc;
	const char *msg;
	int max_menu, cur_menu;
	const int *pointer;
	int xPointer;
	
	// ARK Version
	const char ark_version[24];
	int ver = sctrlHENGetMinorVersion();
 	int major = (ver&0xFF0000)>>16;
	int minor = (ver&0xFF00)>>8;
	int micro = (ver&0xFF);

	#ifdef DEBUG
    if (micro>0) snprintf(ark_version, sizeof(ark_version), " ARK %d.%d.%.2i DEBUG ", major, minor, micro);
    else snprintf(ark_version, sizeof(ark_version), " ARK %d.%d DEBUG ", major, minor);
    #else
    if (micro>0) snprintf(ark_version, sizeof(ark_version), "    ARK %d.%d.%.2i    ", major, minor, micro);
    else snprintf(ark_version, sizeof(ark_version), "      ARK %d.%d     ", major, minor); 
	#endif

	// check & setup video mode
	if( blit_setup() < 0) return -1;

	if(pwidth==720) {
		pointer = xyPoint;
	} else {
		pointer = xyPoint2;
	}

	// show menu list
	blit_set_color(0xffffff,0x8000ff00);
	blit_string(pointer[0], pointer[1], g_messages[MSG_ARK_VSH_MENU]);
	blit_string(pointer[0], 56, ark_version);
	fc = 0xffffff;
 
	for(max_menu=0;max_menu<TMENU_MAX;max_menu++) {
		msg = g_messages[MSG_CUSTOM_LAUNCHER + max_menu];

		switch(config.vsh_bg_color) {
						// Random
						case 0:
						// Red
						case 1: 
							bc = (max_menu==menu_sel) ? 0xff8080 : 0x000000ff;
							blit_set_color(fc,bc);
							break;
						// Light Red
						case 2: 
							bc = (max_menu==menu_sel) ? 0x0000ff : 0xa00000ff;
							blit_set_color(fc,bc);
							break;
						// Orange
						case 3: 
							bc = (max_menu==menu_sel) ? 0x0000ff : 0x0000a5ff;
							blit_set_color(fc,bc);
							break;
						// Light Orange
						case 4: 
							bc = (max_menu==menu_sel) ? 0x0000ff : 0xa000a5ff;
							blit_set_color(fc,bc);
							break;
						// Yellow
						case 5: 
							bc = (max_menu==menu_sel) ? 0x0000ff : 0x0000e6e6;
							blit_set_color(fc,bc);
							break;
						// Light Yellow
						case 6: 
							bc = (max_menu==menu_sel) ? 0x0000ff : 0xa000e6e6;
							blit_set_color(fc,bc);
							break;
						// Green
						case 7:
							bc = (max_menu==menu_sel) ? 0x0000ff : 0x0000b300;
							blit_set_color(fc,bc);
							break;
						// Light Green
						case 8:
							bc = (max_menu==menu_sel) ? 0x0000ff : 0xa000ff00;
							blit_set_color(fc,bc);
							break;
						// Blue
						case 9:
							bc = (max_menu==menu_sel) ? 0x0000ff : 0x00ff0000;
							blit_set_color(fc,bc);
							break;
						// Light Blue
						case 10:
							bc = (max_menu==menu_sel) ? 0x0000ff : 0xa0ff0000;
							blit_set_color(fc,bc);
							break;
						// Indigo
						case 11:
							bc = (max_menu==menu_sel) ? 0x0000ff : 0x0082004b;
							blit_set_color(fc,bc);
							break;
						// Light Indigo
						case 12:
							bc = (max_menu==menu_sel) ? 0x0000ff : 0xa082004b;
							blit_set_color(fc,bc);
							break;
						// Violet
						case 13:
							bc = (max_menu==menu_sel) ? 0x0000ff : 0x00ee82ee;
							blit_set_color(fc,bc);
							break;
						// Light Violet
						case 14:
							bc = (max_menu==menu_sel) ? 0x0000ff : 0xa0ee82ee;
							blit_set_color(fc,bc);
							break;
						// Pink 
						case 15:
							bc = (max_menu==menu_sel) ? 0x0000ff : 0x00cbc0ff;
							blit_set_color(fc,bc);
							break;
						// Light Pink 
						case 16:
							bc = (max_menu==menu_sel) ? 0x0000ff : 0xa0cbc0ff;
							blit_set_color(fc,bc);
							break;
						// Purple 
						case 17:
							bc = (max_menu==menu_sel) ? 0x0000ff : 0x00993366;
							blit_set_color(fc,bc);
							break;
						// Light Purple 
						case 18:
							bc = (max_menu==menu_sel) ? 0x0000ff : 0xa0993366;
							blit_set_color(fc,bc);
							break;
						// Teal 
						case 19:
							bc = (max_menu==menu_sel) ? 0x0000ff : 0x00808000;
							blit_set_color(fc,bc);
							break;
						// Light Teal 
						case 20:
							bc = (max_menu==menu_sel) ? 0x0000ff : 0xa0808000;
							blit_set_color(fc,bc);
							break;
						// Aqua 
						case 21:
							bc = (max_menu==menu_sel) ? 0x0000ff : 0x00cccc00;
							blit_set_color(fc,bc);
							break;
						// Light Aqua 
						case 22:
							bc = (max_menu==menu_sel) ? 0x0000ff : 0xa0cccc00;
							blit_set_color(fc,bc);
							break;
						// Grey 
						case 23:
							bc = (max_menu==menu_sel) ? 0x0000ff : 0x00737373;
							blit_set_color(fc,bc);
							break;
						// Light Grey 
						case 24:
							bc = (max_menu==menu_sel) ? 0x0000ff : 0xa0737373;
							blit_set_color(fc,bc);
							break;
						// Black 
						case 25:
							bc = (max_menu==menu_sel) ? 0x0000ff : 0x00000000;
							blit_set_color(fc,bc);
							break;
						// Light Black 
						case 26:
							bc = (max_menu==menu_sel) ? 0x0000ff : 0xa0000000;
							blit_set_color(fc,bc);
							break;
						// White  
						case 27:
							bc = (max_menu==menu_sel) ? 0x0000ff : 0x00ffffff;
							blit_set_color(fc,bc);
							break;
						// Light White  
						case 28:
							bc = (max_menu==menu_sel) ? 0x0000ff : 0xafffffff;
							blit_set_color(fc,bc);
							break;
						default:	
							bc = (max_menu==menu_sel) ? 0xff8080 :0x0000a5ff;
							blit_set_color(fc,bc);
					}

					switch(config.vsh_fg_color) {
						// Random  
						case 0:
						// White  
						case 1:
							fc = (max_menu==menu_sel) ? 0xffffff : 0x00ffffff;
							blit_set_color(fc,bc);
							break;
						// Orange
						case 2: 
							fc = (max_menu==menu_sel) ? 0xffffff : 0x0000a5ff;
							blit_set_color(fc,bc);
							break;
						// Light Orange
						case 3: 
							fc = (max_menu==menu_sel) ? 0xffffff : 0xa000a5ff;
							blit_set_color(fc,bc);
							break;
						// Yellow
						case 4: 
							fc = (max_menu==menu_sel) ? 0xffffff : 0x0000e6e6;
							blit_set_color(fc,bc);
							break;
						// Light Yellow
						case 5: 
							fc = (max_menu==menu_sel) ? 0xffffff : 0xa000e6e6;
							blit_set_color(fc,bc);
							break;
						// Green
						case 6:
							fc = (max_menu==menu_sel) ? 0xffffff : 0x0000b300;
							blit_set_color(fc,bc);
							break;
						// Light Green
						case 7:
							fc = (max_menu==menu_sel) ? 0xffffff : 0xa000ff00;
							blit_set_color(fc,bc);
							break;
						// Blue
						case 8:
							fc = (max_menu==menu_sel) ? 0xffffff : 0x00ff0000;
							blit_set_color(fc,bc);
							break;
						// Light Blue
						case 9:
							fc = (max_menu==menu_sel) ? 0xffffff : 0xa0ff0000;
							blit_set_color(fc,bc);
							break;
						// Indigo
						case 10:
							fc = (max_menu==menu_sel) ? 0xffffff : 0x0082004b;
							blit_set_color(fc,bc);
							break;
						// Light Indigo
						case 11:
							fc = (max_menu==menu_sel) ? 0xffffff : 0xa082004b;
							blit_set_color(fc,bc);
							break;
						// Violet
						case 12:
							fc = (max_menu==menu_sel) ? 0xffffff : 0x00ee82ee;
							blit_set_color(fc,bc);
							break;
						// Light Violet
						case 13:
							fc = (max_menu==menu_sel) ? 0xffffff : 0xa0ee82ee;
							blit_set_color(fc,bc);
							break;
						// Pink 
						case 14:
							fc = (max_menu==menu_sel) ? 0xffffff : 0x00cbc0ff;
							blit_set_color(fc,bc);
							break;
						// Light Pink 
						case 15:
							fc = (max_menu==menu_sel) ? 0xffffff : 0xa0cbc0ff;
							blit_set_color(fc,bc);
							break;
						// Purple 
						case 16:
							fc = (max_menu==menu_sel) ? 0xffffff : 0x00993366;
							blit_set_color(fc,bc);
							break;
						// Light Purple 
						case 17:
							fc = (max_menu==menu_sel) ? 0xffffff : 0xa0993366;
							blit_set_color(fc,bc);
							break;
						// Teal 
						case 18:
							fc = (max_menu==menu_sel) ? 0xffffff : 0x00808000;
							blit_set_color(fc,bc);
							break;
						// Light Teal 
						case 19:
							fc = (max_menu==menu_sel) ? 0xffffff : 0xa0808000;
							blit_set_color(fc,bc);
							break;
						// Aqua 
						case 20:
							fc = (max_menu==menu_sel) ? 0xffffff : 0x00cccc00;
							blit_set_color(fc,bc);
							break;
						// Light Aqua 
						case 21:
							fc = (max_menu==menu_sel) ? 0xffffff : 0xa0cccc00;
							blit_set_color(fc,bc);
							break;
						// Grey 
						case 22:
							fc = (max_menu==menu_sel) ? 0xffffff : 0x00737373;
							blit_set_color(fc,bc);
							break;
						// Light Grey 
						case 23:
							fc = (max_menu==menu_sel) ? 0xffffff : 0xa0737373;
							blit_set_color(fc,bc);
							break;
						// Black 
						case 24:
							fc = (max_menu==menu_sel) ? 0xffffff : 0x00000000;
							blit_set_color(fc,bc);
							break;
						// Light Black 
						case 25:
							fc = (max_menu==menu_sel) ? 0xffffff : 0xa0000000;
							blit_set_color(fc,bc);
							break;
						// Light Red
						case 26: 
							fc = (max_menu==menu_sel) ? 0xffffff : 0xa00000ff;
							blit_set_color(fc,bc);
							break;
						// Red
						case 27:
							fc = (max_menu==menu_sel) ? 0xffffff : 0x000000ff;
							blit_set_color(fc,bc);
							break;
						// Light White  
						case 28:
							fc = (max_menu==menu_sel) ? 0xffffff : 0xafffffff;
							blit_set_color(fc,bc);
							break;
						default:	
							fc = (max_menu==menu_sel) ? 0xffffff : 0x00ffffff;
							blit_set_color(fc,bc);
					}

		if(msg) {
			switch(max_menu) {
				case TMENU_EXIT:
					//xPointer = pointer[2];
					xPointer = 225;
					break;
				case TMENU_RESET_DEVICE:
					if (cur_language == PSP_SYSTEMPARAM_LANGUAGE_GERMAN) {
						xPointer = pointer[3] - 2 * 8 - 1;
					} else {
						//xPointer = pointer[3];
						xPointer = 192;
					}
					
					break;
				case TMENU_RESET_VSH:
					if (cur_language == PSP_SYSTEMPARAM_LANGUAGE_GERMAN) {
						xPointer = pointer[7] - 2 * 8 - 1;
					} else {
						//xPointer = pointer[7];
						xPointer = 205;
					}
					
					break;
				case TMENU_CUSTOM_LAUNCHER:
				case TMENU_RECOVERY_MENU:
					//xPointer = 168;
					xPointer = 172;
					break;
				case TMENU_ADVANCED_VSH:
					xPointer = 176;
					break;
				case TMENU_SHUTDOWN_DEVICE:
					xPointer = 180;
					break;
				case TMENU_SUSPEND_DEVICE:
					xPointer = 185;
					break;
				default:
					xPointer=pointer[4];
					break;
			}



			cur_menu = max_menu;
			blit_string(xPointer, (pointer[5] + cur_menu)*8, msg);
			msg = item_str[max_menu];
		} 
		
		if(msg) {
				blit_string( (pointer[6] * 8) + 128, (pointer[5] + cur_menu)*8, msg);
			}


	}

	blit_set_color(0x00ffffff,0x00000000);

	return 0;
}

static inline const char *get_enable_disable(int opt)
{
	if(opt) {
		return g_messages[MSG_ENABLE];
	}

	return g_messages[MSG_DISABLE];
}

int menu_setup(void)
{
	int i;

	// preset
	for(i=0;i<TMENU_MAX;i++) {
		item_str[i] = NULL;
		item_fcolor[i] = RGB(255,255,255);
	}

	return 0;
}

int menu_ctrl(u32 button_on)
{
	int direction;

	if( (button_on & PSP_CTRL_SELECT) ||
		(button_on & PSP_CTRL_HOME)) {
		menu_sel = TMENU_EXIT;
		return 1;
	}

	// change menu select
	direction = 0;

	if(button_on & PSP_CTRL_DOWN) direction++;
	if(button_on & PSP_CTRL_UP) direction--;

	menu_sel = limit(menu_sel+direction, 0, TMENU_MAX-1);

	// LEFT & RIGHT
	direction = -2;

	if(button_on & PSP_CTRL_LEFT)   direction = -1;
	if(button_on & PSP_CTRL_CROSS) direction = 0;
	if(button_on & PSP_CTRL_CIRCLE) direction = 0;
	if(button_on & PSP_CTRL_RIGHT)  direction = 1;

	if(direction <= -2)
		return 0;

	switch(menu_sel) {
		case TMENU_CUSTOM_LAUNCHER:
			if(direction==0) {
				return 7; // Custom Launcher menu flag
			}
			break;
		case TMENU_RECOVERY_MENU:
			if(direction==0) {
				return 8; // Recovery menu flag
			}
			break;
		case TMENU_ADVANCED_VSH:
			if(direction == 0) return 15;
			break;
		case TMENU_SHUTDOWN_DEVICE:			
			if(direction==0) {
				return 3; // SHUTDOWN flag
			}
			break;
		case TMENU_RESET_DEVICE:	
			if(direction==0) {
				return 2; // RESET flag
			}
			break;
		case TMENU_RESET_VSH:	
			if(direction==0) {
				return 4; // RESET VSH flag
			}
			break;
		case TMENU_SUSPEND_DEVICE:	
			if(direction==0) {
				return 5; // SUSPEND flag
			}
			break;
		case TMENU_EXIT:
			if(direction==0) return 1; // finish
			break;
	}

	return 0; // continue
}
