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

#ifndef __START_H__
#define __START_H__

extern "C"{

extern int pmode, pwidth, pheight, pbufferwidth, ppixelformat;
extern void *vram_buffer, *pvram, *pvram_bak;

void GetThreads();
void SuspendThreads();
void ResumeThreads();

int sceDmacMemcpy(void *pDst, const void *pSrc, unsigned int uiSize);

SceUID _sceKernelAllocPartitionMemory(SceUID partitionid, const char *name, int type, SceSize size, void *addr);

int __psp_free_heap(void);

}

#endif
