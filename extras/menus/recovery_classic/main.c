#include <pspsdk.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <systemctrl.h>

/*#include "globals.h"
#include "macros.h"
#include "list.h"
#include "settings.h"
#include "plugins.h"
#include "../arkMenu/include/conf.h"
*/


#define SCREEN_WIDTH 58
#define SCREEN_HEIGHT 33 

#define printf pspDebugScreenPrintf

//STMOD_HANDLER previous;

PSP_MODULE_INFO("ARK_Recovery", PSP_MODULE_KERNEL, 1, 0);

//ARKConfig _arkconf;
//ARKConfig* ark_config = &_arkconf;
//extern List plugins;

/*void OnModuleStart(SceModule2 *mod) {
	// abgr
}
*/

int main_thread(SceSize args, void *argp) {        
    //psp_model = kuKernelGetModel();

    //sctrlHENGetArkConfig(&ark_conf);

	pspDebugScreenInit();
	sceDisplayWaitVblankStart();	
	SceCtrlData pad;
	int i;
	char *options[] = {
		"Back",
		"Toggle USB",
		"Run /PSP/GAME/RECOVERY/EBOOT.PBP"
	};

	int size = (sizeof(options) / sizeof(options[0]))-1;

	char selected[] = "* > ";
	char *selected_option = (char*)malloc(64);
	char *option = (char*)malloc(64);
	char special[] = "*";
	int dir = 0;
	while(1) {
		sceCtrlPeekBufferPositive(&pad, 1);
		sceDisplayWaitVblankStart();	


		// MENU

		pspDebugScreenSetTextColor(0xff00ff00);	
		printf("********************************************************************");

		pspDebugScreenSetXY(0, 1);
		printf("* ARK-4 Classic Recovery Menu                                      *");

		pspDebugScreenSetXY(0, 2);
		printf("*                                                                  *");


		// DEFAULT (FIRST) OPTION
		pspDebugScreenSetXY(0, 3);
		if(dir==0) {
			sprintf(selected_option, "%s%s", selected, "Back                                                           *");
			printf(selected_option);
		}
		else {
			sprintf(option, "%s%07s%60s", special, options[0], "*"); 
			printf(option);
		}


		// GAP FILLER BETWEEN OPTIONS
		pspDebugScreenSetXY(0, 4);
		printf("*                                                                  *");



		// SECOND OPTION
		pspDebugScreenSetXY(0, 5);
		if(dir==1) {
			sprintf(selected_option, "%s%s", selected, "Toggle USB                                                     *");
			printf(selected_option);
		}
		else {
			sprintf(option, "%s   %s%54s", special, options[1], "*"); 
			printf(option);
		}


		// GAP FILLER BETWEEN OPTIONS
		pspDebugScreenSetXY(0, 6);
		printf("*                                                                  *");


		// THIRD OPTION
		pspDebugScreenSetXY(0, 7);
		if(dir==2) {
			sprintf(selected_option, "%s%s", selected, "Run /PSP/GAME/RECOVERY/EBOOT.PBP                               *");
			printf(selected_option);
		}
		else {
			sprintf(option, "%s   %s%032s", special, options[2], "*"); 
			printf(option);
		}




		// ADD SIDE BORDERS
		for(i=8;i<SCREEN_HEIGHT;i++) {
			pspDebugScreenSetXY(0, i);
			printf("*                                                                  *");
		}

		// BOTTOM BORDER
		pspDebugScreenSetXY(0, 33);
		printf("********************************************************************");





		// CONTROLS
		if(pad.Buttons & PSP_CTRL_DOWN) {
			sceKernelDelayThread(100000);
			dir++;
			if(dir>size) dir = 0;
		}
		if(pad.Buttons & PSP_CTRL_UP) {
			sceKernelDelayThread(100000);
			dir--;
			if(dir<0) dir = size;
		}

		if((pad.Buttons & (PSP_CTRL_CROSS | PSP_CTRL_CIRCLE)) && dir == 0) {
			pspDebugScreenSetXY(25, 30);
			printf("Good-bye ;-)");
			sceKernelDelayThread(100000);
			return 0;
		}

		sceDisplayWaitVblankStart();	
	}
    
    //previous = sctrlHENSetStartModuleHandler(OnModuleStart);
    
    //sctrlHENGetArkConfig(ark_config);

	free(selected_option);
	free(option);
    return 0;
}

int module_start(int argc, char *argv[])
{
	int	thid;

	thid = sceKernelCreateThread("recovery_thread", main_thread, 16, 0x8000 , 0 ,0);

	if (thid>=0) {
		sceKernelStartThread(thid, 0, 0);
	}
	
	sceKernelWaitThreadEnd(thid, NULL);
	
	return 0;
}

int module_stop(SceSize args, void *argp) {
	return 0;
}
