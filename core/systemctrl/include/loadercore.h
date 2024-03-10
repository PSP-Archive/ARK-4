/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

#ifndef _LOADERCORE_H_
#define _LOADERCORE_H_

 typedef struct SceStubLibrary {
     u32 unk0; //0
     struct SceStubLibrary *next; //4
     const char *libName; //8
     u8 version[2]; //12
     u16 attribute; //14
     u8 stubEntryTableLen; //16
     u8 vStubCount; //17
     u16 stubCount; //18
     u32 *nidTable; //20
     void *stubTable; //24
     void *vStubTable; //28
     u16 unk32; //32
     void *libStubTable; //36
     u32 status; //40
     u32 isUserLib; //44
     char *libName2; //48
     u32 libNameInHeap; //52
 } SceStubLibrary; //size = 56

// init.prx Text Address
extern unsigned int sceInitTextAddr;

// init.prx Custom sceKernelStartModule Handler
extern int (* customStartModule)(int modid, SceSize argsize, void * argp, int * modstatus, SceKernelSMOption * opt);

// Patch Loader Core Module
SceModule2* patchLoaderCore(void);

#endif

