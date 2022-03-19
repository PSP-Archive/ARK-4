#include "rebootex.h"

extern int UnpackBootConfigPatched(char **p_buffer, int length);

int loadcoreModuleStartPSP(void * arg1, void * arg2, void * arg3, int (* start)(void *, void *, void *)){
    loadCoreModuleStartCommon();
    flushCache();
    return start(arg1, arg2, arg3);
}

// patch reboot on psp
void patchRebootBufferPSP(){

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
    _sw(0x27A40004, UnpackBootConfigArg); // addiu $a0, $sp, 4
    _sw(JAL(UnpackBootConfigPatched), UnpackBootConfigCall); // Hook UnpackBootConfig
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
    // Flush Cache
    flushCache();
}

// IO Patches

//io flags
extern volatile int rebootmodule_open;
extern volatile char *p_rmod;
extern volatile int size_rmod;
extern void* rtm_buf;
extern int rtm_size;

//io functions
extern int (* sceBootLfatOpen)(char * filename);
extern int (* sceBootLfatRead)(char * buffer, int length);
extern int (* sceBootLfatClose)(void);

int _sceBootLfatRead(char * buffer, int length)
{
    //load on reboot module
    if(rebootmodule_open && p_rmod != NULL && size_rmod > 0)
    {
        int min;

        //copy load on reboot module
        min = size_rmod < length ? size_rmod : length;
        if (min > 0){
            memcpy(buffer, p_rmod, min);
            p_rmod += min;
            size_rmod -= min;
        }

        //set filesize
        return min;
    }

    //forward to original function
    return sceBootLfatRead(buffer, length);
}

int _sceBootLfatOpen(char * filename)
{

    //load on reboot module open
    if(strcmp(filename, "/rtm.prx") == 0)
    {
    
        //mark for read
        rebootmodule_open = 1;
        p_rmod = rtm_buf;
        size_rmod = rtm_size;

        //return success
        return 0;
    }

    //forward to original function
    return sceBootLfatOpen(filename);
}

int _sceBootLfatClose(void)
{
    //reboot module close
    if(rebootmodule_open && p_rmod != NULL && size_rmod == 0)
    {
        //mark as closed
        rebootmodule_open = 0;
        p_rmod = NULL;
        size_rmod = 0;

        //return success
        return 0;
    }
    
    //forward to original function
    return sceBootLfatClose();
}

void patchRebootIoPSP(){
    rtm_buf = reboot_conf->rtm_mod.buffer;
    rtm_size = reboot_conf->rtm_mod.size;
    int patches = 3;
    for (u32 addr = reboot_start; addr<reboot_end && patches; addr+=4){
        u32 data = _lw(addr);
        if (data == 0x8E840000){
            sceBootLfatOpen = K_EXTRACT_CALL(addr-4);
            _sw(JAL(_sceBootLfatOpen), addr-4);
            patches--;
        }
        else if (data == 0xAE840004){
            sceBootLfatRead = K_EXTRACT_CALL(addr+4);
            _sw(JAL(_sceBootLfatRead), addr+4);
            patches--;
        }
        else if (data == 0xAE930008){
            sceBootLfatClose = K_EXTRACT_CALL(addr-4);
            _sw(JAL(_sceBootLfatClose), addr-4);
            patches--;
        }
    }
    // Flush Cache
    flushCache();
}
