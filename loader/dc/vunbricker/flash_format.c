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
#include "nandoperations.h"
#include "flash_format.h"

extern u8 *big_buffer;
extern u8 *sm_buffer1, *sm_buffer2;
extern int status, progress_text, progress_bar;

int flash_sizes[4];
int totalflash_size;
SceUID phformat_cb;

int flash_spins[4];
int flash_texts[4];
int selected_spin;
char format_error_msg[256];


int OnEditSpinOrBack(int enter);
void FormatFlashPage2();


int OnFormatError(void *param)
{
    vlfGuiMessageDialog(format_error_msg, VLF_MD_TYPE_ERROR | VLF_MD_BUTTONS_NONE);

    vlfGuiRemoveText(status);
    vlfGuiRemoveText(progress_text);
    vlfGuiRemoveProgressBar(progress_bar);

    progress_bar = -1;
    progress_text = -1;
    status = -1;

    dcSetCancelMode(0);
    NandOperationsMenu(2);

    return VLF_EV_RET_REMOVE_HANDLERS;
}

void FormatError(char *fmt, ...)
{
    va_list list;
    
    va_start(list, fmt);
    vsprintf(format_error_msg, fmt, list);
    va_end(list);

    vlfGuiAddEventHandler(0, -1, OnFormatError, NULL);
    sceKernelExitDeleteThread(0);
}

int OnBackFromFormat(int enter)
{
    if (!enter)
    {
        vlfGuiRemoveText(status);
        status = -1;
        vlfGuiCancelBottomDialog();
        NandOperationsMenu(2);
    }
    
    return VLF_EV_RET_NOTHING;
}

int OnFormatComplete(void *param)
{
    vlfGuiRemoveProgressBar(progress_bar);
    vlfGuiRemoveText(progress_text);

    progress_bar = -1;
    progress_text = -1;

    vlfGuiBottomDialog(VLF_DI_BACK, -1, 1, 0, VLF_DEFAULT, OnBackFromFormat);

    SetStatus("Format completed.");
    vlfGuiSetTextXY(status, 150, 110);    

    dcSetCancelMode(0);

    return VLF_EV_RET_REMOVE_HANDLERS;
}

int LoadFormatModules()
{
    SceUID mod = sceKernelLoadModule("flash0:/kd/emc_sm_updater.prx", 0, NULL);
    if (mod < 0 && mod != SCE_KERNEL_ERROR_EXCLUSIVE_LOAD)
        return mod;

    if (mod >= 0)
    {
        mod = sceKernelStartModule(mod, 0, NULL, NULL, NULL);
        if (mod < 0)
        	return mod;
    }

    if (kuKernelGetModel() == 0)
        dcPatchModule("sceNAND_Updater_Driver", 1, 0x0D7E, 0xAC60);
    else
        dcPatchModule("sceNAND_Updater_Driver", 1, 0x0D7E, 0xAC64);

    mod = sceKernelLoadModule("flash0:/kd/lfatfs_updater.prx", 0, NULL);
    if (mod < 0 && mod != SCE_KERNEL_ERROR_EXCLUSIVE_LOAD)
        return mod;

    dcPatchModuleString("sceLFatFs_Updater_Driver", "flash", "flach");

    if (mod >= 0)
    {
        mod = sceKernelStartModule(mod, 0, NULL, NULL, NULL);
        if (mod < 0)
        	return mod;
    }

    mod = sceKernelLoadModule("flash0:/kd/lflash_fatfmt_updater.prx", 0, NULL);
    if (mod < 0 && mod != SCE_KERNEL_ERROR_EXCLUSIVE_LOAD)
        return mod;

    sceKernelDelayThread(10000);

    if (mod >= 0)
    {
        mod = sceKernelStartModule(mod, 0, NULL, NULL, NULL);
        if (mod < 0)
        	return mod;
    }

    mod = sceKernelLoadModule("flash0:/kd/lflash_fdisk.prx", 0, NULL);
    if (mod < 0 && mod != SCE_KERNEL_ERROR_EXCLUSIVE_LOAD)
        return mod;

    if (mod >= 0)
    {
        mod = sceKernelStartModule(mod, 0, NULL, NULL, NULL);
        if (mod < 0)
        	return mod;
    }

    return 0;
}

int PhysicalFormatCallback(int count, u32 arg, void *param)
{
    u16 value, max;
    
    value = arg >> 16;
    max = arg & 0xFFFF;

    SetProgress((95 * value) / max, 0);
    scePowerTick(0);
    
    return 0;
}

int format_thread(SceSize args, void *argp)
{
    int res;
    char *argv[12];
    char part0[10], part1[10], part2[10], part3[10];
    
    dcSetCancelMode(1);

    // Unassign with error ignore (they might have been assigned in a failed attempt of install M33/OFW)
    sceIoUnassign("flach0:");
    sceIoUnassign("flach1:");
    sceIoUnassign("flach2:");
    sceIoUnassign("flach3:");
    
    if (LoadFormatModules() < 0)
    {
        FormatError("Error loading updater modules.");
    }

    sceKernelDelayThread(1200000);

    SetStatus("Physical format...");
    dcRegisterPhysicalFormatCallback(phformat_cb);

    res = sceIoDevctl("lflach0:", 0x03d802, 0, 0, 0, 0);
    dcUnregisterPhysicalFormatCallback();

    if (res < 0)
    {
        FormatError("Error 0x%08X in physical format.\n", res);
    }

    SetProgress(95, 1);
    SetStatus("Creating partitions...");

    sprintf(part0, "%d", flash_sizes[0] / 32);
    sprintf(part1, "%d", flash_sizes[1] / 32);
    sprintf(part2, "%d", flash_sizes[2] / 32);
    sprintf(part3, "%d", flash_sizes[3] / 32);

    argv[0] = "fdisk";
    argv[1] = "-H";
    argv[2] = "2";
    argv[3] = "-S";
    argv[4] = "32";
    argv[5] = part0;
    argv[6] = part1;
    argv[7] = part2;
    argv[8] = part3;
    argv[9] = "0";
    argv[10] = "0";
    argv[11] = "lflach0:0";
        	
    res = dcLflashStartFDisk(12, argv);
    if (res < 0)
    {
        FormatError("Error 0x%08X in fdisk.\n", res);
    }

    sceKernelDelayThread(1400000);
    SetProgress(96, 1);

    SetStatus("Logical format (flash0)...");

    argv[0] = "fatfmt";
    argv[1] = "lflach0:0,0";

    res = dcLflashStartFatfmt(2, argv);
    if (res < 0)
    {
        FormatError("Flash0 format failed: 0x%08X", res);
    }

    sceKernelDelayThread(1200000);
    SetProgress(97, 1);

    SetStatus("Logical format (flash1)...");

    argv[1] = "lflach0:0,1";

    res = dcLflashStartFatfmt(2, argv);
    if (res < 0)
    {
        FormatError("Flash1 format failed: 0x%08X", res);
    }

    sceKernelDelayThread(1200000);
    SetProgress(98, 1);

    SetStatus("Logical format (flash2)...");

    argv[1] = "lflach0:0,2";
    res = dcLflashStartFatfmt(2, argv);
    if (res < 0)
    {
        FormatError("Flash2 format failed: 0x%08X", res);
    }

    sceKernelDelayThread(1200000);
    SetProgress(99, 1);

    SetStatus("Logical format (flash3)...");

    argv[1] = "lflach0:0,3";
    res = dcLflashStartFatfmt(2, argv);
    if (res < 0)
    {
        FormatError("Flash3 format failed: 0x%08X", res);
    }

    SetProgress(100, 1);
    vlfGuiAddEventHandler(0, 1200000, OnFormatComplete, NULL);
    
    return sceKernelExitDeleteThread(0);
}

int OnBeginFormatFlash(int enter)
{
    if (enter)
    {
        vlfGuiCancelPreviousPageControl();
        vlfGuiRemoveText(status);
        
        ClearProgress();
        status = vlfGuiAddText(80, 100, "Loading modules...");

        progress_bar = vlfGuiAddProgressBar(136);	
        progress_text = vlfGuiAddText(240, 148, "0%");
        vlfGuiSetTextAlignment(progress_text, VLF_ALIGNMENT_CENTER);

        SceUID format_thid = sceKernelCreateThread("format_thread", format_thread, 0x18, 0x10000, 0, NULL);
        if (format_thid >= 0)
        {
        	sceKernelStartThread(format_thid, 0, NULL);
        }	
        
        return VLF_EV_RET_REMOVE_HANDLERS | VLF_EV_RET_REMOVE_OBJECTS;
    }
    
    return VLF_EV_RET_NOTHING;
}

int OnSpinSelectDown(void *param)
{
    if (selected_spin != 3)
    {
        int sum;

        vlfGuiSetSpinState(flash_spins[selected_spin], VLF_SPIN_STATE_NOT_FOCUS);
        vlfGuiSetSpinState(flash_spins[selected_spin+1], VLF_SPIN_STATE_FOCUS);
        
        selected_spin++;
        sum = flash_sizes[0] + flash_sizes[1] + flash_sizes[2] + flash_sizes[3];
        vlfGuiSetIntegerSpinMinMax(flash_spins[selected_spin], 96, flash_sizes[selected_spin]+totalflash_size-sum);
    }

    return VLF_EV_RET_REMOVE_EVENT;
}

int OnSpinSelectUp(void *param)
{
    if (selected_spin != 0)
    {
        int sum;

        vlfGuiSetSpinState(flash_spins[selected_spin], VLF_SPIN_STATE_NOT_FOCUS);
        vlfGuiSetSpinState(flash_spins[selected_spin-1], VLF_SPIN_STATE_FOCUS);
        
        selected_spin--;
        sum = flash_sizes[0] + flash_sizes[1] + flash_sizes[2] + flash_sizes[3];
        vlfGuiSetIntegerSpinMinMax(flash_spins[selected_spin], 96, flash_sizes[selected_spin]+totalflash_size-sum);
    }

    return VLF_EV_RET_REMOVE_EVENT;
}

int OnSpinEdited(int enter)
{
    vlfGuiSetSpinState(flash_spins[selected_spin], VLF_SPIN_STATE_FOCUS);
    vlfGuiCancelBottomDialog();
    vlfGuiBottomDialog(VLF_DI_BACK, VLF_DI_EDIT, 1, 0, VLF_DEFAULT, OnEditSpinOrBack);
    vlfGuiAddEventHandler(PSP_CTRL_UP, 0, OnSpinSelectUp, NULL);
    vlfGuiAddEventHandler(PSP_CTRL_DOWN, 0, OnSpinSelectDown, NULL);
    vlfGuiSetPageControlEnable(1);
    
    if (enter)
    {
        vlfGuiGetIntegerSpinValue(flash_spins[selected_spin], &flash_sizes[selected_spin]);
    }
    else
    {
        vlfGuiSetIntegerSpinValue(flash_spins[selected_spin], flash_sizes[selected_spin]);
    }
    
    return VLF_EV_RET_NOTHING;
}

int OnEditSpinOrBack(int enter)
{
    int i;

    vlfGuiRemoveEventHandler(OnSpinSelectUp);
    vlfGuiRemoveEventHandler(OnSpinSelectDown);
    vlfGuiCancelBottomDialog();
    
    if (enter)
    {
        vlfGuiSetPageControlEnable(0);
        vlfGuiBottomDialog(VLF_DI_CANCEL, VLF_DI_ENTER, 1, 0, VLF_DEFAULT, OnSpinEdited);
        vlfGuiSetSpinState(flash_spins[selected_spin], VLF_SPIN_STATE_ACTIVE);
    }
    else
    {
        for (i = 0; i < 4; i++)
        {
        	vlfGuiRemoveText(flash_texts[i]);
        	vlfGuiRemoveSpinControl(flash_spins[i]);
        }

        vlfGuiCancelNextPageControl();		
        NandOperationsMenu(2);
    }
    
    return VLF_EV_RET_NOTHING;
}

int OnFormatFlashPreviousPage(int page)
{
    vlfGuiCancelBottomDialog();
    vlfGuiRemoveText(status);

    FormatFlashPage();

    return VLF_EV_RET_REMOVE_HANDLERS | VLF_EV_RET_REMOVE_OBJECTS;
}

void FormatFlashPage2()
{
    status = vlfGuiAddText(120, 117, "Press * to begin flash format.");
    vlfGuiBottomDialog(-1, VLF_DI_ENTER, 1, 0, VLF_DEFAULT, OnBeginFormatFlash);
    vlfGuiPreviousPageControl(OnFormatFlashPreviousPage);
}

int OnFormatFlashNextPage(int page)
{
    int i;

    for (i = 0; i < 4; i++)
    {
        vlfGuiRemoveText(flash_texts[i]);
        vlfGuiRemoveSpinControl(flash_spins[i]);
    }

    vlfGuiRemoveEventHandler(OnSpinSelectUp);
    vlfGuiRemoveEventHandler(OnSpinSelectDown);
    vlfGuiCancelBottomDialog();
    FormatFlashPage2();
    
    return VLF_EV_RET_REMOVE_HANDLERS | VLF_EV_RET_REMOVE_OBJECTS;
}

void FormatFlashPage()
{
    vlfGuiCancelBottomDialog();
    vlfGuiBottomDialog(VLF_DI_BACK, VLF_DI_EDIT, 1, 0, VLF_DEFAULT, OnEditSpinOrBack);

    flash_texts[0] = vlfGuiAddText(110, 80,  "Flash 0 size:");
    flash_texts[1] = vlfGuiAddText(110, 105, "Flash 1 size:");
    flash_texts[2] = vlfGuiAddText(110, 130, "Flash 2 size:");
    flash_texts[3] = vlfGuiAddText(110, 155, "Flash 3 size:");

    flash_spins[0] = vlfGuiAddIntegerSpinControl(240, 80,  96, totalflash_size, flash_sizes[0], 32, 0, 50, VLF_SPIN_STATE_FOCUS, NULL, " KB");
    flash_spins[1] = vlfGuiAddIntegerSpinControl(240, 105, 96, totalflash_size, flash_sizes[1], 32, 0, 50, VLF_SPIN_STATE_NOT_FOCUS, NULL, " KB");
    flash_spins[2] = vlfGuiAddIntegerSpinControl(240, 130, 96, totalflash_size, flash_sizes[2], 32, 0, 50, VLF_SPIN_STATE_NOT_FOCUS, NULL, " KB");
    flash_spins[3] = vlfGuiAddIntegerSpinControl(240, 155, 96, totalflash_size, flash_sizes[3], 32, 0, 50, VLF_SPIN_STATE_NOT_FOCUS, NULL, " KB");

    int    sum = flash_sizes[0] + flash_sizes[1] + flash_sizes[2] + flash_sizes[3];
    
    vlfGuiSetIntegerSpinMinMax(flash_spins[0], 96, flash_sizes[0]+totalflash_size-sum);
    vlfGuiSetIntegerSpinMinMax(flash_spins[1], 96, flash_sizes[1]+totalflash_size-sum);
    vlfGuiSetIntegerSpinMinMax(flash_spins[2], 96, flash_sizes[2]+totalflash_size-sum);
    vlfGuiSetIntegerSpinMinMax(flash_spins[3], 96, flash_sizes[3]+totalflash_size-sum);

    vlfGuiAddEventHandler(PSP_CTRL_UP, 0, OnSpinSelectUp, NULL);
    vlfGuiAddEventHandler(PSP_CTRL_DOWN, 0, OnSpinSelectDown, NULL);
    vlfGuiNextPageControl(OnFormatFlashNextPage);

    selected_spin = 0;
}

