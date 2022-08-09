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
#define RESET_MASK (PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER | PSP_CTRL_START | PSP_CTRL_UP)

extern ARKConfig* ark_config;

extern void sctrlExitToLauncher();

char* param_key[] = {
    "vsh", "game", "pops"
};

void exitLauncher()
{

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
    param.key = param_key[1];

    // set default mode
    sctrlSESetUmdFile("");
    sctrlSESetBootConfFileIndex(MODE_UMD);
    
    // Trigger Reboot
    sctrlKernelLoadExecVSHWithApitype(0x141, path, &param);
}

void resetGame()
{

    // Load Execute Parameter
    struct SceKernelLoadExecVSHParam param;

    int apitype = sceKernelInitApitype();
    char path[ARK_PATH_SIZE];
    strcpy(path, sceKernelInitFileName());

    // Clear Memory
    memset(&param, 0, sizeof(param));

    // Configure Parameters
    param.size = sizeof(param);
    param.argp = path;
    param.args = strlen(param.argp) + 1;
    if (apitype&0x200 == 0x200) param.key = param_key[0];
    else if (apitype == 0x144 || apitype == 0x155) param.key = param_key[2];
    else param.key = param_key[1];
    
    // Trigger Reboot
    sctrlKernelLoadExecVSHWithApitype(apitype, param.argp, &param);
}

// Gamepad Polling Thread
int control_poller(SceSize args, void * argp)
{
    // Control Buffer
    SceCtrlData pad_data[16];
    
    int (*CtrlPeekBufferPositive)(SceCtrlData *, int) = NULL;
    
    // Endless Loop
    while(1)
    {
        // Save CPU Time (30fps)
        sceKernelDelayThread((unsigned int)(1000000.0f / 30.0f));
        
        // Refuse Operation in Save dialog
        if(sceKernelFindModuleByName("sceVshSDUtility_Module") != NULL) continue;
        
        // Refuse Operation in Dialog
        if(sceKernelFindModuleByName("sceDialogmain_Module") != NULL) continue;
    
        if (CtrlPeekBufferPositive == NULL){
            CtrlPeekBufferPositive = (void *)sctrlHENFindFunction("sceController_Service", "sceCtrl", 0x3A622550);
        }
        else{
            // Clear Memory
            memset(pad_data, 0, sizeof(pad_data));
            
            // Poll Gamepad
            int count = CtrlPeekBufferPositive(pad_data, NELEMS(pad_data));
            // Check for Exit Mask
            for(int i = 0; i < count; i++)
            {
                // Execute launcher
                if((pad_data[i].Buttons & EXIT_MASK) == EXIT_MASK){
                    if (ark_config->launcher[0]){
                        exitLauncher();
                    }
                    else{
                        sctrlKernelExitVSH(NULL);
                    }
                }
                else if ((pad_data[i].Buttons & RESET_MASK) == RESET_MASK){
                    resetGame();
                }
            }
        }
    }
    
    // Exit and Delete Thread
    sceKernelExitDeleteThread(0);
    
    // Never reached return (shut up compiler!)
    return 0;
}

// Start exit game handler
void patchExitGame()
{
    // Get Apitype
    int apitype = sceKernelInitApitype();
    
    if (apitype !=  0x210 && apitype !=  0x220){ // Do nothing on VSH
        // Start Polling Thread
        // Create Thread (with USER_PRIORITY_HIGHEST - 1)
        int uid = sceKernelCreateThread("ExitGamePollThread", control_poller, 16 - 1, 2048, PSP_THREAD_ATTR_VFPU, NULL);
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
}

