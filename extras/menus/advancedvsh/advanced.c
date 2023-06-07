#include "common.h"
#include <psputility.h>
#include <time.h>

extern int pwidth;
extern char umd_path[72];
extern SEConfig cnf;

extern char device_buf[13];
extern char umdvideo_path[256];

extern xyPoint[];
extern xyPoint2[];

extern ARKConfig* ark_config;
extern int cur_battery;
extern t_conf config;
extern u32 swap_xo;

#define SUBMENU_MAX 15

enum {
	SUBMENU_USB_DEVICE,
	SUBMENU_USB_READONLY,
	SUBMENU_UMD_MODE,
	SUBMENU_UMD_VIDEO,
	SUBMENU_FG_COLORS,
	SUBMENU_BG_COLORS,
	SUBMENU_CONVERT_BATTERY,
	SUBMENU_SWAP_XO_BUTTONS,
	SUBMENU_REGION_MODE,
	SUBMENU_UMD_REGION_MODE,
	SUBMENU_IMPORT_CLASSIC_PLUGINS,
	SUBMENU_ACTIVATE_FLASH_WMA,
	SUBMENU_DELETE_HIBERNATION,
	SUBMENU_RANDOM_GAME,
	SUBMENU_GO_BACK,
};

int item_fcolor[SUBMENU_MAX];
const char *subitem_str[SUBMENU_MAX];

static int submenu_sel = SUBMENU_USB_DEVICE;

int submenu_draw(void)
{
	u32 fc,bc;
	const char *msg;
	int submax_menu, subcur_menu;
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
	blit_string(pointer[0], pointer[1], g_messages[MSG_ADVANCED_VSH]);
	blit_string(pointer[0], 56, ark_version);
	fc = 0xffffff;
 
	for(submax_menu=0;submax_menu<SUBMENU_MAX;submax_menu++) {
		msg = g_messages[MSG_USB_DEVICE + submax_menu];
		switch(config.vsh_bg_color) {
						// Random
						case 0:
						// Red
						case 1: 
							bc = (submax_menu==submenu_sel) ? 0xff8080 : 0x000000ff;
							blit_set_color(fc,bc);
							break;
						// Light Red
						case 2: 
							bc = (submax_menu==submenu_sel) ? 0x0000ff : 0xa00000ff;
							blit_set_color(fc,bc);
							break;
						// Orange
						case 3: 
							bc = (submax_menu==submenu_sel) ? 0x0000ff : 0x0000a5ff;
							blit_set_color(fc,bc);
							break;
						// Light Orange
						case 4: 
							bc = (submax_menu==submenu_sel) ? 0x0000ff : 0xa000a5ff;
							blit_set_color(fc,bc);
							break;
						// Yellow
						case 5: 
							bc = (submax_menu==submenu_sel) ? 0x0000ff : 0x0000e6e6;
							blit_set_color(fc,bc);
							break;
						// Light Yellow
						case 6: 
							bc = (submax_menu==submenu_sel) ? 0x0000ff : 0xa000e6e6;
							blit_set_color(fc,bc);
							break;
						// Green
						case 7:
							bc = (submax_menu==submenu_sel) ? 0x0000ff : 0x0000ff00;
							blit_set_color(fc,bc);
							break;
						// Light Green
						case 8:
							bc = (submax_menu==submenu_sel) ? 0x0000ff : 0xa000b300;
							blit_set_color(fc,bc);
							break;
						// Blue
						case 9:
							bc = (submax_menu==submenu_sel) ? 0x0000ff : 0x00ff0000;
							blit_set_color(fc,bc);
							break;
						// Light Blue
						case 10:
							bc = (submax_menu==submenu_sel) ? 0x0000ff : 0xa0ff0000;
							blit_set_color(fc,bc);
							break;
						// Indigo
						case 11:
							bc = (submax_menu==submenu_sel) ? 0x0000ff : 0x0082004b;
							blit_set_color(fc,bc);
							break;
						// Light Indigo
						case 12:
							bc = (submax_menu==submenu_sel) ? 0x0000ff : 0xa082004b;
							blit_set_color(fc,bc);
							break;
						// Violet
						case 13:
							bc = (submax_menu==submenu_sel) ? 0x0000ff : 0x00ee82ee;
							blit_set_color(fc,bc);
							break;
						// Light Violet
						case 14:
							bc = (submax_menu==submenu_sel) ? 0x0000ff : 0xa0ee82ee;
							blit_set_color(fc,bc);
							break;
						// Pink 
						case 15:
							bc = (submax_menu==submenu_sel) ? 0x0000ff : 0x00cbc0ff;
							blit_set_color(fc,bc);
							break;
						// Light Pink 
						case 16:
							bc = (submax_menu==submenu_sel) ? 0x0000ff : 0xa0cbc0ff;
							blit_set_color(fc,bc);
							break;
						// Purple 
						case 17:
							bc = (submax_menu==submenu_sel) ? 0x0000ff : 0x00993366;
							blit_set_color(fc,bc);
							break;
						// Light Purple 
						case 18:
							bc = (submax_menu==submenu_sel) ? 0x0000ff : 0xa0993366;
							blit_set_color(fc,bc);
							break;
						// Teal 
						case 19:
							bc = (submax_menu==submenu_sel) ? 0x0000ff : 0x00808000;
							blit_set_color(fc,bc);
							break;
						// Light Teal 
						case 20:
							bc = (submax_menu==submenu_sel) ? 0x0000ff : 0xa0808000;
							blit_set_color(fc,bc);
							break;
						// Aqua 
						case 21:
							bc = (submax_menu==submenu_sel) ? 0x0000ff : 0x00cccc00;
							blit_set_color(fc,bc);
							break;
						// Light Aqua 
						case 22:
							bc = (submax_menu==submenu_sel) ? 0x0000ff : 0xa0cccc00;
							blit_set_color(fc,bc);
							break;
						// Grey 
						case 23:
							bc = (submax_menu==submenu_sel) ? 0x0000ff : 0x00737373;
							blit_set_color(fc,bc);
							break;
						// Light Grey 
						case 24:
							bc = (submax_menu==submenu_sel) ? 0x0000ff : 0xa0737373;
							blit_set_color(fc,bc);
							break;
						// Black 
						case 25:
							bc = (submax_menu==submenu_sel) ? 0x0000ff : 0x00000000;
							blit_set_color(fc,bc);
							break;
						// Light Black 
						case 26:
							bc = (submax_menu==submenu_sel) ? 0x0000ff : 0xa0000000;
							blit_set_color(fc,bc);
							break;
						// White  
						case 27:
							bc = (submax_menu==submenu_sel) ? 0x0000ff : 0x00ffffff;
							blit_set_color(fc,bc);
							break;
						// Light White  
						case 28:
							bc = (submax_menu==submenu_sel) ? 0x0000ff : 0xafffffff;
							blit_set_color(fc,bc);
							break;
						default:	
							bc = (submax_menu==submenu_sel) ? 0xff8080 : 0x0000a5ff;
							blit_set_color(fc,bc);
					}

					switch(config.vsh_fg_color) {
						// Random
						case 0:
						// White  
						case 1:
							fc = (submax_menu==submenu_sel) ? 0xffffff : 0x00ffffff;
							blit_set_color(fc,bc);
							break;
						// Orange
						case 2: 
							fc = (submax_menu==submenu_sel) ? 0xffffff : 0x0000a5ff;
							blit_set_color(fc,bc);
							break;
						// Light Orange
						case 3: 
							fc = (submax_menu==submenu_sel) ? 0xffffff : 0xa000a5ff;
							blit_set_color(fc,bc);
							break;
						// Yellow
						case 4: 
							fc = (submax_menu==submenu_sel) ? 0xffffff : 0x0000e6e6;
							blit_set_color(fc,bc);
							break;
						// Light Yellow
						case 5: 
							fc = (submax_menu==submenu_sel) ? 0xffffff : 0xa000e6e6;
							blit_set_color(fc,bc);
							break;
						// Green
						case 6:
							fc = (submax_menu==submenu_sel) ? 0xffffff : 0x0000b300;
							blit_set_color(fc,bc);
							break;
						// Light Green
						case 7:
							fc = (submax_menu==submenu_sel) ? 0xffffff : 0xa000ff00;
							blit_set_color(fc,bc);
							break;
						// Blue
						case 8:
							fc = (submax_menu==submenu_sel) ? 0xffffff : 0x00ff0000;
							blit_set_color(fc,bc);
							break;
						// Light Blue
						case 9:
							fc = (submax_menu==submenu_sel) ? 0xffffff : 0xa0ff0000;
							blit_set_color(fc,bc);
							break;
						// Indigo
						case 10:
							fc = (submax_menu==submenu_sel) ? 0xffffff : 0x0082004b;
							blit_set_color(fc,bc);
							break;
						// Light Indigo
						case 11:
							fc = (submax_menu==submenu_sel) ? 0xffffff : 0xa082004b;
							blit_set_color(fc,bc);
							break;
						// Violet
						case 12:
							fc = (submax_menu==submenu_sel) ? 0xffffff : 0x00ee82ee;
							blit_set_color(fc,bc);
							break;
						// Light Violet
						case 13:
							fc = (submax_menu==submenu_sel) ? 0xffffff : 0xa0ee82ee;
							blit_set_color(fc,bc);
							break;
						// Pink 
						case 14:
							fc = (submax_menu==submenu_sel) ? 0xffffff : 0x00cbc0ff;
							blit_set_color(fc,bc);
							break;
						// Light Pink 
						case 15:
							fc = (submax_menu==submenu_sel) ? 0xffffff : 0xa0cbc0ff;
							blit_set_color(fc,bc);
							break;
						// Purple 
						case 16:
							fc = (submax_menu==submenu_sel) ? 0xffffff : 0x00993366;
							blit_set_color(fc,bc);
							break;
						// Light Purple 
						case 17:
							fc = (submax_menu==submenu_sel) ? 0xffffff : 0xa0993366;
							blit_set_color(fc,bc);
							break;
						// Teal 
						case 18:
							fc = (submax_menu==submenu_sel) ? 0xffffff : 0x00808000;
							blit_set_color(fc,bc);
							break;
						// Light Teal 
						case 19:
							fc = (submax_menu==submenu_sel) ? 0xffffff : 0xa0808000;
							blit_set_color(fc,bc);
							break;
						// Aqua 
						case 20:
							fc = (submax_menu==submenu_sel) ? 0xffffff : 0x00cccc00;
							blit_set_color(fc,bc);
							break;
						// Light Aqua 
						case 21:
							fc = (submax_menu==submenu_sel) ? 0xffffff : 0xa0cccc00;
							blit_set_color(fc,bc);
							break;
						// Grey 
						case 22:
							fc = (submax_menu==submenu_sel) ? 0xffffff : 0x00737373;
							blit_set_color(fc,bc);
							break;
						// Light Grey 
						case 23:
							fc = (submax_menu==submenu_sel) ? 0xffffff : 0xa0737373;
							blit_set_color(fc,bc);
							break;
						// Black 
						case 24:
							fc = (submax_menu==submenu_sel) ? 0xffffff : 0x00000000;
							blit_set_color(fc,bc);
							break;
						// Light Black 
						case 25:
							fc = (submax_menu==submenu_sel) ? 0xffffff : 0xa0000000;
							blit_set_color(fc,bc);
							break;
						// Light Red
						case 26: 
							fc = (submax_menu==submenu_sel) ? 0xffffff : 0xa00000ff;
							blit_set_color(fc,bc);
							break;
						// Red
						case 27:
							fc = (submax_menu==submenu_sel) ? 0xffffff : 0x000000ff;
							blit_set_color(fc,bc);
							break;
						// Light White  
						case 28:
							fc = (submax_menu==submenu_sel) ? 0xffffff : 0xafffffff;
							blit_set_color(fc,bc);
							break;
						default:	
							fc = (submax_menu==submenu_sel) ? 0xffffff : 0x0000a5ff;
							blit_set_color(fc,bc);
					}

		if(msg) {
			switch(submax_menu) {
				case SUBMENU_GO_BACK:
					xPointer = pointer[2];
				break;
				case SUBMENU_RANDOM_GAME:
					xPointer = 178;
					break;
				case SUBMENU_DELETE_HIBERNATION:
					if (psp_model == PSP_GO)
						xPointer = 168;
					else{
						msg = " NO HIBERNATION SUPPORT ";
						xPointer = 153;
					}
					break;
				case SUBMENU_IMPORT_CLASSIC_PLUGINS:
				case SUBMENU_ACTIVATE_FLASH_WMA:
					xPointer = 153;
					break;
				default:
					xPointer=pointer[4];
					break;
			}



			subcur_menu = submax_menu;
			blit_string(xPointer, (pointer[5] + subcur_menu)*8, msg);
			msg = subitem_str[submax_menu];

			if (submax_menu == SUBMENU_UMD_REGION_MODE) {
				if(psp_model == PSP_GO || IS_VITA_ADR(ark_config)) msg = "Unsupported";
				blit_string( (pointer[6] * 8) + 128, (pointer[5] + subcur_menu)*8, msg);
			}
			else if(msg) {
				blit_string( (pointer[6] * 8) + 128, (pointer[5] + subcur_menu)*8, msg);
			}

		}

	}

	blit_set_color(0x00ffffff,0x00000000);

	return 0;
}


int submenu_setup(void)
{
	int i;
	const char *bridge;
	const char *umdvideo_disp;

	// preset
	for(i=0;i<SUBMENU_MAX;i++) {
		subitem_str[i] = NULL;
		item_fcolor[i] = RGB(255,255,255);
	}
	


//usb device
	if((cnf.usbdevice>0) && (cnf.usbdevice<5)) {
		scePaf_sprintf(device_buf, "%s %d", g_messages[MSG_FLASH], cnf.usbdevice-1);
		bridge = device_buf;
	} else if (IS_VITA_ADR(ark_config)) {
		scePaf_sprintf(device_buf, "%s", g_messages[MSG_USE_ADRENALINE_SETTINGS]);
		bridge = device_buf;
	}else {
		const char *device;

		if(cnf.usbdevice==5)
			device= g_messages[MSG_UMD_DISC];
		else if(psp_model == PSP_GO)
			device = g_messages[MSG_INTERNAL_STORAGE];
		else
			device = g_messages[MSG_MEMORY_STICK];

		bridge = device;
	}

	umdvideo_disp = strrchr(umdvideo_path, '/');

	if(umdvideo_disp == NULL) {
		umdvideo_disp = umdvideo_path;
	} else {
		umdvideo_disp++;
	}

	if(IS_VITA_ADR(ark_config))
		subitem_str[SUBMENU_UMD_VIDEO] = g_messages[MSG_UNSUPPORTED];
	else
		subitem_str[SUBMENU_UMD_VIDEO] = umdvideo_disp;
	subitem_str[SUBMENU_USB_DEVICE] = bridge;

/*
	switch(cnf.umdmode) {
		case MODE_NP9660:
			subitem_str[SUBMENU_UMD_MODE] = g_messages[MSG_NP9660];
			break;
		case MODE_INFERNO:
			subitem_str[SUBMENU_UMD_MODE] = g_messages[MSG_INFERNO];
			break;
	}
*/
	if(cnf.umdmode == 3) {
		subitem_str[SUBMENU_UMD_MODE] = g_messages[MSG_INFERNO];
	} else if(cnf.umdmode<2) {
		cnf.umdmode = 3;
	} else if(cnf.umdmode>3) {
		cnf.umdmode = 2;
	} else {
		subitem_str[SUBMENU_UMD_MODE] = g_messages[MSG_NP9660];
	}





	switch(cnf.usbdevice_rdonly) {
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

	switch(swap_xo) {
		case XO_CURRENT_O_PRIMARY:
			subitem_str[SUBMENU_SWAP_XO_BUTTONS] = g_messages[MSG_O_PRIM];
			break;
		case XO_CURRENT_X_PRIMARY:
			subitem_str[SUBMENU_SWAP_XO_BUTTONS] = g_messages[MSG_X_PRIM];
			break;
		default:
			subitem_str[SUBMENU_SWAP_XO_BUTTONS] = g_messages[MSG_X_PRIM]; // should never happen?
	}

	switch(cur_battery) {
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

	switch(cnf.vshregion) {
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

	switch(cnf.umdregion) {
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
	

	switch(config.vsh_fg_color) {
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
	switch(config.vsh_bg_color) {
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


int submenu_ctrl(u32 button_on)
{
	int direction;

	if( (button_on & PSP_CTRL_SELECT) ||
		(button_on & PSP_CTRL_HOME)) {
		submenu_sel = SUBMENU_GO_BACK;
		return 1;
	}

	// change menu select
	direction = 0;

	if(button_on & PSP_CTRL_DOWN) direction++;
	if(button_on & PSP_CTRL_UP) direction--;

	submenu_sel = limit(submenu_sel+direction, 0, SUBMENU_MAX-1);

	// LEFT & RIGHT
	direction = -2;

	if(button_on & PSP_CTRL_LEFT)   direction = -1;
	if(button_on & PSP_CTRL_CROSS) direction = 0;
	if(button_on & PSP_CTRL_CIRCLE) direction = 0;
	if(button_on & PSP_CTRL_RIGHT)  direction = 1;

	if(direction <= -2)
		return 0;

	switch(submenu_sel) {
		case SUBMENU_USB_DEVICE:
			if (IS_VITA_ADR(ark_config)) break;
			if(direction) change_usb( direction );
			break;
		case SUBMENU_USB_READONLY:
			if (IS_VITA_ADR(ark_config)) break;
			if (direction) swap_readonly(direction);
			break;
		case SUBMENU_UMD_MODE:
			if(direction) change_umd_mode( direction );
			break;
		case SUBMENU_UMD_VIDEO:
			if (IS_VITA_ADR(ark_config)) break;
			if(direction) {
			   	change_umd_mount_idx(direction);

				if(umdvideo_idx != 0) {
					char *umdpath;

					umdpath = umdvideolist_get(&g_umdlist, umdvideo_idx-1);

					if(umdpath != NULL) {
						strncpy(umdvideo_path, umdpath, sizeof(umdvideo_path));
						umdvideo_path[sizeof(umdvideo_path)-1] = '\0';
					}else {
						goto none;
					}
				} else {
none:
					strcpy(umdvideo_path, g_messages[MSG_NONE]);
				}
			} else {
				return 6; // Mount UMDVideo ISO flag
			}
			break;
		case SUBMENU_IMPORT_CLASSIC_PLUGINS:
			if(direction==0) {
				return 13; // Import Classic Plugins flag 
			}
			break;
		case SUBMENU_DELETE_HIBERNATION:
			if(direction==0) {
				return 10; // Delete Hibernation flag 
			}
			break;
		case SUBMENU_RANDOM_GAME:
			if(direction==0) {
				return 14; // Random Game flag 
			}
			break;
		case SUBMENU_ACTIVATE_FLASH_WMA:
			if(direction==0) {
				return 11; // Activate Flash/WMA flag 
			}
			break;
		case SUBMENU_REGION_MODE:
			if(direction) change_region ( direction, 13 );
			break;
		case SUBMENU_UMD_REGION_MODE:
			if(psp_model == PSP_GO || IS_VITA_ADR(ark_config)) break;
			if(direction) change_umd_region ( direction, 3 );
			break;
		case SUBMENU_SWAP_XO_BUTTONS:
			if (direction==0) {
				return 12; // Swap X/O Buttons flag  
			} 
			break;
		case SUBMENU_CONVERT_BATTERY:
			if(direction==0) {
				return 9; // Convert Battery flag
			}
			break;
		case SUBMENU_FG_COLORS:
			// This will be where I will be adding to set the color
			if(direction) change_fg_color(direction);
			break;
		case SUBMENU_BG_COLORS:
			// This will be where I will be adding to set the color
			if(direction) change_bg_color(direction);
			break;
		case SUBMENU_GO_BACK:
			if(direction==0) return 1; // finish
			break;
	}

	return 0; // continue
}
