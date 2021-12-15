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

static UserFunctions _g_tbl;
static KernelFunctions _k_tbl;
static KxploitFunctions _kxf;

UserFunctions* g_tbl = &_g_tbl;
KernelFunctions* k_tbl = &_k_tbl;
KxploitFunctions* kxf = &_kxf;


// counter for relocated stubs
static u32 curcall = 0x08801000;

// fill UserFunctions
void scanUserFunctions(UserFunctions* tbl){

    memset(g_tbl, 0, sizeof(UserFunctions));
    
    tbl->IoOpen = (void*)RelocImport("IoFileMgrForUser", 0x109F50BC, 0);
    tbl->IoRead = (void*)RelocImport("IoFileMgrForUser", 0x6A638D83, 0);
    tbl->IoClose = (void*)RelocImport("IoFileMgrForUser", 0x810C4BC3, 0);
    tbl->IoWrite = (void*)RelocImport("IoFileMgrForUser", 0x42EC03AC, 0);
    tbl->IoRemove = (void*)RelocImport("IoFileMgrForUser", 0xF27A9C51, 0);
    
    tbl->KernelLibcTime = (void*)RelocImport("UtilsForUser", 0x27CC57F0, 0);
    tbl->KernelLibcClock = (void*)RelocImport("UtilsForUser", 0x91E4F6A7, 0);
    tbl->KernelPowerLock = (void*)RelocImport("sceNetIfhandle_lib", 0xE80F00A4, 0);
    tbl->KernelDcacheWritebackAll = (void*)RelocImport("UtilsForUser", 0x79D1C3FA, 0);
    tbl->KernelIcacheInvalidateAll = (void*)RelocImport("UtilsForUser", 0x920F104A, 0);
    tbl->DisplaySetFrameBuf = (void*)RelocImport("sceDisplay", 0x289D82FE, 0);

    tbl->KernelCreateThread = (void*)RelocImport("ThreadManForUser", 0x446D8DE6, 0);
    tbl->KernelDelayThread = (void*)RelocImport("ThreadManForUser", 0xCEADEB47, 0);
    tbl->KernelStartThread = (void*)RelocImport("ThreadManForUser", 0xF475845D, 0);
    tbl->KernelWaitThreadEnd = (void*)RelocImport("ThreadManForUser", 0x278C0DF5, 0);
    tbl->KernelExitThread = (void*)RelocImport("ThreadManForUser", 0xAA73C935, 0);
    tbl->KernelExitDeleteThread = (void*)RelocImport("ThreadManForUser", 0x809CE29B, 0);

    tbl->KernelCreateVpl = (void*)RelocImport("ThreadManForUser", 0x56C039B5, 0),
    tbl->KernelTryAllocateVpl = (void*)RelocImport("ThreadManForUser", 0xAF36D708, 0),
    tbl->KernelFreeVpl = (void*)RelocImport("ThreadManForUser", 0xB736E9FF, 0),
    tbl->KernelDeleteVpl = (void*)RelocImport("ThreadManForUser", 0x89B3D48C, 0);
    tbl->KernelDeleteFpl = (void*)RelocImport("ThreadManForUser", 0xED1410E0, 0);
    
    tbl->UtilityLoadModule = (void*)RelocImport("sceUtility", 0x2A2B3DE0, 0);
    tbl->UtilityUnloadModule = (void*)RelocImport("sceUtility", 0xE49BFE92, 0);
    tbl->UtilityLoadNetModule = (void*)RelocImport("sceUtility", 0x1579A159, 0);
    tbl->UtilityUnloadNetModule = (void*)RelocImport("sceUtility", 0x64D50C56, 0);
    
    //tbl->SysMemUserForUser_91DE343C = (void*)RelocImport("SysMemUserForUser", 0x91DE343C, 0);
    tbl->KernelFreePartitionMemory = (void*)RelocImport("SysMemUserForUser", 0xB6D61D02, 0);
    tbl->KernelAllocPartitionMemory = (void*)RelocImport("SysMemUserForUser", 0x237DBD4F, 0);

    // savedata functions
    tbl->UtilitySavedataGetStatus = (void*)RelocImport("sceUtility", 0x8874DBE0, 0);
    tbl->UtilitySavedataInitStart = (void*)RelocImport("sceUtility", 0x50C4CD57, 0);
    tbl->UtilitySavedataUpdate = (void*)RelocImport("sceUtility", 0xD4B95FFB, 0);
    tbl->UtilitySavedataShutdownStart = (void*)RelocImport("sceUtility", 0x9790B33C, 0);
    tbl->KernelVolatileMemUnlock = (void*)RelocImport("sceSuspendForUser", 0xA569E425, 0);
    
}

void scanArkFunctions(UserFunctions* tbl){
    // ARK Functions
    tbl->freeMem = &freeMem;
    tbl->FindImportUserRam = &FindImportUserRam;
    tbl->FindImportVolatileRam = &FindImportVolatileRam;
    tbl->FindImportRange = &FindImportRange;
    tbl->RelocSyscall = &RelocSyscall;
    tbl->p5_open_savedata = &p5_open_savedata;
    tbl->p5_close_savedata = &p5_close_savedata;
    tbl->qwikTrick = &qwikTrick;
}

int IsUID(SceUID uid){
    return ((uid > 0 && uid < 0x05000000) && ((uid & 1) == 1));
}

// attempt to free as much memory as possible, some kernel/user exploits need this, others don't
void freeMem(){
    register u32 (* sceKernelGetBlockHeadAddr_)(SceUID) = (void *)RelocImport("SysMemUserForUser", 0x9D9A5BA1, 0);
    register int (* sceKernelDeleteFpl_)(SceUID) = (void *)RelocImport("ThreadManForUser", 0xED1410E0, 0);
    register int (* sceKernelTerminateThread_)(SceUID) = (void *)RelocImport("ThreadManForUser", 0x616403BA, 0);
    register int (* sceKernelSuspendThread_)(SceUID) = (void *)RelocImport("ThreadManForUser", 0x9944F31F, 0);

    register u32 addr;

    for (addr = 0x08800000; addr < 0x0A000000; addr += 4) {
        if (IsUID(*(SceUID*)addr)) {
            if ((sceKernelGetBlockHeadAddr_) && (sceKernelGetBlockHeadAddr_(*(SceUID*)addr) > 0))
                g_tbl->KernelFreePartitionMemory(*(SceUID*)addr);

            if (sceKernelTerminateThread_)
                sceKernelTerminateThread_(*(SceUID*)addr);
            else if (sceKernelSuspendThread_)
                sceKernelSuspendThread_(*(SceUID*)addr);
            
            if (sceKernelDeleteFpl_)
                sceKernelDeleteFpl_(*(SceUID*)addr);
        }
    }
}

void unloadUtilities(){
    u16 utilities[] = { PSP_MODULE_NET_ADHOC, PSP_MODULE_USB_PSPCM, PSP_MODULE_USB_MIC, PSP_MODULE_USB_CAM, \
        PSP_MODULE_USB_GPS, PSP_MODULE_AV_AVCODEC, PSP_MODULE_AV_SASCORE, PSP_MODULE_AV_ATRAC3PLUS, \
        PSP_MODULE_AV_MPEGBASE, PSP_MODULE_AV_MP3, PSP_MODULE_AV_VAUDIO, PSP_MODULE_AV_AAC, PSP_MODULE_AV_G729, \
        PSP_MODULE_NP_COMMON, PSP_MODULE_NP_SERVICE, PSP_MODULE_NP_MATCHING2, PSP_MODULE_NP_DRM
    };
    
    for (int i=0; i<NELEMS(utilities); i++){
        g_tbl->UtilityUnloadModule(utilities[i]);
    }
}

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

void _flush_cache(){
    k_tbl->KernelDcacheWritebackInvalidateAll();
}

// qwikrazor87's trick to get any usermode import from kernel
u32 qwikTrick(char* lib, u32 nid, u32 version){

    u32 ret = 0x08800E00;

    while (*(u32*)ret)
        ret += 8;

    p5_open_savedata(PSP_UTILITY_SAVEDATA_AUTOLOAD);

    u32 addr;
    for (addr = 0x08400000; addr < 0x08800000; addr += 4) {
        if (strcmp("sceVshSDAuto_Module", (char *)addr) == 0)
            break;
    }

    p5_close_savedata();

    addr -= 0xBC;
    *(u32*)0x08800C00 = nid;

    int qwik_trick()
    {
        k_tbl->KernelDelayThread(350);
        u32 timer = 0;

        while (!*(u32*)0x08800D00 && (timer++ < 600)) {
            _sw((u32)lib, addr);
            _sw(version, addr + 4);
            _sw(0x00010005, addr + 8);
            _sw(0x08800C00, addr + 12);
            _sw(0x08800D00, addr + 16);

            k_tbl->KernelDelayThread(0);
        }

        k_tbl->KernelExitThread(0);
        return 0;
    }

    SceUID qwiktrick = k_tbl->KernelCreateThread("qwiktrick", qwik_trick, 8, 512, THREAD_ATTR_USER, NULL);
    k_tbl->KernelStartThread(qwiktrick, 0, NULL);

    p5_open_savedata(PSP_UTILITY_SAVEDATA_AUTOLOAD);

    memcpy((void *)ret, (const void *)0x08800D00, 8);

    _flush_cache();

    p5_close_savedata();

    memset((void *)0x08800D00, 0, 8);

    k_tbl->KernelDeleteThread(qwiktrick);
    
    return ret;
}

u32 FindImportVolatileRam(char *libname, u32 nid){
    return FindImportRange(libname, nid, 0x08400000, 0x08800000);
}

u32 FindImportUserRam(char *libname, u32 nid){
    return FindImportRange(libname, nid, 0x08800000, 0x0A000000);
}

void *RelocSyscall(u32 call){
    
    if (call != 0) {
        while (_lw(curcall))
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

/*
 * These functions are ment for using when initial kernel access has been
 * granted, for example through the mean of a kernel exploit.
 */
void scanKernelFunctions(KernelFunctions* kfuncs){

    memset(kfuncs, 0, sizeof(KernelFunctions));

    kfuncs->KernelIOOpen = (void*)FindFunction("sceIOFileManager", "IoFileMgrForKernel", 0x109F50BC);
    kfuncs->KernelIORead = (void*)FindFunction("sceIOFileManager", "IoFileMgrForKernel", 0x6A638D83);
    kfuncs->KernelIOLSeek = (void*)FindFunction("sceIOFileManager", "IoFileMgrForKernel", 0x27EB27B8);
    kfuncs->KernelIOClose = (void*)FindFunction("sceIOFileManager", "IoFileMgrForKernel", 0x810C4BC3);
    kfuncs->KernelIOWrite = (void*)FindFunction("sceIOFileManager", "IoFileMgrForKernel", 0x42EC03AC);
    kfuncs->KernelIOMkdir = (void*)FindFunction("sceIOFileManager", "IoFileMgrForKernel", 0x06A70004);
    kfuncs->KernelIORmdir = (void*)FindFunction("sceIOFileManager", "IoFileMgrForKernel", 0x1117C65F);
    kfuncs->KernelIODopen = (void*)FindFunction("sceIOFileManager", "IoFileMgrForKernel", 0xB29DDF9C);
    kfuncs->KernelIODread = (void*)FindFunction("sceIOFileManager", "IoFileMgrForKernel", 0xE3EB004C);
    kfuncs->KernelIODclose = (void*)FindFunction("sceIOFileManager", "IoFileMgrForKernel", 0xEB092469);
    kfuncs->KernelIOGetStat = (void*)FindFunction("sceIOFileManager", "IoFileMgrForKernel", 0xACE946E8);
    kfuncs->KernelIORemove = (void*)FindFunction("sceIOFileManager", "IoFileMgrForKernel", 0xF27A9C51);
    kfuncs->IoAssign = (void*)FindFunction("sceIOFileManager", "IoFileMgrForKernel", 0xB2A628C1);
    kfuncs->IoUnassign = (void*)FindFunction("sceIOFileManager", "IoFileMgrForKernel", 0x6D08A871);
    
    kfuncs->KernelIcacheInvalidateAll = (void*)FindFunction("sceSystemMemoryManager", "UtilsForKernel", 0x920F104A);
    kfuncs->KernelDcacheWritebackInvalidateAll = (void*)FindFunction("sceSystemMemoryManager", "UtilsForKernel", 0xB435DEC5);
    kfuncs->KernelDcacheInvalidateRange = (void*)FindFunction("sceSystemMemoryManager", "UtilsForKernel", 0xBFA98062);
    kfuncs->KernelGzipDecompress = (void*)FindFunction("sceSystemMemoryManager", "UtilsForKernel", 0x78934841);
    
    kfuncs->KernelFindModuleByName = (void*)FindFunction("sceLoaderCore", "LoadCoreForKernel", 0xF6B1BF0F);
    
    kfuncs->KernelCreateThread = (void*)FindFunction("sceThreadManager", "ThreadManForKernel", 0x446D8DE6);
    kfuncs->KernelStartThread = (void*)FindFunction("sceThreadManager", "ThreadManForKernel", 0xF475845D);
    kfuncs->KernelDelayThread = (void*)FindFunction("sceThreadManager", "ThreadManForKernel", 0xCEADEB47);
    kfuncs->KernelExitThread = (void*)FindFunction("sceThreadManager", "ThreadManForKernel", 0xAA73C935);
    kfuncs->KernelDeleteThread = (void*)FindFunction("sceThreadManager", "ThreadManForKernel", 0x9FA03CD3);
    kfuncs->waitThreadEnd = (void*)FindFunction("sceThreadManager", "ThreadManForKernel", 0x278C0DF5);
    
    // ARK kernel functions
    kfuncs->FindTextAddrByName = &FindTextAddrByName;
    kfuncs->FindFunction = &FindFunction;
}

u32 FindTextAddrByName(const char *modulename)
{
    u32 kaddr;
    for (kaddr = 0x88000000; kaddr < 0x88400000; kaddr += 4) {
        if (strcmp((const char *)kaddr, modulename) == 0) {
            if ((*(u32*)(kaddr + 0x64) == *(u32*)(kaddr + 0x78)) && \
                (*(u32*)(kaddr + 0x68) == *(u32*)(kaddr + 0x88))) {
                if (*(u32*)(kaddr + 0x64) && *(u32*)(kaddr + 0x68))
                    return *(u32*)(kaddr + 0x64);
            }
        }
    }
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

u32 patchNextInstruction(u32 orig, u32 instr, u32 addr, int skip){
    for (;; addr+=4){
        if (_lw(addr) == orig){
            if (skip==0){
                _sw(instr, addr);
                break;
            }
            else skip--;
        }
    }
    return addr;
}

int p5_open_savedata(int mode)
{
    p5_close_savedata();

    SceUtilitySavedataParam dialog;

    memset(&dialog, 0, sizeof(SceUtilitySavedataParam));
    dialog.base.size = sizeof(SceUtilitySavedataParam);

    dialog.base.language = 1;
    dialog.base.buttonSwap = 1;
    dialog.base.graphicsThread = 0x11;
    dialog.base.accessThread = 0x13;
    dialog.base.fontThread = 0x12;
    dialog.base.soundThread = 0x10;

    dialog.mode = mode;

    g_tbl->UtilitySavedataInitStart(&dialog);

    // Wait for the dialog to initialize
    int status;
    while ((status = g_tbl->UtilitySavedataGetStatus()) < 2)
    {
        g_tbl->KernelDelayThread(100);
        if (status < 0) return 0; // error
    }
    return 1;
}

// Runs the savedata dialog loop
int p5_close_savedata()
{

    int running = 1;
    int last_status = -1;

    while(running) 
    {
        int status = g_tbl->UtilitySavedataGetStatus();
        
        if (status != last_status)
        {
            last_status = status;
        }
        switch(status)
        {
            case PSP_UTILITY_DIALOG_VISIBLE:
                g_tbl->UtilitySavedataUpdate(1);
                break;

            case PSP_UTILITY_DIALOG_QUIT:
                g_tbl->UtilitySavedataShutdownStart();
                break;

            case PSP_UTILITY_DIALOG_NONE:
                running = 0;
                break;

            case PSP_UTILITY_DIALOG_FINISHED:
                break;
            default:
                if (status < 0) // sceUtilitySavedataGetStatus returned error?
                    return 0;
                break;
        }
        g_tbl->KernelDelayThread(100);
    }
    return 1;
}

int isKernel(){
    u32 ra;
    __asm__ volatile ("move %0, $ra;" : "=r"(ra));
    return (ra&0x80000000) != 0;
}

void AccurateError(u32 text_addr, u32 text_size)
{
    u32 counter = 0;
    u32 text_end = text_addr+text_size;

    for (; text_addr < text_end; text_addr += 4)
    {
        u32 code = _lw(text_addr);

        if ((code & 0xFC00FFFF) == 0x34000148)
        {
            counter++;
            _sw((code & 0xFFFF0000) | (0xA000 + counter), text_addr);
        }
    }
}
