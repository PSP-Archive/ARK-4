#include <stdio.h>
#include <pspsdk.h>
#include <pspiofilemgr.h>
#include <psploadexec.h>
#include <psploadexec_kernel.h>
#include <psputility_modules.h>
#include <pspumd.h>
#include <module2.h>
#include <macros.h>
#include <rebootconfig.h>
#include <systemctrl_se.h>
#include <string.h>
#include <ark.h>
#include "functions.h"

void _flush_cache(){
    k_tbl->KernelDcacheWritebackInvalidateAll();
}

int p5_open_savedata(int mode)
{
    p5_close_savedata();

    SceUtilitySavedataParam dialog;

    memset(&dialog, 0, sizeof(SceUtilitySavedataParam));
    dialog.base.size = sizeof(SceUtilitySavedataParam);

    dialog.base.language = 1;
    dialog.base.buttonSwap = 1;
    dialog.base.graphicsThread = 0x11;
    dialog.base.accessThread = 0x13;
    dialog.base.fontThread = 0x12;
    dialog.base.soundThread = 0x10;

    dialog.mode = mode;

    g_tbl->UtilitySavedataInitStart(&dialog);

    // Wait for the dialog to initialize
    int status;
    while ((status = g_tbl->UtilitySavedataGetStatus()) < 2)
    {
        g_tbl->KernelDelayThread(100);
        if (status < 0) return 0; // error
    }
    return 1;
}

// Runs the savedata dialog loop
int p5_close_savedata()
{

    int running = 1;
    int last_status = -1;

    while(running) 
    {
        int status = g_tbl->UtilitySavedataGetStatus();
        
        if (status != last_status)
        {
            last_status = status;
        }
        switch(status)
        {
            case PSP_UTILITY_DIALOG_VISIBLE:
                g_tbl->UtilitySavedataUpdate(1);
                break;

            case PSP_UTILITY_DIALOG_QUIT:
                g_tbl->UtilitySavedataShutdownStart();
                break;

            case PSP_UTILITY_DIALOG_NONE:
                running = 0;
                break;

            case PSP_UTILITY_DIALOG_FINISHED:
                break;
            default:
                if (status < 0) // sceUtilitySavedataGetStatus returned error?
                    return 0;
                break;
        }
        g_tbl->KernelDelayThread(100);
    }
    return 1;
}

// qwikrazor87's trick to get any usermode import from kernel
u32 qwikTrick(char* lib, u32 nid, u32 version){

    u32 ret = 0x08800E00;

    while (*(u32*)ret)
        ret += 8;

    memset((void *)0x08800D00, 0, 8);

    p5_open_savedata(PSP_UTILITY_SAVEDATA_AUTOLOAD);

    u32 addr;
    for (addr = 0x08400000; addr < 0x08800000; addr += 4) {
        if (strcmp("sceVshSDAuto_Module", (char *)addr) == 0)
            break;
    }

    p5_close_savedata();

    addr -= 0xBC;
    *(u32*)0x08800C00 = nid;

    int qwik_trick()
    {
        g_tbl->KernelDelayThread(350);
        u32 timer = 0;

        while (!*(u32*)0x08800D00 && (timer++ < 600)) {
            _sw((u32)lib, addr);
            _sw(version, addr + 4);
            _sw(0x00010005, addr + 8);
            _sw(0x08800C00, addr + 12);
            _sw(0x08800D00, addr + 16);

            g_tbl->KernelDelayThread(0);
        }

        g_tbl->KernelExitDeleteThread(0);
        return 0;
    }

    SceUID qwiktrick = g_tbl->KernelCreateThread("qwiktrick", qwik_trick, 8, 512, THREAD_ATTR_USER, NULL);
    g_tbl->KernelStartThread(qwiktrick, 0, NULL);

    p5_open_savedata(PSP_UTILITY_SAVEDATA_AUTOLOAD);

    memcpy((void *)ret, (const void *)0x08800D00, 8);

    _flush_cache();

    p5_close_savedata();
    
    return ret;
}
