#ifndef PLUGINS_H
#define PLUGINS_H

#include <pspsdk.h>
#include <pspkernel.h>
#include <psputility_sysparam.h>
#include <systemctrl.h>
#include <kubridge.h>
#include <stddef.h>

enum{
    PLACE_MS0,
    PLACE_EF0,
    PLACE_ARK_PATH
};

enum{
    PLUGIN_OFF,
    PLUGIN_ON,
    PLUGIN_REMOVED,
};

typedef struct{
    char* name;
    char* path;
    char* surname;
    int active;
    int place;
} Plugin;

#endif