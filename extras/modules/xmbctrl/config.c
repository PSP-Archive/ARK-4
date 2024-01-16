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
    else if (strcasecmp(path, "defaultclock") == 0){
        config.defaultclock = opt;
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
    else if (strncasecmp(path, "infernocache", 12) == 0){
        char* c = strchr(path, ':');
        FIX_BOOLEAN(opt);
        config.infernocache = opt;
        if (opt && c){
            if (strcasecmp(c+1, "lru") == 0) config.infernocache = 1;
            else if (strcasecmp(c+1, "rr") == 0) config.infernocache = 2;
        }
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
    else if (strcasecmp(path, "noumd") == 0){
        config.noumd = opt;
        return 1;
    }
    else if (strcasecmp(path, "noanalog") == 0){
        config.noanalog = opt;
        return 1;
    }
    else if (strcasecmp(path, "qaflags") == 0){
        config.qaflags = opt;
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

    FIX_BOOLEAN(config.launcher);
    FIX_BOOLEAN(config.disablepause);
    FIX_BOOLEAN(config.skiplogos);
    FIX_BOOLEAN(config.hidepics);
    FIX_BOOLEAN(config.hibblock);
    FIX_BOOLEAN(config.hidemac);
    FIX_BOOLEAN(config.hidedlc);
    FIX_BOOLEAN(config.noumd);
    FIX_BOOLEAN(config.noanalog);
    FIX_BOOLEAN(config.qaflags);
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
    processSetting(fd, line, "defaultclock", config.defaultclock);
    processSetting(fd, line, "launcher", config.launcher);
    processSetting(fd, line, "disablepause", config.disablepause);
    processSetting(fd, line, "highmem", config.highmem);
    processSetting(fd, line, "mscache", config.mscache);
    switch (config.infernocache){
        case 0: processSetting(fd, line, "infernocache", 0); break;
        case 1: processSetting(fd, line, "infernocache:lru", 1); break;
        case 2: processSetting(fd, line, "infernocache:rr", 1); break;
    }
    processSetting(fd, line, "oldplugin", config.oldplugin);
    processSetting(fd, line, "skiplogos", config.skiplogos);
    processSetting(fd, line, "hidepics", config.hidepics);
    processSetting(fd, line, "hibblock", config.hibblock);
    processSetting(fd, line, "hidemac", config.hidemac);
    processSetting(fd, line, "hidedlc", config.hidedlc);
    processSetting(fd, line, "noled", config.noled);
    processSetting(fd, line, "noumd", config.noumd);
    processSetting(fd, line, "noanalog", config.noanalog);
    processSetting(fd, line, "qaflags", config.qaflags);

    for (int i=0; i<custom_config.count; i++){
        sceIoWrite(fd, custom_config.table[i], strlen(custom_config.table[i]));
        sceIoWrite(fd, "\n", 1);
    }

    sceIoClose(fd);

    my_free(line);

    //clear_list(&custom_config);
}
