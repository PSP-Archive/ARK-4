#include "main.h"
#include <functions.h>

#define BUF_SIZE 1024*32

extern ARKConfig* ark_config;

int (*IdStorageReadLeaf)(u16, u8*);
int (*KernelGetUserLevel)();


// Set User Level
int sctrlKernelSetUserLevel(int level)
{
    
    // Backup User Level
    int previouslevel = KernelGetUserLevel();
    
    
    u32 _sceKernelReleaseThreadEventHandler = FindFunction("sceThreadManager", "ThreadManForKernel", 0x72F3C145);
    
    u32 threadman_userlevel_struct = _lh(_sceKernelReleaseThreadEventHandler + 0x4)<<16;
    threadman_userlevel_struct += (short)_lh(_sceKernelReleaseThreadEventHandler + 0x18);
    
    
    // Set User Level
    _sw((level ^ 8) << 28, *(unsigned int *)(threadman_userlevel_struct) + 0x14);
    
    // Flush Cache
    k_tbl->KernelDcacheWritebackInvalidateAll();
    
    // Return previous User Level
    return previouslevel;
}

int dcIdStorageReadLeaf(u16 leafid, u8 *buf)
{
    int level = sctrlKernelSetUserLevel(8);

    int res = IdStorageReadLeaf(leafid, buf);

    sctrlKernelSetUserLevel(level);

    return res;
}

void dump_idStorage(){
    static u8 buf[512];
    int fd = k_tbl->KernelIOOpen("ms0:/idStorage.bin", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    
    for (int i=0; i<0x140; i++){
        dcIdStorageReadLeaf(i, buf);
        k_tbl->KernelIOWrite(fd, buf, 512);
    }
    k_tbl->KernelIOClose(fd);
}

int kthread(SceSize args, void *argp){

    PRTSTR("Dumping idStorage");
    dump_idStorage();
    k_tbl->KernelExitThread(0);

    return 0;
}

void initKernelThread(){
    SceUID kthreadID = k_tbl->KernelCreateThread( "arkflasher", (void*)KERNELIFY(&kthread), 1, 0x20000, PSP_THREAD_ATTR_VFPU, NULL);
    if (kthreadID >= 0){
        // start thread and wait for it to end
        k_tbl->KernelStartThread(kthreadID, 0, NULL);
        k_tbl->waitThreadEnd(kthreadID, NULL);
        k_tbl->KernelDeleteThread(kthreadID);
    }
}

// Kernel Permission Function
void loadKernelArk(){

    KernelGetUserLevel = FindFunction("sceThreadManager", "ThreadManForKernel", 0xF6427665);
    IdStorageReadLeaf = FindFunction("sceIdStorage_Service", "sceIdStorage_driver", 0xEB00C509);
    

    
    if (IdStorageReadLeaf == NULL){
        PRTSTR("ERROR: cannot find import IdStorageReadLeaf");
        return;
    }

    if (KernelGetUserLevel == NULL){
        PRTSTR("ERROR: cannot find import KernelGetUserLevel");
        return;
    }

    

    
    

    initKernelThread();

    

    PRTSTR("Done");
    
    return;
}