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

extern u32 psp_model;
extern int umdvideo_idx;

int menu_draw(void);
int menu_setup(void);
int menu_ctrl(u32 button_on);

#define scePaf_967A56EF_strlen scePaf_strlen
#define scePaf_6439FDBC_memset scePaf_memset
#define scePaf_B6ADE52D_memcmp scePaf_memcmp
#define scePaf_11EFC5FD_sprintf scePaf_sprintf
#define scePaf_15AFC8D3_snprintf scePaf_snprintf
#define scePaf_6BD7452C_memcpy scePaf_memcpy
#define scePaf_98DE3BA6_strcpy scePaf_strcpy

int scePaf_967A56EF_strlen(const char *path);
int scePaf_6439FDBC_memset(void *buff ,int c ,int size);
int scePaf_B6ADE52D_memcmp(const void *path , const void *name , int c);
int scePaf_11EFC5FD_sprintf(char *buffer , const char *format , ...);
int scePaf_15AFC8D3_snprintf(char *buffer,int c , const char *format, ...);
int scePaf_6BD7452C_memcpy(void *path , void *name , int size);
int scePaf_98DE3BA6_strcpy(char *path , const char *name);

int scePaf_strlen_620(const char *path);
int scePaf_memset_620(void *buff ,int c ,int size);
int scePaf_memcmp_620(const void *path , const void *name , int c);
int scePaf_sprintf_620(char *buffer , const char *format , ...);
int scePaf_snprintf_620(char *buffer,int c , const char *format, ...);
int scePaf_memcpy_620(void *path , void *name , int size);
int scePaf_strcpy_620(char *path , const char *name);

int scePaf_strlen_660(const char *path);
int scePaf_memset_660(void *buff ,int c ,int size);
int scePaf_memcmp_660(const void *path , const void *name , int c);
int scePaf_sprintf_660(char *buffer , const char *format , ...);
int scePaf_snprintf_660(char *buffer,int c , const char *format, ...);
int scePaf_memcpy_660(void *path , void *name , int size);
int scePaf_strcpy_660(char *path , const char *name);



extern u32 psp_fw_version;

enum {
    MSG_CUSTOM_LAUNCHER,
    MSG_RECOVERY_MENU,
    MSG_SHUTDOWN_DEVICE,
    MSG_SUSPEND_DEVICE,
    MSG_RESET_DEVICE,
    MSG_RESET_VSH,
    MSG_EXIT,
    MSG_ARK_VSH_MENU,
    MSG_END,
};

extern const char **g_messages;
extern const char *g_messages_en[];
extern int cur_language;

#endif
