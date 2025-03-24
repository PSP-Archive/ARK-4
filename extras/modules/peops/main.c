/*
    Custom Emulator Firmware
    Copyright (C) 2012-2014, ColdBird/Total_Noob/Acid_Snake

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
#include <macros.h>
#include <module2.h>
#include "main.h"

#include "spu/stdafx.h"

PSP_MODULE_INFO("peops", 0x0007, 1, 0);

void (*previous)(void*);

void (* spuWriteRegister)(int reg, int val, int type);
void (* cdrTransferSector)(u8 *sector, int mode);
void (* cdrWriteRegister)(int reg, int val);

int cdr_is_first_sector = 1;
xa_decode_t cdr_xa;

void SPUwait()
{
    sceKernelDelayThread(SPU_DELAY);
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

void sceMeAudioInitPatched(int (* function)(), void *stack)
{
    SPUinit();
    SPUopen();

    SceUID thid = sceKernelCreateThread("spu_thread", spu_thread, SPU_PRIORITY_VERY_HIGH, 0x4000, 0, NULL);
    if(thid >= 0) sceKernelStartThread(thid, 0, NULL);
}

void sceMeAudioNotifyPatched(int a0)
{
    *(u8 *)0x49F40290 = 0;
    (*(u8 *)0x49F40291)++;
    *(u8 *)0x49F40293 = -2;
}

void spuWriteRegisterPatched(int reg, int val, int type)
{
    SPUwriteRegister(reg, val);
    spuWriteRegister(reg, val, type);
}

void cdrTransferSectorPatched(u8 *sector, int mode)
{
    if(mode == 1)
    {
        SPUplayCDDAchannel(sector, 0x930);
    }
    else if(mode == 2)
    {
        if(cdr_is_first_sector != -1)
        {
        	u8 *buf = sector + 0xC;

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

    cdrTransferSector(sector, mode);
}

void cdrWriteRegisterPatched(int reg, int val)
{
    if(reg == 0x1F801801)
    {
        if(val == 6 || val == 27)
        {
        	cdr_is_first_sector = 1;
        }
    }

    cdrWriteRegister(reg, val);
}

void PatchPops(SceModule2* mod)
{

    u32 text_addr = mod->text_addr;

    MAKE_CALL(text_addr + 0x1A038, sceMeAudioInitPatched);

    REDIRECT_FUNCTION(text_addr + 0x3D264, sceMeAudioNotifyPatched);

    HIJACK_FUNCTION(text_addr + 0x7F00, spuWriteRegisterPatched, spuWriteRegister);
    HIJACK_FUNCTION(text_addr + 0x83E8, cdrTransferSectorPatched, cdrTransferSector);
    HIJACK_FUNCTION(text_addr + 0xD1B0, cdrWriteRegisterPatched, cdrWriteRegister);

    sceKernelDcacheWritebackAll();
}

void PeopsOnModuleStart(SceModule2 * mod){

    // Patch PSP POPS SPU
    if (strcmp(mod->modname, "pops") == 0)
    {
        PatchPops(mod);
    }

    // Forward to previous Handler
    if(previous) previous(mod);

}

int module_start(SceSize args, void *argp)
{

    // Register Module Start Handler
    previous = sctrlHENSetStartModuleHandler(PeopsOnModuleStart);
    
    return 0;
}