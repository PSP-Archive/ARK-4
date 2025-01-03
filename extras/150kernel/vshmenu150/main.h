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

#ifndef MAIN_H
#define MAIN_H

#include <psputility.h>
#include <systemctrl.h>

extern const char ** g_messages;
extern const char * g_messages_en[];

enum {
    TYPE_VSH = 0,
    TYPE_GAME,
    TYPE_POPS,
};

struct MenuEntry {
    int info_idx;
    int type;
    int color;
    int (*display_callback)(struct MenuEntry*, char *, int);
    int (*change_value_callback)(struct MenuEntry *, int);
    int (*enter_callback)(struct MenuEntry *);
    void *arg;
};

struct ValueOption {
    s16 *value;
    int limit_start;
    int limit_end;
};

struct Menu {
    int banner_id;
    struct MenuEntry *submenu;
    int submenu_size;
    int cur_sel;
    int banner_color;
};

#define CUR_SEL_COLOR 0xFF
#define MAX_SCREEN_X 68
#define MAX_SCREEN_Y 33
#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)
#define PIXEL_SIZE (4)
#define FRAME_SIZE (BUF_WIDTH * SCR_HEIGHT * PIXEL_SIZE)
#define CTRL_DELAY   100000
#define CTRL_DEADZONE_DELAY 500000
#define ENTER_DELAY  500000
#define EXIT_DELAY   500000
#define CHANGE_DELAY 500000
#define DRAW_BUF (void*)(0x44000000)
#define DISPLAY_BUF (void*)(0x44000000 + FRAME_SIZE)
#define MAX_MENU_NUMBER_PER_PAGE (MAX_SCREEN_Y-5-2-5)
#define MENU_MIN_BACK_COLOR 0x00
#define MENU_MAX_BACK_COLOR 0x40
#define MENU_BACK_COLOR_HALFTIME 1

#define printf proDebugScreenPrintf

extern int g_ctrl_OK;
extern int g_ctrl_CANCEL;
extern int g_display_flip;
extern SEConfig g_config;

extern int cur_language;

u32 ctrl_read(void);
void ctrl_waitreleasekey(u32 key);
void *get_drawing_buffer(void);
void *get_display_buffer(void);

int limit_int(int value, int direct, int limit);
void set_bottom_info(const char *str, int color);
void frame_start(void);
void frame_end(void);
void menu_loop(struct Menu *menu);

void main_menu(void);

const char* get_cache_policy_name(int policy);
const char* get_language_name(s16 lang);

void suspend_vsh_thread(void);
void resume_vsh_thread(void);

void recovery_exit(void);

int get_registry_value(const char *dir, const char *name, u32 *val);
int set_registry_value(const char *dir, const char *name, u32 val);

void clear_language(void);

#endif
