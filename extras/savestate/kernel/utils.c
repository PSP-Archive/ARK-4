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

#define MAX_THREADS 32

SceUID threads[MAX_THREADS];
int thread_count = 0;

void GetThreads()
{	
	int k1 = pspSdkSetK1(0);

	sceKernelGetThreadmanIdList(SCE_KERNEL_TMID_Thread, threads, sizeof(threads) / sizeof(SceUID), &thread_count);

	int i;
	for(i = 0; i < thread_count; i++)
	{
		SceKernelThreadInfo info;
		info.size = sizeof(SceKernelThreadInfo);
		if(sceKernelReferThreadStatus(threads[i], &info) == 0)
		{
			if((info.status & PSP_THREAD_RUNNING) == 0 && (info.status & PSP_THREAD_SUSPEND) == 0)
			{
				if(strcmp(info.name, "ScePowerMain") != 0 && strcmp(info.name, "SceKermitMsfsClose") != 0)
				{
					continue;
				}
/*
				// User only
				if(info.attr & PSP_THREAD_ATTR_USER)
				{
					continue;
				}
*/
			}
		}

		threads[i] = -1;
	}

	pspSdkSetK1(k1);
}

void SuspendThreads()
{
	int k1 = pspSdkSetK1(0);

	int i;
	for(i = thread_count; i > 0; i--)
	{
		if(threads[i] >= 0)
		{
			while(sceKermitIsDone() == 0)
			{
				sceKernelDelayThread(10);
			}

			sceKernelSuspendThread(threads[i]);
		}
	}

	pspSdkSetK1(k1);
}

void ResumeThreads()
{
	int k1 = pspSdkSetK1(0);

	int i;
	for(i = 0; i < thread_count; i++)
	{
		if(threads[i] >= 0)
		{
			sceKernelResumeThread(threads[i]);
		}
	}

	pspSdkSetK1(k1);
}