#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pspsdk.h>
#include <pspsysmem.h>
#include <macros.h>

// patch to make malloc/free thread-safe.

static SceUID sema = -1;
static void* (*og_malloc)(size_t) = NULL;
static void (*og_free)(void*) = NULL;

void* my_malloc(size_t size){
    sceKernelWaitSema(sema, 1, NULL);
    void* res = og_malloc(size);
    sceKernelSignalSema(sema, 1);
    return res;
}

void my_free(void* ptr){
    sceKernelWaitSema(sema, 1, NULL);
    og_free(ptr);
    sceKernelSignalSema(sema, 1);
}

void my_malloc_init(){
    if (sema < 0){
        sema = sceKernelCreateSema("malloc_sema",  0, 1, 1, NULL);
    }
    HIJACK_FUNCTION(&malloc, my_malloc, og_malloc);
    HIJACK_FUNCTION(&free, my_free, og_free);
}