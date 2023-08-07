/********************************
	ipl Flasher 


*********************************/
#include <pspsdk.h>
#include <pspkernel.h>
#include <pspctrl.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <kubridge.h>

#include "kbooti_update.h"
#include "../kbooti_ipl_block_01g.h"

#define IPL_UPDATE_PRX "kbooti_update.prx"
#define IPL_STR "kbooti"
#define IPL_STR_UPPER "KBOOTI"
#define IPL_SIZE 0x21000
#define CIPL_SIZE 0x23D10

PSP_MODULE_INFO("IPLFlasher", 0x0800, 1, 0); 
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VSH);

#define DEVKIT_VER	0x06060010
#define VERSION_STR	"6.60"

#define printf pspDebugScreenPrintf
#define WHITE 0xFFFFF1
#define GREEN 0x0000FF00

char msg[256];
int model;
static u8 orig_ipl[0x24000] __attribute__((aligned(64)));

int ReadFile(char *file, int seek, void *buf, int size)
{
	SceUID fd = sceIoOpen(file, PSP_O_RDONLY, 0);
	if (fd < 0)
		return fd;

	if (seek > 0)
	{
		if (sceIoLseek(fd, seek, PSP_SEEK_SET) != seek)
		{
			sceIoClose(fd);
			return -1;
		}
	}

	int read = sceIoRead(fd, buf, size);
	
	sceIoClose(fd);
	return read;
}


////////////////////////////////////////
void ErrorExit(int milisecs, char *fmt, ...) 
{
	va_list list;
	va_start(list, fmt);
	vsprintf(msg, fmt, list);
	va_end(list);
	printf(msg);
	sceKernelDelayThread(milisecs*1000);
	sceKernelExitGame(); 
}
////////////////////////////////////////
u8 nand_buff[0x40000];
void flash_ipl(int size)
{

	printf("Flashing " IPL_STR_UPPER "...");

	if (pspKbootiUpdateKbooti(kbooti_ipl_block_01g, (sizeof(kbooti_ipl_block_01g))) < 0)
		ErrorExit(5000,"Failed to write " IPL_STR "!\n");

	printf("Done.\n");

}

int main() 
{
	int devkit, baryon_ver, size;
	SceUID kpspident;
	SceUID mod;

	pspDebugScreenInit();
	pspDebugScreenSetTextColor(WHITE);
	devkit = sceKernelDevkitVersion();

	if(devkit != DEVKIT_VER) {
		ErrorExit(5000,"FW ERROR!\n");
	}

	kpspident = pspSdkLoadStartModule("kpspident.prx", PSP_MEMORY_PARTITION_KERNEL);

	if (kpspident < 0) {
		ErrorExit(5000, "kpspident.prx loaded failed\n");
	}

	if (sceSysconGetBaryonVersion(&baryon_ver) < 0) {
		ErrorExit(5000, "Could not determine baryon version!\n");
	}

	if (baryon_ver != 0x00020601) {
		ErrorExit(5000, "Not a DTP-T1000!\n");
	}

	//load module
	mod = sceKernelLoadModule(IPL_UPDATE_PRX, 0, NULL);

	if (mod < 0) {
		ErrorExit(5000,"Could not load " IPL_UPDATE_PRX "!\n");
	}

	mod = sceKernelStartModule(mod, 0, NULL, NULL, NULL);

	if (mod < 0) {
		ErrorExit(5000,"Could not start module!\n");
	}

	size = pspKbootiUpdateGetKbootiSize();

	if(size < 0) {
		ErrorExit(5000,"Failed to get " IPL_STR "!\n");
	}

	printf("\nCustom " IPL_STR " Flasher for "VERSION_STR".\n\n\n");

	int ipl_type = 0;

	if(size == CIPL_SIZE) {
		printf("Custom " IPL_STR " is installed\n");
		ipl_type = 1;
	} else if( size == IPL_SIZE ) {
		printf("Raw " IPL_STR " \n");
	} else {
		printf(IPL_STR " size;%08X\n", size);
		ErrorExit(5000,"Unknown " IPL_STR "!\n");
	}

	printf(" Press X to ");

	if( ipl_type ) {
		printf("Re");
	}

	printf("install C" IPL_STR_UPPER "\n");

	if( ipl_type ) {
		printf(" Press O to Erase C" IPL_STR_UPPER " and Restore Raw " IPL_STR_UPPER "\n");
	}

	printf(" Press R to cancel\n\n");
    
	while (1) {
        SceCtrlData pad;
        sceCtrlReadBufferPositive(&pad, 1);

		if (pad.Buttons & PSP_CTRL_CROSS) {
			flash_ipl( size );
			break; 
		} else if ( (pad.Buttons & PSP_CTRL_CIRCLE) && ipl_type ) {		
			printf("Flashing " IPL_STR_UPPER "...");

			if (pspKbootiUpdateRestoreKbooti() < 0) {
				ErrorExit(5000,"Failed to write " IPL_STR "!\n");
			}

			printf("Done.\n");
			break; 
		} else if (pad.Buttons & PSP_CTRL_RTRIGGER) {
			ErrorExit(2000,"Cancelled by user.\n");
		}

		sceKernelDelayThread(10000);
	}

	ErrorExit(5000,"\nInstall complete. Restarting in 5 seconds...\n");

	return 0;
}
