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
#include <graphics.h>

// Exit Button Mask
#define EXIT_MASK (PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER | PSP_CTRL_START | PSP_CTRL_DOWN)

extern ARKConfig* ark_config;
extern int disable_plugins;
extern int disable_settings;

void exitLauncher()
{

    // Refuse Operation in Save dialog
	if(sceKernelFindModuleByName("sceVshSDUtility_Module") != NULL) return;
	
	// Refuse Operation in Dialog
	if(sceKernelFindModuleByName("sceDialogmain_Module") != NULL) return;

    // Load Execute Parameter
    struct SceKernelLoadExecVSHParam param;
    
    // set exit app
    char path[ARK_PATH_SIZE];
    strcpy(path, ark_config->arkpath);
    if (ark_config->recovery) strcat(path, ARK_RECOVERY);
    else if (ark_config->launcher[0]) strcat(path, ark_config->launcher);
    else strcat(path, ARK_MENU);
    
    // Clear Memory
    memset(&param, 0, sizeof(param));

    // Configure Parameters
    param.size = sizeof(param);
    param.args = strlen(path) + 1;
    param.argp = path;
    param.key = "game";

    // set default mode
    sctrlSESetUmdFile("");
    sctrlSESetBootConfFileIndex(MODE_UMD);
    
    // Trigger Reboot
    sctrlKernelLoadExecVSHWithApitype(0x141, path, &param);
}

static void startExitThread(){
	// Exit to custom launcher
	int k1 = pspSdkSetK1(0);
	int uid = sceKernelCreateThread("ExitGamePollThread", exitLauncher, 16 - 1, 2048, 0, NULL);
	sceKernelStartThread(uid, 0, NULL);
	sceKernelWaitThreadEnd(uid, NULL);
	pspSdkSetK1(k1);
}

// Gamepad Hook #1
int (*CtrlPeekBufferPositive)(SceCtrlData *, int) = NULL;
int peek_positive(SceCtrlData * pad_data, int count)
{
	// Capture Gamepad Input
	count = CtrlPeekBufferPositive(pad_data, count);
	
	// Check for Exit Mask
	if((pad_data[0].Buttons & EXIT_MASK) == EXIT_MASK)
	{
		startExitThread();
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
	if((pad_data[0].Buttons & EXIT_MASK) == 0)
	{
		startExitThread();
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
	if((pad_data[0].Buttons & EXIT_MASK) == EXIT_MASK)
	{
		startExitThread();
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
	if((pad_data[0].Buttons & EXIT_MASK) == 0)
	{
		startExitThread();
	}
	
	// Return Number of Input Frames
	return count;
}

void initController(SceModule2* mod){
	CtrlPeekBufferPositive = (void *)sctrlHENFindFunction("sceController_Service", "sceCtrl", 0x3A622550);
    CtrlPeekBufferNegative = (void *)sctrlHENFindFunction("sceController_Service", "sceCtrl", 0xC152080A);
    CtrlReadBufferPositive = (void *)sctrlHENFindFunction("sceController_Service", "sceCtrl", 0x1F803938);
    CtrlReadBufferNegative = (void *)sctrlHENFindFunction("sceController_Service", "sceCtrl", 0x60B81F86);
}

void checkControllerInput(){
	SceCtrlData pad_data;
	CtrlPeekBufferPositive(&pad_data, 1);
	if ((pad_data.Buttons & PSP_CTRL_START) == PSP_CTRL_START) disable_plugins = 1;
	if ((pad_data.Buttons & PSP_CTRL_SELECT) == PSP_CTRL_SELECT) disable_settings = 1;
}

// Hook Gamepad Input
void patchController(void)
{
	// Hook Gamepad Input Syscalls (user input hooking only)
	sctrlHENPatchSyscall((void *)CtrlPeekBufferPositive, peek_positive);
	sctrlHENPatchSyscall((void *)CtrlPeekBufferNegative, peek_negative);
	sctrlHENPatchSyscall((void *)CtrlReadBufferPositive, read_positive);
	sctrlHENPatchSyscall((void *)CtrlReadBufferNegative, read_negative);
}