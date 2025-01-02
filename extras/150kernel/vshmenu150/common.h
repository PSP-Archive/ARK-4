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

enum {
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
