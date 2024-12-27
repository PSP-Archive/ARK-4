#include <stdio.h>
#include <pspsdk.h>
#include <pspiofilemgr.h>
#include <psploadexec.h>
#include <psploadexec_kernel.h>
#include <psputility_modules.h>
#include <pspumd.h>
#include <module2.h>
#include <macros.h>
#include <rebootconfig.h>
#include <systemctrl_se.h>
#include <string.h>
#include <ark.h>
#include "functions.h"

// counter for relocated stubs
static u32 curcall = 0x08801000;

int AddressInRange(u32 addr, u32 lower, u32 higher){
    return (addr >= lower && addr < higher);
}

u32 FindImportRange(char *libname, u32 nid, u32 lower, u32 higher){
    u32 i;
    for(i = lower; i < higher; i += 4) {
        SceLibraryStubTable *stub = (SceLibraryStubTable *)i;

        if((stub->libname != libname) && AddressInRange((u32)stub->libname, lower, higher) \
            && AddressInRange((u32)stub->nidtable, lower, higher) && AddressInRange((u32)stub->stubtable, lower, higher)) {
            if(strcmp(libname, stub->libname) == 0) {
                u32 *table = stub->nidtable;

                int j;
                for(j = 0; j < stub->stubcount; j++) {
                    if(table[j] == nid) {
                        return ((u32)stub->stubtable + (j * 8));
                    }
                }
            }
        }
    }

    return 0;
}

u32 FindImportVolatileRam(char *libname, u32 nid){
    return FindImportRange(libname, nid, 0x08400000, 0x08800000);
}

u32 FindImportUserRam(char *libname, u32 nid){
    return FindImportRange(libname, nid, 0x08800000, 0x0A000000);
}

void *RelocSyscall(u32 call){
    
    if (call != 0) {
        //while (_lw(curcall))
            curcall += 8;

        *(u32*)curcall = *(u32*)call;
        *(u32*)(curcall + 4) = *(u32*)(call + 4);

        return (void *)curcall;
    }

    return 0;
}

void* RelocImport(char* libname, u32 nid, int ram){
    return RelocSyscall((ram)? FindImportVolatileRam(libname, nid) : FindImportUserRam(libname, nid));
}

u32 FindSysMemStruct(){
    u32 kaddr;
    for (kaddr = 0x88000000; kaddr < 0x88400000; kaddr += 4) {
        if (strcmp((const char *)kaddr, "sceSystemMemoryManager") == 0) {
            if (AddressInRange(_lw(kaddr-8), 0x88000000, 0x88400000)
                && _lw(kaddr+0x64) == 0x88000000
                && _lw(kaddr+0x68) ){
                    return kaddr-8;
            }
        }
    }
    return 0;
}

u32 FindModuleByName(const char *modulename){
    u32 mod = FindSysMemStruct();
    while (mod){
        if (strcmp(mod+8, modulename) == 0){
            return mod;
        }
        mod = _lw(mod);
    }
    return 0;
}

u32 FindTextAddrByName(const char *modulename)
{
    u32 mod = FindModuleByName(modulename);

    if (mod)
        return *(u32*)(mod + 0x6C);

    return 0;
}

u32 FindFunction(const char *module, const char *library, u32 nid)
{
    SceModule* mod = (SceModule*)FindModuleByName(module);
    
    if (mod) {
        // Fetch Export Table Start Address
        void * entTab = mod->ent_top;
        
        // Iterate Exports
        for (int i = 0; i < mod->ent_size;)
        {
            // Cast Export Table Entry
            struct SceLibraryEntryTable * entry = (struct SceLibraryEntryTable *)(entTab + i);
            
            // Found Matching Library
            if(entry->libname != NULL && 0 == strcmp(entry->libname, library))
            {
                // Accumulate Function and Variable Exports
                unsigned int total = entry->stubcount + entry->vstubcount;
                
                // NID + Address Table
                unsigned int * vars = entry->entrytable;
                
                // Exports available
                if(total > 0)
                {
                    // Iterate Exports
                    for(int j = 0; j < total; j++)
                    {
                        // Found Matching NID
                        if(vars[j] == nid) return vars[total + j];
                    }
                }
            }
            
            // Move Pointer
            i += (entry->len * 4);
        }
    }
    return 0;
}
