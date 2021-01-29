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

#ifndef _KUBRIDGE_H_
#define _KUBRIDGE_H_

#include <pspsdk.h>
#include <pspkernel.h>
#include <pspsysmem_kernel.h>

// Load Modules (without restrictions)
SceUID kuKernelLoadModule(const char * path, int flags, SceKernelLMOption * option);

// Return Apitype
int kuKernelInitApitype(void);

// Return Boot Device
int kuKernelBootFrom(void);

// Return Module Filename
int kuKernelInitFileName(char * initfilename);

// Return User Level
int kuKernelGetUserLevel(void);

// Allow Memory Protection Changes from User Mode Application
int kuKernelSetDdrMemoryProtection(void * addr, int size, int prot);

// Return Model Number
int kuKernelGetModel(void);

// Read Dword from Kernel
unsigned int kuKernelPeekw(void * addr);

// Write Dword into Kernel
void kuKernelPokew(void * addr, unsigned int value);

// Copy Memory Range
void * kuKernelMemcpy(void * dest, const void * src, unsigned int num);

// Get Key Config (aka. Application Type)
int kuKernelInitKeyConfig(void);

#endif

