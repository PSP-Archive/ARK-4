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

#ifndef _SYSMEM_H_
#define _SYSMEM_H_

#include <module2.h>

// Patch System Memory Manager
void patchSystemMemoryManager(void);

// Patch Game ID Getter
void patchGameInfoGetter(SceModule2 * mod);

// Fixed Game Info Getter Function
void * SysMemForKernel_EF29061C_Fixed(void);

#endif

