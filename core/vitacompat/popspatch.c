#include <pspsdk.h>
#include <systemctrl.h>
#include <macros.h>
#include "popspatch.h"

#define PSP_SPU_REGISTER 0x49F40000

extern ARKConfig* ark_config;

// SPU Status
static int running = 0;
static int spu_plugin = -1; // spu thread UID

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
    
    sceKernelStartThread(sceKernelCreateThread("SPUThread", spuThread, 0x10, 32 * 1024, 0, NULL), 0, NULL);
    
    // Restore Permission Level
    pspSdkSetK1(k1);
}

void patchVitaPopsSpu(SceModule2 * mod)
{

    if (spu_plugin>=0) return; // don't patch pops if spu plugin loaded
    
    // Fetch Text Address
    unsigned int text_addr = mod->text_addr;

    UNUSED(text_addr);
    // Replace Media Engine SPU Background Thread Starter
    hookImportByNID(mod, "sceMeAudio", 0xDE630CD2, _sceMeAudio_DE630CD2);
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

    colorDebug(0xff);

    char path[ARK_PATH_SIZE];
    int result = 0;
    int status = 0;
    int startResult = 0;

    // load spu module
    /*
    memset(path, 0, ARK_PATH_SIZE);
    strcpy(path, ark_config->arkpath);
    strcat(path, "PS1SPU.PRX");
    result = sceKernelLoadModule(path, 0, NULL);

    spu_plugin = result; // remember spu plugin UID

    static char g_DiscID[32];
    u16 paramType = 0;
    u32 paramLength = sizeof(g_DiscID);
    sctrlGetInitPARAM("DISC_ID", &paramType, &paramLength, g_DiscID);
    startResult = sceKernelStartModule(result, strlen(g_DiscID) + 1, g_DiscID, &status, NULL);
    printk("%s: fname %s load 0x%08X, start 0x%08X -> 0x%08X\r\n", __func__, path, result, startResult, status);
    */
    
    // load pops module
    result = sceKernelLoadModule("flash0:/kd/pops_01g.prx", flag, opt); // ARK IO patch will try to load local first
    printk("%s: fname %s flag 0x%08X -> 0x%08X\r\n", __func__, fname, flag, result);

    return result;
}

void patchVitaPopsman(SceModule2* mod){
    u32 text_addr = mod->text_addr;
    u32 top_addr = text_addr + mod->text_size;
    for (u32 addr=text_addr; addr<top_addr; addr+=4){
        u32 data = _lw(addr);
        if (data == 0x004C800B){ // patch loadmodule to load our own spu plugin
            _sw(JAL(myKernelLoadModule), addr - 0x2C);
            if (addr == text_addr+0x00001F0C){ // patch PSP 6.60 POPS
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
            }
            break;
        }
    }
}
