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
#include <psputilsforkernel.h>
#include <stdio.h>
#include <string.h>
#include <macros.h>
#include <ark.h>
#include <systemctrl.h>
#include "nidresolver.h"
#include "imports.h"

// Find Import Library Pointer
SceLibraryStubTable * findImportLib(SceModule2 * pMod, char * library)
{
    // Invalid Arguments
    if(pMod == NULL || library == NULL) return NULL;
    
    // Import Stub Table Start Address
    void * stubTab = pMod->stub_top;
    
    // Iterate Stubs
    int i = 0; while(i < pMod->stub_size)
    {
        // Cast Import Table
        SceLibraryStubTable * pImp = (SceLibraryStubTable *)(stubTab + i);
        
        // Matching Library discovered
        if(pImp->libname != NULL && strcmp(pImp->libname, library) == 0)
        {
            // Return Address
            return pImp;
        }
        
        // Move Pointer
        i += pImp->len * 4;
    }
    
    // Import Library not found
    return NULL;
}

// Find Import Stub Address
unsigned int findImportByNID(SceModule2 * pMod, char * library, unsigned int nid)
{
    // Find Import Library
    SceLibraryStubTable * pImp = findImportLib(pMod, library);
    
    // Found Import Library
    if(pImp != NULL)
    {
        // Iterate Imports
        int i = 0; for(; i < pImp->stubcount; i++)
        {
            // Matching Function NID
            if(pImp->nidtable[i] == nid)
            {
                // Return Function Stub Address
                return (unsigned int)(pImp->stubtable + 8 * i);
            }
        }
    }
    
    // Import Stub not found
    return 0;
}

// Hook Function in Module Import Stubs
// This function autodetects whether Syscalls are used or not...
// Manual exporting in exports.exp is still required however for Syscalls to work.
int hookImportByNID(SceModule2 * pMod, char * library, unsigned int nid, void * func)
{
    // Invalid Arguments
    if(pMod == NULL || library == NULL) return -1;
    
    // Find Module Import Stub
    unsigned int stub = findImportByNID(pMod, library, nid);
    
    // Import Stub not found
    if(stub == 0)
    {
        // Get NID Resolver
        NidResolverLib * resolver = getNidResolverLib(library);
        
        // Found Resolver for Library
        if(resolver != NULL)
        {
            // Resolve NID
            nid = getNidReplacement(resolver, nid);
            
            // Attempt it again...
            stub = findImportByNID(pMod, library, nid);
            
            // Import Stub not found
            if(stub == 0) return -3;
        }
        
        // Resolver Library not found
        else return -2;
    }
    
    // Function as 16-Bit Unsigned Integer
    unsigned int func_int = (unsigned int)func;
    
    // Dummy Return
    if(func_int <= 0xFFFF)
    {
        // Create Dummy Return
        _sw(JR_RA, stub);
        _sw(LI_V0(func_int), stub + 4);
    }
    
    // Normal Hook
    else
    {
        // Syscall Hook
        if ((stub & 0x80000000) == 0 && (func_int & 0x80000000) != 0)
        {
            // Query Syscall Number
            int syscall = sceKernelQuerySystemCall(func);
            
            // Not properly exported in exports.exp
            if(syscall < 0) return -3;
            
            // Create Syscall Hook
            _sw(JR_RA, stub);
            _sw(SYSCALL(syscall), stub + 4);
        }
        
        // Direct Jump Hook
        else
        {
            // Create Direct Jump Hook
            _sw(JUMP(func), stub);
            _sw(NOP, stub + 4);
        }
    }
    
    // Invalidate Cache
    sceKernelDcacheWritebackInvalidateRange((void *)stub, 8);
    sceKernelIcacheInvalidateRange((void *)stub, 8);
    
    // Return Success
    return 0;
}

