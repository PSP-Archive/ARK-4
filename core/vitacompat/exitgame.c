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
#include <pspinit.h>
#include <pspkernel.h>
#include <pspctrl.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <macros.h>
#include <string.h>
#include <globals.h>
#include <functions.h>

// Exit Button Mask
#define EXIT_MASK (PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER | PSP_CTRL_START | PSP_CTRL_DOWN)

extern ARKConfig* ark_config;

// Exit to Launcher
void exitToLauncher(void)
{
	
	// Refuse Operation in Save dialog
	if(sceKernelFindModuleByName("sceVshSDUtility_Module") != NULL) return;
	
	// Refuse Operation in Dialog
	if(sceKernelFindModuleByName("sceDialogmain_Module") != NULL) return;
	
	// Load Execute Parameter
	struct SceKernelLoadExecVSHParam param;
	
	// backup configuration to user ram
	memcpy(ark_conf_backup, ark_config, sizeof(ARKConfig));
	
	// Clear Memory
	memset(&param, 0, sizeof(param));

	char path[ARK_PATH_SIZE];
	strcpy(path, ark_config->arkpath);
	strcat(path, ARK_MENU);
	
	// Configure Parameters
	param.size = sizeof(param);
	param.args = strlen(path) + 1;
	param.argp = path;
	param.key = "game";

	// Always use inferno to simulate UMD driver
	sctrlSESetBootConfFileIndex(MODE_INFERNO);
	
	// Trigger Reboot
	sctrlKernelLoadExecVSHWithApitype(0x141, path, &param);
}

// Gamepad Hook #1
int (*CtrlPeekBufferPositive)(SceCtrlData *, int) = NULL;
int peek_positive(SceCtrlData * pad_data, int count)
{
	// Capture Gamepad Input
	count = CtrlPeekBufferPositive(pad_data, count);
	
	// Check for Exit Mask
	int i = 0; for(; i < count; i++) if((pad_data[i].Buttons & EXIT_MASK) == EXIT_MASK)
	{
		// Exit to PRO VSH
		exitToLauncher();
	}
	
	// Return Number of Input Frames
	return count;
}

// Gamepad Hook #2
int (*CtrlPeekBufferNegative)(SceCtrlData *, int) = NULL;
int peek_negative(SceCtrlData * pad_data, int count)
{
	// Capture Gamepad Input
	count = CtrlPeekBufferNegative(pad_data, count);
	
	// Check for Exit Mask
	int i = 0; for(; i < count; i++) if((pad_data[i].Buttons & EXIT_MASK) == 0)
	{
		// Exit to PRO VSH
		exitToLauncher();
	}
	
	// Return Number of Input Frames
	return count;
}

// Gamepad Hook #3
int (*CtrlReadBufferPositive)(SceCtrlData *, int) = NULL;
int read_positive(SceCtrlData * pad_data, int count)
{
	// Capture Gamepad Input
	count = CtrlReadBufferPositive(pad_data, count);
	
	// Check for Exit Mask
	int i = 0; for(; i < count; i++) if((pad_data[i].Buttons & EXIT_MASK) == EXIT_MASK)
	{
		// Exit to PRO VSH
		exitToLauncher();
	}
	
	// Return Number of Input Frames
	return count;
}

// Gamepad Hook #4
int (*CtrlReadBufferNegative)(SceCtrlData *, int) = NULL;
int read_negative(SceCtrlData * pad_data, int count)
{
	// Capture Gamepad Input
	count = CtrlReadBufferNegative(pad_data, count);
	
	// Check for Exit Mask
	int i = 0; for(; i < count; i++) if((pad_data[i].Buttons & EXIT_MASK) == 0)
	{
		// Exit to PRO VSH
		exitToLauncher();
	}
	
	// Return Number of Input Frames
	return count;
}

// Gamepad Polling Thread
int control_poller(SceSize args, void * argp)
{
	// Control Buffer
	SceCtrlData pad_data[16];
	
	// Endless Loop
	while(1)
	{
		// Clear Memory
		memset(pad_data, 0, sizeof(pad_data));
		
		// Poll Gamepad
		peek_positive(pad_data, NELEMS(pad_data));
		
		// Save CPU Time (30fps)
		sceKernelDelayThread((unsigned int)(1000000.0f / 30.0f));
	}
	
	// Exit and Delete Thread
	sceKernelExitDeleteThread(0);
	
	// Never reached return (shut up compiler!)
	return 0;
}

// Hook Gamepad Input
void patchController(void)
{
    CtrlPeekBufferPositive = (void *)sctrlHENFindFunction("sceController_Service", "sceCtrl", 0x3A622550);
    CtrlPeekBufferNegative = (void *)sctrlHENFindFunction("sceController_Service", "sceCtrl", 0xC152080A);
    CtrlReadBufferPositive = (void *)sctrlHENFindFunction("sceController_Service", "sceCtrl", 0x1F803938);
    CtrlReadBufferNegative = (void *)sctrlHENFindFunction("sceController_Service", "sceCtrl", 0x60B81F86);
	// Hook Gamepad Input Syscalls (user input hooking only)
	sctrlHENPatchSyscall(CtrlPeekBufferPositive, peek_positive);
	sctrlHENPatchSyscall(CtrlPeekBufferNegative, peek_negative);
	sctrlHENPatchSyscall(CtrlReadBufferPositive, read_positive);
	sctrlHENPatchSyscall(CtrlReadBufferNegative, read_negative);
}

// Start Gamepad Polling Thread
void startControlPoller(void)
{
	// Create Thread (with USER_PRIORITY_HIGHEST - 1)
	int uid = sceKernelCreateThread("ExitGamePollThread", control_poller, 16 - 1, 1024, 0, NULL);
	
	// Created Thread Handle
	if(uid >= 0)
	{
		// Start Thread
		if(sceKernelStartThread(uid, 0, NULL) < 0)
		{
			// Delete Thread on Start Error
			sceKernelDeleteThread(uid);
		}
	}
}

// POPS Specific Hooks
void hookPOPSExit(void)
{
	// Hook scePopsManExitVSHKernel
	sctrlHENPatchSyscall((void *)sctrlHENFindFunction("scePops_Manager", "scePopsMan", 0x0090B2C8), exitToLauncher);
}

// Start exit game handler
void patchExitGame()
{
	// Get Apitype
	int apitype = sceKernelInitApitype();
	
	// Not POPS Apitype (aka. no PSP homescreen available)
	if(apitype != 0x144)
	{
		// Hook Gamepad Input
		patchController();
		
		// Start Polling Thread
		startControlPoller();
	}
	
	// POPS Apitype
	else
	{
		// Hook POPS Exit Calls
		hookPOPSExit();
	}
	
	// Flush Cache
	flushCache();
}

