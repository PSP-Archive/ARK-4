#include "list.h"

void add_list(List* list, void* item){
    if (list->table == NULL){
        list->table = malloc(sizeof(void*)*8);
        memset(list->table, 0, sizeof(void*)*8);
        list->max = 8;
        list->count = 0;
    }
    if (list->count >= list->max){
        void** old_table = list->table;
        list->max *= 2;
        list->table = malloc(sizeof(void*)*list->max);
        memset(list->table, 0, sizeof(void*)*list->max);
        for (int i=0; i<list->count; i++){
            list->table[i] = old_table[i];
        }
        free(old_table);
    }
    list->table[list->count++] = item;
}

void clear_list(List* list, void (*cleaner)(void*)){
    if (list->table == NULL) return;
    for (int i=0; i<list->count; i++){
        cleaner(list->table[i]);
    }
    free(list->table);
    list->table = NULL;
    list->count = 0;
    list->max = 0;
}