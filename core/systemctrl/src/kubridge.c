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
#include <kubridge.h>
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

SceUID kuKernelLoadModuleWithApitype2(int apitype, const char *path, int flags, SceKernelLMOption *option)
{
    u32 k1;
    int ret = -1;

    k1 = pspSdkSetK1(0);
    
    SceUID (*KernelLoadModuleWithApitype2)(int apitype, const char *path, int flags, SceKernelLMOption *option) = NULL;
    KernelLoadModuleWithApitype2 = sctrlHENFindFunction("sceModuleManager", "ModuleMgrForKernel", 0x2B7FC10D);
    
    if (KernelLoadModuleWithApitype2)
        ret = KernelLoadModuleWithApitype2(apitype, path, flags, option);
        
    pspSdkSetK1(k1);

    return ret;
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

int kuKernelInitMode()
{

    // Elevate Permission Level
    unsigned int k1 = pspSdkSetK1(0);

    // Forward Call
	int result = sceKernelInitKeyConfig();
	
	// Restore Permission Level
    pspSdkSetK1(k1);
    
    // Return Result
    return result;
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

int kuKernelFindModuleByName(char *modname, SceModule2 *mod)
{
    SceModule2 *pmod;

    if(modname == NULL || mod == NULL) {
        return -1;
    }

    pmod = (SceModule2*) sceKernelFindModuleByName(modname);

    if(pmod == NULL) {
        return -2;
    }

    memcpy(mod, pmod, sizeof(*pmod));
    
    return 0;
}

int kuKernelCall(void *func_addr, struct KernelCallArg *args)
{
    u32 k1, level;
    u64 ret;
    u64 (*func)(u32, u32, u32, u32, u32, u32, u32, u32, u32, u32, u32, u32);

    if(func_addr == NULL || args == NULL) {
        return -1;
    }

    k1 = pspSdkSetK1(0);
    level = sctrlKernelSetUserLevel(8);
    func = func_addr;
    ret = (*func)(args->arg1, args->arg2, args->arg3, args->arg4, args->arg5, args->arg6, args->arg7, args->arg8, args->arg9, args->arg10, args->arg11, args->arg12);
    args->ret1 = (u32)(ret);
    args->ret2 = (u32)(ret >> 32);
    sctrlKernelSetUserLevel(level);
    pspSdkSetK1(k1);

    return 0;
}

struct KernelCallArgExtendStack {
    struct KernelCallArg args;
    void *func_addr;
};

static int kernel_call_stack(struct KernelCallArgExtendStack *args_stack)
{
    u64 ret;
    struct KernelCallArg *args;
    int (*func)(u32, u32, u32, u32, u32, u32, u32, u32, u32, u32, u32, u32);

    args = &args_stack->args;
    func = args_stack->func_addr;
    ret = (*func)(args->arg1, args->arg2, args->arg3, args->arg4, args->arg5, args->arg6, args->arg7, args->arg8, args->arg9, args->arg10, args->arg11, args->arg12);
    args->ret1 = (u32)(ret);
    args->ret2 = (u32)(ret >> 32);

    return 0;
}

int kuKernelCallExtendStack(void *func_addr, struct KernelCallArg *args, int stack_size)
{
    u32 k1, level;
    int ret;
    struct KernelCallArgExtendStack args_stack;

    if(func_addr == NULL || args == NULL) {
        return -1;
    }

    k1 = pspSdkSetK1(0);
    level = sctrlKernelSetUserLevel(8);
    memcpy(&args_stack.args, args, sizeof(*args));
    args_stack.func_addr = func_addr;
    ret = sceKernelExtendKernelStack(stack_size, (void*)&kernel_call_stack, &args_stack);
    sctrlKernelSetUserLevel(level);
    pspSdkSetK1(k1);

    return ret;
}

void kuKernelGetUmdFile(char *umdfile, int size)
{
    strncpy(umdfile, GetUmdFile(), size);
}

void kuKernelIcacheInvalidateAll(void)
{
    u32 k1 = pspSdkSetK1(0);
    flushCache();
    pspSdkSetK1(k1);
}
