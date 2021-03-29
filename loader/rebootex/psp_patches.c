#include "rebootex.h"

extern int UnpackBootConfigPatched(char **p_buffer, int length);

int loadcoreModuleStartPSP(void * arg1, void * arg2, void * arg3, int (* start)(void *, void *, void *)){
    loadCoreModuleStartCommon();
    flushCache();
    return start(arg1, arg2, arg3);
}

// patch reboot on psp
void patchRebootBufferPSP(u32 reboot_start, u32 reboot_end){
    // due to cache inconsistency, we must do these patches first and also make sure we read as little ram as possible
    for (u32 addr = (u32)UnpackBootConfig; addr<reboot_end; addr+=4){
        u32 data = _lw(addr);
        if (data == 0x24D90001){  // rebootexcheck5
            u32 a = addr;
            do {a-=4;} while (_lw(a) != NOP);
            _sw(NOP, a-4); // Killing Branch Check bltz ...
            break;
        }
    }
    int patches = 4;
    for (u32 addr = reboot_start; addr<reboot_end && patches; addr+=4){
        u32 data = _lw(addr);
        if (data == 0x02A0E821){ // found loadcore jump on PSP
            _sw(0x00113821, addr-4); // move a3, s1
            _sw(JUMP(loadcoreModuleStartPSP), addr);
            _sw(0x02A0E821, addr + 4);
            patches--;
        }
        else if (data == 0x2C860040){ // kdebug patch
            _sw(0x03E00008, addr-4); // make it return 1
            _sw(0x24020001, addr); // rebootexcheck1
            patches--;
        }
        else if (data == 0x34650001){ // rebootexcheck2
            _sw(NOP, addr-4); // Killing Branch Check bltz ...
            patches--;
        }
        else if (data == 0x00903021 && _lw(addr+4) == 0x00D6282B){ // rebootexcheck3 and rebootexcheck4
            u32 a = addr;
            do {a-=4;} while (_lw(a) != NOP);
            _sw(NOP, a-4); // Killing Branch Check beqz
            _sw(NOP, addr+8); // Killing Branch Check bltz ...
            patches--;
        }
    }
}
