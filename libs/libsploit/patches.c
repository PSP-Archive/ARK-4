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

u32 _findJAL(u32 addr, int reversed, int skip){
    if (addr != 0)
    {
        int add = 4;
        if (reversed)
            add = -4;
        for(;;addr+=add) {
            if ((_lw(addr) >= 0x0C000000) && (_lw(addr) < 0x10000000)){
                if (skip == 0)
                    return (((_lw(addr) & 0x03FFFFFF) << 2) | 0x80000000);
                else
                    skip--;
            }
        }
    }

    return 0;
}

u32 FindFirstBEQ(u32 addr){
    for (;;addr+=4){
        if ((_lw(addr) & 0xFC000000) == 0x10000000)
            return addr;
    }
    return 0;
}

u32 findRefInGlobals(char* libname, u32 addr, u32 ptr){
    while (strcmp(libname, (char*)addr))
        addr++;
    
    if (addr%4)
        addr &= -0x4; // align to 4 bytes

    while (_lw(addr += 4) != ptr);
    
    return addr;
}

#ifdef DEBUG
void AccurateError(u32 text_addr, u32 text_size, u16 error)
{
    u32 counter = 0;
    u32 text_end = text_addr+text_size;

    for (; text_addr < text_end; text_addr += 4)
    {
        u32 code = _lw(text_addr);

        if ((code & 0xFC00FFFF) == (0x34000000|error))
        {
            counter++;
            _sw((code & 0xFFFF0000) | (0xA000 + counter), text_addr);
        }
    }
}
#endif
