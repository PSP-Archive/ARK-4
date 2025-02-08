// LightSpeed Installer - 2011, The Z & frostegater

#include <pspsdk.h>
#include <pspkernel.h>
#include <pspctrl.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <kubridge.h>

#include "660launcher.h"
#include "kbin.h"
#include "660launcherDC.h"
#include "660_sm_X_plugin.h"
#include "660_sm_X_icon.h"
#include "660_sm_go_plugin.h"
#include "660_sm_go_icon.h"
#include "660_orig_X_plugin.h"
#include "660_orig_X_icon.h"
#include "660_orig_go_plugin.h"
#include "660_orig_go_icon.h"
#include "660_dc_X_plugin.h"
#include "660_dc_X_icon.h"
#include "660_dc_go_plugin.h"
#include "660_dc_go_icon.h"

PSP_MODULE_INFO("FinalSpeed", 0x0800, 9, 9); 
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VSH);

#define printf pspDebugScreenPrintf
#define setc pspDebugScreenSetXY
#define setcolor pspDebugScreenSetTextColor
#define setbcolor pspDebugScreenSetBackColor

#define RGB(r, g, b) (0xFF000000 | ((b)<<16) | ((g)<<8) | (r))
#define RED RGB(255, 0, 0)
#define GREEN RGB(0, 255, 0)
#define BLUE RGB(0, 0, 255)
#define BLACK RGB(0, 0, 0)
#define ORANGE RGB(255, 127, 0)
#define WHITE RGB(255, 255, 255)
#define YELLOW RGB(255, 255, 0)
#define GRAY RGB(103, 120, 137)

void Exit(char *fmt, ...)
{
    char msg[256];
    va_list list;
    va_start(list, fmt);
    vsprintf(msg, fmt, list);
    va_end(list);
    printf("%s\n Press X to exit.", msg);

    sceKernelDelayThread(1000 * 100);
    while(1)
    {
    	SceCtrlData pad;
    	sceCtrlReadBufferPositive(&pad, 1);

    	if(pad.Buttons & PSP_CTRL_CROSS)
    		break;
    }

    sceKernelExitGame(); 
}

int flash_file(char *file, void *file_name,  int file_size) 
{
    sceIoRemove(file);
    printf(" Writing File %s...", file);
    SceUID fd = sceIoOpen(file, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC , 511 );
    if(fd < 0)
          Exit("\n Cannot open file for writing."); 

    int written = sceIoWrite(fd, file_name, file_size);
    if(written != file_size) 
    {
    	sceIoClose(fd);
    	Exit("\n Cannot write file.");
    }

    sceIoClose(fd);
    printf(" OK\n");

    return 0;
}

int assign_flash_zero()
{
    int ret = sceIoUnassign("flash0:");
    ret += sceIoAssign("flash0:", "lflash0:0,0", "flashfat0:", IOASSIGN_RDWR, NULL, 0);

    return ret;
}

void printfc(int x, int y, char *fmt, ...)
{    
    char msg[256];

    va_list list;
    va_start(list, fmt);
    vsprintf(msg, fmt, list);
    va_end(list);

    setc(x, y);
    printf(msg);
    setcolor(WHITE);
}

//from ospbt by cory1492
void wait_release(unsigned int buttons)
{
    SceCtrlData pad;

    sceCtrlReadBufferPositive(&pad, 1);
    while(pad.Buttons & buttons)
    	sceCtrlReadBufferPositive(&pad, 1);
}

//from ospbt by cory1492
unsigned int wait_press(unsigned int buttons)
{
    SceCtrlData pad;

    sceCtrlReadBufferPositive(&pad, 1);
    while(1)
    {
    	if(pad.Buttons & buttons)
    		return pad.Buttons & buttons;

    	sceCtrlReadBufferPositive(&pad, 1);
    }

    return 0;
}

char *lspath[] =
{
    "ms0:/PSP/APP",
    "ms0:/PSP/APP/NPIA00013",
    "ms0:/PSP/APP/NPEG00012",
    "ms0:/PSP/APP/NPIA00013/EBOOT.PBP",
    "ms0:/PSP/APP/NPEG00012/EBOOT.PBP",
    "ms0:/PSP/APP/NPIA00013/K.BIN",
    "ms0:/PSP/APP/NPEG00012/K.BIN"
};

int main(int argc, char *argv[]) 
{
    pspDebugScreenInit();
    int devkit = sceKernelDevkitVersion(),
    psp_model = kuKernelGetModel(),
    cursor = 0;

    if(argv[0][0] == 'e' && argv[0][1] == 'f' && psp_model == 4)
    {
    	lspath[0] = "ef0:/PSP/APP";
    	lspath[1] = "ef0:/PSP/APP/NPIA00013";
    	lspath[2] = "ef0:/PSP/APP/NPEG00012";
    	lspath[3] = "ef0:/PSP/APP/NPIA00013/EBOOT.PBP";
    	lspath[4] = "ef0:/PSP/APP/NPEG00012/EBOOT.PBP";
    	lspath[5] = "ef0:/PSP/APP/NPIA00013/K.BIN";
    	lspath[6] = "ef0:/PSP/APP/NPEG00012/K.BIN";
    }

    if(devkit != 0x06060110 && devkit != 0x06060010)
    	Exit("\nSorry. This program supports only firmwares 6.60 and 6.61!");

    printf("\n FinalSpeed - The Z, frostegater\n\n");

    while(1)
    {
    	if(cursor > 3)
    		cursor = 0;
    	else if(cursor < 0)
    		cursor = 3;

    	if(cursor == 0) setbcolor(GRAY);
    	printfc(3, 4, " Install SensMe XMB Mod.                ");
    	setbcolor(BLACK);
    	if(cursor == 1) setbcolor(GRAY);
    	printfc(3, 5, " Install Comic XMB Mod.                 ");
    	setbcolor(BLACK);
    	if(cursor == 2) setbcolor(GRAY);
    	printfc(3, 6, " Restore original files.                ");
    	setbcolor(BLACK);
    	if(cursor == 3) setbcolor(GRAY);
    	printfc(3, 7, " Exit.                                  ");
    	setbcolor(BLACK);

    	int i;
    	for(i = 0; i < 4; i++)
    		printfc(1, 4 + i, " ");

    	setcolor(BLUE);
    	printfc(1, 4 + cursor, ">");

    	unsigned int Buttons = wait_press(PSP_CTRL_CROSS | PSP_CTRL_UP | PSP_CTRL_DOWN);
    	wait_release(PSP_CTRL_CROSS | PSP_CTRL_UP | PSP_CTRL_DOWN);

    	SceCtrlData pad;
    	sceCtrlReadBufferPositive(&pad, 1);

    	if(Buttons & PSP_CTRL_CROSS)
    	{
    		if(cursor != 3)
    		{
    			setc(0, 9);
    			if(assign_flash_zero() < 0)
    				Exit("\n Error in 'flash0:/' assign.");
    			if(devkit == 0x06060010 || 0x06060110)
    			{
    				if(psp_model == 4)
    				{
    					if(cursor == 0)//SensMe
    					{
    						sceIoMkdir(lspath[0], 777);
    						sceIoMkdir(lspath[1], 777);
    						flash_file(lspath[3], _660launcher, size__660launcher);
    						flash_file(lspath[5], _kbin, size__kbin);
    						flash_file("flash0:/vsh/resource/topmenu_icon.rco", _660_sm_go_icon, size__660_sm_go_icon);
    						flash_file("flash0:/vsh/resource/topmenu_plugin.rco", _660_sm_go_plugin, size__660_sm_go_plugin);
    					}
    					else if(cursor == 1)//DigitalComics
    					{
    						sceIoMkdir(lspath[0], 777);
    						sceIoMkdir(lspath[2], 777);
    						flash_file(lspath[4], _660launcherDC, size__660launcherDC);
    						flash_file(lspath[6], _kbin, size__kbin);							
    						flash_file("flash0:/vsh/resource/topmenu_icon.rco", _660_dc_go_icon, size__660_dc_go_icon);
    						flash_file("flash0:/vsh/resource/topmenu_plugin.rco", _660_dc_go_plugin, size__660_dc_go_plugin);
    					}
    					else if(cursor == 2)//Original
    					{
    						sceIoRemove(lspath[4]);
    						sceIoRemove(lspath[3]);
    						flash_file("flash0:/vsh/resource/topmenu_icon.rco", _660_orig_go_icon, size__660_orig_go_icon);
    						flash_file("flash0:/vsh/resource/topmenu_plugin.rco", _660_orig_go_plugin, size__660_orig_go_plugin);
    					}
    				}
    				else
    				{
    					if(cursor == 0)//SensMe
    					{
    						sceIoMkdir(lspath[0], 777);
    						sceIoMkdir(lspath[1], 777);
    						flash_file(lspath[3], _660launcher, size__660launcher);
    						flash_file(lspath[5], _kbin, size__kbin);							
    						flash_file("flash0:/vsh/resource/topmenu_icon.rco", _660_sm_X_icon, size__660_sm_X_icon);
    						flash_file("flash0:/vsh/resource/topmenu_plugin.rco", _660_sm_X_plugin, size__660_sm_X_plugin);
    					}
    					else if(cursor == 1)//DigitalComics
    					{
    						sceIoMkdir(lspath[0], 777);
    						sceIoMkdir(lspath[2], 777);
    						flash_file(lspath[4], _660launcherDC, size__660launcherDC);
    						flash_file(lspath[6], _kbin, size__kbin);							
    						flash_file("flash0:/vsh/resource/topmenu_icon.rco", _660_dc_X_icon, size__660_dc_X_icon);
    						flash_file("flash0:/vsh/resource/topmenu_plugin.rco", _660_dc_X_plugin, size__660_dc_X_plugin);
    					}
    					else if(cursor == 2)//Original
    					{
    						sceIoRemove(lspath[4]);
    						sceIoRemove(lspath[3]);
    						sceIoRemove(lspath[5]);
    						sceIoRemove(lspath[6]);							
    						flash_file("flash0:/vsh/resource/topmenu_icon.rco", _660_orig_X_icon, size__660_orig_X_icon);
    						flash_file("flash0:/vsh/resource/topmenu_plugin.rco", _660_orig_X_plugin, size__660_orig_X_plugin);
    					}
    				}
    			}
    		}
    		else
    			Exit("");

    		break;
    	}
    	else if(Buttons & PSP_CTRL_UP)
    		cursor--;
    	else if(Buttons & PSP_CTRL_DOWN)
    		cursor++;
    }

    Exit("\n Installation completed.");

    return 0;
}
