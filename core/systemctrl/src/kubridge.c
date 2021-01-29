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

#include <pspsdk.h>
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <pspthreadman_kernel.h>
#include <pspsysevent.h>
#include <pspiofilemgr.h>
#include <pspsysmem_kernel.h>
#include <pspinit.h>
#include <stdio.h>
#include <string.h>
#include "imports.h"

// Load Modules (without restrictions)
SceUID kuKernelLoadModule(const char * path, int flags, SceKernelLMOption * option)
{
    // Elevate Permission Level
    unsigned int k1 = pspSdkSetK1(0);
    
    // Load Module
    int result = sceKernelLoadModule(path, flags, option);
    
    // Restore Permission Level
    pspSdkSetK1(k1);
    
    // Return Result
    return result;
}

// Return Apitype
int kuKernelInitApitype(void)
{
    // Elevate Permission Level
    unsigned int k1 = pspSdkSetK1(0);
    
    // Forward Call
    int result = sceKernelInitApitype();
    
    // Restore Permission Level
    pspSdkSetK1(k1);
    
    // Return Result
    return result;
}

// Return Boot Device
int kuKernelBootFrom(void)
{
    // Elevate Permission Level
    unsigned int k1 = pspSdkSetK1(0);
    
    // Forward Call
    int result = sceKernelBootFrom();
    
    // Restore Permission Level
    pspSdkSetK1(k1);
    
    // Return Result
    return result;
}

// Return Module Filename
int kuKernelInitFileName(char * initfilename)
{
    // Elevate Permission Level
    unsigned int k1 = pspSdkSetK1(0);
    
    // Forward Call
    char * string = sceKernelInitFileName();
    
    // Copy String
    strcpy(initfilename, string);
    
    // Restore Permission Level
    pspSdkSetK1(k1);
    
    // Return Success
    return 0;
}

// Return User Level
int kuKernelGetUserLevel(void)
{
    // Elevate Permission Level
    unsigned int k1 = pspSdkSetK1(0);
    
    // Forward Call
    int result = sceKernelGetUserLevel();
    
    // Restore Permission Level
    pspSdkSetK1(k1);
    
    // Return Result
    return result;
}

// Allow Memory Protection Changes from User Mode Application
int kuKernelSetDdrMemoryProtection(void * addr, int size, int prot)
{
    // Elevate Permission Level
    unsigned int k1 = pspSdkSetK1(0);
    
    // Forward Call
    int result = sceKernelSetDdrMemoryProtection(addr, size, prot);
    
    // Restore Permission Level
    pspSdkSetK1(k1);
    
    // Return Result
    return result;
}

// Return Model Number
int kuKernelGetModel(void)
{
    // Elevate Permission Level
    unsigned int k1 = pspSdkSetK1(0);
    
    // Forward Call
    int result = sceKernelGetModel();
    
    // Restore Permission Level
    pspSdkSetK1(k1);
    
    // Return Result
    return result;
}

// Read Dword from Kernel
unsigned int kuKernelPeekw(void * addr)
{
    // Return Dword
    return _lw((unsigned int)addr);
}

// Write Dword into Kernel
void kuKernelPokew(void * addr, unsigned int value)
{
    // Write Dword
    _sw(value, (unsigned int)addr);
}

// Copy Memory Range
void * kuKernelMemcpy(void * dest, const void * src, unsigned int num)
{
    // Elevate Permission Level
    unsigned int k1 = pspSdkSetK1(0);
    
    // Forward Call
    void * address = memcpy(dest, src, num);
    
    // Restore Permission Level
    pspSdkSetK1(k1);
    
    // Return Result
    return address;
}

// Get Key Config (aka. Application Type)
int kuKernelInitKeyConfig(void)
{
    // Elevate Permission Level
    unsigned int k1 = pspSdkSetK1(0);
    
    // Forward Call
    int apptype = sceKernelApplicationType();
    
    // Restore Permission Level
    pspSdkSetK1(k1);
    
    // Return Result
    return apptype;
}

void kuKernelGetUmdFile(char *umdfile, int size)
{
    strncpy(umdfile, GetUmdFile(), size);
}
