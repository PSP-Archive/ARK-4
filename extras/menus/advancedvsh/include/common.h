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

#include <pspkernel.h>
#include <pspctrl.h>
#include <pspdisplay.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "systemctrl_se.h"
#include "ui.h"
#include "blit.h"
#include "globals.h"
#include "macros.h"

#include "../arkMenu/include/conf.h"

#if !defined(CONFIG_635) && !defined(CONFIG_620) && !defined(CONFIG_639) && !defined(CONFIG_660) && !defined(CONFIG_661)
#error You have to define CONFIG_620 or CONFIG_635 or CONFIG_639 or CONFIG_660 or CONFIG_661
#endif

extern u32 psp_model;
extern int umdvideo_idx;

int menu_draw(void);
int submenu_draw(void);
int menu_setup(void);
int submenu_setup(void);
int menu_ctrl(u32 button_on);
int submenu_ctrl(u32 button_on);

int cpu2no(int cpu);
int bus2no(int cpu);
void change_clock(int dir , int flag);
void change_usb(int dir );
void change_colors(int dir );
void change_umd_mode(int dir );
void change_umd_mount_idx(int dir);
void change_plugins(int dir , int flag);
void change_bool_option(int *p, int direction);
void change_region(int dir, int max);

int scePaf_strlen(const char *path);
int scePaf_memset(void *buff ,int c ,int size);
int scePaf_memcmp(const void *path , const void *name , int c);
int scePaf_sprintf(char *buffer , const char *format , ...);
int scePaf_snprintf(char *buffer,int c , const char *format, ...);
int scePaf_memcpy(void *path , void *name , int size);
int scePaf_strcpy(char *path , const char *name);

typedef struct _UmdVideoEntry {
	char *path;
	struct _UmdVideoEntry *next;
} UmdVideoEntry;

typedef struct _UmdVideoList {
	UmdVideoEntry head, *tail;
	size_t count;
} UmdVideoList;

int umdvideolist_add(UmdVideoList *list, const char *path);
char *umdvideolist_get(UmdVideoList *list, size_t n);
size_t umdvideolist_count(UmdVideoList *list);
void umdvideolist_clear(UmdVideoList *list);
int umdvideolist_find(UmdVideoList *list, const char *search);
void umdvideolist_init(UmdVideoList *list);

extern u32 psp_fw_version;
extern UmdVideoList g_umdlist;

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
	MSG_NP9660,
	MSG_INFERNO,
	MSG_USB_DEVICE,
	MSG_USB_READONLY,
	MSG_UMD_ISO_MODE,
	MSG_ISO_VIDEO_MOUNT,
	MSG_FG_COLORS,
	MSG_BG_COLORS,
	MSG_CONVERT_BATTERY,
	MSG_CHANGE_REGION,
	MSG_CHANGE_UMD_REGION,
	MSG_SWAP_XO_BUTTONS,
	MSG_IMPORT_CLASSIC_PLUGINS,
	MSG_ACTIVATE_FLASH_WMA,
	MSG_DELETE_HIBERNATION,
	MSG_RANDOM_GAME,
	MSG_GO_BACK,
	MSG_NORMAL_TO_PANDORA,
	MSG_PANDORA_TO_NORMAL,
	MSG_CUSTOM_LAUNCHER,
	MSG_RECOVERY_MENU,
	MSG_ADVANCED_VSH,
	MSG_SHUTDOWN_DEVICE,
	MSG_SUSPEND_DEVICE,
	MSG_RESET_DEVICE,
	MSG_RESET_VSH,
	MSG_EXIT,
	MSG_ARK_VSH_MENU,
	MSG_UNSUPPORTED,
	MSG_USE_ADRENALINE_SETTINGS,
	MSG_FLASH,
	MSG_UMD_DISC,
	MSG_INTERNAL_STORAGE,
	MSG_MEMORY_STICK,
	MSG_NONE,
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
	MSG_END,
};

extern const char **g_messages;
extern const char *g_messages_en[];
extern int cur_language;

#endif
