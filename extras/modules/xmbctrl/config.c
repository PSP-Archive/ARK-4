#include <pspsdk.h>
#include <pspkernel.h>
#include <psputility_sysparam.h>
#include <systemctrl.h>
#include <kubridge.h>
#include <stddef.h>

#include <ark.h>

#include "main.h"
#include "list.h"
#include "settings.h"

enum{
    CLOCK_AUTO,
    OVERCLOCK,
    DEFAULTCLOCK,
    POWERSAVE
};

extern CFWConfig config;
extern ARKConfig* ark_config;

List custom_config;

static void convertClockConfig(unsigned char cfg, unsigned char value){
    // XMB only
    if (cfg == VSH_ONLY){
        config.clock_vsh = value;
    }
    // XMB and Game
    else if (cfg == ALWAYS_ON){
        config.clock_game = value;
        config.clock_vsh = value;
    }
    // Game only
    else if (cfg != DISABLED){
        config.clock_game = value;
    }
}

static int processConfigLine(char* runlevel, char* path, char* enabled){
    
    int opt = runlevelConvert(runlevel, enabled);

    if (opt == CUSTOM) return 0;

    if (strcasecmp(path, "usbcharge") == 0){
        config.usbcharge = opt;
        return 1;
    }
    else if (strcasecmp(path, "overclock") == 0){
        convertClockConfig(opt, OVERCLOCK);
        return 1;
    }
    else if (strcasecmp(path, "powersave") == 0){
        convertClockConfig(opt, POWERSAVE);
        return 1;
    }
    else if (strcasecmp(path, "defaultclock") == 0){
        convertClockConfig(opt, DEFAULTCLOCK);
        return 1;
    }
    else if (strcasecmp(path, "wpa2") == 0){
        config.wpa2 = opt;
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
    else if (strncasecmp(path, "skiplogos", 9) == 0){
        char* c = strchr(path, ':');
        FIX_BOOLEAN(opt);
        config.skiplogos = opt;
        if (opt && c){
            if (strcasecmp(c+1, "gameboot") == 0) config.skiplogos = 2;
            else if (strcasecmp(c+1, "coldboot") == 0) config.skiplogos = 3;
        }
        return 1;
    }
    else if (strncasecmp(path, "hidepics", 8) == 0){
        char* c = strchr(path, ':');
        FIX_BOOLEAN(opt);
        config.hidepics = opt;
        if (opt && c){
            if (strcasecmp(c+1, "pic0") == 0) config.hidepics = 2;
            else if (strcasecmp(c+1, "pic1") == 0) config.hidepics = 3;
        }
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
    else if (strncasecmp(path, "region_", 7) == 0){
        char* c = strchr(path, '_')+1;
        if (strcasecmp(c, "us") == 0){
            config.umdregion = (opt)?1:0;
        }
        else if (strcasecmp(c, "eu") == 0){
            config.umdregion = (opt)?2:0;
        }
        else if (strcasecmp(c, "jp") == 0){
            config.umdregion = (opt)?3:0;
        }
        return 1;
    }
    else if (strncasecmp(path, "fakeregion_", 11) == 0){
        int r = atoi(path+11);
        config.vshregion = (opt)?r:0;
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
    memset(&config, 0, sizeof(config));
    clear_list(&custom_config, &list_cleaner);

    char path[ARK_PATH_SIZE];
    strcpy(path, ark_config->arkpath);
    strcat(path, ARK_SETTINGS);
    SceUID check = sceIoOpen(path, PSP_O_RDONLY, 0);
    if(check < 0) {
        memset(path, 0, sizeof(path));
        strcpy(path, ARK_SETTINGS_FLASH);
    }
    else {
        sceIoClose(check);
    }


    ProcessConfigFile(path, &processConfigLine, &processCustomConfig);

    FIX_BOOLEAN(config.usbcharge);
    FIX_BOOLEAN(config.launcher);
    FIX_BOOLEAN(config.highmem);
    FIX_BOOLEAN(config.mscache);
    FIX_BOOLEAN(config.disablepause);
    FIX_BOOLEAN(config.oldplugin);
    FIX_BOOLEAN(config.hibblock);
    FIX_BOOLEAN(config.hidemac);
    FIX_BOOLEAN(config.hidedlc);
    FIX_BOOLEAN(config.noumd);
    FIX_BOOLEAN(config.noanalog);
    FIX_BOOLEAN(config.noled);
    FIX_BOOLEAN(config.qaflags);
    FIX_BOOLEAN(config.wpa2);
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

static void saveClockSetting(int output, char* category, int opt){
    if (opt){
        sceIoWrite(output, category, strlen(category));
        sceIoWrite(output, ", ", 2);
        switch (opt){
            case 1: sceIoWrite(output, "overclock, on", 13); break;
            case 2: sceIoWrite(output, "defaultclock, on", 16); break;
            case 3: sceIoWrite(output, "powersave, on", 13); break;
            default: sceIoWrite(output, "overclock, off", 14); break;
        }
        sceIoWrite(output, "\n", 1);
    }
}

void saveSettings(){

    char path[ARK_PATH_SIZE];
    strcpy(path, ark_config->arkpath);
    strcat(path, ARK_SETTINGS);

    int fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

    if (fd < 0){
        memset(path, 0, sizeof(path));
        strcpy(path, ARK_SETTINGS_FLASH);
        fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
    }

    char* line = my_malloc(LINE_BUFFER_SIZE);

    processSetting(fd, line, "usbcharge", config.usbcharge);

    if (config.clock_game == config.clock_vsh){
        // save both at once
        saveClockSetting(fd, "always", config.clock_game);
    }
    else {
        // Save CPU Clock in-game
        saveClockSetting(fd, "game", config.clock_game);
        // Save CPU Clock in-vsh
        saveClockSetting(fd, "vsh", config.clock_vsh);
    }

    processSetting(fd, line, "wpa2", config.wpa2);
    processSetting(fd, line, "launcher", config.launcher);
    processSetting(fd, line, "highmem", config.highmem);
    processSetting(fd, line, "mscache", config.mscache);
    switch (config.infernocache){
        case 0: processSetting(fd, line, "infernocache", 0); break;
        case 1: processSetting(fd, line, "infernocache:lru", 1); break;
        case 2: processSetting(fd, line, "infernocache:rr", 1); break;
    }
    processSetting(fd, line, "disablepause", config.disablepause);
    processSetting(fd, line, "oldplugin", config.oldplugin);
    processSetting(fd, line, "hibblock", config.hibblock);
    switch (config.skiplogos){
        case 0: processSetting(fd, line, "skiplogos", 0); break;
        case 1: processSetting(fd, line, "skiplogos", 1); break;
        case 2: processSetting(fd, line, "skiplogos:gameboot", 1); break;
        case 3: processSetting(fd, line, "skiplogos:coldboot", 1); break;
    }
    switch (config.hidepics){
        case 0: processSetting(fd, line, "hidepics", 0); break;
        case 1: processSetting(fd, line, "hidepics", 1); break;
        case 2: processSetting(fd, line, "hidepics:pic0", 1); break;
        case 3: processSetting(fd, line, "hidepics:pic1", 1); break;
    }
    processSetting(fd, line, "hidemac", config.hidemac);
    processSetting(fd, line, "hidedlc", config.hidedlc);
    processSetting(fd, line, "noled", config.noled);
    processSetting(fd, line, "noumd", config.noumd);
    processSetting(fd, line, "noanalog", config.noanalog);
    processSetting(fd, line, "qaflags", config.qaflags);
    switch (config.umdregion){
        case 1: processSetting(fd, line, "region_us", VSH_ONLY); break;
        case 2: processSetting(fd, line, "region_eu", VSH_ONLY); break;
        case 3: processSetting(fd, line, "region_jp", VSH_ONLY); break;
    }
    if (config.vshregion > 0){
        char tmp[32];
        snprintf(tmp, 32, "fakeregion_%d", config.vshregion);
        processSetting(fd, line, tmp, VSH_ONLY);
    }

    for (int i=0; i<custom_config.count; i++){
        sceIoWrite(fd, custom_config.table[i], strlen(custom_config.table[i]));
        sceIoWrite(fd, "\n", 1);
    }

    sceIoClose(fd);

    my_free(line);

    //clear_list(&custom_config);
}
