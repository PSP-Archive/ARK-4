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

#include "systemctrl_se.h"
#include "config.h"

extern u32 psp_model;
extern u32 psp_fw_version;

extern SEConfig conf;

int vshpatch_init(void);
void patch_update_plugin_module(SceModule *mod);
void patch_SceUpdateDL_Library(u32 text_addr);

int vshCtrlDeleteHibernation(void);

#endif
