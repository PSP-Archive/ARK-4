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
#include "globals.h"
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
    u32 addr = FindTextAddrByName(module);
    
    if (addr) {
        u32 maxaddr = 0x88400000;
        for (; addr < maxaddr; addr += 4) {
            if (strcmp(library, (const char *)addr) == 0) {
                
                u32 libaddr = addr;

                while (*(u32*)(addr -= 4) != libaddr);

                u32 exports = (u32)(*(u16*)(addr + 10) + *(u8*)(addr + 9));
                u32 jump = exports * 4;

                addr = *(u32*)(addr + 12);

                while (exports--) {
                    if (*(u32*)addr == nid){
                        return *(u32*)(addr + jump);
                    }
                    addr += 4;
                }

                return 0;
            }
        }
    }
    return 0;
}
