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

#ifndef __COMMON_H
#define __COMMON_H


enum {
	MSG_DEFAULT = 0,
	MSG_JAPAN,
	MSG_AMERICA,
	MSG_EUROPE,
	MSG_KOREA,
	MSG_UNITED_KINGDOM,
	MSG_LATIN_AMERICA,
	MSG_AUSTRALIA,
	MSG_HONG_KONG,
	MSG_TAIWAN,
	MSG_RUSSIA,
	MSG_CHINA,
	MSG_DEBUG_I,
	MSG_DEBUG_II,
	MSG_DISABLE,
	MSG_ENABLE,
	MSG_NORMAL,
	MSG_TOGGLE,
	MSG_X_PRIM,
	MSG_O_PRIM,
	MSG_USB_DEVICE,
	MSG_USB_READONLY,
	MSG_ISO_VIDEO_MOUNT,
	MSG_FG_COLORS,
	MSG_BG_COLORS,
	MSG_FONT,
	MSG_MENU_DESIGN,
	MSG_MAIN_MENU,
	MSG_CONVERT_BATTERY,
	MSG_CHANGE_REGION,
	MSG_CHANGE_UMD_REGION,
	MSG_SWAP_XO_BUTTONS,
	MSG_IMPORT_CLASSIC_PLUGINS,
	MSG_ACTIVATE_FLASH_WMA,
	MSG_DELETE_HIBERNATION,
	MSG_RANDOM_GAME,
	MSG_GO_BACK,
	MSG_NO_HIBERNATION,
	MSG_NORMAL_TO_PANDORA,
	MSG_PANDORA_TO_NORMAL,
	MSG_ARK_VSH_MENU,
	MSG_ADVANCED_VSH,
	MSG_RECOVERY_MENU,
	MSG_SHUTDOWN_DEVICE,
	MSG_SUSPEND_DEVICE,
	MSG_RESET_DEVICE,
	MSG_RESET_VSH,
	MSG_EXIT,
	MSG_UNSUPPORTED,
	MSG_USE_ADRENALINE_SETTINGS,
	MSG_FLASH,
	MSG_UMD_DISC,
	MSG_INTERNAL_STORAGE,
	MSG_MEMORY_STICK,
	MSG_NONE,
	MSG_CLASSIC,
	MSG_SIMPLE,
	MSG_ADVANCED,
	MSG_NEW,
	MSG_RANDOM,
	MSG_RED,
	MSG_LITE_RED,
	MSG_ORANGE,
	MSG_LITE_ORANGE,
	MSG_YELLOW,
	MSG_LITE_YELLOW,
	MSG_GREEN,
	MSG_LITE_GREEN,
	MSG_BLUE,
	MSG_LITE_BLUE,
	MSG_INDIGO,
	MSG_LITE_INDIGO,
	MSG_VIOLET,
	MSG_LITE_VIOLET,
	MSG_PINK,
	MSG_LITE_PINK,
	MSG_PURPLE,
	MSG_LITE_PURPLE,
	MSG_TEAL,
	MSG_LITE_TEAL,
	MSG_AQUA,
	MSG_LITE_AQUA,
	MSG_GREY,
	MSG_LITE_GREY,
	MSG_BLACK,
	MSG_LITE_BLACK,
	MSG_WHITE,
	MSG_LITE_WHITE,
	MSG_END
};

extern const char **g_messages;
extern const char *g_messages_en[];
extern int cur_language;

#endif
