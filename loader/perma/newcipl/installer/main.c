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

#include "pspipl_update.h"

#include "../payload_01G.h"
#include "../payload_02G.h"
#include "../payload_03G.h"

PSP_MODULE_INFO("IPLFlasher", 0x0800, 1, 0); 
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VSH);

#define printf pspDebugScreenPrintf
#define WHITE 0xFFFFF1
#define GREEN 0x0000FF00

u32 sceSysregGetTachyonVersion(void);		// 0xE2A5D1EE

char msg[256];
int model;
static u8 orig_ipl[0x24000] __attribute__((aligned(64)));

unsigned char* ipl_table[] = {
	(unsigned char*)payload_01G,
	(unsigned char*)payload_02G,
	(unsigned char*)payload_03G,
};

unsigned char* ipl_block_large = NULL;

static int supported_models = sizeof(ipl_table)/sizeof(ipl_table[0]);

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
void flash_ipl(int size, u16 key)
{

	printf("Flashing IPL...");

//	if(ReadFile("ipl_block.bin", 0 , ipl_block_large , 0x4000) < 0)
//		ErrorExit(5000,"Failed to load custom ipl!\n");

	if(pspIplUpdateClearIpl() < 0)
		ErrorExit(5000,"Failed to clear ipl!\n");

	if (pspIplUpdateSetIpl( ipl_block_large , size + 0x4000, key ) < 0)
		ErrorExit(5000,"Failed to write ipl!\n");

	printf("Done.\n");

}

/*
int is_ta88v3(void)
{
	u32 model, tachyon;

	tachyon = sceSysregGetTachyonVersion();
	model = kuKernelGetModel();

	if(model == 1 && tachyon == 0x00600000) {
		return 1;
	}

	return 0;
}
*/

int main() 
{
	int devkit, size;
	SceUID kpspident;
	SceUID mod;
	u16 ipl_key = 0;

	pspDebugScreenInit();
	pspDebugScreenSetTextColor(WHITE);
	devkit = sceKernelDevkitVersion();

	if(devkit != 0x06060010 && devkit != 0x06060110) {
		ErrorExit(5000,"FW ERROR!\n");
	}

	kpspident = pspSdkLoadStartModule("kpspident.prx", PSP_MEMORY_PARTITION_KERNEL);

	if (kpspident < 0) {
		ErrorExit(5000, "kpspident.prx loaded failed\n");
	}

	model = kuKernelGetModel();

	if(model > supported_models || ipl_table[model] == NULL) {
		ErrorExit(5000,"This installer does not support this model.\n");
	}

	ipl_block_large = ipl_table[model];
	if (model > 2) ipl_key = 1;

	//load module
	mod = sceKernelLoadModule("ipl_update.prx", 0, NULL);

	if (mod < 0) {
		ErrorExit(5000,"Could not load ipl_update.prx!\n");
	}

	mod = sceKernelStartModule(mod, 0, NULL, NULL, NULL);

	if (mod < 0) {
		ErrorExit(5000,"Could not start module!\n");
	}

	size = pspIplUpdateGetIpl(orig_ipl);

	if(size < 0) {
		ErrorExit(5000,"Failed to get ipl!\n");
	}

	printf("\nCustom ipl Flasher for 6.6x.\n\n\n");

	int ipl_type = 0;

	if( size == 0x24000 ) {
		printf("Custom ipl is installed\n");
		size -= 0x4000;
		memmove( ipl_block_large + 0x4000 , orig_ipl + 0x4000 , size);
		ipl_type = 1;
	} else if( size == 0x20000 ) {
		printf("Raw ipl \n");
		memmove( ipl_block_large + 0x4000, orig_ipl, size);
	} else {
		printf("ipl size;%08X\n", size);
		ErrorExit(5000,"Unknown ipl!\n");
	}

	printf(" Press X to ");

	if( ipl_type ) {
		printf("Re");
	}

	printf("install CIPL\n");

	if( ipl_type ) {
		printf(" Press O to Erase CIPL and Restore Raw IPL\n");
	}

	printf(" Press R to cancel\n\n");
    
	while (1) {
        SceCtrlData pad;
        sceCtrlReadBufferPositive(&pad, 1);

		if (pad.Buttons & PSP_CTRL_CROSS) {
			flash_ipl( size, ipl_key );
			break; 
		} else if ( (pad.Buttons & PSP_CTRL_CIRCLE) && ipl_type ) {		
			printf("Flashing IPL...");

			if(pspIplUpdateClearIpl() < 0) {
				ErrorExit(5000,"Failed to clear ipl!\n");
			}

			if (pspIplUpdateSetIpl( ipl_block_large + 0x4000 , size,  ipl_key) < 0) {
				ErrorExit(5000,"Failed to write ipl!\n");
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
