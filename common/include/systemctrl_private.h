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

#ifndef SYSTEMCTRL_PRIVATE_H
#define SYSTEMCTRL_PRIVATE_H

#ifdef __cplusplus
extern "C"{
#endif

#include "module2.h"
#include <ark.h>

/*
 * This File contains the required headers for exported, but internally used
 * only functions of systemctrl.prx.
 */

extern ARKConfig* ark_config;

extern int (* DisplaySetFrameBuf)(void*, int, int, int);

// Get PSID hash
int sctrlKernelGetPSIDHash(unsigned char psidHash[16]);

// Initialize printk
int printkInit(const char* filename);

// Log info
int printk(char *fmt, ...)__attribute__((format (printf, 1, 2)));

// Log info in cached buf
int printkCached(char *fmt, ...)__attribute__((format (printf, 1, 2)));

// Output all info remaining in the memory to file
int printkSync(void);

// Lock printk
void printkLock(void);

// Unlock printk
void printkUnlock(void);

// Install Single JAL Trace
void installJALTrace(unsigned int address);

// Install Memory Region JAL Trace
void installMemoryJALTrace(unsigned int start, unsigned int size);

// Install Whole-Module JAL Trace (NOT STABLE! DON'T DO IT IF NOT DESPERATE!)
void installModuleJALTrace(SceModule2 * module);

// Set ARK's execution environment configuration
void sctrlHENSetArkConfig(ARKConfig* conf);

#ifdef __cplusplus
}
#endif

#endif
