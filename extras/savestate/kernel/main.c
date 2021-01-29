/*
	TN SaveState Plugin
	Copyright (C) 2014, Total_Noob

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../common.h"
#include "main.h"

PSP_MODULE_INFO("SaveState", 0x1007, 1, 0);

STMOD_HANDLER previous;

void logmsg(char *msg)
{
	SceUID fd = sceIoOpen("ms0:/kernel_log.txt", PSP_O_WRONLY | PSP_O_CREAT, 0777);
	if(fd >= 0)
	{
		sceIoLseek(fd, 0, PSP_SEEK_END);
		sceIoWrite(fd, msg, strlen(msg));
		sceIoClose(fd);
	}
}

void ClearCaches()
{
	sceKernelDcacheWritebackAll();
	sceKernelIcacheClearAll();
}

int HoldButtons(SceCtrlData *pad, u32 buttons, u32 time)
{
	if((pad->Buttons & buttons) == buttons)
	{
		u32 time_start = sceKernelGetSystemTimeLow();

		while((pad->Buttons & buttons) == buttons)
		{
			sceKernelDelayThread(100 * 1000);
			sceCtrlPeekBufferPositive(pad, 1);

			if((sceKernelGetSystemTimeLow() - time_start) >= time)
			{
				return 1;
			}
		}
	}

	return 0;
}

#include <pspsysmem_kernel.h>

void LoadUserModule()
{
	int i;
	for(i = 0; i < 12; i++)
	{
		PspSysmemPartitionInfo info;
		memset(&info, 0, sizeof(PspSysmemPartitionInfo));
		info.size = sizeof(PspSysmemPartitionInfo);
		if(sceKernelQueryMemoryPartitionInfo(i, &info) == 0)
		{
			log("Partition %d: 0x%08X-0x%08X (0x%08X/0x%08X)\n", i, info.startaddr, info.startaddr + info.memsize, sceKernelPartitionTotalFreeMemSize(i), info.memsize);
		}
	}

	SceKernelLMOption lmoption;
	memset(&lmoption, 0, sizeof(SceKernelLMOption));
	lmoption.size = sizeof(SceKernelLMOption);
	lmoption.mpidtext = 11;
	lmoption.mpiddata = 11;
	lmoption.access = 1;

	SceUID mod = sceKernelLoadModule("ms0:/PSP/SAVEDATA/NPEZ00053SLOT00/SUSER.PRX", 0, &lmoption);
	if(mod >= 0) sceKernelStartModule(mod, 0, NULL, NULL, NULL);
}

int ctrl_thread(SceSize args, void *argp)
{
    while(1)
	{
		SceCtrlData pad;
		sceCtrlPeekBufferPositive(&pad, 1);

		if(HoldButtons(&pad, PSP_CTRL_START, 1 * 1000 * 1000))
		{
			LoadUserModule();
		}

		sceKernelDelayThread(100 * 1000);
	}

	return 0;
}

int OnModuleStart(SceModule2 *mod)
{

	if(!previous)
		return 0;

	return previous(mod);
}

int module_start(SceSize args, void *argp)
{
	SceUID thid = sceKernelCreateThread("ctrl_thread", ctrl_thread, 0x10, 0x1000, 0, NULL);
	if(thid >= 0) sceKernelStartThread(thid, 0, NULL);
	
	previous = sctrlHENSetStartModuleHandler(OnModuleStart);

	return 0;
}
