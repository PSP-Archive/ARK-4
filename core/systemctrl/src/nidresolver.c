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
#include <stdio.h>
#include <string.h>
#include <macros.h>
#include <module2.h>
#include <systemctrl.h>
#include <systemctrl_private.h>
#include <ark.h>
#include "missingfunc.h"
#include "nidresolver.h"

// Original NID Filler
int (* g_origNIDFiller)(void *lib, unsigned int nid, unsigned int unk2, unsigned int unk3) = NULL;
static int (*sceKernelLinkLibraryEntries)(void *buf, int size) = NULL;
static int (*sceKernelLinkLibraryEntriesForUser)(u32 unk0, void *buf, int size) = NULL;

// NID Table
NidResolverLib * nidTable = NULL;
unsigned int nidTableSize = 0;

// LLE Handler
void (*lle_handler)(void*) = NULL;

// Get NID Resolver Library
NidResolverLib * getNidResolverLib(const char * libName)
{
    // Iterate NID Table
    int i = 0; for(; i < nidTableSize; ++i)
    {
        // Found Matching Library
        if(nidTable[i].enabled && 0 == strcmp(libName, nidTable[i].name))
        {
            // Return Library Reference
            return &nidTable[i];
        }
    }
    
    // Library not found or disabled
    return NULL;
}

// Speedy NID Search
static int binarySearch(const NidResolverEntry * nids, unsigned int n, unsigned int oldNid)
{
    // Lowest, Highest and Middle Position
    int low = 0;
    int high = n - 1;
    int mid = 0;
    
    // Search Loop
    while (low <= high)
    {
        // Calculate Middle
        mid = (low + high) / 2;
        
        // Found Matching NID
        if(oldNid == nids[mid].old)
        {
            // Return Index
            return mid;
        }
        
        // Target has to be "before" this one...
        else if(oldNid < nids[mid].old)
        {
            // Set Maximum Index
            high = mid - 1;
        }
        
        // Target has to be "after" this one...
        else
        {
            // Set Minimum Index
            low = mid + 1;
        }
    }
    
    // NID not found
    return -1;
}

// Resolve NID
unsigned int getNidReplacement(const NidResolverLib *lib, unsigned int nid)
{
    // Find NID Index
    int i = binarySearch(lib->nidtable, lib->nidcount, nid);
    
    // Found NID Index
    if(i >= 0) 
    {
        // Get New NID
        unsigned int newNid = lib->nidtable[i].new;
        
        // Valid NID
        if(newNid != UNKNOWNNID) return newNid;
    }
    
    // Return Original NID
    return nid;
}

NidMissingEntry nidMissEntrySysclib[] =
{
    { 0x89B79CB1, (unsigned int)ownstrcspn,  },
    { 0x62AE052F, (unsigned int)ownstrspn,   },
    { 0x87F8D2DA, (unsigned int)ownstrtok,   },
    { 0x1AB53A58, (unsigned int)ownstrtok_r, },
    { 0xD3D1A3B9, (unsigned int)strncat,     },
    { 0x1D83F344, (unsigned int)ownstrtol,   },
    { 0x909C228B, (unsigned int)&ownsetjmp,  }, // setjmp
    { 0x18FE80DB, (unsigned int)&ownlongjmp, }, // longjmp
};

NidMissingResolver missing_SysclibForKernel =
{
    "SysclibForKernel",
    nidMissEntrySysclib,
    NELEMS(nidMissEntrySysclib),
};

/////////////////////////////////////////////////////////////////////////

NidMissingEntry missing_LoadCoreForKernel_entries[] =
{
    { 0xD8779AC6, 0, },
    { 0x2952F5AC, 0, }, // @sceLoaderCore@ + 0x7298
};

NidMissingResolver missing_LoadCoreForKernel =
{
    "LoadCoreForKernel",
    missing_LoadCoreForKernel_entries,
    NELEMS(missing_LoadCoreForKernel_entries),
};

NidMissingResolver *g_missing_resolver[] =
{
    &missing_SysclibForKernel,
    &missing_LoadCoreForKernel,
};

/////////////////////////////////////////////////////////////////////////

// Missing NID Resolver
unsigned int resolveMissingNid(const char * libName, unsigned int nid)
{
    // Iterate Missing Library Resolver
    int i = 0; for(; i < NELEMS(g_missing_resolver); ++i)
    {
        // Fetch Resolver
        NidMissingResolver * cur = g_missing_resolver[i];
        
        // Matching Library
        if(0 == strcmp(cur->libname, libName))
        {
            // Iterate NIDs
            int j = 0; for(; j < cur->size; ++j)
            {
                // Matching NID
                if(nid == cur->entry[j].nid)
                {
                    // Return Function Pointer
                    return cur->entry[j].fp;
                }
            }
            
            // Stop Searching
            break;
        }
    }
    
    // Unimplemented NID
    return 0;
}

// Fill Library Stubs
int fillLibraryStubs(void * lib, unsigned int nid, void * stub, unsigned int nidPos)
{
    // Result
    int result = 0;
    
    u32 stubtable = _lw((unsigned int)stub + 24);

    // NidResolverEx
    if (lle_handler)
    {
        lle_handler(stubtable);
    }
    
    // Calculate Stub Destination Address
    unsigned int dest = nidPos * 8 + stubtable;
    
    // ???
    if(_lw((unsigned int)stub+52) != 0) goto exit;
    
    // Find Version
    unsigned int * version = (unsigned int *)sctrlHENFindFunction((void *)_lw((unsigned int)stub+36), NULL, 0x11B97506);
    
    // Invalid Version
    if(version != NULL && (*version >> 16) == 0x0606) goto exit;
    
    // Get Module Name
    const char * name = (const char *)_lw((unsigned int)lib + 68);
    
    int is_user_mode = ((u32 *)stub)[0x34/4];
    
    if (!is_user_mode){
        // Resolve Missing NID
        unsigned int targetAddress = resolveMissingNid(name, nid);
        // Missing Function
        if(targetAddress != 0)
        {
            // Write Stub
            _sw(JUMP(targetAddress), dest);
            _sw(NOP, dest + 4);
            
            // Early Exit
            return -1;
        }
    }
    
    // Get Library Resolver
    NidResolverLib * resolver = getNidResolverLib(name);
    
    // Got Library Resolver
    if(resolver != NULL)
    {
        // Resolve NID
        nid = getNidReplacement(resolver, nid);
    }
    
exit:
    // Forward Call
    result = g_origNIDFiller(lib, nid, -1, 0);
    
    // Original NID Filler failed
    if(result < 0)
    {
        // Store Dummy Stub
        _sw(SYSCALL(0x15), dest);
        _sw(NOP, dest + 4);
        
        // Early Exit
        return -1;
    }
    
    // Return Result
    return result;
}

// Create Sorted NID List
static void NidInsertSort(NidResolverEntry * base, int n, int (* cmp)(const NidResolverEntry *, const NidResolverEntry *))
{
    // Temporary NID Resolver Entry for Swapping
    NidResolverEntry saved;
    
    // Iterate NIDs
    int j = 1; for(; j < n; ++j)
    {
        // Previous Index
        int i = j - 1;
        
        // Fetch NID Resolver Entry
        NidResolverEntry * value = &base[j];
        
        // Compare Data
        while(i >= 0 && cmp(&base[i], value) > 0) i--;
        
        // Alread sorted
        if(++i == j) continue;
        
        // Exchange NID Resolver Entries
        memmove(&saved, value, sizeof(saved));
        memmove(&base[i + 1], &base[i], sizeof(saved) * (j - i));
        memmove(&base[i], &saved, sizeof(saved));
    }
}

// NID Compare Algorithm for Binary Sorting
static int NidCompare(const NidResolverEntry * nid_a, const NidResolverEntry * nid_b)
{
    // Smaller
    if(nid_a->old < nid_b->old) return 0;

    // Bigger or Equal
    return 1;
}

// Sorted Table Setup
static void NidSortTable(NidResolverLib *table, unsigned int size)
{
    // Iterate Resolver Libraries
    unsigned int i = 0; for(; i < size; ++i)
    {
        // Insert into Sorted List
        NidInsertSort(table[i].nidtable, table[i].nidcount, &NidCompare);
    }
}

// Setup NID Resolver in sceLoaderCore
void setupNidResolver(SceModule2* mod)
{
    // Link 660 NID Resolver Table
    nidTable = nidTable660;
    nidTableSize = nidTableSize660;
    
    // Binary Sort Table
    NidSortTable(nidTable, nidTableSize);
    
    u32 text_addr = mod->text_addr;
    u32 topaddr = mod->text_addr+mod->text_size;
    int patches = 5;
    for (u32 addr=text_addr; addr<topaddr && patches; addr+=4){
        u32 data = _lw(addr);
        if (data == 0xADA00004 && _lw(addr+8) == 0xADAE0000){
            // Don't write Syscall 0x15 if Resolve failed
            _sw(NOP, addr);
            _sw(NOP, addr+8);
            patches--;
        }
        else if (data == 0x8D450000){
            // Patch NID filler function
            g_origNIDFiller = (void *)K_EXTRACT_CALL(addr + 8);
            _sw(0x02203021, addr + 4);
            _sw(JAL(fillLibraryStubs), addr+8);
            _sw(0x02403821, addr + 12);
            patches--;
            
        }
        else if (data == 0x7CC51240){ // sceKernelIcacheClearAll
            missing_LoadCoreForKernel_entries[0].fp = addr-4;
            patches--;
        }
        else if (data == 0x7CC51180){ // sceKernelDcacheWBinvAll
            missing_LoadCoreForKernel_entries[1].fp = addr-4;
            patches--;
        }
    }
    
    flushCache();
}

