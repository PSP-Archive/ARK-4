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

#ifndef _MODULEMANAGER_H_
#define _MODULEMANAGER_H_

#include <module2.h>

// Module Start Handler
extern void (* g_module_start_handler)(SceModule2 *);

// Internal Module Manager Apitype Field
extern int * kernel_init_apitype;

// Internal Module Manager Init Filename Field
extern char ** kernel_init_filename;

// Internal Module Manager Init Application Type Field
extern int * kernel_init_application_type;

// sceModuleManager Patch
void patchModuleManager(void);

// Executable Patch #1
int PatchExec1(unsigned char * buffer, int * check);

// Executable Patch #2
int PatchExec2(unsigned char * buffer, int * check);

// Executable Patch #3
int PatchExec3(unsigned char * buffer, int * check, int isplain, int checkresult);

#endif

