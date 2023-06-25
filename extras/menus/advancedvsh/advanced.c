#include "common.h"
#include <psputility.h>
#include <time.h>

#include "systemctrl.h"
#include "clock.h"

#include "vsh.h"
#include "fonts.h"
#include "advanced.h"

extern int pwidth;
extern char umd_path[72];

extern char device_buf[13];
extern char umdvideo_path[256];

extern int xyPoint[];
extern int xyPoint2[];
extern u32 colors[];

extern vsh_Menu *g_vsh_menu;
int sub_stop_stock = 0;


int item_fcolor[SUBMENU_MAX];
const char *subitem_str[SUBMENU_MAX];
static int submenu_sel = SUBMENU_USB_DEVICE;


int submenu_draw(void) {
	char msg[128] = {0};
	int submax_menu, subcur_menu;
	const int *pointer;
	u32 fc, bc;

	// ARK Version
	char ark_version[24];
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
		scePaf_snprintf(ark_version, sizeof(ark_version), "    ARK %d.%d.%.2i    ", major, minor, micro);
	else 
		scePaf_snprintf(ark_version, sizeof(ark_version), "    ARK %d.%d    ", major, minor); 
	#endif

	// check & setup video mode
	if(blit_setup() < 0) 
		return -1;
	
	if(pwidth == 720)
		pointer = xyPoint;
	else
		pointer = xyPoint2;

	// show menu title & ARK version
	blit_set_color(0xffffff,0x8000ff00);
	scePaf_snprintf(msg, 128, " %s ", g_messages[MSG_ADVANCED_VSH]);
	blit_string_ctr(pointer[1], msg);
	blit_string_ctr(56, ark_version);
	fc = 0xffffff;
	

	for (submax_menu = 0; submax_menu < SUBMENU_MAX; submax_menu++) {
		// do color stuff
		if (submax_menu==submenu_sel){
			bc = (g_vsh_menu->config.ark_menu.vsh_bg_color < 2 || g_vsh_menu->config.ark_menu.vsh_bg_color > 28)? 0xff8080:0x0000ff;
			fc = 0xffffff;
		}
		else{
			bc = colors[g_vsh_menu->config.ark_menu.vsh_bg_color];
			switch(g_vsh_menu->config.ark_menu.vsh_fg_color){
				case 0: case 1: fc = colors[27]; break;
				case 27: fc = colors[1]; break;
				default: fc = colors[g_vsh_menu->config.ark_menu.vsh_fg_color]; break;
			}
		}
		blit_set_color(fc,bc);

		// display menu
		if(g_messages[MSG_USB_DEVICE + submax_menu]) {		
			int submenu_start_x, submenu_start_y;
			subcur_menu = submax_menu;
			
			// set the y position
			submenu_start_y = (pointer[5] + subcur_menu) * 8;
			
			int width = 0, padding = 0, temp = 0, i;
			// submenus between USB_DEVICE and UMD_REGION are the only ones with subitems
			if (submax_menu >= SUBMENU_USB_DEVICE && submax_menu <= SUBMENU_UMD_REGION_MODE) {
				// check if there's a subitem and print it
				int subitem_start_x = (pointer[6] * 8) + 128;
				if(subitem_str[submax_menu]) {
					// check if PSP Go or PSVita because UMD Region mode is unsupported on them
					if ((g_vsh_menu->psp_model == PSP_GO || 
						IS_VITA_ADR(g_vsh_menu->config.p_ark)) && 
						submax_menu == SUBMENU_UMD_REGION_MODE) {
						// write the unsupported string
						scePaf_snprintf(msg, 128, "%s", g_messages[MSG_UNSUPPORTED]);
					} else {
						// otherwise write the subitem string
						scePaf_snprintf(msg, 128, "%s", subitem_str[submax_menu]);
					}
					
					blit_string(subitem_start_x, submenu_start_y, msg);
				}
				
				// find widest submenu up until the UMD region option
				for (i = submax_menu; i < SUBMENU_UMD_REGION_MODE; i++){
					temp = scePaf_strlen(g_messages[MSG_USB_DEVICE + i]);
					if (temp > width)
						width = temp;
				}
				
				// left-justify submenu options that have a subitem next to it
				scePaf_snprintf(msg, 128, " %-*s ", width, g_messages[MSG_USB_DEVICE + submax_menu]);
				
				// the submenu start x position is calculated from :
				// subitem start x position - length in pixel of the submenu string - length in pixel of a whitespace
				submenu_start_x = subitem_start_x - blit_get_string_width(msg) - blit_get_string_width(" ");
				blit_string(submenu_start_x, submenu_start_y, msg);
			// for all other submenus (ie those with no subitems)
			} else {
				// find widest submenu after the UMD region option
				for (i = submax_menu; i < SUBMENU_MAX; i++){
					temp = scePaf_strlen(g_messages[MSG_USB_DEVICE + i]);
					if (temp > width)
						width = temp;
					// if model is not PSPGo, check the "no hibernation support" string width too
					if (g_vsh_menu->psp_model != PSP_GO && i == SUBMENU_DELETE_HIBERNATION) {
						temp = scePaf_strlen(g_messages[MSG_NO_HIBERNATION]);
						if (temp > width)
							width = temp;
					}
				}
				
				// center-justify submenu options
				if (g_vsh_menu->psp_model != PSP_GO && submax_menu == SUBMENU_DELETE_HIBERNATION) {
					// hibernation mode unsupported if model is not PSP Go
					padding = (width - scePaf_strlen(g_messages[MSG_NO_HIBERNATION])) / 2;
					scePaf_snprintf(msg, 128, " %*s%s%*s ", padding, "", g_messages[MSG_NO_HIBERNATION], padding, "");
				} else {
					padding = (width - scePaf_strlen(g_messages[MSG_USB_DEVICE + submax_menu])) / 2;
					scePaf_snprintf(msg, 128, " %*s%s%*s ", padding, "", g_messages[MSG_USB_DEVICE + submax_menu], padding, "");
				}
				
				blit_string_ctr(submenu_start_y, msg);
			}
		}
	}

	blit_set_color(0x00ffffff,0x00000000);
	return 0;
}


int submenu_setup(void) {
	int i;
	const char *bridge;
	const char *umdvideo_disp;

	// preset
	for (i = 0; i < SUBMENU_MAX; i++) {
		subitem_str[i] = NULL;
		item_fcolor[i] = RGB(255,255,255);
	}
	
	//usb device
	if ((g_vsh_menu->config.se.usbdevice > 0) && (g_vsh_menu->config.se.usbdevice < 5)) {
		scePaf_sprintf(device_buf, "%s %d", g_messages[MSG_FLASH], g_vsh_menu->config.se.usbdevice - 1);
		bridge = device_buf;
	} else if (IS_VITA_ADR(g_vsh_menu->config.p_ark)) {
		scePaf_sprintf(device_buf, "%s", g_messages[MSG_USE_ADRENALINE_SETTINGS]);
		bridge = device_buf;
	} else {
		const char *device;
		if(g_vsh_menu->config.se.usbdevice==5)
			device= g_messages[MSG_UMD_DISC];
		else if(g_vsh_menu->psp_model == PSP_GO)
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

	if (IS_VITA_ADR(g_vsh_menu->config.p_ark))
		subitem_str[SUBMENU_UMD_VIDEO] = g_messages[MSG_UNSUPPORTED];
	else
		subitem_str[SUBMENU_UMD_VIDEO] = umdvideo_disp;

	if (g_vsh_menu->config.se.umdmode == 3)
		subitem_str[SUBMENU_UMD_MODE] = g_messages[MSG_INFERNO];
	else if (g_vsh_menu->config.se.umdmode < 2)
		g_vsh_menu->config.se.umdmode = 3;
	else if (g_vsh_menu->config.se.umdmode > 3)
		g_vsh_menu->config.se.umdmode = 2;
	else 
		subitem_str[SUBMENU_UMD_MODE] = g_messages[MSG_NP9660];

	if (g_vsh_menu->config.ark_menu.vsh_font)
		subitem_str[SUBMENU_FONT] = font_list()[g_vsh_menu->config.ark_menu.vsh_font - 1];
	else
		subitem_str[SUBMENU_FONT] = g_messages[MSG_DEFAULT];

	switch (g_vsh_menu->config.se.usbdevice_rdonly) {
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

	switch (g_vsh_menu->status.swap_xo) {
		case XO_CURRENT_O_PRIMARY:
			subitem_str[SUBMENU_SWAP_XO_BUTTONS] = g_messages[MSG_O_PRIM];
			break;
		case XO_CURRENT_X_PRIMARY:
			subitem_str[SUBMENU_SWAP_XO_BUTTONS] = g_messages[MSG_X_PRIM];
			break;
		default:
			subitem_str[SUBMENU_SWAP_XO_BUTTONS] = g_messages[MSG_X_PRIM]; // should never happen?
	}

	switch (g_vsh_menu->battery) {
		case NORMAL_TO_PANDORA:
			subitem_str[SUBMENU_CONVERT_BATTERY] = g_messages[MSG_NORMAL_TO_PANDORA];
			break;
		case PANDORA_TO_NORMAL:
			subitem_str[SUBMENU_CONVERT_BATTERY] = g_messages[MSG_PANDORA_TO_NORMAL];
			break;
		case UNSUPPORTED:
			subitem_str[SUBMENU_CONVERT_BATTERY] = g_messages[MSG_UNSUPPORTED];
			break;
	}

	switch(g_vsh_menu->config.se.vshregion) {
		case FAKE_REGION_DISABLED:
			subitem_str[SUBMENU_REGION_MODE] = g_messages[MSG_DEFAULT];
			break;
		case FAKE_REGION_JAPAN:
			subitem_str[SUBMENU_REGION_MODE] = g_messages[MSG_JAPAN];
			break;
		case FAKE_REGION_AMERICA:
			subitem_str[SUBMENU_REGION_MODE] = g_messages[MSG_AMERICA];
			break;
		case FAKE_REGION_EUROPE:
			subitem_str[SUBMENU_REGION_MODE] = g_messages[MSG_EUROPE];
			break;
		case FAKE_REGION_KOREA:
			subitem_str[SUBMENU_REGION_MODE] = g_messages[MSG_KOREA];
			break;
		case FAKE_REGION_UNITED_KINGDOM:
			subitem_str[SUBMENU_REGION_MODE] = g_messages[MSG_UNITED_KINGDOM];
			break;
		case FAKE_REGION_LATIN_AMERICA:
			subitem_str[SUBMENU_REGION_MODE] = g_messages[MSG_LATIN_AMERICA];
			break;
		case FAKE_REGION_AUSTRALIA:
			subitem_str[SUBMENU_REGION_MODE] = g_messages[MSG_AUSTRALIA];
			break;
		case FAKE_REGION_HONGKONG:
			subitem_str[SUBMENU_REGION_MODE] = g_messages[MSG_HONG_KONG];
			break;
		case FAKE_REGION_TAIWAN:
			subitem_str[SUBMENU_REGION_MODE] = g_messages[MSG_TAIWAN];
			break;
		case FAKE_REGION_RUSSIA:
			subitem_str[SUBMENU_REGION_MODE] = g_messages[MSG_RUSSIA];
			break;
		case FAKE_REGION_CHINA:
			subitem_str[SUBMENU_REGION_MODE] = g_messages[MSG_CHINA];
			break;
		case FAKE_REGION_DEBUG_TYPE_I:
			subitem_str[SUBMENU_REGION_MODE] = g_messages[MSG_DEBUG_I];
			break;
		case FAKE_REGION_DEBUG_TYPE_II:
			subitem_str[SUBMENU_REGION_MODE] = g_messages[MSG_DEBUG_II];
			break;
		default:
			subitem_str[SUBMENU_REGION_MODE] = g_messages[MSG_DISABLE];
	}

	switch(g_vsh_menu->config.se.umdregion) {
		case UMD_REGION_DEFAULT:
			subitem_str[SUBMENU_UMD_REGION_MODE] = g_messages[MSG_DEFAULT];
			break;
		case UMD_REGION_JAPAN:
			subitem_str[SUBMENU_UMD_REGION_MODE] = g_messages[MSG_JAPAN];
			break;
		case UMD_REGION_AMERICA:
			subitem_str[SUBMENU_UMD_REGION_MODE] = g_messages[MSG_AMERICA];
			break;
		case UMD_REGION_EUROPE:
			subitem_str[SUBMENU_UMD_REGION_MODE] = g_messages[MSG_EUROPE];
			break;
		default:
			subitem_str[SUBMENU_UMD_REGION_MODE] = g_messages[MSG_DEFAULT];
			break;
	}
	

	switch(g_vsh_menu->config.ark_menu.vsh_fg_color) {
		case FG_RANDOM:
			subitem_str[SUBMENU_FG_COLORS] = g_messages[MSG_RANDOM];
			break;
		case FG_RED:
			subitem_str[SUBMENU_FG_COLORS] = g_messages[MSG_RED];
			break;
		case FG_LITE_RED:
			subitem_str[SUBMENU_FG_COLORS] = g_messages[MSG_LITE_RED];
			break;
		case FG_ORANGE:
			subitem_str[SUBMENU_FG_COLORS] = g_messages[MSG_ORANGE];
			break;
		case FG_LITE_ORANGE:
			subitem_str[SUBMENU_FG_COLORS] = g_messages[MSG_LITE_ORANGE];
			break;
		case FG_YELLOW:
			subitem_str[SUBMENU_FG_COLORS] = g_messages[MSG_YELLOW];
			break;
		case FG_LITE_YELLOW:
			subitem_str[SUBMENU_FG_COLORS] = g_messages[MSG_LITE_YELLOW];
			break;
		case FG_GREEN:
			subitem_str[SUBMENU_FG_COLORS] = g_messages[MSG_GREEN];
			break;
		case FG_LITE_GREEN:
			subitem_str[SUBMENU_FG_COLORS] = g_messages[MSG_LITE_GREEN];
			break;
		case FG_BLUE:
			subitem_str[SUBMENU_FG_COLORS] = g_messages[MSG_BLUE];
			break;
		case FG_LITE_BLUE:
			subitem_str[SUBMENU_FG_COLORS] = g_messages[MSG_LITE_BLUE];
			break;
		case FG_INDIGO:
			subitem_str[SUBMENU_FG_COLORS] = g_messages[MSG_INDIGO];
			break;
		case FG_LITE_INDIGO:
			subitem_str[SUBMENU_FG_COLORS] = g_messages[MSG_LITE_INDIGO];
			break;
		case FG_VIOLET:
			subitem_str[SUBMENU_FG_COLORS] = g_messages[MSG_VIOLET];
			break;
		case FG_LITE_VIOLET:
			subitem_str[SUBMENU_FG_COLORS] = g_messages[MSG_LITE_VIOLET];
			break;
		case FG_PINK:
			subitem_str[SUBMENU_FG_COLORS] = g_messages[MSG_PINK];
			break;
		case FG_LITE_PINK:
			subitem_str[SUBMENU_FG_COLORS] = g_messages[MSG_LITE_PINK];
			break;
		case FG_PURPLE:
			subitem_str[SUBMENU_FG_COLORS] = g_messages[MSG_PURPLE];
			break;
		case FG_LITE_PURPLE:
			subitem_str[SUBMENU_FG_COLORS] = g_messages[MSG_LITE_PURPLE];
			break;
		case FG_TEAL:
			subitem_str[SUBMENU_FG_COLORS] = g_messages[MSG_TEAL];
			break;
		case FG_LITE_TEAL:
			subitem_str[SUBMENU_FG_COLORS] = g_messages[MSG_LITE_TEAL];
			break;
		case FG_AQUA:
			subitem_str[SUBMENU_FG_COLORS] = g_messages[MSG_AQUA];
			break;
		case FG_LITE_AQUA:
			subitem_str[SUBMENU_FG_COLORS] = g_messages[MSG_LITE_AQUA];
			break;
		case FG_GREY:
			subitem_str[SUBMENU_FG_COLORS] = g_messages[MSG_GREY];
			break;
		case FG_LITE_GREY:
			subitem_str[SUBMENU_FG_COLORS] = g_messages[MSG_LITE_GREY];
			break;
		case FG_BLACK:
			subitem_str[SUBMENU_FG_COLORS] = g_messages[MSG_BLACK];
			break;
		case FG_LITE_BLACK:
			subitem_str[SUBMENU_FG_COLORS] = g_messages[MSG_LITE_BLACK];
			break;
		case FG_WHITE:
			subitem_str[SUBMENU_FG_COLORS] = g_messages[MSG_WHITE];
			break;
		case FG_LITE_WHITE:
			subitem_str[SUBMENU_FG_COLORS] = g_messages[MSG_LITE_WHITE];
			break;
		default:
			subitem_str[SUBMENU_FG_COLORS] = g_messages[MSG_WHITE];
	}
	switch(g_vsh_menu->config.ark_menu.vsh_bg_color) {
		case BG_RANDOM:
			subitem_str[SUBMENU_BG_COLORS] = g_messages[MSG_RANDOM];
			break;
		case BG_RED:
			subitem_str[SUBMENU_BG_COLORS] = g_messages[MSG_RED];
			break;
		case BG_LITE_RED:
			subitem_str[SUBMENU_BG_COLORS] = g_messages[MSG_LITE_RED];
			break;
		case BG_ORANGE:
			subitem_str[SUBMENU_BG_COLORS] = g_messages[MSG_ORANGE];
			break;
		case BG_LITE_ORANGE:
			subitem_str[SUBMENU_BG_COLORS] = g_messages[MSG_LITE_ORANGE];
			break;
		case BG_YELLOW:
			subitem_str[SUBMENU_BG_COLORS] = g_messages[MSG_YELLOW];
			break;
		case BG_LITE_YELLOW:
			subitem_str[SUBMENU_BG_COLORS] = g_messages[MSG_LITE_YELLOW];
			break;
		case BG_GREEN:
			subitem_str[SUBMENU_BG_COLORS] = g_messages[MSG_GREEN];
			break;
		case BG_LITE_GREEN:
			subitem_str[SUBMENU_BG_COLORS] = g_messages[MSG_LITE_GREEN];
			break;
		case BG_BLUE:
			subitem_str[SUBMENU_BG_COLORS] = g_messages[MSG_BLUE];
			break;
		case BG_LITE_BLUE:
			subitem_str[SUBMENU_BG_COLORS] = g_messages[MSG_LITE_BLUE];
			break;
		case BG_INDIGO:
			subitem_str[SUBMENU_BG_COLORS] = g_messages[MSG_INDIGO];
			break;
		case BG_LITE_INDIGO:
			subitem_str[SUBMENU_BG_COLORS] = g_messages[MSG_LITE_INDIGO];
			break;
		case BG_VIOLET:
			subitem_str[SUBMENU_BG_COLORS] = g_messages[MSG_VIOLET];
			break;
		case BG_LITE_VIOLET:
			subitem_str[SUBMENU_BG_COLORS] = g_messages[MSG_LITE_VIOLET];
			break;
		case BG_PINK:
			subitem_str[SUBMENU_BG_COLORS] = g_messages[MSG_PINK];
			break;
		case BG_LITE_PINK:
			subitem_str[SUBMENU_BG_COLORS] = g_messages[MSG_LITE_PINK];
			break;
		case BG_PURPLE:
			subitem_str[SUBMENU_BG_COLORS] = g_messages[MSG_PURPLE];
			break;
		case BG_LITE_PURPLE:
			subitem_str[SUBMENU_BG_COLORS] = g_messages[MSG_LITE_PURPLE];
			break;
		case BG_TEAL:
			subitem_str[SUBMENU_BG_COLORS] = g_messages[MSG_TEAL];
			break;
		case BG_LITE_TEAL:
			subitem_str[SUBMENU_BG_COLORS] = g_messages[MSG_LITE_TEAL];
			break;
		case BG_AQUA:
			subitem_str[SUBMENU_BG_COLORS] = g_messages[MSG_AQUA];
			break;
		case BG_LITE_AQUA:
			subitem_str[SUBMENU_BG_COLORS] = g_messages[MSG_LITE_AQUA];
			break;
		case BG_GREY:
			subitem_str[SUBMENU_BG_COLORS] = g_messages[MSG_GREY];
			break;
		case BG_LITE_GREY:
			subitem_str[SUBMENU_BG_COLORS] = g_messages[MSG_LITE_GREY];
			break;
		case BG_BLACK:
			subitem_str[SUBMENU_BG_COLORS] = g_messages[MSG_BLACK];
			break;
		case BG_LITE_BLACK:
			subitem_str[SUBMENU_BG_COLORS] = g_messages[MSG_LITE_BLACK];
			break;
		case BG_WHITE:
			subitem_str[SUBMENU_BG_COLORS] = g_messages[MSG_WHITE];
			break;
		case BG_LITE_WHITE:
			subitem_str[SUBMENU_BG_COLORS] = g_messages[MSG_LITE_WHITE];
			break;
		default:
			subitem_str[SUBMENU_BG_COLORS] = g_messages[MSG_RED];
	}

	return 0;
}


int submenu_ctrl(u32 button_on) {
	int direction;

	if ((button_on & PSP_CTRL_SELECT) || (button_on & PSP_CTRL_HOME)) {
		submenu_sel = SUBMENU_GO_BACK;
		return 1;
	}

	// change menu select
	direction = 0;

	if (button_on & PSP_CTRL_DOWN) 
		direction++;
	if (button_on & PSP_CTRL_UP) 
		direction--;

	#define ROLL_OVER(val, min, max) ( ((val) < (min)) ? (max): ((val) > (max)) ? (min) : (val) )
	submenu_sel = ROLL_OVER(submenu_sel + direction, 0, SUBMENU_MAX - 1);

	// LEFT & RIGHT
	direction = -2;

	if(button_on & PSP_CTRL_LEFT)
		direction = -1;
	if(button_on & PSP_CTRL_CROSS)
		direction = 0;
	if(button_on & PSP_CTRL_CIRCLE)
		direction = 0;
	if(button_on & PSP_CTRL_RIGHT) 
		direction = 1;

	if(direction <= -2)
		return 0;

	switch(submenu_sel) {
		case SUBMENU_USB_DEVICE:
			if (IS_VITA_ADR(g_vsh_menu->config.p_ark)) 
				break;
			if (direction) 
				change_usb(direction);
			break;
		case SUBMENU_USB_READONLY:
			if (IS_VITA_ADR(g_vsh_menu->config.p_ark)) 
				break;
			if (direction) 
				swap_readonly(direction);
			break;
		case SUBMENU_UMD_MODE:
			if (direction) 
				change_umd_mode(direction);
			break;
		case SUBMENU_UMD_VIDEO:
			if (IS_VITA_ADR(g_vsh_menu->config.p_ark)) 
				break;
			if (direction) {
			   	change_umd_mount_idx(direction);

				if(umdvideo_idx != 0) {
					char *umdpath;
					umdpath = umdvideolist_get(&g_umdlist, umdvideo_idx-1);

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
			if (direction == 0)
				return 13; // Import Classic Plugins flag 
			break;
		case SUBMENU_DELETE_HIBERNATION:
			if (direction == 0)
				return 10; // Delete Hibernation flag 
			break;
		case SUBMENU_RANDOM_GAME:
			if (direction == 0)
				return 14; // Random Game flag 
			break;
		case SUBMENU_ACTIVATE_FLASH_WMA:
			if (direction == 0)
				return 11; // Activate Flash/WMA flag 
			break;
		case SUBMENU_REGION_MODE:
			if (direction) 
				change_region(direction, 13);
			break;
		case SUBMENU_UMD_REGION_MODE:
			if (g_vsh_menu->psp_model == PSP_GO || IS_VITA_ADR(g_vsh_menu->config.p_ark)) 
				break;
			if (direction) 
				change_umd_region(direction, 3);
			break;
		case SUBMENU_SWAP_XO_BUTTONS:
			if (direction == 0)
				return 12; // Swap X/O Buttons flag  
			break;
		case SUBMENU_CONVERT_BATTERY:
			if(direction == 0)
				return 9; // Convert Battery flag
			break;
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
				font_load(g_vsh_menu);
			}
			break;
		case SUBMENU_GO_BACK:
			if(direction==0) 
				return 1; // finish
			break;
	}

	return 0; // continue
}


void subbutton_func(vsh_Menu *vsh) {
	int res;
	// submenu control
	switch(vsh->status.submenu_mode) {
		case 0:	
			if ((vsh->buttons.pad.Buttons & ALL_CTRL) == 0)
				vsh->status.submenu_mode = 1;
			break;
		case 1:
			res = submenu_ctrl(vsh->buttons.new_buttons_on);

			if (res != 0) {
				sub_stop_stock = res;
				vsh->status.submenu_mode = 2;
			}
			break;
		case 2: // exit waiting 
			// exit submenu
			if ((vsh->buttons.pad.Buttons & ALL_CTRL) == 0)
				vsh->status.sub_stop_flag = sub_stop_stock;
			break;
	}
}
