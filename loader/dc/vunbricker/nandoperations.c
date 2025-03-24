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
#include "idstools.h"

extern u8 *big_buffer;
extern u8 *sm_buffer1, *sm_buffer2;
extern int status, progress_text, progress_bar;
extern int last_percentage;

int cancel_dump;
char dump_error_msg[256];
char restore_error_msg[256];
char end_msg[384];
int new_x = -1, new_y = -1;
int cancel_dump;

int OnDumpError(void *param)
{
    vlfGuiMessageDialog(dump_error_msg, VLF_MD_TYPE_ERROR | VLF_MD_BUTTONS_NONE);

    vlfGuiRemoveText(status);
    vlfGuiRemoveText(progress_text);
    vlfGuiRemoveProgressBar(progress_bar);

    progress_bar = -1;
    progress_text = -1;
    status = -1;

    vlfGuiCancelBottomDialog();
    NandOperationsMenu(0);

    return VLF_EV_RET_REMOVE_HANDLERS;
}

void DumpError(char *fmt, ...)
{
    va_list list;
    
    va_start(list, fmt);
    vsprintf(dump_error_msg, fmt, list);
    va_end(list);

    vlfGuiAddEventHandler(0, -1, OnDumpError, NULL);
    sceKernelExitDeleteThread(0);
}

int OnBackFromDump(int enter)
{
    if (!enter)
    {
        vlfGuiRemoveText(status);
        status = -1;
        vlfGuiCancelBottomDialog();
        NandOperationsMenu(0);
    }
    
    return VLF_EV_RET_NOTHING;
}

int OnDumpComplete(void *param)
{
    vlfGuiRemoveProgressBar(progress_bar);
    vlfGuiRemoveText(progress_text);

    progress_bar = -1;
    progress_text = -1;

    vlfGuiCancelBottomDialog();
    vlfGuiBottomDialog(VLF_DI_BACK, -1, 1, 0, VLF_DEFAULT, OnBackFromDump);

    SetStatus(end_msg);

    if (new_x >= 0)
    {
        vlfGuiSetTextXY(status, new_x, new_y);
        new_x = -1;
        new_y = -1;
    }

    return VLF_EV_RET_REMOVE_HANDLERS;
}

int OnCancelDump(int enter)
{
    if (!enter && vlfGuiMessageDialog("Are you sure you want to cancel?", VLF_MD_TYPE_NORMAL | VLF_MD_BUTTONS_YESNO | VLF_MD_INITIAL_CURSOR_NO) == VLF_MD_YES)
    {
        if (last_percentage < 100)
        {
        	SetStatus("Cancelling... ");
        	cancel_dump = 1;
        	return VLF_EV_RET_REMOVE_HANDLERS;
        }
    }
    
    return VLF_EV_RET_NOTHING;
}

int OnCancelCompleted(void *param)
{
    vlfGuiRemoveProgressBar(progress_bar);
    vlfGuiRemoveText(progress_text);
    vlfGuiRemoveText(status);

    progress_bar = -1;
    progress_text = -1;
    status = -1;

    vlfGuiCancelBottomDialog();
    NandOperationsMenu(0);

    return VLF_EV_RET_REMOVE_HANDLERS;
}

int dump_thread(SceSize args, void *argp)
{
    SceUID fd;
    u32 pagesize, ppb, totalblocks, extrasize, blocksize, totalpages;
    int nbfit;
    int i, j;
    int badblocks[28], nbb = 0;    

    dcGetNandInfo(&pagesize, &ppb, &totalblocks);

    // myKprintf("pagesize: 0x%X\n", pagesize);
    // myKprintf("ppb: 0x%X\n", ppb);
    // myKprintf("totalblocks: 0x%X\n", totalblocks);

    extrasize = pagesize / 32;
    blocksize = (pagesize+extrasize) * ppb;
    totalpages = totalblocks*ppb;

    nbfit = BIG_BUFFER_SIZE / blocksize;

    fd = sceIoOpen("ms0:/nand-dump.bin", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    if (fd < 0)
    {
        DumpError("Error 0x%08X creating nand-dump.bin", fd);
    }

    for (i = 0; i < totalpages; )
    {
        u8 *p;
        memset(big_buffer, 0xff, nbfit*blocksize);

        p = big_buffer;
        
        for (j = 0; j < nbfit && i < totalpages; j++)
        {
        	dcLockNand(0);
        	
        	if (dcReadNandBlock(i, p) == -1)
        	{
        		if (nbb < 28)
        		{
        			badblocks[nbb++] = i / ppb;
        		}
        	}

        	dcUnlockNand();

        	if (cancel_dump)
        	{
        		sceIoClose(fd);
        		sceIoRemove("ms0:/nand-dump.bin");
        		vlfGuiAddEventHandler(0, 400000, OnCancelCompleted, NULL);

        		return sceKernelExitDeleteThread(0);
        	}

        	i += ppb;
        	p += (528*ppb);

        	SetGenericProgress(i, totalpages, 0);
        	scePowerTick(0);
        }		

        sceIoWrite(fd, big_buffer, j*blocksize);
    }

    sceIoClose(fd);
    SetProgress(100, 1);

    strcpy(end_msg, "Dump completed.\n");

    if (nbb > 0)
    {
        strcat(end_msg, "The following bad blocks were found:\n\n");

        for (i = 0; i < nbb; i++)
        {
        	if (i && (i % 7) == 0)
        	{
        		strcat(end_msg, "\n");
        	}
        	
        	sprintf(end_msg+strlen(end_msg), "%d", badblocks[i]);

        	if (i == (nbb-1))
        	{
        		strcat(end_msg, ".");
        	}
        	else
        	{
        		strcat(end_msg, ", ");
        	}			
        }
    }
    else
    {
        new_x = 150;
        new_y = 110;
    }

    vlfGuiAddEventHandler(0, 700000, OnDumpComplete, NULL);
    
    return sceKernelExitDeleteThread(0);
}

void DumpNand()
{
    cancel_dump = 0;

    ClearProgress();
    status = vlfGuiAddText(80, 100, "Dumping nand...");

    progress_bar = vlfGuiAddProgressBar(136);    
    progress_text = vlfGuiAddText(240, 148, "0%");
    vlfGuiSetTextAlignment(progress_text, VLF_ALIGNMENT_CENTER);

    vlfGuiCancelBottomDialog();
    vlfGuiBottomDialog(VLF_DI_CANCEL, -1, 1, 0, VLF_DEFAULT, OnCancelDump);

    SceUID dump_thid = sceKernelCreateThread("dump_thread", dump_thread, 0x18, 0x10000, 0, NULL);
    if (dump_thid >= 0)
    {
        sceKernelStartThread(dump_thid, 0, NULL);
    }    
}

int OnRestoreError(void *param)
{
    vlfGuiMessageDialog(restore_error_msg, VLF_MD_TYPE_ERROR | VLF_MD_BUTTONS_NONE);

    vlfGuiRemoveText(status);
    vlfGuiRemoveText(progress_text);
    vlfGuiRemoveProgressBar(progress_bar);

    progress_bar = -1;
    progress_text = -1;
    status = -1;
    
    dcSetCancelMode(0);
    NandOperationsMenu(1);

    return VLF_EV_RET_REMOVE_HANDLERS;
}

void RestoreError(char *fmt, ...)
{
    va_list list;
    
    va_start(list, fmt);
    vsprintf(restore_error_msg, fmt, list);
    va_end(list);

    vlfGuiAddEventHandler(0, -1, OnRestoreError, NULL);
    sceKernelExitDeleteThread(0);
}

int OnRestoreComplete(void *param)
{
    vlfGuiRemoveProgressBar(progress_bar);
    vlfGuiRemoveText(progress_text);

    dcSetCancelMode(0);

    SetStatus("Restore is complete.\nA shutdown or a reboot is required.");
    AddShutdownRebootBD(0);
    
    progress_bar = -1;
    progress_text = -1;    

    return VLF_EV_RET_REMOVE_HANDLERS;
}

int restore_thread(SceSize args, void *argp)
{
    u32 pagesize, ppb, totalblocks, extrasize, blocksize, totalpages, totalsize;
    int nbfit;
    SceUID fd;
    int i, j, k;
    int n, m;
    u32 ppn = 0;
    u8 *user, *spare;
    u8 *p, *q, *r;
    int error = 0;

    dcGetNandInfo(&pagesize, &ppb, &totalblocks);

    extrasize = pagesize / 32;
    blocksize = (pagesize+extrasize) * ppb;
    totalpages = totalblocks*ppb;
    totalsize = totalpages*(pagesize+extrasize);
    nbfit = BIG_BUFFER_SIZE / blocksize;

    user = malloc64(ppb*pagesize);
    spare = malloc64(ppb*extrasize);

    dcSetCancelMode(1);

    if (totalsize != (33*1024*1024) && totalsize != (66*1024*1024))
    {
        RestoreError("Nand info not expected.\n");
    }

    fd = sceIoOpen("ms0:/nand-dump.bin", PSP_O_RDONLY, 0);
    if (fd < 0)
    {
        DumpError("Error 0x%08X opening nand-dump.bin", fd);
    }

    n = totalblocks / nbfit;
    
    if ((totalblocks % nbfit) != 0)
    {
        n++;
    }

    dcLockNand(1);

    for (i = 0; i < n; i++)
    {
        sceIoRead(fd, big_buffer, nbfit*blocksize);
        
        p = big_buffer;

        if (i == (n-1))
        {
        	m = totalblocks % nbfit;
        	if (m == 0)
        		m = nbfit;
        }
        else
        {
        	m = nbfit;
        }

        for (j = 0; j < m; j++)
        {
        	q = user;
        	r = spare;
        	
        	for (k = 0; k < 32; k++)
        	{
        		memcpy(q, p, 512);
        		memcpy(r, p+512, 16);

        		p += 528;
        		q += 512;
        		r += 16;
        	}

        	if (ppn >= totalpages)
        	{
        		dcUnlockNand();
        		RestoreError("Break\n");
        	}
        	
        	if (1)
        	{
        		if (dcEraseNandBlock(ppn) >= 0)
        		{
        			if (dcWriteNandBlock(ppn, user, spare) < 0)
        				error++;//printf("Error writing block 0x%08X\n", ppn);
        		}
        		else
        		{
        			error++;
        			//printf("Error erasing block ppn=0x%08X\n", ppn);
        		}			
        	}

        	if (error > 100)
        	{
        		dcUnlockNand();
        		RestoreError("There are being too many write/erase errors.\n");
        	}

        	ppn += 32;
        	SetGenericProgress(ppn, totalpages, 0);
        }
    }

    dcUnlockNand();

    sceIoClose(fd);
    free(user);
    free(spare);

    SetProgress(100, 1);
    vlfGuiAddEventHandler(0, 700000, OnRestoreComplete, NULL);

    return sceKernelExitDeleteThread(0);
}

int RestoreNand()
{
    SceIoStat stat;
    u32 dummy, nandsize;
    u64 dummy64;

    if (vlfGuiMessageDialog("Physical nand restore can be dangerous if your nand has more bad blocks than when you did the dump.\nAre you sure you want to continue?", VLF_MD_TYPE_NORMAL | VLF_MD_BUTTONS_YESNO | VLF_MD_INITIAL_CURSOR_NO) != VLF_MD_YES)
    {
        return VLF_EV_RET_NOTHING;
    }

    if (sceIoGetstat("ms0:/nand-dump.bin", &stat) < 0)
    {
        vlfGuiMessageDialog("nand-dump.bin not found at root.", VLF_MD_TYPE_ERROR | VLF_MD_BUTTONS_NONE);
        return VLF_EV_RET_NOTHING;
    }

    dcGetHardwareInfo(&dummy, &dummy, &dummy, &dummy, &dummy64, &dummy, &nandsize);

    nandsize += (nandsize / 32);
    
    if (stat.st_size != nandsize)
    {
        vlfGuiMessageDialog("nand-dump.bin has not the correct size for this hardware.", VLF_MD_TYPE_ERROR | VLF_MD_BUTTONS_NONE);
        return VLF_EV_RET_NOTHING;
    }

    ClearProgress();
    status = vlfGuiAddText(80, 100, "Restoring nand...");

    progress_bar = vlfGuiAddProgressBar(136);    
    progress_text = vlfGuiAddText(240, 148, "0%");
    vlfGuiSetTextAlignment(progress_text, VLF_ALIGNMENT_CENTER);

    vlfGuiCancelBottomDialog();
    
    SceUID restore_thid = sceKernelCreateThread("restore_thread", restore_thread, 0x18, 0x10000, 0, NULL);
    if (restore_thid >= 0)
    {
        sceKernelStartThread(restore_thid, 0, NULL);
    }    

    return VLF_EV_RET_REMOVE_OBJECTS | VLF_EV_RET_REMOVE_HANDLERS;
}

int OnNandOperationsSelect(int sel)
{
    switch (sel)
    {
        case 0:
        	DumpNand();
        break;

        case 1:
        	return RestoreNand();
        break;

        case 2:
        	FormatFlashPage();
        break;

        case 3:
        	vlfGuiCancelBottomDialog();
        	vlfGuiCancelCentralMenu();
        	IdStorageMenu(0);
        	return VLF_EV_RET_NOTHING;
        break;
    }    
    
    return VLF_EV_RET_REMOVE_OBJECTS | VLF_EV_RET_REMOVE_HANDLERS;
}

int OnBackToMainMenuFromNO(int enter)
{
    if (!enter)
    {
        vlfGuiCancelCentralMenu();
        vlfGuiCancelBottomDialog();
        MainMenu(2);		
    }

    return VLF_EV_RET_NOTHING;
}

void NandOperationsMenu(int sel)
{
    char *items[] =
    {
        "Dump NAND",
        "Restore NAND",
        "Format Lflash",
        "IDStorage tools",
    };

    //vlfGuiCentralMenu(kuKernelGetModel() > 2 ? 3 : 4, items, sel, OnNandOperationsSelect, 0, -8);
    vlfGuiCentralMenu(4, items, sel, OnNandOperationsSelect, 0, -8);
    vlfGuiBottomDialog(VLF_DI_BACK, VLF_DI_ENTER, 1, 0, VLF_DEFAULT, OnBackToMainMenuFromNO);
}



