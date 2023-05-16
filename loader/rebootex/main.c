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

#include "rebootex.h"

#ifdef REBOOTEX
#define END_BUF_STR "ApplyPspRelSection"
#ifdef MS_IPL
#include "syscon.h"
#endif
#else
#define END_BUF_STR "StopBoot"
#define SYSCON_CTRL_RTRG 0x00000400
#define SYSCON_CTRL_HOME 0x00001000

ARKConfig _arkconf = {
    .magic = ARK_CONFIG_MAGIC,
#ifndef MS_IPL
    .arkpath = "ms0:/PSP/SAVEDATA/ARK_01234/", // default path for ARK files
    .exploit_id = "cIPL",
#else
    .arkpath = ARK_DC_PATH "/ARK_01234/", // default path for ARK files
    .exploit_id = "DC",
#endif
    .launcher = {0},
    .exec_mode = PSP_ORIG, // run ARK in PSP mode
    .recovery = 0,
};
#endif

RebootConfigARK* reboot_conf = (RebootConfigARK*)REBOOTEX_CONFIG;
ARKConfig* ark_config = (ARKConfig*)ARK_CONFIG;

// sceReboot Main Function
int (* sceReboot)(int, int, int, int, int, int, int) = (void *)(REBOOT_TEXT);

// Instruction Cache Invalidator
void (* sceRebootIcacheInvalidateAll)(void) = NULL;

// Data Cache Invalidator
void (* sceRebootDacheWritebackInvalidateAll)(void) = NULL;

// Sony PRX Decrypter Function Pointer
int (* SonyPRXDecrypt)(void *, unsigned int, unsigned int *) = NULL;
int (* origCheckExecFile)(unsigned char * addr, void * arg2) = NULL;

// UnpackBootConfig
int (* UnpackBootConfig)(char * buffer, int length) = NULL;
extern int UnpackBootConfigPatched(char **p_buffer, int length);
u32 UnpackBootConfigCall = 0;
u32 UnpackBootConfigArg = 0;
u32 reboot_start = 0;
u32 reboot_end = 0;

//io flags
int rebootmodule_set = 0;
int rebootmodule_open = 0;
char *p_rmod = NULL;
int size_rmod = 0;
void* rtm_buf = NULL;
int rtm_size = 0;

//io functions
int (* sceBootLfatOpen)(char * filename) = NULL;
int (* sceBootLfatRead)(char * buffer, int length) = NULL;
int (* sceBootLfatClose)(void) = NULL;

// PRO GZIP Decrypt Support
int PROPRXDecrypt(void * prx, unsigned int size, unsigned int * newsize)
{
    // GZIP Packed PRX File
    if ( (_lb((unsigned)prx + 0x150) == 0x1F && _lb((unsigned)prx + 0x151) == 0x8B)
            || (*(unsigned int *)(prx + 0x130) == 0xC01DB15D) )
    {
        // Read GZIP Size
        unsigned int compsize = *(unsigned int *)(prx + 0xB0);
        
        // Return GZIP Size
        *newsize = compsize;
        
        // Remove PRX Header
        memcpy(prx, prx + 0x150, compsize);
        
        // Fake Decrypt Success
        return 0;
    }
    
    // Decrypt Sony PRX Files
    return SonyPRXDecrypt(prx, size, newsize);
}


int CheckExecFilePatched(unsigned char * addr, void * arg2)
{
    //scan structure
    //6.31 kernel modules use type 3 PRX... 0xd4~0x10C is zero padded
    int pos = 0; for(; pos < 0x38; pos++)
    {
        //nonzero byte encountered
        if(addr[pos + 212])
        {
            //forward to unsign function?
            return origCheckExecFile(addr, arg2);
        }
    }

    //return success
    return 0;
}

u32 loadCoreModuleStartCommon(u32 module_start){

    // Calculate Text Address and size
    u32 text_addr = module_start-0xAF8;
    u32 top_addr = text_addr+0x8000; // read 32KB at most (more than enough to scan loadcore)
    
    // Fetch Original Decrypt Function Stub
    SonyPRXDecrypt = (void *)FindImportRange("memlmd", 0xEF73E85B, text_addr, top_addr);
    origCheckExecFile = (void *)FindImportRange("memlmd", 0x6192F715, text_addr, top_addr);

    u32 decrypt_call = JAL(SonyPRXDecrypt);
    u32 check_call = JAL(origCheckExecFile);

    // Hook Signcheck Function Calls
    int count = 0;
    u32 addr;
    for (addr = text_addr; addr<top_addr; addr+=4){
        u32 data = _lw(addr);
        if (data == decrypt_call){
            _sw(JAL(PROPRXDecrypt), addr);
        }
        else if (data == check_call){
            _sw(JAL(CheckExecFilePatched), addr);
        }
        else if (data == 0x26E50028){
            // Don't break on unresolved syscalls
            _sw(0x00001021, addr-20);
        }
    }

    return text_addr;
}

// Invalidate Instruction and Data Cache
void flushCache(void)
{
    // Invalidate Data Cache
    sceRebootDacheWritebackInvalidateAll();
    // Invalidate Instruction Cache
    sceRebootIcacheInvalidateAll();
}

// Common rebootex patches for PSP and Vita
u32 findRebootFunctions(u32 reboot_start){
    register void (* Icache)(void) = NULL;
    register void (* Dcache)(void) = NULL;
    u32 reboot_end = 0;
    // find functions
    for (u32 addr = reboot_start; ; addr+=4){
        u32 data = _lw(addr);
        if (data == 0xBD01FFC0){ // sceRebootDacheWritebackInvalidateAll
            u32 a = addr;
            do {a-=4;} while (_lw(a) != 0x40088000);
            Dcache = (void*)a;
        }
        else if (data == 0xBD14FFC0){ // sceRebootIcacheInvalidateAll
            u32 a = addr;
            do {a-=4;} while (_lw(a) != 0x40088000);
            Icache = (void*)a;
        }
#ifdef REBOOTEX
        else if (data == 0x8FA50008 && _lw(addr+8) == 0x8FA40004){ // UnpackBootConfig
            UnpackBootConfigArg = addr+8;
            u32 a = addr;
            do { a+=4; } while (_lw(a) != 0x24060001);
            UnpackBootConfig = K_EXTRACT_CALL(a-4);
            UnpackBootConfigCall = a-4;
        }
#else
        else if (data == 0x8FA40004){ // UnpackBootConfig
            if (_lw(addr+8) == 0x8FA50008) {
                UnpackBootConfigArg = addr;
                UnpackBootConfig = K_EXTRACT_CALL(addr+4);
                UnpackBootConfigCall = addr+4;
            }
            else if (_lw(addr+4) == 0x8FA50008) {
                UnpackBootConfigArg = addr;
                UnpackBootConfig = K_EXTRACT_CALL(addr+8);
                UnpackBootConfigCall = addr+8;
            }
        }
#endif
        else if ((data == _lw(addr+4)) && (data & 0xFC000000) == 0xAC000000){ // Patch ~PSP header check
            // Returns size of the buffer on loading whatever modules
            _sw(0xAFA50000, addr+4); // sw a1, 0(sp)
            _sw(0x20A30000, addr+8); // move v1, a1
        }
        else if (strcmp(END_BUF_STR, (char*)addr) == 0){
            reboot_end = (addr & -0x4); // found end of reboot buffer
            break;
        }
    }
    sceRebootIcacheInvalidateAll = Icache;
    sceRebootDacheWritebackInvalidateAll = Dcache;
    Icache();
    Dcache();
    return reboot_end;
}

// Entry Point
int _arkReboot(int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7)
{
    #ifdef DEBUG
    colorDebug(0xff00);
    #endif
    
#if defined(REBOOTEX) && defined(MS_IPL)
    // GPIO enable
	REG32(0xbc10007c) |= 0xc8;
	__asm("sync"::);
	
	sceSysconInit();
	sceSysconCtrlMsPower(1);
#endif

#ifdef PAYLOADEX
    u32 ctrl = _lw(BOOT_KEY_BUFFER);

    if ((ctrl & SYSCON_CTRL_HOME) == 0) {
        return sceReboot(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
    }

    if ((ctrl & SYSCON_CTRL_RTRG) == 0) {
        _arkconf.recovery = 1;
    }

    memcpy(ark_config, &_arkconf, sizeof(ARKConfig));
#endif

    reboot_start = REBOOT_TEXT;
    reboot_end = findRebootFunctions(reboot_start); // scan for reboot functions
    
    // patch reboot buffer
    patchRebootBuffer();
    
    // Forward Call
    return sceReboot(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
}
