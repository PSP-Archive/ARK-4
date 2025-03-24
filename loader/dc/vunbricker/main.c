#include <pspsdk.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <pspsuspend.h>
#include <psppower.h>
#include <pspreg.h>
#include <psprtc.h>
#include <psputils.h>
#include <systemctrl.h>
#include <kubridge.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include <pspipl_update.h>
#include <vlf.h>

#include "dcman.h"
#include "main.h"
#include "install.h"
#include "nandoperations.h"

PSP_MODULE_INFO("VUnbricker", 0x800, 2, 0);
PSP_MAIN_THREAD_ATTR(0);

#define SMALL_BUFFER_SIZE    2000000
int BIG_BUFFER_SIZE;

u8 *big_buffer;
u8 *sm_buffer1, *sm_buffer2;

extern int flash_sizes[4];
extern int totalflash_size;
extern SceUID phformat_cb;
extern int PhysicalFormatCallback(int count, u32 arg, void *param);

int progress_text = -1, progress_bar = -1;
int status = -1;


int last_percentage = -1;
int last_time;
char pg_text[20];
char st_text[128];

int DoProgressUpdate(void *param)
{
    vlfGuiProgressBarSetProgress(progress_bar, last_percentage);
    vlfGuiSetText(progress_text, pg_text);
    
    return VLF_EV_RET_REMOVE_HANDLERS;
}

void ClearProgress()
{
    last_time = 0;
    last_percentage = -1;
}

void SetProgress(int percentage, int force)
{
    int st =  sceKernelGetSystemTimeLow();
    
    if (force || (percentage > last_percentage && st >= (last_time+520000)))
    {    	
        sprintf(pg_text, "%d%%", percentage);
        last_percentage = percentage;
        last_time = st;
        vlfGuiAddEventHandler(0, -2, DoProgressUpdate, NULL);
    }
}

int DoStatusUpdate(void *param)
{
    vlfGuiSetText(status, st_text);
    return VLF_EV_RET_REMOVE_HANDLERS;
}

void SetStatus(char *status)
{
    strcpy(st_text, status);
    vlfGuiAddEventHandler(0, -2, DoStatusUpdate, NULL);
}

void SetGenericProgress(int value, int max, int force)
{
    u32 prog;
    
    prog = ((100 * value) / max);
    SetProgress(prog, force);
}

int OnShutdownOrReboot(int enter)
{
    void *wi;

    vlfGuiAddWaitIcon(&wi);

    enter ^= vlfGuiGetButtonConfig();

    if (enter)
    {
        scePowerRequestStandby();
    }
    else
    {
        scePowerRequestColdReset(0);
    }
    
    return VLF_EV_RET_REMOVE_HANDLERS;
}

void AddShutdownRebootBD(int shutdownonly)
{
    vlfGuiCustomBottomDialog((shutdownonly) ? NULL : "Reboot", "Shutdown", 0, vlfGuiGetButtonConfig(), 85, OnShutdownOrReboot);
}

#define N_HI_ITEMS    10
int hi_texts[N_HI_ITEMS];

int OnBackToMainMenuFromHI(int enter)
{
    if (!enter)
    {
        int i;

        for (i = 0; i < N_HI_ITEMS; i++)
        {
        	vlfGuiRemoveText(hi_texts[i]);
        }

        vlfGuiCancelBottomDialog();
        MainMenu(3);
    }

    return VLF_EV_RET_NOTHING;
}

char *mobos[] =
{
    "TA-079v1",
    "TMU-001v1",
    "TA-079v2",
    "TMU-001v2",
    "TA-079v3",
    "TMU-002",
    "TA-079v4",
    "TA-079v5",
    "TA-081v1",
    "TA-081v2",
    "TA-082",
    "TA-086",
    "TA-085v1",
    "TA-085v2",
    "TA-088v1/v2",
    "TA-090v1",
    "TA-088v3",
    "TA-090v2",
    "TA-090v3",
    "TA-092",
    "TA-091",
    "TA-094",
    "TA-093v1",
    "TA-093v2",
    "TA-095v1",
    "TA-095v2",
    "TA-095v3",
    "TA-095v4",
    "TA-096/097",
    "O_O'"
};

typedef struct U64
{
    u32 low;
    u32 high;
} U64;

void HardwareInfo()
{
    u32 tachyon, baryon, pommel, mb, fuseconfig, nandsize;
    U64 fuseid;
    int model;
    char *model_str;
    int i;

    memset(hi_texts, 0xFF, sizeof(hi_texts));
    
    dcGetHardwareInfo(&tachyon, &baryon, &pommel, &mb, (void *)&fuseid, &fuseconfig, &nandsize);
    model = kuKernelGetModel();

    if (model == 0)
        model_str = "(Phat)";
    else if (model == 1)
        model_str = "(Slim)";
    else if (model == 4)
        model_str = "(Go)";
    else if (model == 10)
        model_str = "(Street)";
    else
        model_str = "(Bright)";

    hi_texts[0] = vlfGuiAddTextF(40, 80, "Model: %02dg %s", model+1, model_str);
    hi_texts[1] = vlfGuiAddTextF(245, 80, "Motherboard: %s", mobos[mb]);
    hi_texts[2] = vlfGuiAddText(40, 105, "Tachyon:");
    hi_texts[3] = vlfGuiAddTextF(120, 105, "0x%08X",  tachyon);
    hi_texts[4] = vlfGuiAddTextF(245, 105, "Baryon: 0x%08X", baryon);
    hi_texts[5] = vlfGuiAddText(40, 130, "Pommel:");
    hi_texts[6] = vlfGuiAddTextF(120, 130, "0x%08X", pommel);
    hi_texts[7] = vlfGuiAddTextF(245, 130, "Nand size: %dMB", nandsize / 1048576);
    hi_texts[8] = vlfGuiAddText(40, 155, "Fuse ID:");
    hi_texts[9] = vlfGuiAddTextF(120, 155, "0x%04X%08X", fuseid.high, fuseid.low);
    
    for (i = 0; i < N_HI_ITEMS; i++)
    {
        vlfGuiSetTextSize(hi_texts[i], 0.75f);
    }
    
    vlfGuiBottomDialog(VLF_DI_BACK, -1, 1, 0, VLF_DEFAULT, OnBackToMainMenuFromHI);
}

int OnMainMenuSelect(int sel)
{
    void *wi;
    
    switch (sel)
    {
        case 0:
        	vlfGuiCancelBottomDialog();
        	return Install(FW_ARK);
        break;

        case 1:
        	vlfGuiCancelBottomDialog();
        	return Install(FW_OFW);
        break;

        case 2:
        	vlfGuiCancelBottomDialog();
        	vlfGuiCancelCentralMenu();
        	NandOperationsMenu(0);
        	return VLF_EV_RET_NOTHING;
        break;
    
        case 3:
        	vlfGuiCancelBottomDialog();
        	HardwareInfo();
        break;
    
        case 4:
        {
        	vlfGuiAddWaitIcon(&wi);
        	sctrlKernelExitVSH(NULL);
        	return VLF_EV_RET_REMOVE_HANDLERS;
        }

        break;
    
        case 5:
        {			
        	vlfGuiAddWaitIcon(&wi);
        	scePowerRequestStandby();
        	return VLF_EV_RET_REMOVE_HANDLERS;
        }
        	
        break;

        case 6:
        {
        	
        	vlfGuiAddWaitIcon(&wi);
        	scePowerRequestColdReset(0);
        	return VLF_EV_RET_REMOVE_HANDLERS;
        }
        	
        break;
    }    
    
    return VLF_EV_RET_REMOVE_OBJECTS | VLF_EV_RET_REMOVE_HANDLERS;
}

void MainMenu(int sel)
{
    char *items[] =
    {
        "Install 6.61 ARK",
        "Install 6.61",
        "NAND operations",
        "Hardware Info",
        "Boot 6.61 ARK from MemoryStick",
        "Shutdown",
        "Reboot Device"
    };

    vlfGuiCentralMenu(7, items, sel, OnMainMenuSelect, 0, 0);
    vlfGuiBottomDialog(-1, VLF_DI_ENTER, 1, 0, VLF_DEFAULT, NULL);
}

static int CallbackThread(SceSize args, void *argp)
{
    phformat_cb = sceKernelCreateCallback("PhFormatProgressCallback", (void *)PhysicalFormatCallback, NULL);
    sceKernelSleepThreadCB();

    return 0;
}


int app_main()
{
    void *bi;
    u64 tick;
    SceKernelUtilsMt19937Context ctx;
    u32 rnd;
    u32 nandsize;
    u32 md5[4];

    SceUID thid = sceKernelCreateThread("VUnbrickerCallbackThread", CallbackThread, 0x11, 0xFA0, 0, 0);
    sceKernelStartThread(thid, 0, 0);    
    
    dcGetHardwareInfo(NULL, NULL, NULL, NULL, NULL, NULL, &nandsize);

    if (nandsize < (64*1024*1024))
    {
        flash_sizes[0] = 24576;
        flash_sizes[1] = 4096;
        flash_sizes[2] = 1024;
        flash_sizes[3] = 960;
    }
    else
    {
        flash_sizes[0] = 41984;
        flash_sizes[1] = 5120;
        flash_sizes[2] = 4096;
        flash_sizes[3] = 9344;
    }

    totalflash_size = flash_sizes[0]+flash_sizes[1]+flash_sizes[2]+flash_sizes[3];

    if (sceRtcGetCurrentTick(&tick) < 0)
        tick = sceKernelGetSystemTimeWide();

    sceKernelUtilsMd5Digest((u8 *)&tick, sizeof(u64), (u8 *)md5);
    sceKernelUtilsMt19937Init(&ctx, md5[0] ^ md5[1] ^ md5[2] ^ md5[3]);
    rnd = sceKernelUtilsMt19937UInt(&ctx) % 12;

    void *data;

    u32 model = kuKernelGetModel();
    if ( model == 4 )
        data = ReadFileAllocEx("flash0:/vsh/resource/01-12_03g.bmp", rnd*6176, 6176, NULL);
    else
        data = ReadFileAllocEx("flash0:/vsh/resource/01-12.bmp", rnd*6176, 6176, NULL);

    if (!data || vlfGuiSetBackgroundFileBuffer(data, 6176) < 0)
    {
        vlfGuiSetBackgroundPlane(0xFF000000);
    }

    if (data)
        free(data);
        
    vlfGuiSetLanguage(1);
    vlfGuiSetButtonConfig(1);

    vlfGuiChangeCharacterByButton('*', VLF_ENTER);
    vlfGuiCacheResource("system_plugin");
    vlfGuiCacheResource("system_plugin_fg");

    vlfGuiSetModelSystem();
    vlfGuiAddBatteryIconSystem(&bi, 10*1000*1000);
    vlfGuiAddClock();

    sm_buffer1 = malloc64(SMALL_BUFFER_SIZE);
    sm_buffer2 = malloc64(SMALL_BUFFER_SIZE);
    BIG_BUFFER_SIZE = (25*1024*1024);
    big_buffer = malloc64(BIG_BUFFER_SIZE);

    if (!big_buffer)
    {
        BIG_BUFFER_SIZE = (4*1024*1024);
        big_buffer = malloc64(BIG_BUFFER_SIZE);
    }

    if (!sm_buffer1 || !sm_buffer2 || !big_buffer)
    {
        __asm("break\n");
        while (1);
    }

    MainMenu(0);
    
    while (1)
    {
        vlfGuiDrawFrame();
    }    

       return 0;
}


