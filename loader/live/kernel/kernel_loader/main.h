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

#ifndef EXPLOITMAIN_H
#define EXPLOITMAIN_H

#include <sdk.h>
#include <psploadexec.h>
#include <psploadexec_kernel.h>
#include <psputility_modules.h>
#include <module2.h>
#include <macros.h>
#include <systemctrl_se.h>
#include <string.h>
#include <functions.h>
#include "libs/graphics/graphics.h"
#include "kxploit.h"

#define MAX_SAVE_SIZE 128

ARKConfig* ark_config;

// Function Prototypes
void kernelContentFunction(void);
int LoadReboot(void *, unsigned int, void *, unsigned int);
void setK1Kernel(void);
void clearBSS(void);

#endif
