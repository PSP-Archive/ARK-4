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
extern SEConfig se_config;
extern int disable_plugins;
extern int disable_settings;

int exitLauncher()
{

	int k1 = pspSdkSetK1(0);

    // Refuse Operation in Save dialog
	if(sceKernelFindModuleByName("sceVshSDUtility_Module") != NULL) return 0;
	
	// Refuse Operation in Dialog
	if(sceKernelFindModuleByName("sceDialogmain_Module") != NULL) return 0;

    // Load Execute Parameter
    struct SceKernelLoadExecVSHParam param;

    // set exit app
    char path[ARK_PATH_SIZE];
    strcpy(path, ark_config->arkpath);
    if (ark_config->recovery) strcat(path, ARK_RECOVERY);
    else if (ark_config->launcher[0]) strcat(path, ark_config->launcher);
    else strcat(path, VBOOT_PBP);

	// clear screen on PS1 games
	if (sceKernelInitKeyConfig() == PSP_INIT_KEYCONFIG_POPS){
		memset(0x44000000, 0, 512 * 272 * 4);
		_sw(0x44000000, 0xBC800100);
	}

	SceIoStat stat; int res = sceIoGetstat(path, &stat);

	if (res >= 0){
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
		ark_config->recovery = 0; // reset recovery mode for next reboot
		sctrlKernelLoadExecVSHWithApitype(0x141, path, &param);
	}
	else if (ark_config->recovery || (strcmp(ark_config->launcher, "PROSHELL") == 0)){
		// no recovery app? try classic module
		strcpy(path, ark_config->arkpath);
		strcat(path, RECOVERY_PRX);
		res = sceIoGetstat(path, &stat);
		if (res < 0){
			// try flash0
			strcpy(path, RECOVERY_PRX_FLASH);
		}
		SceUID modid = sceKernelLoadModule(path, 0, NULL);
		if(modid >= 0) {
			sceKernelStartModule(modid, strlen(path) + 1, path, NULL, NULL);
			ark_config->recovery = 0; // reset recovery mode for next reboot
			ark_config->launcher[0] = 0; // reset launcher mode for next reboot
			pspSdkSetK1(k1);
			return 0;
		}
	}

	ark_config->recovery = 0;
	return sctrlKernelExitVSH(NULL);
}

static void startExitThread(){
	// Exit to custom launcher
	int k1 = pspSdkSetK1(0);
	int intc = pspSdkDisableInterrupts();
	if (sctrlGetThreadUIDByName("ExitGamePollThread") >= 0){
		pspSdkEnableInterrupts(intc);
		return; // already exiting
	}
	int uid = sceKernelCreateThread("ExitGamePollThread", exitLauncher, 1, 4096, 0, NULL);
	pspSdkEnableInterrupts(intc);
	sceKernelStartThread(uid, 0, NULL);
	sceKernelWaitThreadEnd(uid, NULL);
	sceKernelDeleteThread(uid);
	pspSdkSetK1(k1);
}

int (*CtrlSetSamplingMode)(int) = NULL;
int (*CtrlSetSamplingCycle)(int) = NULL;
static void remove_analog_input(SceCtrlData *data)
{
	if(data == NULL)
		return;
	// wat???
	if(!se_config.noanalog) {
		CtrlSetSamplingCycle(0);	
		CtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
	}
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

	if (se_config.noanalog){
		remove_analog_input(pad_data);
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

	if (se_config.noanalog){
		remove_analog_input(pad_data);
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

	if (se_config.noanalog){
		remove_analog_input(pad_data);
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

	if (se_config.noanalog){
		remove_analog_input(pad_data);
	}
	
	// Return Number of Input Frames
	return count;
}
static int isRecoveryMode(){
    if (ark_config->recovery) return 1; // recovery mode set
	char* filename = sceKernelInitFileName();
	if (filename == NULL) return 0; // not running any app
	// check if running a recovery app
    return (strstr(filename, "RECOVERY") != NULL || strstr(filename, "recovery") != NULL);
}

void checkControllerInput(){
	if (isRecoveryMode()){
		disable_plugins = 1;
		disable_settings = 1;
	}
	else {
		SceCtrlData pad_data;
		CtrlPeekBufferPositive(&pad_data, 1);
		if ((pad_data.Buttons & PSP_CTRL_START) == PSP_CTRL_START) disable_plugins = 1;
		if ((pad_data.Buttons & PSP_CTRL_SELECT) == PSP_CTRL_SELECT) disable_settings = 1;
		if ((pad_data.Buttons & PSP_CTRL_RTRIGGER) == PSP_CTRL_RTRIGGER) ark_config->recovery = 1;
	}
}

// Hook Gamepad Input
void patchController(SceModule2* mod)
{

	CtrlPeekBufferPositive = (void *)sctrlHENFindFunction("sceController_Service", "sceCtrl_driver", 0x3A622550);
    CtrlPeekBufferNegative = (void *)sctrlHENFindFunction("sceController_Service", "sceCtrl_driver", 0xC152080A);
    CtrlReadBufferPositive = (void *)sctrlHENFindFunction("sceController_Service", "sceCtrl_driver", 0x1F803938);
    CtrlReadBufferNegative = (void *)sctrlHENFindFunction("sceController_Service", "sceCtrl_driver", 0x60B81F86);
    CtrlSetSamplingCycle = (void *)sctrlHENFindFunction("sceController_Service", "sceCtrl", 0x6A2774F3);
    CtrlSetSamplingMode = (void *)sctrlHENFindFunction("sceController_Service", "sceCtrl", 0x1F4011E6);

	// Hook Gamepad Input
	HIJACK_FUNCTION(CtrlPeekBufferPositive, peek_positive, CtrlPeekBufferPositive);
	HIJACK_FUNCTION(CtrlPeekBufferNegative, peek_negative, CtrlPeekBufferNegative);
	HIJACK_FUNCTION(CtrlReadBufferPositive, read_positive, CtrlReadBufferPositive);
	HIJACK_FUNCTION(CtrlReadBufferNegative, read_negative, CtrlReadBufferNegative);
	HIJACK_FUNCTION(CtrlSetSamplingCycle, remove_analog_input, CtrlSetSamplingCycle);
	HIJACK_FUNCTION(CtrlSetSamplingMode, remove_analog_input, CtrlSetSamplingMode);
}
