#include <pspsdk.h>

PSP_MODULE_INFO("FTPlibPSP", 0x0007, 1, 0);

void* malloc(int size){
    int uid = sceKernelAllocPartitionMemory(2, "", 1, size+sizeof(int), NULL);
    int* ptr = sceKernelGetBlockHeadAddr(uid);
    if (ptr){
        ptr[0] = uid;
        return &(ptr[1]);
    }
    return NULL;
}

void free(int* ptr){
    if (ptr){
        int uid = ptr[-1];
        sceKernelFreePartitionMemory(uid);
    }
}

void _start() __attribute__ ((weak, alias ("module_start")));
int module_start(){
    return 0;
}
