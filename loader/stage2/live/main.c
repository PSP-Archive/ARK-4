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
#include "libs/graphics/graphics.h"
#include "flash_dump.h"
#include "reboot.h"
#include <functions.h>

//char* savepath = (char*)0x08803000;
char* running_ark = "Running ARK-4 in ?PS? mode";
char* test_eboot = "ms0:/EBOOT.PBP";
char* menu_boot = "?BOOT.PBP";

// Sony Reboot Buffer Loader
int (* _LoadReboot)(void *, unsigned int, void *, unsigned int) = NULL;

// LoadExecVSHWithApitype Direct Call
int (* _KernelLoadExecVSHWithApitype)(int, char *, struct SceKernelLoadExecVSHParam *, int) = NULL;

// K.BIN entry point
void (* kEntryPoint)() = (void*)KXPLOIT_LOADADDR;

void initKxploitFile(){
	char k_path[SAVE_PATH_SIZE+10];
	strcpy(k_path, g_tbl->config->arkpath);
	strcat(k_path, "K.BIN");
	PRTSTR(k_path);
	SceUID fd = g_tbl->IoOpen(k_path, PSP_O_RDONLY, 0);
	PRTSTR1("Read: %d", g_tbl->IoRead(fd, (void *)KXPLOIT_LOADADDR, 0x4000));
	g_tbl->IoClose(fd);
	g_tbl->KernelDcacheWritebackAll();
	PRTSTR("Init Kxploit");
	kEntryPoint();
}

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
    
    getArkFunctions();

    // copy the path of the save
	g_tbl->config = arg0;

	// make PRTSTR available for payloads
	g_tbl->prtstr = (void *)&PRTSTR11;

	// init screen
	initScreen(g_tbl->DisplaySetFrameBuf);

	// Output Exploit Reach Screen
	PRTSTR("Starting");
	
	// read kxploit file into memory and initialize it
	initKxploitFile();
	
	PRTSTR("Doing kernel exploit");

    char* err = NULL;	
	if (kxf->stubScanner() == 0){
		// Corrupt Kernel
		if (kxf->doExploit() >= 0){
			// Flush Cache
			g_tbl->KernelDcacheWritebackAll();

			// Refresh screen (vram buffer screws it)
			cls();
			
			// Output Loading Screen
			PRTSTR("Loading");
			
			// Trigger Kernel Permission Callback
			kxf->executeKernel(KERNELIFY(&kernelContentFunction));
		}
		else{
			err = "Exploit failed";
		}
	}
	else{
		err = "Scan failed";
	}
	
	PRTSTR(err);
	while(1){};

	return 0;
}

// Kernel Permission Function
void kernelContentFunction(void){
	// Switch to Kernel Permission Level
	setK1Kernel();
	
	kernelifyArkFunctions();
	
	PRTSTR("Kernel mode reached");

	kxf->repairInstruction();

    // determine global configuration that affects how ARK behaves
	memcpy(ark_config, KERNELIFY(g_tbl->config), sizeof(ARKConfig));
	
	PRTSTR("Scanning kernel functions");
	// get kernel functions
	getKernelFunctions();
	
	#if FLASH_DUMP == 1
	initKernelThread();
	return;
	#else
	running_ark[17] = (IS_PSP)? ' ' : 'e';
	running_ark[20] = (IS_VITA_POPS)? 'X':'P';
	PRTSTR(running_ark);

	// Find LoadExec Module
	SceModule2 * loadexec = k_tbl->KernelFindModuleByName("sceLoadExec");
	
	// Find Reboot Loader Function
	_LoadReboot = (void *)loadexec->text_addr;
	
	// Find LoadExec Functions
	_KernelLoadExecVSHWithApitype = (void *)findFirstJALForFunction("sceLoadExec", "LoadExecForKernel", 0xD8320A28);
	
	// make the common loadexec patches
	patchLoadExecCommon(loadexec, (u32)LoadReboot);

	flashPatch();

	// Invalidate Cache
	k_tbl->KernelIcacheInvalidateAll();
	k_tbl->KernelDcacheWritebackInvalidateAll();


	// Prepare Homebrew Reboot
	char * ebootpath = ark_config->menupath;
	struct SceKernelLoadExecVSHParam param;
	memset(&param, 0, sizeof(param));
	param.size = sizeof(param);
	param.args = strlen(ebootpath) + 1;
	param.argp = ebootpath;
	param.key = "game";

	// Trigger Reboot
	/*
	void (*KernelExitGame)() = FindFunction("sceLoadExec", "LoadExecForUser", 0x05572A5F);
	PRTSTR1("Trigger reboot at %p", KernelExitGame);
	KernelExitGame();
	*/
	PRTSTR1("Running Menu at %s", ebootpath);
	//PRTSTR1("LoadExec at %p", _KernelLoadExecVSHWithApitype);
	//PRTSTR1("Sysmem 1: %p", _lw(0x88000000+0x0000CBB8));
	//PRTSTR1("Sysmem 2: %p", _lw(0x88000000+0x0000CBF0));
	_KernelLoadExecVSHWithApitype(0x141, ebootpath, KERNELIFY(&param), 0x10000);
	#endif
}

// Fake K1 Kernel Setting
void setK1Kernel(void){
	// Set K1 to Kernel Value
	__asm__ (
		"nop\n"
		"lui $k1,0x0\n"
	);
}

// Clear BSS Segment of Payload
void clearBSS(void){
	// BSS Start and End Address from Linkfile
	extern char __bss_start, __bss_end;
	
	// Clear Memory
	memset(&__bss_start, 0, &__bss_end - &__bss_start);
}
