#include <pspsdk.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <systemctrl.h>

#include <main.h>
#include <list.h>
#include <plugins.h>

extern ARKConfig* ark_config;
extern CFWConfig config;
extern List plugins;

#define SCREEN_WIDTH 58
#define SCREEN_HEIGHT 33 

#define printf pspDebugScreenPrintf

static int plugins_to_text(char** paths, char** states, int dir){
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
    printf("********************************************************************");

    pspDebugScreenSetXY(0, 2);
    printf(header);
    pspDebugScreenSetXY(0, 3);
    printf("*                                                                  *");

    for (int i=0; i<size; i++){
        if (start+i >= osize) break;
        pspDebugScreenSetXY(0, 4 + 2*i);
        char tmp[70];
        strcpy(tmp, "* ");
        if (dir == start+i){
            strcat(tmp, "> ");
        }
        strcat(tmp, options[start+i]);

        int len = strlen(tmp);
        int padding = 60 - len;
        for (int j=0; j<padding; j++) tmp[len+j] = ' ';
        strcpy(tmp+len+padding, states[start+i]);

        len = strlen(tmp);
        padding = 67 - len;
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

void plugins_submenu(){

    SceCtrlData pad;
    int dir = 0;

    char* header = "* Plugins Manager                                                  *";
    char** paths = malloc(sizeof(char*)*plugins.count);
    char** states = malloc(sizeof(char*)*plugins.count);

    int size = plugins_to_text(paths, states, dir);

    draw_submenu(header, paths, states, size, dir);

	while(1) {

        sceDisplayWaitVblankStart();

        sceCtrlPeekBufferPositive(&pad, 1);
		
		// CONTROLS
		if(pad.Buttons & PSP_CTRL_DOWN) {
            sceKernelDelayThread(200000);
			dir++;
			if(dir>=size) dir = 0;

            draw_submenu(header, paths, states, size, dir);
		}
		if(pad.Buttons & PSP_CTRL_UP) {
            sceKernelDelayThread(200000);
			dir--;
			if(dir<0) dir = size-1;
            
            draw_submenu(header, paths, states, size, dir);
		}
		if((pad.Buttons & (PSP_CTRL_CROSS | PSP_CTRL_CIRCLE | PSP_CTRL_LEFT | PSP_CTRL_RIGHT))) {
            sceKernelDelayThread(200000);
            
            Plugin* plugin = plugins.table[dir];
            plugin->active = !plugin->active;

            size = plugins_to_text(paths, states, dir);
            
            draw_submenu(header, paths, states, size, dir);
        }
        if((pad.Buttons & PSP_CTRL_TRIANGLE)) {
            sceKernelDelayThread(200000);
            break;
        }
        
	}

}

void settings_submenu(){

}