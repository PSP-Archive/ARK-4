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

#include "main.h"
#include <loadexec_patch.h>
#include "reboot.h"
#include <functions.h>
#include "kermit.h"

#define TEST_EBOOT "ms0:/EBOOT.PBP"

char* running_ark = "Running ARK-4 in ?PS? mode";

ARKConfig* ark_config = NULL;

extern void loadKernelArk();

// K.BIN entry point
void (* kEntryPoint)() = (void*)KXPLOIT_LOADADDR;

void autoDetectDevice(ARKConfig* config);
void initKxploitFile();
void kernelContentFunction(void);

// Entry Point
int exploitEntry(ARKConfig* arg0, FunctionTable* arg1) __attribute__((section(".text.startup")));
int exploitEntry(ARKConfig* arg0, FunctionTable* arg1){

    // Clear BSS Segment
    clearBSS();

    // init function table
    if (arg1 == NULL)
        scanUserFunctions();
    else
        memcpy(g_tbl, arg1, sizeof(FunctionTable));
    
    scanArkFunctions();

    // copy the path of the save
    g_tbl->config = ark_config = arg0;

    // make PRTSTR available for payloads
    g_tbl->prtstr = (void *)&PRTSTR11;

    // init screen
    initScreen(g_tbl->DisplaySetFrameBuf);

    // Output Exploit Reach Screen
    if (arg0->exec_mode == DEV_UNK) autoDetectDevice(arg0);
    running_ark[17] = (IS_PSP(arg0->exec_mode))? ' ' : 'v';
    running_ark[20] = (IS_VITA_POPS(arg0->exec_mode))? 'X':'P';
    PRTSTR(running_ark);
    
    // read kxploit file into memory and initialize it
    initKxploitFile();
    
    char* err = NULL;
    PRTSTR("Scanning stubs for kxploit...");
    if (kxf->stubScanner(g_tbl) == 0){
        // Corrupt Kernel
        PRTSTR("Doing kernel exploit...");
        if (kxf->doExploit() == 0){
            // Flush Cache
            g_tbl->KernelDcacheWritebackAll();

            // Output Loading Screen
            PRTSTR("Escalating privilages...");
            // Trigger Kernel Permission Callback
            kxf->executeKernel(KERNELIFY(&kernelContentFunction));
            err = "Could not execute kernel function";
        }
        else{
            err = "Exploit failed!";
        }
    }
    else{
        err = "Scan failed!";
    }
    
    PRTSTR1("ERROR: %s", err);
    while(1){};

    return 0;
}

void autoDetectDevice(ARKConfig* config){
    // determine if can write eboot.pbp (not possible on PS Vita)
    int test = g_tbl->IoOpen(TEST_EBOOT, PSP_O_CREAT|PSP_O_TRUNC|PSP_O_WRONLY, 0777);
    g_tbl->IoWrite(test, "test", sizeof("test"));
    g_tbl->IoClose(test);
    int res = g_tbl->IoRemove(TEST_EBOOT);
    config->exec_mode = (res < 0)? PS_VITA : PSP_ORIG;
}

void initKxploitFile(){
    char k_path[ARK_PATH_SIZE];
    strcpy(k_path, g_tbl->config->arkpath);
    strcat(k_path, K_FILE);
    PRTSTR1("Loading Kxploit at %s", k_path);
    SceUID fd = g_tbl->IoOpen(k_path, PSP_O_RDONLY, 0);
    g_tbl->IoRead(fd, (void *)KXPLOIT_LOADADDR, 0x4000);
    g_tbl->IoClose(fd);
    g_tbl->KernelDcacheWritebackAll();
    kEntryPoint(kxf);
}

// Fake K1 Kernel Setting
void setK1Kernel(void){
    // Set K1 to Kernel Value
    __asm__ (
        "nop\n"
        "lui $k1,0x0\n"
    );
}

// Kernel Permission Function
void kernelContentFunction(void){

    if (!isKernel(0)){
        return; // we don't have kernel privilages? better error out than crash
    }

    // Switch to Kernel Permission Level
    setK1Kernel();
    
    g_tbl->prtstr = KERNELIFY(g_tbl->prtstr);
    kxf->repairInstruction = KERNELIFY(kxf->repairInstruction);
    
    // move global configuration to static address in user space so that reboot and CFW modules can find it
    if (ark_config != ark_conf_backup) memcpy(ark_conf_backup, ark_config, sizeof(ARKConfig));
    
    ark_config = ark_conf_backup;
    
    PRTSTR("Scanning kernel functions");
    // get kernel functions
    scanKernelFunctions();
    
    // repair damage done by kernel exploit
    kxf->repairInstruction();

    loadKernelArk();
}

// Clear BSS Segment of Payload
void clearBSS(void){
    // BSS Start and End Address from Linkfile
    extern char __bss_start, __bss_end;
    
    // Clear Memory
    memset(&__bss_start, 0, &__bss_end - &__bss_start);
}
