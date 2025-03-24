#include <pspsdk.h>
#include <pspdebug.h>
#include <pspiofilemgr.h>
#include <pspctrl.h>
#include <pspinit.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <ark.h>
#include "macros.h"
#include "../../../loader/rebootex/pspbtcnf.h"

PSP_MODULE_INFO("ARKInstaller", 0x800, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VSH | PSP_THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(4096);

#define BUF_SIZE 16*1024
#define KERNELIFY(a) ((u32)a|0x80000000)

#define setbcolor pspDebugScreenSetBackColor
#define setcolor pspDebugScreenSetTextColor
#define setc pspDebugScreenSetXY
#define printf pspDebugScreenPrintf

#define RGB(r, g, b) (0xFF000000 | ((b)<<16) | ((g)<<8) | (r))
#define RED RGB(255, 0, 0)
#define GREEN RGB(0, 255, 0)
#define BLUE RGB(0, 0, 255)
#define BLACK RGB(0, 0, 0)
#define ORANGE RGB(255, 127, 0)
#define WHITE RGB(255, 255, 255)
#define YELLOW RGB(255, 255, 0)
#define GRAY RGB(103, 120, 137)

ARKConfig ark_config;

static u32 fatms371_uninstall = 0;
static u32 deadef_uninstall = 0;

void open_flash();

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



struct {
    char* orig;
    char* dest;
} flash_files[] = {
    {IDSREG_PRX, IDSREG_PRX_FLASH},
    {XMBCTRL_PRX, XMBCTRL_PRX_FLASH},
    {USBDEV_PRX, USBDEV_PRX_FLASH},
    {VSH_MENU, VSH_MENU_FLASH},
    {RECOVERY_PRX, RECOVERY_PRX_FLASH},
	{UPDATER_FILE, UPDATER_FILE_FLASH},
    {ARK_SETTINGS, ARK_SETTINGS_FLASH},
};


static const int N_FLASH_FILES = (sizeof(flash_files)/sizeof(flash_files[0]));

void install(char *argv[]) {
    pspDebugScreenClear();

    printf("ARK Full Installer Started\n");


    u32 my_ver = (ARK_MAJOR_VERSION << 16) | (ARK_MINOR_VERSION << 8) | ARK_MICRO_VERSION;
    u32 cur_ver = sctrlHENGetMinorVersion();
    int major = (cur_ver&0xFF0000)>>16;
    int minor = (cur_ver&0xFF00)>>8;
    int micro = (cur_ver&0xFF);

    printf("Version %d.%d.%.2i\n", major, minor, micro);

    printf("Opening flash0 for writing\n");
    open_flash();

    char path[ARK_PATH_SIZE];
    int len = strlen(argv[0])-8;
    char fatms[len];
    for (int i=0; i<N_FLASH_FILES; i++){
    	if(strstr(flash_files[i].orig, "fatms") && (kuKernelGetModel() == PSP_GO)) continue;
    	else if(strstr(flash_files[i].orig, "fatms")) {
    		snprintf(fatms, strlen(argv[0])-8, "%s", argv[0]);
    		strcpy(path, fatms);
    		strcat(path, flash_files[i].orig);
    	}
    	else {
    		strcpy(path, ark_config.arkpath);
    		strcat(path, flash_files[i].orig);
    	}
    		printf("Installing %s to %s\n", flash_files[i].orig, flash_files[i].dest);
    		copy_file(path, flash_files[i].dest);
    }

    // Kill Main Thread
    printf("Exiting...\n");
    sceKernelDelayThread(1000000);
    sceKernelExitGame();

}



void uninstall() {
    SceIoStat stat;
    pspDebugScreenClear();
    open_flash();
    for (int i=0; i<N_FLASH_FILES; i++){
    	if(sceIoGetstat(flash_files[i].dest, &stat) < 0) {
    		return;
    	}
    	else {
    		printf("Removing %s\n", flash_files[i].dest);
    		sceIoRemove(flash_files[i].dest);
    	}
    }

    // Kill Main Thread
    sceKernelDelayThread(100000);
    sceKernelExitGame();
    return;
}

void pops4tool() {
    static char* pops_files[] = {
        "kd/pstbtcnf.bin", "kd/popsman.prx", "kd/pops_01g.prx", "vsh/module/libpspvmc.prx"
    };
    SceIoStat stat;

    open_flash();
    for (int i=0; i<NELEMS(pops_files); i++){
        char flash_path[256];
        sprintf(flash_path, "flash0:/%s", pops_files[i]);
        if (sceIoGetstat(pops_files[i], &stat) >= 0){
            printf("Installing %s\n", flash_path);
            copy_file(pops_files[i], flash_path);
        }
    	else {
    		printf("\n\nError files not found.\n");
    		printf("Exiting...\n");
    		sceKernelDelayThread(1000000);
    		sceKernelExitGame();
    	}
    }
    printf("\n\nExiting...\n");
    sceKernelDelayThread(1000000);
    sceKernelExitGame();
}

void fatms371_mod(u32 _uninstall) {

    static char* fatms371_files[] = { "kd/_fatms371.prx", "kd/_fatmshlp.prx" };
    SceIoStat stat;

    open_flash();
    for (int i=0; i<NELEMS(fatms371_files); i++){
        char flash_path[256];
        sprintf(flash_path, "flash0:/%s", fatms371_files[i]);
    	if(!_uninstall) {
            if (sceIoGetstat(fatms371_files[i], &stat) >= 0){
                printf("Installing %s\n", flash_path);
                copy_file(fatms371_files[i], flash_path);
            }
    	}
    	else {
    		if (sceIoGetstat(flash_path, &stat) >= 0){
                printf("Removing %s\n", flash_path);
    			sceIoRemove(flash_path);
            }
    	}

    }
    printf("\n\nExiting...\n");
    sceKernelDelayThread(1000000);
    sceKernelExitGame();
}

void deadef_mod(u32 _uninstall) {

    static char* deadef_files[] = { "kd/pstbtcnf_05g.bin", "kd/deadef.prx" };
    SceIoStat stat;

    open_flash();
    for (int i=0; i<NELEMS(deadef_files); i++){
        char flash_path[256];
        sprintf(flash_path, "flash0:/%s", deadef_files[i]);
    	if(!_uninstall) {
            if (sceIoGetstat(deadef_files[i], &stat) >= 0){
                printf("Installing %s\n", flash_path);
                copy_file(deadef_files[i], flash_path);
            }
    	}
    	else {
    		if (sceIoGetstat(flash_path, &stat) >= 0){
                printf("Removing %s\n", flash_path);
    			sceIoRemove(flash_path);
            }
    	}

    }
    printf("\n\nExiting...\n");
    sceKernelDelayThread(1000000);
    sceKernelExitGame();
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

// Entry Point
int main(int argc, char * argv[])
{


    sctrlHENGetArkConfig(&ark_config);
    
    // Initialize Screen Output
    pspDebugScreenInit();

    if (ark_config.magic != ARK_CONFIG_MAGIC){
        printf("ERROR: not running ARK\n");
        sceKernelDelayThread(100000);
        sceKernelExitGame();
    }

    printf("\n\n\tARK-4 Full Installer\n");

    int cursor = 0;
    while(1) {

    	if(kuKernelGetModel() == PSP_GO) {
    		if(cursor > 3)
    			cursor = 0;
    	}
    	if(sctrlHENIsToolKit()) {
    		if(cursor > 4)
    			cursor = 0;
    	}
    	else if(!sctrlHENIsToolKit() && kuKernelGetModel() != PSP_GO && cursor > 3) cursor = 0;
    	if(cursor < 0) {
    		if(sctrlHENIsToolKit())
    			cursor = 4;
    		else
    			cursor = 3;
    	}


    	if(cursor == 0) setbcolor(GRAY);
    	printfc(3, 4, " Install              ");
    	setbcolor(BLACK);
    	if(cursor == 1) setbcolor(GRAY);
    	printfc(3, 5, " Uninstall            ");
    	setbcolor(BLACK);
    	if(cursor == 2) setbcolor(GRAY);
    	if (sctrlHENIsToolKit()) {
    		if(cursor == 2)
    			setbcolor(GRAY);
    		printfc(3, 6, " Install popsTool     ");
    		setbcolor(BLACK);
    		if(cursor == 3)
    			setbcolor(GRAY);
    		int fatms371_check = sceIoOpen("flash0:/kd/_fatms371.prx", PSP_O_RDONLY, 0);
    		int fatms371_help_check = sceIoOpen("flash0:/kd/_fatmshlp.prx", PSP_O_RDONLY, 0);
    		if( fatms371_check >= 0 || fatms371_help_check >= 0) {
    			fatms371_uninstall = 1;
    			printfc(3, 7, " Uninstall fatms371_mod ");
    			sceIoClose(fatms371_check);
    			sceIoClose(fatms371_help_check);
    		}
    		else
    			printfc(3, 7, " Install fatms371_mod ");
    		setbcolor(BLACK);
    	}
    	else if((kuKernelGetModel() == PSP_GO)) {
    		if(cursor == 2)
    			setbcolor(GRAY);
			SceIoStat stat;
    		int cnf_check = sceIoGetstat("flash0:/kd/pstbtcnf_05g.bin", &stat);
    		int deadef_check = sceIoGetstat("flash0:/kd/deadef.prx", &stat);
    		if( cnf_check >= 0 || deadef_check >= 0) {
    			deadef_uninstall = 1;
    			printfc(3, 6, " Uninstall DeadEf mod ");
    		}
    		else {
    			printfc(3, 6, " Install DeadEf mod ");
    		}
			setbcolor(BLACK);
			if(cursor == 3)
    			setbcolor(GRAY);
    		printfc(3, 7, " Exit                 ");
    		setbcolor(BLACK);
    	}
    	else if(!sctrlHENIsToolKit() && kuKernelGetModel() != PSP_GO) {
    		if(cursor == 2)
    			setbcolor(GRAY);
			SceIoStat stat;
    		int fatms371_check = sceIoGetstat("flash0:/kd/_fatms371.prx", &stat);
    		int fatms371_help_check = sceIoGetstat("flash0:/kd/_fatmshlp.prx", &stat);
    		if( fatms371_check >= 0 || fatms371_help_check >= 0) {
    			fatms371_uninstall = 1;
    			printfc(3, 6, " Uninstall fatms371_mod ");
    		}
    		else {
    			printfc(3, 6, " Install fatms371_mod ");
    		}
    		setbcolor(BLACK);
    	}

    	if((!sctrlHENIsToolKit() && kuKernelGetModel() != PSP_GO)) {
    		if(cursor == 3)
    			setbcolor(GRAY);
    		printfc(3, 7, " Exit                 ");
    		setbcolor(BLACK);
    	}
    	if((sctrlHENIsToolKit() && kuKernelGetModel() != PSP_GO)) {
    		if(cursor == 4)
    			setbcolor(GRAY);
    		printfc(3, 8, " Exit                 ");
    		setbcolor(BLACK);
    	}

    	int i;
    	for(i = 0; i < (sctrlHENIsToolKit() ? 5 : 4); i++)
    		printfc(1, 4 + i, " ");

    	setcolor(BLUE);
    	printfc(1, 4 + cursor, ">");

    	u32 Buttons = wait_press(PSP_CTRL_CROSS | PSP_CTRL_UP | PSP_CTRL_DOWN);
    	wait_release(PSP_CTRL_CROSS | PSP_CTRL_UP | PSP_CTRL_DOWN);

    	SceCtrlData pad;
    	sceCtrlReadBufferPositive(&pad, 1);
    	if (Buttons & PSP_CTRL_CROSS) {

    		setc(0, 9);
    		if(cursor == 0)
    			install(argv);
    		if(cursor == 1)
    			uninstall();
    		if(cursor == 2 && sctrlHENIsToolKit())
    			pops4tool();
    		if(cursor == 2 && !sctrlHENIsToolKit() && kuKernelGetModel() != PSP_GO && fatms371_uninstall == 0)
    			fatms371_mod(0);
    		if(cursor == 2 && !sctrlHENIsToolKit() && kuKernelGetModel() != PSP_GO && fatms371_uninstall == 1)
    			fatms371_mod(1);
			if(cursor == 2 && !sctrlHENIsToolKit() && kuKernelGetModel() == PSP_GO && deadef_uninstall == 0)
				deadef_mod(0);
				if(cursor == 2 && !sctrlHENIsToolKit() && kuKernelGetModel() == PSP_GO && deadef_uninstall == 1)
				deadef_mod(1);
    		if(cursor == 2 && (kuKernelGetModel() == PSP_GO)) {
    			printf("\n\nExiting...\n");
    			sceKernelDelayThread(1000000);
    			sceKernelExitGame();
    		}
    		if(cursor == 3 && !sctrlHENIsToolKit()) {
    			printf("\n\nExiting...\n");
    			sceKernelDelayThread(1000000);
    			sceKernelExitGame();
    		}
    		if(cursor == 3 && sctrlHENIsToolKit() && fatms371_uninstall == 0)
    			fatms371_mod(0);
    		if(cursor == 3 && sctrlHENIsToolKit() && fatms371_uninstall == 1)
    			fatms371_mod(1);
    		if(cursor == 4 && sctrlHENIsToolKit()) {
    			printf("\n\nExiting...\n");
    			sceKernelDelayThread(1000000);
    			sceKernelExitGame();
    		}

    	}
    	else if(Buttons & PSP_CTRL_UP)
    		cursor--;
    	else if(Buttons & PSP_CTRL_DOWN)
    		cursor++;
    }
    // Exit Function
    return 0;
}

// Exit Point
int module_stop(SceSize args, void * argp)
{
    // Return Success
    return 0;
}

void open_flash(){
    while(sceIoUnassign("flash0:") < 0) {
        sceKernelDelayThread(500000);
    }
    while (sceIoAssign("flash0:", "lflash0:0,0", "flashfat0:", 0, NULL, 0)<0){
        sceKernelDelayThread(500000);
    }
    while(sceIoUnassign("flash1:") < 0) {
        sceKernelDelayThread(500000);
    }
    while (sceIoAssign("flash1:", "lflash0:0,1", "flashfat1:", 0, NULL, 0)<0){
        sceKernelDelayThread(500000);
    }
}

void copy_file(char* orig, char* dest){
    static u8 buf[BUF_SIZE];
    int fdr = sceIoOpen(orig, PSP_O_RDONLY, 0);
    int fdw = sceIoOpen(dest, PSP_O_WRONLY|PSP_O_CREAT|PSP_O_TRUNC, 0777);
    while (1){
        int read = sceIoRead(fdr, buf, BUF_SIZE);
        if (read <= 0) break;
        sceIoWrite(fdw, buf, read);
    }
    sceIoClose(fdr);
    sceIoClose(fdw);
}

