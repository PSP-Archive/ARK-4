#include <pspsdk.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <systemctrl.h>

#include <main.h>

#define SCREEN_WIDTH 58
#define SCREEN_HEIGHT 33 

#define printf pspDebugScreenPrintf

//STMOD_HANDLER previous;

PSP_MODULE_INFO("ClassicRecovery", PSP_MODULE_USER, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER | PSP_THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(4096);

int psp_model;
ARKConfig _arkconf;
ARKConfig* ark_config = &_arkconf;
CFWConfig config;

extern int usb_is_enabled;
extern int proshell_main();

static int launchRecoveryApp(){
	struct SceKernelLoadExecVSHParam param;
	const char *p = "ms0:/PSP/GAME/RECOVERY/EBOOT.PBP";
	int apitype = 0x141;

	memset(&param, 0, sizeof(param));
	param.size = sizeof(param);
	param.args = strlen(p) + 1;
	param.argp = p;
	param.key = "game";
	sctrlKernelLoadExecVSHWithApitype(apitype, p, &param);

	// SHOULD NOT REALLY GET HERE
	return 0;
}

static int selected_choice(u32 choice) {
    int ret;

    switch (choice){

    case 0:
        pspDebugScreenSetXY(25, 30);
        printf("Good-bye ;-)");
        sceKernelDelayThread(100000);
        return 0;
    case 1:
        //TODO USB Toggle
        pspDebugScreenSetXY(25, 30);
        if (usb_is_enabled){
            printf("Disabling USB...");
            USB_disable();
        }
        else{
            printf("Enabling USB...");
            USB_enable();
        }
        sceKernelDelayThread(1000000);
        return 1;
    case 2:
        loadSettings();
        settings_submenu();
        saveSettings();
        return 1;
    case 3:
        loadPlugins();
        plugins_submenu();
        savePlugins();
        return 1;
    case 4:
		proshell_main();
        return 1;
    case 5:
		pspDebugScreenSetXY(20, 30);
		printf("Booting RECOVERY/EBOOT.PBP");
        sceKernelDelayThread(2000000);
        launchRecoveryApp();
        return 0;
    }

}

static void draw(char** options, int size, int dir){
    pspDebugScreenSetXY(0, 1);
    pspDebugScreenSetTextColor(0xFFD800);
    printf("********************************************************************");

    pspDebugScreenSetXY(0, 2);
    printf("* ARK-4 Classic Recovery Menu                                      *");
    pspDebugScreenSetXY(0, 3);
    printf("*                                                                  *");

    for (int i=0; i<=size; i++){
        pspDebugScreenSetXY(0, 4 + 2*i);
        char tmp[70];
        strcpy(tmp, "* ");
        if (dir == i){
            strcat(tmp, "> ");
        }
        strcat(tmp, options[i]);
        int len = strlen(tmp);
        int padding = 67 - len;
        for (int j=0; j<padding; j++) tmp[len+j] = ' ';
        tmp[len+padding] = '*';
        tmp[len+padding+1] = 0;
        printf(tmp);

        pspDebugScreenSetXY(0, 5 + 2*i);            
        printf("*                                                                  *");
    }

    // ADD SIDE BORDERS
    for (int i=pspDebugScreenGetY(); i<SCREEN_HEIGHT; i++) {
        pspDebugScreenSetXY(0, i);
        printf("*                                                                  *");
    }

    // BOTTOM BORDER
    pspDebugScreenSetXY(0, 33);
    printf("********************************************************************");
}

int main(SceSize args, void *argp) {

    psp_model = kuKernelGetModel();

    sctrlHENGetArkConfig(ark_config);

	pspDebugScreenInit();

	SceCtrlData pad;
    char *options[] = {
        "Exit",
        "Toggle USB",
        "Custom Firmware Settings",
        "Plugins Manager",
        "PRO Shell",
        "Run /PSP/GAME/RECOVERY/EBOOT.PBP"
    };

	int size = (sizeof(options) / sizeof(options[0]))-1;
	int dir = 0;

    draw(options, size, dir);

	while(1) {

        sceDisplayWaitVblankStart();

        sceCtrlPeekBufferPositive(&pad, 1);
		
		// CONTROLS
		if(pad.Buttons & PSP_CTRL_DOWN) {
            sceKernelDelayThread(200000);
			dir++;
			if(dir>size) dir = 0;

            draw(options, size, dir);
		}
		if(pad.Buttons & PSP_CTRL_UP) {
            sceKernelDelayThread(200000);
			dir--;
			if(dir<0) dir = size;
            
            draw(options, size, dir);
		}
		if((pad.Buttons & (PSP_CTRL_CROSS | PSP_CTRL_CIRCLE))) {
            sceKernelDelayThread(200000);
            int ret = selected_choice(dir);
            if(ret==0) break;
            
            
            draw(options, size, dir);
			//else return ret;
        }
	}

    sceKernelExitGame();
    return 0;
}
