#include "funcs.h"

int sceKernelLoadModule661(char* a0, int a1, void* a2){
    static int (*orig)(char*, int, void*) = NULL;

    if (orig == NULL){
        orig = sctrlHENFindFunction("sceModuleManager", "ModuleMgrForKernel", 0x939E4270);
    }

    return orig(a0, a1, a2);
}

int sceKernelLoadModuleBufferBootInitBtcnf661(void* a0, void* a1, int a2, void* a3){
    static int (*orig)(void*, void*, int, void*) = NULL;

    if (orig == NULL){
        orig = sctrlHENFindFunction("sceModuleManager", "ModuleMgrForKernel", 0x1CF0B794);
    }

    return orig(a0, a1, a2, a3);
}

int sceKermitRegisterVirtualIntrHandler661(u32 a0, void* a1){
    static int (*orig)(int, void*) = NULL;

    if (orig == NULL){
        orig = sctrlHENFindFunction("sceKermit_Driver", "sceKermit_driver", 0x5280B410);
    }

    return orig(a0, a1);
}

int sceKernelAllocPartitionMemory661(int a0, char* a1, int a2, int a3, void* a4){
    static int (*orig)(int, char*, int, int, void*) = NULL;

    if (orig == NULL){
        orig = sctrlHENFindFunction("sceSystemMemoryManager", "SysMemForKernel", 0x7158CE7E);
    }

    return orig(a0, a1, a2, a3, a4);
}

void* sceKernelGetBlockHeadAddr661(int a0){
    static void* (*orig)(int) = NULL;

    if (orig == NULL){
        orig = sctrlHENFindFunction("sceSystemMemoryManager", "SysMemForKernel", 0xF12A62F7);
    }

    return orig(a0);
}

int sceKernelFreePartitionMemory661(int a0){
    static int (*orig)(int) = NULL;

    if (orig == NULL){
        orig = sctrlHENFindFunction("sceSystemMemoryManager", "SysMemForKernel", 0xC1A26C6F);
    }

    return orig(a0);
}

void* sceKernelGetGameInfo661(){
    static int (*orig)() = NULL;

    if (orig == NULL){
        orig = sctrlHENFindFunction("sceSystemMemoryManager", "SysMemForKernel", 0xEF29061C);
    }

    return orig();
}

int sceKermitSendRequest661(void* a0, int a1, int a2, int a3, int a4, void* a5){
    static int (*orig)(void*, int, int, int, int, void*) = NULL;

    if (orig == NULL){
        orig = sctrlHENFindFunction("sceKermit_Driver", "sceKermit_driver",0x36666181);
    }

    return orig(a0, a1, a2, a3, a4, a5);
}

void* sceKernelFindModuleByName661(char* a0){
    static int (*orig)(char*) = NULL;

    if (orig == NULL){
        orig = sctrlHENFindFunction("sceLoaderCore", "LoadCoreForKernel", 0xF6B1BF0F);
    }

    return orig(a0);
}