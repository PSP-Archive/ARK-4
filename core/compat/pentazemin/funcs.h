#ifndef FUNCS_H
#define FUNCS_H

#include <pspsdk.h>

int sceKernelLoadModule661(char*, int, void*);

int sceKernelLoadModuleBufferBootInitBtcnf661(void*, void*, int, void*);

int sceKermitRegisterVirtualIntrHandler661(u32, void*);

int sceKernelAllocPartitionMemory661(int, char*, int, int, void*);

void* sceKernelGetBlockHeadAddr661(int);

int sceKernelFreePartitionMemory661(int);

void* sceKernelGetGameInfo661();

int sceKermitSendRequest661(void*, int, int, int, int, void*);

void* sceKernelFindModuleByName661(char*);

#endif