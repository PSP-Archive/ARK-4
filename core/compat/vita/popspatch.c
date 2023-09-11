#include <pspsdk.h>
#include <systemctrl.h>
#include <macros.h>
#include "popspatch.h"
#include "functions.h"

#define PSP_SPU_REGISTER 0x49F40000

extern ARKConfig* ark_config;

// SPU Status
static int running = 0; // dummy spu
static int spu_running = 0; // spu plugin

// SPU Background Thread
int spuThread(SceSize args, void * argp)
{
    // Set SPU Status
    running = 1;
    
    // Endless Loop
    while(running)
    {
        
        // Clear SPU Thread Busy Flag
        _sb(-2, PSP_SPU_REGISTER + 0x293);
        
        // Set left number of to-be-processed samples to 0 (done processing)
        _sh(0, PSP_SPU_REGISTER + 0x290);
        
        // Disable SPU in PSX SPU Status Register
        // (I don't know if this is clever... we shouldn't do that in the final SPU plugin...)
        // spuWriteCallback(0x1AA, spuReadCallback(0x1AA, SIGNED_SHORT) & 0x7FFF, SIGNED_SHORT);
        
        // Synchronize to 2 Milliseconds
        sceKernelDelayThread(2000);
    }
    
    // Destroy Background Thread
    sceKernelExitDeleteThread(0);
    
    // Shut up GCC
    return 0;
}

// SPU Background Thread Starter
void _sceMeAudio_DE630CD2(void * loopCore, void * stack)
{
    // Right now the Audio Thread doesn't work properly yet...
    // This is due to its nature of being optimized for Media Engine.
    // Especially the fact that it is using $ra for calculations are deadly
    // for the Main CPU... thus... a temporary "No-Sound-Workaround".
    // unsigned int text_addr = (unsigned int)loopCore;
    
    // This Patch fixes the Movie & CDDA Blocking Issue for now...
    // unsigned int waitForAudio = 0x83E8;
    // _sw(JR_RA, text_addr + waitForAudio);
    // _sw(LI_V0(0), text_addr + waitForAudio + 4);
    
    // Flush Cache
    // flushCache();

    // Elevate Permission Level
    unsigned int k1 = pspSdkSetK1(0);
    int thid = sceKernelCreateThread("SPUThread", spuThread, 0x10, 32 * 1024, 0, NULL);
    sceKernelStartThread(thid, 0, NULL);
    // Restore Permission Level
    pspSdkSetK1(k1);
}

// Shutdown SPU
void spuShutdown(void)
{
    if (running){
        // Set Shutdown Flag
        running = 0;
        // Wait a second (doesn't hurt us)
        sceKernelDelayThread(1000000);
    }
}

static int myKernelLoadModule(char * fname, int flag, void * opt)
{

    char path[ARK_PATH_SIZE];
    int result = 0;
    int status = 0;
    int startResult = 0;

    // load spu module
    strcpy(path, ark_config->arkpath);
    strcat(path, PS1SPU_PRX);
    result = sceKernelLoadModule(path, 0, NULL);

    if (result >= 0){
        spu_running = 1;
        static char g_DiscID[32];
        u16 paramType = 0;
        u32 paramLength = sizeof(g_DiscID);
        sctrlGetInitPARAM("DISC_ID", &paramType, &paramLength, g_DiscID);
        startResult = sceKernelStartModule(result, strlen(g_DiscID) + 1, g_DiscID, &status, NULL);
    }
    
    #ifdef DEBUG
    printk("%s: fname %s load 0x%08X, start 0x%08X -> 0x%08X\r\n", __func__, path, result, startResult, status);
    #endif
    
    // load pops module from ARK savedata path
    strcpy(path, ark_config->arkpath);
    strcat(path, "POPS.PRX");
    result = sceKernelLoadModule(path, flag, opt);

    //if (result<0) result = sceKernelLoadModule("flash0:/kd/pops_660.prx", flag, opt); // load pops modules from injected flash0
    //if (result<0) result = sceKernelLoadModule(fname, flag, opt); // passthrough

    #ifdef DEBUG
    printk("%s: fname %s flag 0x%08X -> 0x%08X\r\n", __func__, fname, flag, result);
    #endif

    //PRTSTR1("Load result: %p", result);
    //sceKernelDelayThread(3000000);

    return result;
}

void patchPspPopsman(SceModule2* mod){
    u32 text_addr = mod->text_addr;
    u32 top_addr = text_addr + mod->text_size;
    /*
    for (u32 addr=text_addr; addr<top_addr; addr+=4){
        u32 data = _lw(addr);
        if (data == 0x7C1D2804){
            u32 a = addr;
            do { a-=4; } while (_lw(a) != 0x27BDFFE0); // find start of function
            MAKE_DUMMY_FUNCTION_RETURN_0(a);
        }
        else if (data == 0x3C0BBC10 || data == 0x3C0CBC10){
            MAKE_DUMMY_FUNCTION_RETURN_0(addr-40);
        }
        else if (data == 0x0004882B){
            MAKE_DUMMY_FUNCTION_RETURN_0(addr-8);
        }
        else if (data == 0x8CC607F0){
            u32 a = addr;
            do { a+=4; } while (_lw(a) != 0x27BD0010);
            MAKE_DUMMY_FUNCTION_RETURN_0(a+4);
            do { a+=4; } while (_lw(a) != 0x03E00008);
            MAKE_DUMMY_FUNCTION_RETURN_0(a+8);
        }
        else if (data == 0x70000000 && _lw(addr+8) == NOP){
            u32 a = addr;
            do { a+=4; } while (_lw(a)&0xFFFF0000 != 0x3C030000);
            MAKE_DUMMY_FUNCTION_RETURN_0(a);
        }
        else if (data == 0x3444006C){
            u32 a = addr;
            do { a-=4; } while (_lw(a) != 0x27BDFFF0); // find start of function
            MAKE_DUMMY_FUNCTION_RETURN_0(a);
        }
        else if (data == 0x3404AC44 && _lw(addr+20) == 0x001B1AC0){
            MAKE_DUMMY_FUNCTION_RETURN_0(addr+20);
        }
        else if (data == 0x00002821 && _lw(addr+8) == 0x00003021){
            _sw(JAL(myKernelLoadModule), addr + 4);
        }
    }
    */

    // TN hacks
    _sw(JR_RA, text_addr + 0x2F88);
    _sw(LI_V0(0), text_addr + 0x2F88 + 4);

    _sw(JR_RA, text_addr + 0x35D8);
    _sw(LI_V0(0), text_addr + 0x35D8 + 4);
    
    _sw(JR_RA, text_addr + 0x3514);
    _sw(LI_V0(0), text_addr + 0x3514 + 4);
    
    _sw(JR_RA, text_addr + 0x3590);
    _sw(LI_V0(0), text_addr + 0x3590 + 4);
    
    _sw(JR_RA, text_addr + 0x35AC);
    _sw(LI_V0(0), text_addr + 0x35AC + 4);
    
    _sw(JR_RA, text_addr + 0x31EC);
    _sw(LI_V0(0), text_addr + 0x31EC + 4);

    // Coldbird hacks
    _sw(JR_RA, text_addr + 0x0000342C);
    _sw(LI_V0(0), text_addr + 0x0000342C + 4);

    _sw(JR_RA, text_addr + 0x00003490);
    _sw(LI_V0(0), text_addr + 0x00003490 + 4);
    
    // patch loadmodule to load our own pops.prx
    _sw(JAL(myKernelLoadModule), text_addr + 0x00001EE0);
}

void patchPspPopsSpu(SceModule2 * mod)
{
    // Fetch Text Address
    unsigned int text_addr = mod->text_addr;

    int patches = 2;
    for (u32 i = 0; i < mod->text_size; i += 4)
    {
        u32 addr = mod->text_addr + i;
        u32 data = _lw(addr);

        // Replace Media Engine SPU Background Thread Starter
        if (data == 0x24050260 && !spu_running){
            u32 a = addr;
            do { a+=4; } while (_lw(a) != 0x8FBF0004); // find end of function
            u32 stub = U_EXTRACT_CALL(a-8);
            REDIRECT_SYSCALL(stub, _sceMeAudio_DE630CD2);
            patches--;
        }
        // Fix index length (enable CDDA)
        else if ((data == 0x14C00014 && _lw(addr + 4) == 0x24E2FFFF) ||
            (data == 0x14A00014 && _lw(addr + 4) == 0x24C2FFFF))
        {
            _sh(0x1000, addr + 2);
            _sh(0, addr + 4);
            patches--;
        }
    }
}