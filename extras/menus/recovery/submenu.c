#include <stdbool.h>
#include <pspsdk.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <systemctrl.h>

#include <main.h>
#include <list.h>
#include <plugins.h>

#include "globals.h"

extern ARKConfig* ark_config;
extern CFWConfig config;
extern List plugins;

extern int psp_model;


#define SCREEN_WIDTH 58
#define SCREEN_HEIGHT 33 

typedef struct {
    int max;
    int* value;
    char* name;
    char** opts;
} Setting;

char* ark_settings_options[] = {
    (char*)"Disabled",
    (char*)"Always",
    (char*)"Game",
    (char*)"UMD/ISO",
    (char*)"Homebrew",
    (char*)"PS1",
    (char*)"XMB",
    (char*)"Launcher"
};

#define N_OPTS sizeof(ark_settings_options)/sizeof(ark_settings_options[0])

char* ark_settings_boolean[] = {
    (char*)"Off",
    (char*)"On"
};

char* ark_settings_infernocache[] = {
    (char*)"Off",
    (char*)"LRU",
    (char*)"RR"
};

// PSP 1K
Setting settings_items_1k[] =
{
    { N_OPTS, &(config.overclock), "Overclock", ark_settings_options },
    { N_OPTS, &(config.powersave), "PowerSave", ark_settings_options },
    { N_OPTS, &(config.defaultclock), "Balanced Energy Mode", ark_settings_options },
    { 2, &(config.launcher), "Autoboot Launcher", ark_settings_boolean },
    { N_OPTS, &(config.mscache), "Memory Stick Speedup", ark_settings_options },
    { 3, &(config.infernocache), "Inferno Cache", ark_settings_infernocache },
    { 2, &(config.skiplogos), "Skip Sony Logos", ark_settings_boolean },
    { 2, &(config.hidepics), "Hide PIC0 and PIC1", ark_settings_boolean },
    { 2, &(config.hidemac), "Hide MAC Address", ark_settings_boolean },
    { 2, &(config.hidedlc), "Hide DLC", ark_settings_boolean },
    { N_OPTS, &(config.noled), "Turn off LEDs", ark_settings_options },
    { 2, &(config.noumd), "Disable UMD Drive", ark_settings_boolean },
    { 2, &(config.noanalog), "Disable Analog Stick", ark_settings_boolean },
    { 2, &(config.qaflags), "Enable QA Flags", ark_settings_boolean },
};

#define N_SETTINGS_1K (sizeof(settings_items_1k)/sizeof(settings_items_1k[0]))

// PSP Slim
Setting settings_items_slim[] =
{
    { N_OPTS, &(config.usbcharge), "USB Charge", ark_settings_options },
    { N_OPTS, &(config.overclock), "Overclock", ark_settings_options },
    { N_OPTS, &(config.powersave), "PowerSave", ark_settings_options },
    { N_OPTS, &(config.defaultclock), "Balanced Energy Mode", ark_settings_options },
    { 2, &(config.launcher), "Autoboot Launcher", ark_settings_boolean },
    { N_OPTS, &(config.highmem), "Force Extra Memory", ark_settings_options },
    { N_OPTS, &(config.mscache), "Memory Stick Speedup", ark_settings_options },
    { 3, &(config.infernocache), "Inferno Cache", ark_settings_infernocache },
    { 2, &(config.skiplogos), "Skip Sony Logos", ark_settings_boolean },
    { 2, &(config.hidepics), "Hide PIC0 and PIC1", ark_settings_boolean },
    { 2, &(config.hidemac), "Hide MAC Address", ark_settings_boolean },
    { 2, &(config.hidedlc), "Hide DLC", ark_settings_boolean },
    { N_OPTS, &(config.noled), "Turn off LEDs", ark_settings_options },
    { 2, &(config.noumd), "Disable UMD Drive", ark_settings_boolean },
    { 2, &(config.noanalog), "Disable Analog Stick", ark_settings_boolean },
    { 2, &(config.qaflags), "Enable QA Flags", ark_settings_boolean },
};

#define N_SETTINGS_SLIM (sizeof(settings_items_slim)/sizeof(settings_items_slim[0]))

// PSP GO
Setting settings_items_go[] =
{
    { N_OPTS, &(config.usbcharge), "USB Charge", ark_settings_options },
    { N_OPTS, &(config.overclock), "Overclock", ark_settings_options },
    { N_OPTS, &(config.powersave), "PowerSave", ark_settings_options },
    { N_OPTS, &(config.defaultclock), "Balanced Energy Mode", ark_settings_options },
    { 2, &(config.launcher), "Autoboot Launcher", ark_settings_boolean },
    { 2, &(config.disablepause), "Disable Pause on PSP Go", ark_settings_boolean },
    { N_OPTS, &(config.highmem), "Force Extra Memory", ark_settings_options },
    { N_OPTS, &(config.mscache), "Memory Stick Speedup", ark_settings_options },
    { 3, &(config.infernocache), "Inferno Cache", ark_settings_infernocache },
    { N_OPTS, &(config.oldplugin), "Old Plugin Support on PSP Go", ark_settings_options },
    { 2, &(config.skiplogos), "Skip Sony Logos", ark_settings_boolean },
    { 2, &(config.hidepics), "Hide PIC0 and PIC1", ark_settings_boolean },
    { 2, &(config.hibblock), "Prevent hibernation deletion on PSP Go", ark_settings_boolean },
    { 2, &(config.hidemac), "Hide MAC Address", ark_settings_boolean },
    { 2, &(config.hidedlc), "Hide DLC", ark_settings_boolean },
    { N_OPTS, &(config.noled), "Turn off LEDs", ark_settings_options },
    { 2, &(config.noanalog), "Disable Analog Stick", ark_settings_boolean },
    { 2, &(config.qaflags), "Enable QA Flags", ark_settings_boolean },
};

#define N_SETTINGS_GO (sizeof(settings_items_go)/sizeof(settings_items_go[0]))

// PSP 110000 (Street)
Setting settings_items_street[] =
{
    { N_OPTS, &(config.usbcharge), "USB Charge", ark_settings_options },
    { N_OPTS, &(config.overclock), "Overclock", ark_settings_options },
    { N_OPTS, &(config.powersave), "PowerSave", ark_settings_options },
    { N_OPTS, &(config.defaultclock), "Balanced Energy Mode", ark_settings_options },
    { 2, &(config.launcher), "Autoboot Launcher", ark_settings_boolean },
    { 2, &(config.disablepause), "Disable Pause on PSP Go", ark_settings_boolean },
    { N_OPTS, &(config.highmem), "Force Extra Memory", ark_settings_options },
    { N_OPTS, &(config.mscache), "Memory Stick Speedup", ark_settings_options },
    { 3, &(config.infernocache), "Inferno Cache", ark_settings_infernocache },
    { 2, &(config.skiplogos), "Skip Sony Logos", ark_settings_boolean },
    { 2, &(config.hidepics), "Hide PIC0 and PIC1", ark_settings_boolean },
    { 2, &(config.hidedlc), "Hide DLC", ark_settings_boolean },
    { N_OPTS, &(config.noled), "Turn off LEDs", ark_settings_options },
    { 2, &(config.noumd), "Disable UMD Drive", ark_settings_boolean },
    { 2, &(config.noanalog), "Disable Analog Stick", ark_settings_boolean },
    { 2, &(config.qaflags), "Enable QA Flags", ark_settings_boolean },
};

#define N_SETTINGS_STREET (sizeof(settings_items_street)/sizeof(settings_items_street[0]))

Setting settings_items_adr[] =
{
    { N_OPTS, &(config.overclock), "PSP Overclock", ark_settings_options },
    { N_OPTS, &(config.powersave), "PowerSave", ark_settings_options },
    { N_OPTS, &(config.defaultclock), "Balanced Energy Mode", ark_settings_options },
    { 2, &(config.launcher), "Autoboot Launcher", ark_settings_boolean },
    { N_OPTS, &(config.highmem), "Force Extra Memory", ark_settings_options },
    { N_OPTS, &(config.mscache), "Memory Stick Speedup", ark_settings_options },
    { 3, &(config.infernocache), "Inferno Cache", ark_settings_infernocache },
    { 2, &(config.skiplogos), "Skip Sony Logos", ark_settings_boolean },
    { 2, &(config.hidepics), "Hide PIC0 and PIC1", ark_settings_boolean },
    { 2, &(config.hidemac), "Hide MAC Address", ark_settings_boolean },
    { 2, &(config.hidedlc), "Hide DLC", ark_settings_boolean },
    { N_OPTS, &(config.noled), "Turn off LEDs", ark_settings_options },
    { 2, &(config.noumd), "Disable UMD Drive", ark_settings_boolean },
    { 2, &(config.noanalog), "Disable Analog Stick", ark_settings_boolean },
    { 2, &(config.qaflags), "QA Flags", ark_settings_boolean },
};

#define N_SETTINGS_ADR (sizeof(settings_items_adr)/sizeof(settings_items_adr[0]))

Setting settings_items_vita[] =
{
    { N_OPTS, &(config.overclock), "PSP Overclock", ark_settings_options },
    { N_OPTS, &(config.powersave), "PowerSave", ark_settings_options },
    { N_OPTS, &(config.defaultclock), "Balanced Energy Mode", ark_settings_options },
    { N_OPTS, &(config.mscache), "Memory Stick Speedup", ark_settings_options },
    { 3, &(config.infernocache), "Inferno Cache", ark_settings_infernocache },
};

#define N_SETTINGS_VITA (sizeof(settings_items_vita)/sizeof(settings_items_vita[0]))

void logtext(char* text){
    int fd = sceIoOpen("ms0:/log.txt", PSP_O_WRONLY|PSP_O_APPEND|PSP_O_CREAT, 0777);
    sceIoWrite(fd, text, strlen(text));
    sceIoClose(fd);
}

static void settings_to_text(char** names, char** states, Setting* settings_items, int size){
    for(int i = 0; i < size; i++){
        names[i] = settings_items[i].name;
        states[i] = settings_items[i].opts[*(settings_items[i].value)];
    }
}

static int plugins_to_text(char** paths, char** states, int* pos, int dir){
    int ret = 0;
    for (int i=0; i<plugins.count; i++){
        Plugin* plugin = plugins.table[i];
        if (plugin->name != NULL){
            if (dir == ret){
                paths[ret] = plugin->path;
            }
            else{
                paths[ret] = strrchr(plugin->path, '/') + 1;
            }
            pos[ret] = i;
            states[ret++] = (plugin->active)? "On" : "Off";
        }
    }
    return ret;
}

static void draw_submenu(char* header, char** options, char** states, int size, int dir){
    int start = 0;
    int osize = size;
    if (size > (SCREEN_HEIGHT/2)-3){
        size = (SCREEN_HEIGHT/2)-3;
    }

    if (dir >= size){
        start = dir;
    }

    pspDebugScreenSetXY(0, 1);
    pspDebugScreenSetTextColor(0xFFD800);
    pspDebugScreenPrintf("********************************************************************");

    pspDebugScreenSetXY(0, 2);
    pspDebugScreenPrintf("* ");
    pspDebugScreenPrintf(header);
    pspDebugScreenPrintf(" *");
    pspDebugScreenSetXY(66, 2);
    pspDebugScreenPrintf(" *");
    pspDebugScreenSetXY(0, 3);
    for (int i=0; i<strlen(header)+4; i++) pspDebugScreenPrintf("*");
    pspDebugScreenSetXY(66, 3);
    pspDebugScreenPrintf(" *");
    pspDebugScreenSetXY(0, 4);
    pspDebugScreenPrintf("*                                                                  *");

    for (int i=0; i<size; i++){
        if (start+i >= osize) break;
        pspDebugScreenSetTextColor(0xFFD800);
        pspDebugScreenSetXY(0, 5 + 2*i);
        pspDebugScreenPrintf("* ");
        if (dir != start+i){
            pspDebugScreenSetTextColor(0xFFFFFF);
        }

        char tmp[53];
        strncpy(tmp, options[start+i], sizeof(tmp)-1);
        if (strlen(options[start+i]) > sizeof(tmp)-1) strcpy(tmp+sizeof(tmp)-4, "...");
        tmp[sizeof(tmp)-1] = 0;
        pspDebugScreenPrintf(tmp);

        int len = strlen(tmp)+2;
        int padding = 55 - len;
        for (int j=0; j<padding; j++) pspDebugScreenPrintf(" ");
        pspDebugScreenPrintf(states[start+i]);

        len += (padding + strlen(states[start+i]));
        padding = 67 - len;
        for (int j=0; j<padding; j++) pspDebugScreenPrintf(" ");
        pspDebugScreenSetTextColor(0xFFD800);
        pspDebugScreenPrintf("*");

        pspDebugScreenSetXY(0, 6 + 2*i);            
        pspDebugScreenPrintf("*                                                                  *");
    }

    // ADD SIDE BORDERS
    for (int i=pspDebugScreenGetY(); i<SCREEN_HEIGHT; i++) {
        pspDebugScreenSetXY(0, i);
        pspDebugScreenPrintf("*                                                                  *");
    }

    // BOTTOM BORDER
    pspDebugScreenSetXY(0, 33);
    pspDebugScreenPrintf("********************************************************************");
}

void plugins_submenu(){

    SceCtrlData pad;
    int dir = 0;

    char* header = "Plugins Manager";
    char** paths = malloc(sizeof(char*)*plugins.count);
    char** states = malloc(sizeof(char*)*plugins.count);
    int* pos = malloc(sizeof(char*)*plugins.count);

    int size = plugins_to_text(paths, states, pos, dir);

    draw_submenu(header, paths, states, size, dir);

	while(1) {

        sceDisplayWaitVblankStart();

        sceCtrlPeekBufferPositive(&pad, 1);
		
		// CONTROLS
		if((pad.Buttons & PSP_CTRL_DOWN) == PSP_CTRL_DOWN) {
            sceKernelDelayThread(200000);
			dir++;
			if(dir>=size) dir = 0;

            //logtext("plugins_to_text\n");
            size = plugins_to_text(paths, states, pos, dir);
            //logtext("draw_submenu\n");
            draw_submenu(header, paths, states, size, dir);
            //logtext("done\n");
		}
		if((pad.Buttons & PSP_CTRL_UP) == PSP_CTRL_UP) {
            sceKernelDelayThread(200000);
			dir--;
			if(dir<0) dir = size-1;
            
            size = plugins_to_text(paths, states, pos, dir);
            draw_submenu(header, paths, states, size, dir);
		}
		if( (pad.Buttons & PSP_CTRL_CROSS) == PSP_CTRL_CROSS
              || (pad.Buttons & PSP_CTRL_CIRCLE) == PSP_CTRL_CIRCLE
              || (pad.Buttons & PSP_CTRL_LEFT) == PSP_CTRL_LEFT
              || (pad.Buttons & PSP_CTRL_RIGHT) == PSP_CTRL_RIGHT) {
            sceKernelDelayThread(200000);
            
            Plugin* plugin = plugins.table[pos[dir]];
            plugin->active = !plugin->active;

            size = plugins_to_text(paths, states, pos, dir);
            draw_submenu(header, paths, states, size, dir);
        }
        if((pad.Buttons & PSP_CTRL_TRIANGLE) == PSP_CTRL_TRIANGLE) {
            sceKernelDelayThread(200000);
            break;
        }
        
	}

    free(paths);
    free(states);
    free(pos);
}

void settings_submenu(){
    SceCtrlData pad;
    int dir = 0;
	char** paths;
	char** states;
	int size;
    Setting* settings_items;
    char* header = "Custom Firmware Settings";

	if(IS_VITA(ark_config)) {
		if(IS_VITA_ADR(ark_config)) {
			size = N_SETTINGS_ADR;
			settings_items = settings_items_adr;
		}
		else {
			size = N_SETTINGS_VITA;
			settings_items = settings_items_vita;
		}
	}
	else if(psp_model == PSP_1000) {
    	size = N_SETTINGS_1K;
        settings_items = settings_items_1k;
	}

	else if(psp_model == PSP_GO) {
    	size = N_SETTINGS_GO;
        settings_items = settings_items_go;
	}
	else if(psp_model == PSP_11000) {
    	size = N_SETTINGS_STREET;
        settings_items = settings_items_street;
	}
	else {
    	size = N_SETTINGS_SLIM;
        settings_items = settings_items_slim;
    }

    paths = malloc(sizeof(char*)*size);
	states = malloc(sizeof(char*)*size);

    settings_to_text(paths, states, settings_items, size);

    draw_submenu(header, paths, states, size, dir);

	while(1) {

        sceDisplayWaitVblankStart();

        sceCtrlPeekBufferPositive(&pad, 1);
		
		// CONTROLS
		if((pad.Buttons & PSP_CTRL_DOWN) == PSP_CTRL_DOWN) {
            sceKernelDelayThread(200000);
			dir++;
			if(dir>=size) dir = 0;

            draw_submenu(header, paths, states, size, dir);
		}
		if((pad.Buttons & PSP_CTRL_UP) == PSP_CTRL_UP) {
            sceKernelDelayThread(200000);
			dir--;
			if(dir<0) dir = size-1;
            
            draw_submenu(header, paths, states, size, dir);
		}
        if((pad.Buttons & PSP_CTRL_LEFT) == PSP_CTRL_LEFT) {
            sceKernelDelayThread(200000);
			int* value = settings_items[dir].value;
			int max = settings_items[dir].max;
            *value = (*value) - 1;
			if(*value<0) *value = max-1;
            
            settings_to_text(paths, states, settings_items, size);
            draw_submenu(header, paths, states, size, dir);
		}
        if((pad.Buttons & PSP_CTRL_CROSS) == PSP_CTRL_CROSS
              || (pad.Buttons & PSP_CTRL_CIRCLE) == PSP_CTRL_CIRCLE
              || (pad.Buttons & PSP_CTRL_RIGHT) == PSP_CTRL_RIGHT) {
            sceKernelDelayThread(200000);
			int* value = settings_items[dir].value;
			int max = settings_items[dir].max;
            *value = (*value) + 1;
			if(*value>=max) *value = 0;
            
            settings_to_text(paths, states, settings_items, size);
            draw_submenu(header, paths, states, size, dir);
		}
        if((pad.Buttons & PSP_CTRL_TRIANGLE) == PSP_CTRL_TRIANGLE) {
            sceKernelDelayThread(200000);
            break;
        }
        
	}

    free(paths);
    free(states);
}
