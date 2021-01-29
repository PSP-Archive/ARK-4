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
#include <systemctrl.h>
#include <macros.h>
#include <psxspu.h>

#define PSP_SPU_REGISTER 0x49F40000

extern ARKConfig* ark_config;

// SPU Status
static int running = 0;

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
    
    // Spawn Background Thread
    sceKernelStartThread(sceKernelCreateThread("SPUThread", spuThread, 0x10, 32 * 1024, 0, NULL), 0, NULL);
    
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
    memset(path, 0, ARK_PATH_SIZE);
    strcpy(path, ark_config->arkpath);
    strcat(path, "PS1SPU.PRX");
    result = sceKernelLoadModule(path, 0, NULL);

    static char g_DiscID[32];
    u16 paramType = 0;
    u32 paramLength = sizeof(g_DiscID);
    sctrlGetInitPARAM("DISC_ID", &paramType, &paramLength, g_DiscID);
    startResult = sceKernelStartModule(result, strlen(g_DiscID) + 1, g_DiscID, &status, NULL);
    printk("%s: fname %s load 0x%08X, start 0x%08X -> 0x%08X\r\n", __func__, path, result, startResult, status);
    
    // load pops module
    result = sceKernelLoadModule("flash0:/kd/660pops.prx" /*path*/, flag, opt);
    printk("%s: fname %s flag 0x%08X -> 0x%08X\r\n", __func__, fname, flag, result);

    return result;
}

void replacePSXSPU(SceModule2 * mod)
{
    // Fetch Text Address
    unsigned int text_addr = mod->text_addr;

    UNUSED(text_addr);
    // Replace Media Engine SPU Background Thread Starter
    hookImportByNID(mod, "sceMeAudio", 0xDE630CD2, _sceMeAudio_DE630CD2);
    
    /*
    #ifdef WITH_PEOPS_SPU
    // Replace SPU Register Read Callback
    void * read_stub_addr = (void *)(text_addr + 0x000D5424 + 8);
    patchSyscallStub(spuReadCallback, read_stub_addr);
    unsigned int spuRCB = text_addr + 0x85F4;
    _sw(0x27BDFFFC, spuRCB); // addiu $sp, $sp, -4
    _sw(0xAFBF0000, spuRCB + 4); // sw $ra, 0($sp)
    _sw(JAL(read_stub_addr), spuRCB + 8); // jal callback stub
    _sw(NOP, spuRCB + 12); // nop
    _sw(0x8FBF0000, spuRCB + 16); // lw $ra, 0($sp)
    _sw(0x03E00008, spuRCB + 20); // jr $ra
    _sw(0x27BD0004, spuRCB + 24); // addiu $sp, $sp, 4
    
    // Replace SPU Register Write Callback
    void * write_stub_addr = read_stub_addr + 8;
    patchSyscallStub(spuWriteCallback, write_stub_addr);
    unsigned int spuWCB = text_addr + 0x7F00;
    _sw(0x27BDFFFC, spuWCB); // addiu $sp, $sp, -4
    _sw(0xAFBF0000, spuWCB + 4); // sw $ra, 0($sp)
    _sw(JAL(write_stub_addr), spuWCB + 8); // jal callback stub
    _sw(NOP, spuWCB + 12); // nop
    _sw(0x8FBF0000, spuWCB + 16); // lw $ra, 0($sp)
    _sw(0x03E00008, spuWCB + 20); // jr $ra
    _sw(0x27BD0004, spuWCB + 24); // addiu $sp, $sp, 4
    #endif
    */
}

void patchVitaPopsman(SceModule2* mod){
    u32 text_addr = mod->text_addr;
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
    
    // patch loadmodule to load our own spu plugin
    _sw(JAL(myKernelLoadModule), text_addr + 0x00001EE0);
}
