#ifndef LIST_H
#define LIST_H

#include <pspsdk.h>
#include <pspkernel.h>
#include <psputility_sysparam.h>
#include <systemctrl.h>
#include <kubridge.h>
#include <stddef.h>

typedef struct List{
    void** table;
    int count;
    int max;
} List;

void add_list(List* list, void* item);

void clear_list(List* list, void (*cleaner)(void*));

#endif
