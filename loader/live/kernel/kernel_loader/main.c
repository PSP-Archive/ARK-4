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
#include <functions.h>

#define TEST_EBOOT "ms0:/EBOOT.PBP"

char* running_ark = "Running ARK-4 in ?PS? mode";

ARKConfig default_config = {
    .magic = ARK_CONFIG_MAGIC,
    .arkpath = DEFAULT_ARK_PATH,
    .exploit_id = LIVE_EXPLOIT_ID,
    .launcher = {0},
    .exec_mode = DEV_UNK,
    .recovery = 0,
};
ARKConfig* ark_config = NULL;

extern void loadKernelArk();

// K.BIN entry point
void (* kEntryPoint)() = (void*)KXPLOIT_LOADADDR;

void autoDetectDevice(ARKConfig* config);
int initKxploitFile();
void kernelContentFunction(void);

static void pops_vram_handler(u32 vram){
    SoftRelocateVram(vram, NULL);
}

// Entry Point
int exploitEntry(ARKConfig* arg0, UserFunctions* arg1, char* kxploit_file) __attribute__((section(".text.startup")));
int exploitEntry(ARKConfig* arg0, UserFunctions* arg1, char* kxploit_file){

    // Clear BSS Segment
    clearBSS();

    // init function table
    if (arg1 == NULL)
        scanUserFunctions(g_tbl);
    else
        memcpy(g_tbl, arg1, sizeof(UserFunctions));
    
    if (arg0 == NULL) arg0 = &default_config;
    
    scanArkFunctions(g_tbl);

    // copy the path of the save
    g_tbl->config = ark_config = arg0;

    // make PRTSTR available for payloads
    g_tbl->prtstr = (void *)&PRTSTR11;

    // init screen
    initScreen(g_tbl->DisplaySetFrameBuf);

    if (arg0->exec_mode == DEV_UNK)
        autoDetectDevice(arg0); // attempt to autodetect configuration

    // Output Exploit Reach Screen
    running_ark[20] = 'P';
    if (IS_PSP(arg0)){
        running_ark[17] = ' '; // show 'PSP'
    }
    else{
        running_ark[17] = 'e'; // show 'ePSP'
        if (IS_VITA_POPS(arg0)){
            running_ark[20] = 'X'; // show 'ePSX'
            // configure to handle POPS screen
            initVitaPopsVram();
            setScreenHandler(&pops_vram_handler);
        }
    }
    PRTSTR(running_ark);
    
    if (isKernel()){ // already in kernel mode?
        kernelContentFunction();
        return 0;
    }
    
    // read kxploit file into memory and initialize it
    char* err = NULL;
    int res = 0;
    PRTSTR("Reading kxploit file...");
    if ((res=initKxploitFile(kxploit_file)) == 0){
        PRTSTR("Scanning stubs for kxploit...");
        if ((res=kxf->stubScanner(g_tbl)) == 0){
            // Corrupt Kernel
            PRTSTR("Doing kernel exploit...");
            if ((res=kxf->doExploit()) == 0){
                // Flush Cache
                g_tbl->KernelDcacheWritebackAll();
                // Output Loading Screen
                PRTSTR("Escalating privilages...");
                // Trigger Kernel Permission Callback
                kxf->executeKernel(KERNELIFY(&kernelContentFunction));
                err = "Could not execute kernel function";
                res = -1;
            }
            else{
                err = "Exploit failed!";
            }
        }
        else{
            err = "Scan failed!";
        }
    }
    else{
        err = "Could not open kxploit file!";
    }
    
    PRTSTR2("ERROR (%d): %s", res, err);
    while(1){};

    return res;
}

void autoDetectDevice(ARKConfig* config){
    // determine if can write eboot.pbp (not possible on PS Vita)
    int test = g_tbl->IoOpen(TEST_EBOOT, PSP_O_CREAT|PSP_O_TRUNC|PSP_O_WRONLY, 0777);
    g_tbl->IoWrite(test, "test", sizeof("test"));
    g_tbl->IoClose(test);
    int res = g_tbl->IoRemove(TEST_EBOOT);
    config->exec_mode = (res < 0)? PS_VITA : PSP_ORIG;
    // TODO: determine if VitaPops
}

int initKxploitFile(char* kxploit_file){
    char k_path[ARK_PATH_SIZE];
    if (kxploit_file == NULL){
        // try to find kxploit file in ARK install path 
        strcpy(k_path, g_tbl->config->arkpath);
        strcat(k_path, K_FILE);
    }
    else{
        // use specified kxploit file
        strcpy(k_path, kxploit_file);
    }
    PRTSTR1("Loading Kxploit at %s", k_path);
    SceUID fd = g_tbl->IoOpen(k_path, PSP_O_RDONLY, 0);
    if (fd < 0) return -1;
    g_tbl->IoRead(fd, (void *)KXPLOIT_LOADADDR, 0x4000);
    g_tbl->IoClose(fd);
    g_tbl->KernelDcacheWritebackAll();
    kEntryPoint(kxf);
    return 0;
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

    if (!isKernel()){
        return; // we don't have kernel privilages? better error out than crash
    }

    // Switch to Kernel Permission Level
    setK1Kernel();
    
    g_tbl->prtstr = KERNELIFY(g_tbl->prtstr);
    kxf->repairInstruction = KERNELIFY(kxf->repairInstruction);
    
    PRTSTR("Scanning kernel functions");
    // get kernel functions
    scanKernelFunctions(k_tbl);
    
    // repair damage done by kernel exploit
    kxf->repairInstruction(k_tbl);

    loadKernelArk();
}

// Clear BSS Segment of Payload
void clearBSS(void){
    // BSS Start and End Address from Linkfile
    extern char __bss_start, __bss_end;
    
    // Clear Memory
    memset(&__bss_start, 0, &__bss_end - &__bss_start);
}

u32 sctrlHENFindFunction(char* mod, char* lib, u32 nid){
    return FindFunction(mod, lib, nid);
}
