/*
    6.39 TN-A, XmbControl
    Copyright (C) 2011, Total_Noob
    Copyright (C) 2011, Frostegater

    main.h: XmbControl main header file
    
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __MAIN_H__
#define __MAIN_H__

#define sysconf_console_id 4
#define sysconf_console_action 2
#define sysconf_console_action_arg 2

enum {
    sysconf_tnconfig_action_arg = 0x1000,
    sysconf_plugins_action_arg = 0x1002,
    sysconf_custom_launcher_arg = 0x1003,
    sysconf_custom_app_arg = 0x1004,
    sysconf_150_reboot_arg = 0x1005,
};

typedef struct
{
    int magic;
    int usbcharge;
    int clock_game;
    int clock_vsh;
    int wpa2;
    int launcher;
    int highmem;
    int mscache;
    int infernocache;
    int disablepause;
    int oldplugin;
    int hibblock;
    int skiplogos;
    int hidepics;
    int hidemac;
    int hidedlc;
    int noled;
    int noumd;
    int noanalog;
    int umdregion;
    int vshregion;
    int qaflags;
} CFWConfig;

typedef struct
{
    char text[48];
    int play_sound;
    int action;
    int action_arg;
} SceContextItem;

typedef struct
{
    int id;
    int relocate;
    int action;
    int action_arg;
    SceContextItem *context;
    char *subtitle;
    int unk;
    char play_sound;
    char memstick;
    char umd_icon;
    char image[4];
    char image_shadow[4];
    char image_glow[4];
    char text[0x25];
} SceVshItem;

typedef struct
{
    void *unk;
    int id;
    char *regkey;
    char *text;
    char *subtitle;
    char *page;
} SceSysconfItem;

typedef struct
{
    u8 id;
    u8 type;
    u16 unk1;
    u32 label;
    u32 param;
    u32 first_child;
    int child_count;
    u32 next_entry;
    u32 prev_entry;
    u32 parent;
    u32 unknown[2];
} SceRcoEntry;

int sce_paf_private_wcslen(wchar_t *);
int sce_paf_private_sprintf(char *, const char *, ...);
void *sce_paf_private_memcpy(void *, void *, int);
void *sce_paf_private_memset(void *, char, int);
int sce_paf_private_strlen(char *);
char *sce_paf_private_strcpy(char *, const char *);
char *sce_paf_private_strncpy(char *, const char *, int);
int sce_paf_private_strcmp(const char *, const char *);
int sce_paf_private_strncmp(const char *, const char *, int);
char *sce_paf_private_strchr(const char *, int);
char *sce_paf_private_strrchr(const char *, int);
int sce_paf_private_strpbrk(const char *, const char *);
int sce_paf_private_strtoul(const char *, char **, int);
void *sce_paf_private_malloc(int);
void sce_paf_private_free(void *);

wchar_t *scePafGetText(void *, char *);
int PAF_Resource_GetPageNodeByID(void *, char *, SceRcoEntry **);
int PAF_Resource_ResolveRefWString(void *, u32 *, int *, char **, int *);

int vshGetRegistryValue(u32 *, char *, void *, int , int *);
int vshSetRegistryValue(u32 *, char *, int , int *);

int sceVshCommonGuiBottomDialog(void *a0, void *a1, void *a2, int (* cancel_handler)(), void *t0, void *t1, int (* handler)(), void *t3);

int vctrlVSHExitVshMenu(void *conf);

int vctrlVSHExitVSHMenu(void *conf, char *videoiso, int disctype);

#endif
