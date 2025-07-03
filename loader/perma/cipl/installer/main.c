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
#include "kbooti_update.h"

#include <ipl_block_large.h>
#include <ipl_block_01g.h>
#include <kbooti_ipl_block_01g.h>

#include <cipl_01G.h>
#include <cipl_02G.h>
#include <cipl_03G.h>
#include <cipl_04G.h>
#include <cipl_05G.h>
#include <cipl_07G.h>
#include <cipl_09G.h>
#include <cipl_11G.h>

#include <origipl_01G.h>
#include <origipl_02G.h>
#include <origipl_03G.h>
#include <origipl_04G.h>
#include <origipl_05G.h>
#include <origipl_07G.h>
#include <origipl_09G.h>
#include <origipl_11G.h>
#include <ark.h>

PSP_MODULE_INFO("IPLFlasher", 0x0800, 1, 0); 
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VSH);

#define printf pspDebugScreenPrintf
#define WHITE 0xFFFFF1
#define GREEN 0x0000FF00

#define KBOOTI_UPDATE_PRX "kbooti_update.prx"

//#define ORIG_IPL_SIZE 126976
#define ORIG_IPL_SIZE 127312
#define ORIG_IPL_MAX_SIZE 131072
#define CLASSIC_CIPL_SIZE 147456
#define NEW_CIPL_SIZE 184320

u32 sceSysregGetTachyonVersion(void);        // 0xE2A5D1EE

ARKConfig ark_config;
char msg[256];
int model;
static u8 big_buf[256*1024] __attribute__((aligned(64)));
static int reboot = -1;

u8* ipl_block = ipl_block_large;

int devkit, baryon_ver;

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

////////////////////////////////////////
void ErrorExit(int milisecs, char *fmt, ...) 
{
    va_list list;
    va_start(list, fmt);
    vsprintf(msg, fmt, list);
    va_end(list);
    printf(msg);
    sceKernelDelayThread(milisecs*1000);
    if(reboot)
        scePowerRequestColdReset(0);
    else
        sceKernelExitGame(); 
}

void loadIplUpdateModule(){
    SceUID mod;
    //load module
    mod = sceKernelLoadModule("ipl_update.prx", 0, NULL);

    if (mod == 0x80020139) return; // SCE_ERROR_KERNEL_EXCLUSIVE_LOAD

    if (mod < 0) {
        ErrorExit(5000,"Could not load ipl_update.prx!\n");
    }

    mod = sceKernelStartModule(mod, 0, NULL, NULL, NULL);

    if (mod == 0x80020133) return; // SCE_ERROR_KERNEL_MODULE_ALREADY_STARTED

    if (mod < 0) {
        ErrorExit(5000,"Could not start module!\n");
    }
}

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

void classicipl_menu(){

    int size;
    (void)size_ipl_block_large;
    pspDebugScreenClear();

    printf("Classic cIPL installation.\n");

    if( model == 0 ) {
        memcpy( ipl_block_large , ipl_block_01g, 0x4000);
    }

    loadIplUpdateModule();

    size = pspIplUpdateGetIpl(big_buf);

    if(size < 0) {
        ErrorExit(5000,"Failed to get IPL!\n");
    }

    printf("\nCustom IPL Flasher for 6.6x.\n\n\n");

    int ipl_type = 0;

    
    if (size > ORIG_IPL_MAX_SIZE){
        if( size == CLASSIC_CIPL_SIZE ) {
        	printf("Custom IPL is installed\n");
        	size -= 0x4000;
        	memmove( ipl_block_large + 0x4000 , big_buf + 0x4000 , size);
        	ipl_type = 1;
        } else {
        	pspDebugScreenClear();
        	return newipl_menu(NULL);
        }
    } else {
        printf("Original IPL \n");
        memmove( ipl_block_large + 0x4000, big_buf, size);
    }

    printf(" Press X to ");

    if( ipl_type ) {
        printf("Re");
    }

    printf("install cIPL\n");

    if( ipl_type ) {
        printf(" Press O to Erase cIPL and Restore Original IPL\n");
    }

    printf(" Press R to cancel\n\n");
    
    SceCtrlData pad;
    while (1) {
        sceCtrlReadBufferPositive(&pad, 1);

        if (pad.Buttons & PSP_CTRL_CROSS) {
        	printf("Flashing cIPL...");

        	if(pspIplUpdateClearIpl() < 0)
        		ErrorExit(5000,"Failed to clear IPL!\n");

        	if (pspIplUpdateSetIpl(ipl_block_large, size+0x4000, 0 ) < 0)
        		ErrorExit(5000,"Failed to write cIPL!\n");

        	printf("Done.\n");
        	break; 
        } else if ( (pad.Buttons & PSP_CTRL_CIRCLE) && ipl_type ) {		
        	printf("Flashing IPL...");

        	if(pspIplUpdateClearIpl() < 0) {
        		ErrorExit(5000,"Failed to clear IPL!\n");
        	}

        	if (pspIplUpdateSetIpl( ipl_block_large + 0x4000 , size, 0 ) < 0) {
        		ErrorExit(5000,"Failed to write IPL!\n");
        	}

        	printf("Done.\n");
        	break; 
        } else if (pad.Buttons & PSP_CTRL_RTRIGGER) {
        	ErrorExit(2000,"Cancelled by user.\n");
        }

        sceKernelDelayThread(10000);
    }

    reboot = 1;
    ErrorExit(5000,"\nInstall complete. Restarting in 5 seconds...\n");
}

void devtoolipl_menu(){

    int size;
    SceUID mod;

    printf("DevTool cIPL installation.\n");

    //load module
    mod = sceKernelLoadModule(KBOOTI_UPDATE_PRX, 0, NULL);

    if (mod < 0) {
        ErrorExit(5000,"Could not load " KBOOTI_UPDATE_PRX "!\n");
    }

    mod = sceKernelStartModule(mod, 0, NULL, NULL, NULL);

    if (mod < 0) {
        ErrorExit(5000,"Could not start module!\n");
    }

    size = pspKbootiUpdateGetKbootiSize();

    if(size < 0) {
        ErrorExit(5000,"Failed to get kbooti!\n");
    }

    printf("\nCustom kbooti Flasher for kbooti.\n\n\n");

    int ipl_type = 0;
    const int IPL_SIZE = 0x21000;
    const int CIPL_SIZE = 0x23D10;

    if(size == CIPL_SIZE) {
        printf("Custom kbooti is installed\n");
        ipl_type = 1;
    } else if( size == IPL_SIZE ) {
        printf("Original kbooti \n");
    } else {
        printf("kbooti size: %08X\n", size);
        ErrorExit(5000,"Unknown kbooti!\n");
    }

    printf(" Press X to ");

    if( ipl_type ) {
        printf("Re");
    }

    printf("install cKBOOTI\n");

    if( ipl_type ) {
        printf(" Press O to Erase Custom KBOOTI and Restore Original KBOOTI\n");
    }

    printf(" Press R to cancel\n\n");
    
    SceCtrlData pad;
    while (1) {
        sceCtrlReadBufferPositive(&pad, 1);

        if (pad.Buttons & PSP_CTRL_CROSS) {
        	printf("Flashing cKBOOTI...");
        	if (pspKbootiUpdateKbooti(kbooti_ipl_block_01g, (sizeof(kbooti_ipl_block_01g))) < 0)
        		ErrorExit(5000,"Failed to write kbooti!\n");

        	printf("Done.\n");
        	break; 
        } else if ( (pad.Buttons & PSP_CTRL_CIRCLE) && ipl_type ) {		
        	printf("Flashing KBOOTI...");

        	if (pspKbootiUpdateRestoreKbooti() < 0) {
        		ErrorExit(5000,"Failed to write kbooti!\n");
        	}

        	printf("Done.\n");
        	break; 
        } else if (pad.Buttons & PSP_CTRL_RTRIGGER) {
        	ErrorExit(2000,"Cancelled by user.\n");
        }

        sceKernelDelayThread(10000);
    }

    reboot = 1;
    ErrorExit(5000,"\nInstall complete. Restarting in 5 seconds...\n");
}


void newipl_menu(const char* config){
    int size = NEW_CIPL_SIZE;
    u16 ipl_key = 0;
    u8 allow_classic_install = 0;

    static unsigned char* ipl_table[] = {
        (unsigned char*)cipl_01G,
        (unsigned char*)cipl_02G,
        (unsigned char*)cipl_03G,
        (unsigned char*)cipl_04G,
        (unsigned char*)cipl_05G,
        (unsigned char*)NULL, // 6g
        (unsigned char*)cipl_07G,
        (unsigned char*)NULL, // 8g
        (unsigned char*)cipl_09G,
        (unsigned char*)NULL, // 10g
        (unsigned char*)cipl_11G,
    };

    static unsigned char* orig_ipl_table[] = {
        (unsigned char*)origipl_01G,
        (unsigned char*)origipl_02G,
        (unsigned char*)origipl_03G,
        (unsigned char*)origipl_04G,
        (unsigned char*)origipl_05G,
        (unsigned char*)NULL, // 6g
        (unsigned char*)origipl_07G,
        (unsigned char*)NULL, // 8g
        (unsigned char*)origipl_09G,
        (unsigned char*)NULL, // 10g
        (unsigned char*)origipl_11G,
    };

    int supported_models = sizeof(ipl_table)/sizeof(ipl_table[0]);

    printf("New cIPL installation.\n");

    if(model >= supported_models || ipl_table[model] == NULL) {
        ErrorExit(5000,"This installer does not support this model.\n");
    }

    u8* ipl_block = ipl_table[model];
    if (model > 1){
        ipl_key = (model==4)?2:1;
    }

    loadIplUpdateModule();

    printf("\nCustom IPL Flasher for 6.61 running on %dg\n\n\n", (model+1));

    if (config){
        printf("\nUsing config <%s>, if this is incorrect, DO NOT PROCEED!\n\n", config);
    }

    printf(" Press X to install cIPL\n");

    printf(" Press O to restore Original IPL.\n");

    if(!config && model < 2 && !is_ta88v3()) {
	    allow_classic_install = 1;
	    printf(" Press L to intall Classic IPL.\n");
    }

    printf(" Press R to cancel\n\n");
    
    SceCtrlData pad;
    while (1) {
        sceCtrlReadBufferPositive(&pad, 1);
        if (pad.Buttons & PSP_CTRL_CROSS) {
        	printf("Flashing cIPL...");
        	if(pspIplUpdateClearIpl() < 0)
        		ErrorExit(5000,"Failed to clear IPL!\n");

        	if (pspIplUpdateSetIpl(ipl_block, size, 0 ) < 0)
        		ErrorExit(5000,"Failed to write cIPL!\n");
        	break; 
        } else if ( (pad.Buttons & PSP_CTRL_CIRCLE) ) {		
        	printf("Flashing IPL...");

        	ipl_block = orig_ipl_table[model];
        	size = ORIG_IPL_SIZE;

        	if(pspIplUpdateClearIpl() < 0) {
        		ErrorExit(5000,"Failed to clear IPL!\n");
        	}

        	if (pspIplUpdateSetIpl( ipl_block, size,  ipl_key) < 0) {
        		ErrorExit(5000,"Failed to write IPL!\n");
        	}

        	printf("Done.\n");
        	break; 
        } else if ((pad.Buttons & PSP_CTRL_LTRIGGER) && allow_classic_install) {
            return classicipl_menu();	
    	} else if (pad.Buttons & PSP_CTRL_RTRIGGER) {
        	ErrorExit(2000,"Cancelled by user.\n");
        }

        sceKernelDelayThread(10000);
    }

    open_flash();
    if (config && size != ORIG_IPL_SIZE){
        // other CFW installed
        int fd = sceIoOpen("flash0:/arkcipl.cfg", PSP_O_WRONLY|PSP_O_CREAT|PSP_O_TRUNC, 0777);
        sceIoWrite(fd, config, strlen(config));
        sceIoClose(fd);
    }
    else {
        // OFW/ARK installed
        sceIoRemove("flash0:/arkcipl.cfg");
    }

    ErrorExit(5000,"\nInstall complete. Restarting in 5 seconds...\n");

}


int main() 
{
    SceUID kpspident;

    pspDebugScreenInit();
    pspDebugScreenSetTextColor(WHITE);
    devkit = sceKernelDevkitVersion();

    // check if running 6.60 or 6.61
    if(devkit != 0x06060010 && devkit != 0x06060110) {
        ErrorExit(5000,"FW ERROR! Use on 6.60 or 6.61 only.\n");
    }

    // check if running infinity
    SceModule2 infinity_mod;
    SceIoStat dc_stat;
    if (kuKernelFindModuleByName("InfinityControl", &infinity_mod) == 0 && sceIoGetstat("ms0:/TM/DCARK/msipl.raw", &dc_stat)<0){
        ErrorExit(5000, "ERROR: installing cIPL over Infinity is risky, make sure you install DC-ARK first before doing this!");
    }

    kpspident = pspSdkLoadStartModule("kpspident.prx", PSP_MEMORY_PARTITION_KERNEL);

    if (kpspident < 0) {
        ErrorExit(5000, "kpspident.prx loaded failed\n");
    }

    model = kuKernelGetModel();

    if (sceSysconGetBaryonVersion(&baryon_ver) < 0) {
        ErrorExit(5000, "Could not determine baryon version!\n");
    }

    // check if running ARK
    memset(&ark_config, 0, sizeof(ARKConfig));
    sctrlArkGetConfig(&ark_config);
    if (ark_config.magic != ARK_CONFIG_MAGIC){
        if (sctrlHENFindFunction("SystemControl", "KUBridge", 0x9060F69D) != NULL){
        	newipl_menu("cfw=pro");
        }
        else {
        	newipl_menu("cfw=me");
        }
        return 0;
    }

    switch(sctrlHENIsToolKit()) {
	    case 0: newipl_menu(NULL); break;
	    case 1: classicipl_menu(); break;
	    case 2: devtoolipl_menu(); break;
    }

    return 0;
}
