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

enum {
    PRO_RECOVERY_MENU = 0,
    MAIN_MENU,
    ENTERING,
    EXITING,
    BACK,
    DEFAULT,
    ENABLED,
    DISABLED,
    TOGGLE_USB,
    USB_ENABLED,
    USB_DISABLED,
    CONFIGURATION,
    FAKE_REGION,
    RECOVERY_FONT,
    ISO_MODE,
    NORMAL,
    MARCH33,
    NP9660,
    INFERNO,
    XMB_USBDEVICE,
    FLASH0,
    FLASH1,
    FLASH2,
    FLASH3,
    UMD9660,
    USB_CHARGE,
    SLIM_COLOR,
    HTMLVIEWER_CUSTOM_SAVE_LOCATION,
    HIDE_MAC,
    SKIP_SONY_LOGO,
    SKIP_GAME_BOOT,
    HIDE_PIC,
    FLASH_PROTECT,
    USE_VERSION_TXT,
    USE_USBVERSION_TXT,
    USE_CUSTOM_UPDATE_SERVER,
    PREVENT_HIB_DEL,
    ADVANCED,
    XMB_PLUGIN,
    GAME_PLUGIN,
    POPS_PLUGIN,
    USE_NODRM_ENGINE,
    HIDE_CFW_DIRS,
    BLOCK_ANALOG_INPUT,
    OLD_PLUGINS_SUPPORT,
    ISO_CACHE,
    ISO_CACHE_TOTAL_SIZE,
    ISO_CACHE_NUMBER,
    ISO_CACHE_POLICY,
    ALLOW_NON_LATIN1_ISO_FILENAME,
    MSSPEED_UP,
    NONE,
    POP,
    GAME,
    VSH,
    POP_GAME,
    GAME_VSH,
    VSH_POP,
    ALWAYS,
    CPU_SPEED,
    XMB_CPU_BUS,
    GAME_CPU_BUS,
    PLUGINS,
    SYSTEM_STORAGE,
    MEMORY_STICK,
    PLUGINS_ON_SYSTEM_STORAGE,
    PLUGINS_ON_MEMORY_STICK,
    REGISTERY_HACKS,
    WMA_ACTIVATED,
    FLASH_ACTIVATED,
    BUTTONS_SWAPPED,
    CONFIRM_BUTTON_IS_X,
    CONFIRM_BUTTON_IS_O,
    ACTIVATE_WMA,
    ACTIVATE_FLASH,
    SWAP_BUTTONS,
    SWAP_BUTTONS_FULL,
    DELETE_HIBERNATION,
    HIBERNATION_DELETED,
    RUN_RECOVERY_EBOOT,
    SHUTDOWN_DEVICE,
    SUSPEND_DEVICE,
    RESET_DEVICE,
    RESET_VSH,
    PAGE,
    JAPAN,
    AMERICA,
    EUROPE,
    KOREA,
    UNITED_KINGDOM,
    MEXIQUE,
    AUSTRALIA,
    HONGKONG,
    TAIWAN,
    RUSSIA,
    CHINA,
    DEBUG_TYPE_I,
    DEBUG_TYPE_II,
    RETAIL_HIGH_MEMORY,
    MAC_SPOOFER,
    MSG_END,
};

extern const char ** g_messages;
extern const char * g_messages_en[];

enum {
    TYPE_NORMAL = 0,
    TYPE_SUBMENU = 1,
};

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

#define RECOVERY_EBOOT_PATH "ms0:/PSP/GAME/RECOVERY/EBOOT.PBP"
#define RECOVERY_EBOOT_PATH_EF0 "ef0:/PSP/GAME/RECOVERY/EBOOT.PBP"

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

const char *get_bool_name(int boolean);
const char* get_fake_region_name(int fakeregion);
const char *get_iso_name(int iso_mode);
const char* get_usbdevice_name(int usbdevice);
int get_cpu_number(int cpu);
int get_bus_number(int cpu);
int get_cpu_freq(int number);
int get_bus_freq(int number);
const char *get_plugin_name(int type);
const char* get_cache_policy_name(int policy);
const char* get_language_name(s16 lang);

void suspend_vsh_thread(void);
void resume_vsh_thread(void);

void recovery_exit(void);

int get_registry_value(const char *dir, const char *name, u32 *val);
int set_registry_value(const char *dir, const char *name, u32 val);

int plugins_menu(struct MenuEntry *entry);

int toggle_usb(struct MenuEntry *entry);
void exit_usb(void);

void clear_language(void);

#endif
