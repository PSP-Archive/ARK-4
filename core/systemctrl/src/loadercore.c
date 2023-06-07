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
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <stdio.h>
#include <string.h>
#include <macros.h>
#include <module2.h>
#include <systemctrl.h>
#include <systemctrl_private.h>
#include <globals.h>
#include <functions.h>
#include "imports.h"
#include "modulemanager.h"
#include "nidresolver.h"
#include "plugin.h"
#include "elf.h"
#include "loadercore.h"
#include "cryptography.h"
#include "rebootconfig.h"
#include "graphics.h"
#include "libs/colordebugger/colordebugger.h"

// init.prx Text Address
unsigned int sceInitTextAddr = 0;

// Plugin Loader Status
int pluginLoaded = 0;

// Real Executable Check Function Pointer
int (* ProbeExec1)(u8 *buffer, int *check) = NULL;
int (* ProbeExec2)(u8 *buffer, int *check) = NULL;

// Sony PRX Decrypter Function Pointer
int (* SonyPRXDecrypt)(void *, unsigned int, unsigned int *) = NULL;
int (* origCheckExecFile)(unsigned char * addr, void * arg2) = NULL;

// init.prx Custom sceKernelStartModule Handler
int (* customStartModule)(int modid, SceSize argsize, void * argp, int * modstatus, SceKernelSMOption * opt) = NULL;

// Executable Check #1
int _ProbeExec1(u8 *buffer, int *check)
{
    // Check Executable (we patched our files with shifted attributes so this works)
    int result = ProbeExec1(buffer, check);
    
    // Grab Executable Magic
    unsigned int magic = *(unsigned int *)(buffer);
    
    // ELF File
    if(magic == 0x464C457F)
    {
        // Recover Attributes (which we shifted before)
        unsigned short realattr = *(unsigned short *)(buffer + check[19]);
        
        // Mask Attributes
        unsigned short attr = realattr & 0x1E00;
        
        // Kernel Module
        if(attr != 0)
        {
            // Fetch OFW-detected Attributes
            unsigned short attr2 = *(u16*)((void*)(check)+0x58);
            
            // OFW Attributes don't match
            if((attr2 & 0x1E00) != attr)
            {
                // Now they do. :)
                *(u16*)((void*)(check)+0x58) = realattr;
            }
        }
        
        // Flip Switch
        if(check[18] == 0) check[18] = 1;
    }
    
    // Return Result
    return result;
}

// Executable Check #2
int _ProbeExec2(u8 *buffer, int *check)
{
    // Check Executable
    int result = ProbeExec2(buffer, check);
    
    // Grab Executable Magic
    unsigned int magic = *(unsigned int *)(buffer);
    
    // Plain Static ELF Executable
    if(magic == 0x464C457F && IsStaticElf(buffer))
    {
        // Fake UMD Apitype (as its the only one that allows Static ELFs... and even that, only as LoadExec Target)
        check[2] = 0x120;
        
        // Invalid Module Info Section
        if(check[19] == 0)
        {
            // Search String Table
            char * strtab = GetStrTab(buffer);
            
            // Found it! :D
            if(strtab != NULL)
            {
                // Cast ELF Header
                Elf32_Ehdr * header = (Elf32_Ehdr *)buffer;
                
                // Section Header Start Pointer
                unsigned char * pData = buffer + header->e_shoff;
                
                // Iterate Section Headers
                for (int i = 0; i < header->e_shnum; i++)
                {
                    // Cast Section Header
                    Elf32_Shdr * section = (Elf32_Shdr *)pData;
                    
                    // Found Module Info Section
                    if(strcmp(strtab + section->sh_name, ".rodata.sceModuleInfo") == 0)
                    {
                        // Fix Section Pointer
                        check[19] = section->sh_offset;
                        check[22] = 0;
                        
                        // Stop Search
                        break;
                    }
                    
                    // Move to next Section
                    pData += header->e_shentsize;
                }
            }
        }
    }
    
    // Return Result
    return result;
}

// Executable File Check
int KernelCheckExecFile(unsigned char * buffer, int * check)
{
    // Patch Executable
    int result = PatchExec1(buffer, check);
    
    // PatchExec1 isn't enough... :(
    if(result != 0)
    {
        // Check Executable
        int checkresult = sceKernelCheckExecFile(buffer, check);
        
        // Grab Executable Magic
        unsigned int magic = *(unsigned int *)(buffer);
        
        // Patch Executable
        result = PatchExec3(buffer, check, magic == 0x464C457F, checkresult);
    }
    
    // Return Result
    return result;
}

static void loadXmbControl(){
    int apitype = sceKernelInitApitype();
    if (apitype == 0x200 || apitype ==  0x210 || apitype ==  0x220 || apitype == 0x300){
        // load XMB Control Module
        char path[ARK_PATH_SIZE];
        strcpy(path, ark_config->arkpath);
        strcat(path, XMBCTRL_PRX);
        int modid = sceKernelLoadModule(path, 0, NULL);
        if (modid < 0) modid = sceKernelLoadModule("flash0:/kd/ark_xmbctrl.prx", 0, NULL); // retry flash0
        if (modid >= 0) sceKernelStartModule(modid, 0, NULL, NULL, NULL);
    }
}

static void checkArkPath(){
    int res = sceIoDopen(ark_config->arkpath);
    if (res < 0){
        // fix for PSP-Go with dead ef
        if (ark_config->arkpath[0]=='e' && ark_config->arkpath[1]=='f'){
            ark_config->arkpath[0] = 'm'; ark_config->arkpath[1] = 's';
            if ((res=sceIoDopen(ark_config->arkpath))>=0){
                sceIoDclose(res);
                return;
            }
        }
        // no ARK install folder, default to SEPLUGINS
        strcpy(ark_config->arkpath, "ms0:/SEPLUGINS/");
    }
    else{
        sceIoDclose(res);
    }
}

// Init Start Module Hook
int InitKernelStartModule(int modid, SceSize argsize, void * argp, int * modstatus, SceKernelSMOption * opt)
{
    extern u8 rebootex_config[];
    SceModule2* mod = (SceModule2*) sceKernelFindModuleByUID(modid);

    int result = -1;

    // Custom Handler registered
    if(customStartModule != NULL)
    {
        // Forward to Handler
        result = customStartModule(modid, argsize, argp, modstatus, opt);
        if (result >= 0) return result;
    }
    
    // VSH replacement
    if (strcmp(mod->modname, "vsh_module") == 0){
        if (ark_config->recovery || ark_config->launcher[0]){ // system in recovery or launcher mode
            exitLauncher(); // reboot VSH into custom menu
        }
    }

    // load settings before starting mediasync
    if (!pluginLoaded && strcmp(mod->modname, "sceMediaSync") == 0)
    {
        // Check ARK install path
        checkArkPath();
        // Check controller input to disable settings and/or plugins
        checkControllerInput();
        // load settings
        loadSettings();
    }
    
    // start module
    if (result < 0) result = sceKernelStartModule(modid, argsize, argp, modstatus, opt);
    
    // MediaSync not yet started... too early to load plugins.
    if (!pluginLoaded && strcmp(mod->modname, "sceMediaSync") == 0)
    {
        // Load XMB Control
        loadXmbControl();
        // Load Plugins
        LoadPlugins();
        // Remember it
        pluginLoaded = 1;
    }

    return result;
}

// sceKernelStartModule Hook
int patch_sceKernelStartModule_in_bootstart(int (* bootstart)(SceSize, void *), void * argp)
{
    u32 StartModule = JUMP(sctrlHENFindFunction("sceModuleManager", "ModuleMgrForUser", 0x50F0C1EC));
    u32 addr = (u32)bootstart;
    int patches = 1;
    for (;patches; addr+=4){
        if (_lw(addr) == StartModule){
            // Replace Stub
            _sw(JUMP(InitKernelStartModule), addr);
            _sw(NOP, addr + 4);
            patches--;
        }
    }
    // Passthrough
    return bootstart(4, argp);
}

// Patch Loader Core Module
SceModule2* patchLoaderCore(void)
{

    // Find Module
    SceModule2* mod = (SceModule2 *)sceKernelFindModuleByName("sceLoaderCore");

    // Fetch Text Address
    u32 start_addr = mod->text_addr;
    u32 topaddr = mod->text_addr+mod->text_size;

    // restore rebootex pointers to original
    u32 rebootex_decrypt = 0;
    u32 rebootex_checkexec = 0;
    SonyPRXDecrypt = (void*)sctrlHENFindFunction("sceMemlmd", "memlmd", 0xEF73E85B);
    origCheckExecFile = (void*)sctrlHENFindFunction("sceMemlmd", "memlmd", 0x6192F715);
    // find patched functions pointing to rebootex
    int found = 0;
    for (u32 addr = start_addr; addr<topaddr&&!found; addr+=4){
        u32 data = _lw(addr);
        switch (data){
        case 0x35450200: rebootex_checkexec = K_EXTRACT_CALL(addr+12);
        case 0x35250200: rebootex_decrypt = K_EXTRACT_CALL(addr-0x18); found=1;
        default: break;
        }
    }

    // override the checkExec reference in the module globals
    u32 checkExec = sctrlHENFindFunction("sceLoaderCore", "LoadCoreForKernel", 0xD3353EC4);
    u32 ref = findRefInGlobals("LoadCoreForKernel", checkExec, checkExec);
    _sw((unsigned int)KernelCheckExecFile, ref);
    // Flush Cache
    flushCache();

    // start the dynamic patching    
    u32 rebootex_decrypt_call = JAL(rebootex_decrypt);
    u32 rebootex_checkexec_call = JAL(rebootex_checkexec);
    for (u32 addr = start_addr; addr<topaddr; addr+=4){
        u32 data = _lw(addr);
        if (data == JAL(checkExec)){
            // Hook sceKernelCheckExecFile
            _sw(JAL(KernelCheckExecFile), addr);
        }
        else if (data == rebootex_decrypt_call){ // Not doing this will keep them pointing into Reboot Buffer... which gets unloaded...
            _sw(JAL(SonyPRXDecrypt), addr); // Fix memlmd_EF73E85B Calls that we broke intentionally in Reboot Buffer
        }
        else if (data == rebootex_checkexec_call){
            _sw(JAL(origCheckExecFile), addr); // Fix memlmd_6192F715 Calls that we broke intentionally in Reboot Buffer
        }
        else{
            switch (data){
            case 0x02E0F809: 
                // Hook sceInit StartModule Call
                _sw(JAL(patch_sceKernelStartModule_in_bootstart), addr);
                // Move Real Bootstart into Argument #1
                _sw(0x02E02021, addr+4);
                break;
            case 0x30ABFFFF:    ProbeExec1 = (void *)addr-0x100;     break;        // Executable Check Function #1
            case 0x01E63823:    ProbeExec2 = (void *)addr-0x78;      break;        // Executable Check Function #2
            case 0x30894000:    _sw(0x3C090000, addr);               break;        // Allow Syscalls
            case 0x00E8282B:    _sh(0x1000, addr + 6);               break;        // Remove POPS Check
            case 0x01A3302B:    _sw(NOP, addr+4);                    break;        // Remove Invalid PRX Type (0x80020148) Check
            }
        }
    }
    // Flush Cache
    flushCache();
    
    // Patch Relocation Type 7 to 0 (this makes more homebrews load)
    {
    u32 addr = ref; // addr = mod->text_addr would also work, we generally just want it to be pointing at the code
    while (strcmp((char*)addr, "sceSystemModule")) addr++; // scan for this string, reloc_type comes a few fixed bytes after
    _sw(_lw(addr+0x7C), addr+0x98);
    }
    
    // Flush Cache
    flushCache();
    
    // Hook Executable Checks
    for (u32 addr=start_addr; addr<topaddr; addr+=4){
        if (_lw(addr) == JAL(ProbeExec1))
            _sw(JAL(_ProbeExec1), addr);
        else if (_lw(addr) == JAL(ProbeExec2))
            _sw (JAL(_ProbeExec2), addr);
    }

    // Flush Cache
    flushCache();

    return mod;
}
