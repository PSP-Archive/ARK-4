#include "funcs.h"

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