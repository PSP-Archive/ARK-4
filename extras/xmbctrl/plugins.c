#include "globals.h"

#include "list.h"
#include "settings.h"
#include "plugins.h"
#include "main.h"

List plugins;

extern ARKConfig* ark_config;

static char* sample_plugin_path = "ULUS01234, ms0:/SEPLUGINS/example.prx";

int cur_place = 0;

static void list_cleaner(void* item){
    Plugin* plugin = (Plugin*)item;
    my_free(plugin->path);
    if (plugin->name) my_free(plugin->name);
    if (plugin->surname) my_free(plugin->surname);
    my_free(plugin);
}

static void processCustomLine(char* line){
    Plugin* plugin = (Plugin*)my_malloc(sizeof(Plugin));
    memset(plugin, 0, sizeof(Plugin));
    plugin->path = line;
    plugin->place = cur_place;
    add_list(&plugins, plugin);
}

static void processPlugin(char* runlevel, char* path, char* enabled){
    int n = plugins.count;
    char* name = my_malloc(20);
    snprintf(name, 20, "plugin_%d", n);

    char* surname = my_malloc(20);
    snprintf(surname, 20, "plugins%d", n);

    int path_len = strlen(runlevel) + strlen(path) + 10;
    char* full_path = (char*)my_malloc(path_len);
    snprintf(full_path, path_len, "%s, %s", runlevel, path);

    Plugin* plugin = (Plugin*)my_malloc(sizeof(Plugin));
    plugin->name = name;
    plugin->surname = surname;
    plugin->path = full_path;
    plugin->place = cur_place;
    plugin->active = isRunlevelEnabled(enabled);

    add_list(&plugins, plugin);

    return 1;
}

void loadPlugins(){
    clear_list(&plugins, &list_cleaner);

    char path[ARK_PATH_SIZE];
    strcpy(path, ark_config->arkpath);
    strcat(path, "PLUGINS.TXT");
    
    cur_place = PLACE_ARK_PATH;
    ProcessConfigFile(path, &processPlugin, &processCustomLine);
    
    cur_place = PLACE_MS0;
    ProcessConfigFile("ms0:/SEPLUGINS/PLUGINS.TXT", &processPlugin, &processCustomLine);
    
    cur_place = PLACE_EF0;
    ProcessConfigFile("ef0:/SEPLUGINS/PLUGINS.TXT", &processPlugin, &processCustomLine);

    if (plugins.count == 0){
        // Add example plugin
        Plugin* plugin = (Plugin*)my_malloc(sizeof(Plugin));
        plugin->name = (char*)my_malloc(20);
        plugin->surname = (char*)my_malloc(20);
        plugin->path = (char*)my_malloc(strlen(sample_plugin_path)+1);
        plugin->active = 1;
        plugin->place = 0;
        strcpy(plugin->name, "plugin_0");
        strcpy(plugin->surname, "plugins0");
        strcpy(plugin->path, sample_plugin_path);
        add_list(&plugins, plugin);
    }
}

void savePlugins(){

    if (plugins.count == 0) return;

    if (plugins.count == 1){
        Plugin* plugin = (Plugin*)(plugins.table[0]);
        if (strcmp(plugin->path, sample_plugin_path) == 0){
            return;
        }
    }

    char path[ARK_PATH_SIZE];
    strcpy(path, ark_config->arkpath);
    strcat(path, "PLUGINS.TXT");

    int fd[3] = {
        sceIoOpen("ms0:/SEPLUGINS/PLUGINS.TXT", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777),
        sceIoOpen("ef0:/SEPLUGINS/PLUGINS.TXT", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777),
        sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777)
    };

    for (int i=0; i<plugins.count; i++){
        Plugin* plugin = (Plugin*)(plugins.table[i]);
        if (plugin->active == PLUGIN_REMOVED) continue;
        sceIoWrite(fd[plugin->place], plugin->path, strlen(plugin->path));
        if (plugin->name != NULL){
            char* sep = ", ";
            char* enabled = (plugin->active)? "on" : "off";
            sceIoWrite(fd[plugin->place], sep, strlen(sep));
            sceIoWrite(fd[plugin->place], enabled, strlen(enabled));
        }
        sceIoWrite(fd[plugin->place], "\n", 1);
    }

    sceIoClose(fd[0]);
    sceIoClose(fd[1]);
    sceIoClose(fd[2]);
}