/*
    Custom Emulator Firmware
    Copyright (C) 2012-2014, Total_Noob

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    (at your option) any later version.
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <common.h>
#include "main.h"
#include "systemctrl.h"
#include "systemctrl_private.h"
#include "module2.h"
#include "macros.h"

#include "spu/stdafx.h"

PSP_MODULE_INFO("peops", 0, 1, 0);

STMOD_HANDLER previous;

PeopsConfig config;

void (* libspuWrite)(int reg, int val, int type);
void (* libspuCdrWrite)(int reg, int val);
void (* libspuXaFunction)(char *sector, int mode);

int cdr_is_first_sector = 1;
xa_decode_t cdr_xa;

SceUID spu_thid = -1;

void SPUwait()
{
    switch(config.spuupdatemode)
    {
        case 0:
            sceDisplayWaitVblankStart();
            break;

        case 1:
            sceKernelDelayThread(2 * 1000);
            break;

        case 2:
            sceKernelDelayThread(20 * 1000);
            break;
    }
}

int spu_thread(SceSize args, void *argp)
{
    while(1)
    {
        SPUupdate();
        SPUwait();
    }

    return 0;
}

int priorities[] = { 17, 24, 32, 40, 48 };
#define N_PRIORITIES (sizeof(priorities) / sizeof(int))

void sceMeAudioInitPatched(int (* function)(), void *stack)
{
    SPUinit();
    SPUopen();

    int priority = priorities[config.sputhreadpriority % N_PRIORITIES];

    spu_thid = sceKernelCreateThread("spu_thread", spu_thread, priority, 0x4000, 0, NULL);
    if(spu_thid >= 0) sceKernelStartThread(spu_thid, 0, NULL);
}

void libspuWritePatched(int reg, int val, int type)
{
    SPUwriteRegister(reg, val);
    libspuWrite(reg, val, type);
}

void libspuCdrWritePatched(int reg, int val)
{
    if(reg == 0x1F801801)
    {
        if(val == 6 || val == 27)
        {
            cdr_is_first_sector = 1;
        }
    }

    libspuCdrWrite(reg, val);
}

void libspuXaFunctionPatched(char *sector, int mode)
{
    u8 *buf = (u8 *)sector + 0xC;

    if(mode == 2)
    {
        if(cdr_is_first_sector != -1)
        {
            if(buf[4 + 2] & 0x4)
            {
                int ret = xa_decode_sector(&cdr_xa, buf + 4, cdr_is_first_sector);
                if(ret == 0)
                {
                    SPUplayADPCMchannel(&cdr_xa);
                    cdr_is_first_sector = 0;
                }
                else
                {
                    cdr_is_first_sector = -1;
                }
            }
        }
    }

    libspuXaFunction(sector, mode);
}

void OnModuleStart(SceModule2 *mod)
{
    char *modname = mod->modname;
    u32 text_addr = mod->text_addr;

    if(strcmp(modname, "pops") == 0)
    {
        MAKE_JUMP_PATCH(text_addr + 0x1A038, sceMeAudioInitPatched);

        HIJACK_FUNCTION(text_addr + 0x7F00, libspuWritePatched, libspuWrite);

        if(config.enablexaplaying)
        {
            HIJACK_FUNCTION(text_addr + 0xD1B0, libspuCdrWritePatched, libspuCdrWrite);
            HIJACK_FUNCTION(text_addr + 0x83E8, libspuXaFunctionPatched, libspuXaFunction);
        }

        sceKernelDcacheWritebackAll();
    }

    if(previous)
        previous(mod);
}

void load_game_profile(char * gameid)
{
    
    #ifdef DEBUG
    // Output Pointer (just in case)
    printk("gameid pointer: %08X\n", (u32)gameid);
    
    // Output Debug Message
    printk("Loading PEOPS audio profile for %s\n", gameid);
    #endif
    
    loadGameSettings(gameid, &config);
}

int module_start(SceSize args, void *argp)
{
    load_game_profile((char *)argp);
    
    /*
    SceUID fd = sceIoOpen("ms0:/peopsconf.bin", PSP_O_RDONLY, 0777);
    
    if (fd>=0){
        sceIoRead(fd, &config, sizeof(config));
        sceIoClose(fd);
    }
    */
    
    previous = sctrlHENSetStartModuleHandler(OnModuleStart);
    return 0;
}

int module_stop(SceSize args, void *argp)
{
    if(spu_thid >= 0)
    {
        sceKernelTerminateDeleteThread(spu_thid);
        sceKernelWaitThreadEnd(spu_thid, NULL);
    }

    SPUclose();
    SPUshutdown();

    return 0;
}
