#include <pspsdk.h>
#include <pspinit.h>
#include <pspkernel.h>
#include <pspctrl.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <macros.h>
#include <string.h>

// Exit Button Mask
#define EXIT_MASK (PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER | PSP_CTRL_START | PSP_CTRL_DOWN)

int (*CtrlPeekBufferPositive)(SceCtrlData *, int) = NULL;

// Gamepad Polling Thread
int control_poller(SceSize args, void * argp)
{
	// Control Buffer
	SceCtrlData pad_data;
	
	// Endless Loop
	while(1)
	{
		// Clear Memory
		memset(&pad_data, 0, sizeof(pad_data));

		// Poll Gamepad
		CtrlPeekBufferPositive(&pad_data, 1);

        if((pad_data.Buttons & EXIT_MASK) == EXIT_MASK){
            exitLauncher();
        }
		
		// Save CPU Time (30fps)
		sceKernelDelayThread((unsigned int)(1000000.0f / 30.0f));
	}
	
	// Exit and Delete Thread
	sceKernelExitDeleteThread(0);
	
	// Never reached return (shut up compiler!)
	return 0;
}

// Start Gamepad Polling Thread
void startControlPoller(void)
{

    CtrlPeekBufferPositive = (void *)sctrlHENFindFunction("sceController_Service", "sceCtrl", 0x3A622550);

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

