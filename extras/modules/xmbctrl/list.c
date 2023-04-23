#include "list.h"

void* my_malloc(size_t size){
    SceUID uid = sceKernelAllocPartitionMemory(2, "", PSP_SMEM_High, size+sizeof(u32), NULL);
    int* ptr = sceKernelGetBlockHeadAddr(uid);
    ptr[0] = uid;
    return &(ptr[1]);
}

void my_free(int* ptr){
    int uid = ptr[-1];
    sceKernelFreePartitionMemory(uid);
}

void add_list(List* list, void* item){
    if (list->table == NULL){
        list->table = my_malloc(sizeof(void*)*8);
        memset(list->table, 0, sizeof(void*)*8);
        list->max = 8;
        list->count = 0;
    }
    if (list->count >= list->max){
        void** old_table = list->table;
        list->max *= 2;
        list->table = my_malloc(sizeof(void*)*list->max);
        memset(list->table, 0, sizeof(void*)*list->max);
        for (int i=0; i<list->count; i++){
            list->table[i] = old_table[i];
        }
        my_free(old_table);
    }
    list->table[list->count++] = item;
}

void clear_list(List* list, void (*cleaner)(void*)){
    if (list->table == NULL) return;
    for (int i=0; i<list->count; i++){
        cleaner(list->table[i]);
    }
    my_free(list->table);
    list->table = NULL;
    list->count = 0;
    list->max = 0;
}