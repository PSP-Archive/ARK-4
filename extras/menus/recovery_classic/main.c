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

int module_start(SceSize args, void *argp) {        


	//_sw(0x44000000, 0xBC800100);
	
    //psp_model = kuKernelGetModel();

    //sctrlHENGetArkConfig(&ark_conf);

	pspDebugScreenInit();
	SceCtrlData pad;
	int i;
	while(1) {
		//sceKernelDelayThread(50000);
		sceDisplayWaitVblankStart();	
		sceCtrlPeekBufferPositive(&pad, 1);
		if(pad.Buttons & PSP_CTRL_CIRCLE) {
			//sceKernelExitGame();
			return 0;
		}


		pspDebugScreenSetTextColor(0xff00ff00);	
		printf("********************************************************************");
		pspDebugScreenSetXY(0, 1);
		printf("* ARK-4 Classic Recovery Menu                                      *");
		for(i=2;i<SCREEN_HEIGHT;i++) {
			pspDebugScreenSetXY(0, i);
			printf("*                                                                  *");
		}
		pspDebugScreenSetXY(0, 33);
		printf("********************************************************************");
		//printf("Test");
	}
    
    //previous = sctrlHENSetStartModuleHandler(OnModuleStart);
    
    //sctrlHENGetArkConfig(ark_config);

    return 0;
}
int module_stop(SceSize args, void *argp) {
	return 0;
}
