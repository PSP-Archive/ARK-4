#include <pspsdk.h>
#include <pspkernel.h>
#include <psputility_sysparam.h>
#include <systemctrl.h>
#include <kubridge.h>
#include <stddef.h>

#include "globals.h"

#include "main.h"
#include "list.h"
#include "settings.h"

extern CFWConfig config;
extern ARKConfig* ark_config;

List custom_config;

static int processConfigLine(char* runlevel, char* path, char* enabled){
    
    int opt = runlevelConvert(runlevel, enabled);

    if (opt == CUSTOM) return 0;

    if (strcasecmp(path, "usbcharge") == 0){
        config.usbcharge = opt;
        return 1;
    }
    else if (strcasecmp(path, "overclock") == 0){
        config.overclock = opt;
        return 1;
    }
    else if (strcasecmp(path, "powersave") == 0){
        config.powersave = opt;
        return 1;
    }
    else if (strcasecmp(path, "launcher") == 0){
        config.launcher = opt;
        return 1;
    }
    else if (strcasecmp(path, "disablepause") == 0){
        config.disablepause = opt;
        return 1;
    }
    else if (strcasecmp(path, "highmem") == 0){
        config.highmem = opt;
        return 1;
    }
    else if (strcasecmp(path, "mscache") == 0){
        config.mscache = opt;
        return 1;
    }
    else if (strcasecmp(path, "infernocache") == 0){
        config.infernocache = opt;
        return 1;
    }
    else if (strcasecmp(path, "oldplugin") == 0){
        config.oldplugin = opt;
        return 1;
    }
    else if (strcasecmp(path, "skiplogos") == 0){
        config.skiplogos = opt;
        return 1;
    }
    else if (strcasecmp(path, "hidepics") == 0){
        config.hidepics = opt;
        return 1;
    }
    else if (strcasecmp(path, "hibblock") == 0){
        config.hibblock = opt;
        return 1;
    }
    else if (strcasecmp(path, "hidemac") == 0){
        config.hidemac = opt;
        return 1;
    }
    else if (strcasecmp(path, "hidedlc") == 0){
        config.hidedlc = opt;
        return 1;
    }
    else if (strcasecmp(path, "noled") == 0){
        config.noled = opt;
        return 1;
    }
    return 0;
}

static void processCustomConfig(char* line){
    add_list(&custom_config, line);
}

static void list_cleaner(void* item){
    my_free(item);
}

void loadSettings(){
    clear_list(&custom_config, &list_cleaner);

    char path[ARK_PATH_SIZE];
    strcpy(path, ark_config->arkpath);
    strcat(path, "SETTINGS.TXT");
    ProcessConfigFile(path, &processConfigLine, &processCustomConfig);
}

static void processSetting(int fd, char* line, char* name, int setting){
    switch (setting){
    default:
    case DISABLED:            snprintf(line, LINE_BUFFER_SIZE, "always, %s, off\n", name);  break;
    case ALWAYS_ON:           snprintf(line, LINE_BUFFER_SIZE, "always, %s, on\n", name);   break;
    case GAME_ONLY:           snprintf(line, LINE_BUFFER_SIZE, "game, %s, on\n", name);     break;
    case UMD_ONLY:            snprintf(line, LINE_BUFFER_SIZE, "umd, %s, on\n", name);      break;
    case HOMEBREW_ONLY:       snprintf(line, LINE_BUFFER_SIZE, "homebrew, %s, on\n", name); break;
    case POPS_ONLY:           snprintf(line, LINE_BUFFER_SIZE, "pops, %s, on\n", name);     break;
    case VSH_ONLY:            snprintf(line, LINE_BUFFER_SIZE, "vsh, %s, on\n", name);      break;
    case LAUNCHER_ONLY:       snprintf(line, LINE_BUFFER_SIZE, "launcher, %s, on\n", name); break;
    }
    sceIoWrite(fd, line, strlen(line));
}

void saveSettings(){

    char path[ARK_PATH_SIZE];
    strcpy(path, ark_config->arkpath);
    strcat(path, "SETTINGS.TXT");

    int fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

    if (fd < 0){
        return;
    }

    char* line = my_malloc(LINE_BUFFER_SIZE);

    processSetting(fd, line, "usbcharge", config.usbcharge);
    processSetting(fd, line, "overclock", config.overclock);
    processSetting(fd, line, "powersave", config.powersave);
    processSetting(fd, line, "launcher", config.launcher);
    processSetting(fd, line, "disablepause", config.disablepause);
    processSetting(fd, line, "highmem", config.highmem);
    processSetting(fd, line, "mscache", config.mscache);
    processSetting(fd, line, "infernocache", config.infernocache);
    processSetting(fd, line, "oldplugin", config.oldplugin);
    processSetting(fd, line, "skiplogos", config.skiplogos);
    processSetting(fd, line, "hidepics", config.hidepics);
    processSetting(fd, line, "hibblock", config.hibblock);
    processSetting(fd, line, "hidemac", config.hidemac);
    processSetting(fd, line, "hidedlc", config.hidedlc);
    processSetting(fd, line, "noled", config.noled);

    for (int i=0; i<custom_config.count; i++){
        sceIoWrite(fd, custom_config.table[i], strlen(custom_config.table[i]));
        sceIoWrite(fd, "\n", 1);
    }

    sceIoClose(fd);

    my_free(line);

    //clear_list(&custom_config);
}
