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

#include "module2.h"

/*
 * This File contains the required headers for exported, but internally used
 * only functions of systemctrl.prx.
 */

// Initialize Kernel Heap
int oe_mallocinit(void);

// Allocate Memory for Kernel Heap
void * oe_malloc(unsigned int size);

// Return Memory to Kernel Heap
void oe_free(void * p);

// Terminate Kernel Heap
int oe_mallocterminate(void);

// Get PSID hash
int sctrlKernelGetPSIDHash(unsigned char psidHash[16]);

#ifdef DEBUG
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
#else
#define printkInit(...)
#define printk(...)
#define printkCached(...)
#define printkSync()
#define printkLock()
#define printkUnlock()
#define installJALTrace(...)
#define installMemoryJALTrace(...)
#define installModuleJALTrace(...)
#endif

#endif
