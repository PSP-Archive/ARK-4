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

#include "spu/stdafx.h"

PSP_MODULE_INFO("peops", 0, 1, 0);

STMOD_HANDLER previous;

TNConfig config;

void (* libspuWrite)(int reg, int val, int type);
void (* libspuXaFunction)(u8 *sector, int mode);

int is_first_sector = 1;
xa_decode_t xa;

SceUID spu_thid = -1;

int spu_thread(SceSize args, void *argp)
{
	while(1)
	{
		SPUupdate();
		sceKernelDelayThread(2 * 1000);
	}

	return 0;
}

void sceMeAudioInitPatched(int (* function)(), void *stack)
{
	SPUinit();
	SPUopen();

	spu_thid = sceKernelCreateThread("spu_thread", spu_thread, 0x11, 0x8000, 0, NULL);
	if(spu_thid >= 0) sceKernelStartThread(spu_thid, 0, NULL);
}

void sceMeAudioNotifyPatched(int a0)
{
	*(short *)0x49F40290 = 0;
	*(char *)0x49F40293 = -2;
}

void libspuWritePatched(int reg, int val, int type)
{
	SPUwriteRegister(reg, val);
	libspuWrite(reg, val, type);
}

void libspuXaFunctionPatched(u8 *sector, int mode)
{
	u8 *buf = sector + 12;

	if(is_first_sector != -1)
	{
		if(buf[4 + 2] & 0x4)
		{
			int ret = xa_decode_sector(&xa, buf + 4, is_first_sector);
			if(ret == 0)
			{
				SPUplayADPCMchannel(&xa);
				is_first_sector = 0;
			}
			else
			{
				is_first_sector = -1;
			}
		}
	}

	libspuXaFunction(sector, mode);
}

int OnModuleStart(SceModule2 *mod)
{
	char *modname = mod->modname;
	u32 text_addr = mod->text_addr;

	if(strcmp(modname, "pops") == 0)
	{
		MAKE_CALL(text_addr + 0x1A038, sceMeAudioInitPatched);
		REDIRECT_FUNCTION(text_addr + 0x3D264, sceMeAudioNotifyPatched);

		HIJACK_FUNCTION(text_addr + 0x7F00, libspuWritePatched, libspuWrite);

		if(config.enablexaplaying)
		{
			HIJACK_FUNCTION(text_addr + 0x83E8, libspuXaFunctionPatched, libspuXaFunction);
		}

		sceKernelDcacheWritebackAll();
	}

	if(!previous)
		return 0;

	return previous(mod);
}

int module_start(SceSize args, void *argp)
{
	sctrlSEGetConfig(&config);
	previous = sctrlHENSetStartModuleHandler(OnModuleStart);
	return 0;
}

int module_stop(SceSize args, void *argp)
{
	if(spu_thid >= 0) sceKernelTerminateDeleteThread(spu_thid);

	SPUclose();
	SPUshutdown();

	return 0;
}