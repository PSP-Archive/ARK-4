/* DO NOT INCLUDE THIS FILE ANYWHERE OTHER THAN vshmenu.cpp ! */

/* structure defining a common entry layout */

enum{
    DISABLED,
    ALWAYS_ON,
    GAME_ONLY,
    UMD_ONLY,
    HOMEBREW_ONLY,
    POPS_ONLY,
    VSH_ONLY
};

enum{
    REGION_DEFAULT,
    REGION_JAPAN,
    REGION_AMERICA,
    REGION_EUROPE
};

typedef struct {
    unsigned char usbcharge;
    unsigned char overclock;
    unsigned char powersave;
    unsigned char launcher;
    unsigned char disablepause;
    unsigned char highmem;
    unsigned char mscache;
    unsigned char infernocache;
    unsigned char oldplugin;
    unsigned char regionchange;
}ArkConf;

ArkConf ark_config;

#define MAX_ARK_OPTIONS 7
#define ARK_OPTIONS { \
    "Disabled", \
    "Always", \
    "Game", \
    "UMD", \
    "Homebrew", \
    "Pops", \
    "VSH" \
}

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[MAX_ARK_OPTIONS];
} usbcharge = {
    "USB Charge",
    MAX_ARK_OPTIONS,
    0,
    &(ark_config.usbcharge),
    ARK_OPTIONS
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[MAX_ARK_OPTIONS];
} overclock = {
    "OverClock",
    MAX_ARK_OPTIONS,
    0,
    &(ark_config.overclock),
    ARK_OPTIONS
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[MAX_ARK_OPTIONS];
} powersave = {
    "PowerSave",
    MAX_ARK_OPTIONS,
    0,
    &(ark_config.powersave),
    ARK_OPTIONS
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[MAX_ARK_OPTIONS];
} launcher = {
    "Autoboot Launcher",
    MAX_ARK_OPTIONS,
    0,
    &(ark_config.launcher),
    ARK_OPTIONS
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[MAX_ARK_OPTIONS];
} disablepause = {
    "Disable PSP Go Pause",
    MAX_ARK_OPTIONS,
    0,
    &(ark_config.disablepause),
    ARK_OPTIONS
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[MAX_ARK_OPTIONS];
} highmem = {
    "Unlock Extra Memory",
    MAX_ARK_OPTIONS,
    0,
    &(ark_config.highmem),
    ARK_OPTIONS
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[MAX_ARK_OPTIONS];
} mscache = {
    "Memory Stick Speedup",
    MAX_ARK_OPTIONS,
    0,
    &(ark_config.mscache),
    ARK_OPTIONS
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[MAX_ARK_OPTIONS];
} infernocache = {
    "Inferno Cache",
    MAX_ARK_OPTIONS,
    0,
    &(ark_config.infernocache),
    ARK_OPTIONS
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[MAX_ARK_OPTIONS];
} oldplugin = {
    "Old Plugins on PSP Go",
    MAX_ARK_OPTIONS,
    0,
    &(ark_config.oldplugin),
    ARK_OPTIONS
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[MAX_ARK_OPTIONS];
} regionchange = {
    "UMD Region Change",
    4,
    0,
    &(ark_config.regionchange),
    {"Default", "Japan", "America", "Europe"}
};

settings_entry* ark_conf_entries[] = {
    (settings_entry*)&usbcharge,
    (settings_entry*)&overclock,
    (settings_entry*)&powersave,
    (settings_entry*)&launcher,
    (settings_entry*)&disablepause,
    (settings_entry*)&highmem,
    (settings_entry*)&mscache,
    (settings_entry*)&infernocache,
    (settings_entry*)&oldplugin,
    (settings_entry*)&regionchange,
};

#define MAX_ARK_CONF 10

bool isComment(string line){
    return (line[0] == '#' || line[0] == ';' || (line[0]=='/'&&line[1]=='/'));
}

bool isRunlevelEnabled(string line){
    return (line == "on" || line == "1" || line == "enabled" || line == "true");
}

static unsigned char runlevelConvert(string runlevel, string enable){
    if (!isRunlevelEnabled(enable)) return DISABLED;
    else if (strcasecmp(runlevel.c_str(), "always") == 0 || strcasecmp(runlevel.c_str(), "all") == 0){
        return ALWAYS_ON;
    }
    else if (strcasecmp(runlevel.c_str(), "game") == 0){
        return GAME_ONLY;
    }
    else if (strcasecmp(runlevel.c_str(), "umd") == 0){
        return UMD_ONLY;
    }
    else if (strcasecmp(runlevel.c_str(), "homebrew") == 0){
        return HOMEBREW_ONLY;
    }
    else if (strcasecmp(runlevel.c_str(), "pops") == 0){
        return POPS_ONLY;
    }
    else if (strcasecmp(runlevel.c_str(), "vsh") == 0){
        return VSH_ONLY;
    }
    return DISABLED;
}

static unsigned char* configConvert(string conf){
    if (strcasecmp(conf.c_str(), "usbcharge") == 0){
        return &(ark_config.usbcharge);
    }
    else if (strcasecmp(conf.c_str(), "overclock") == 0){
        return &(ark_config.overclock);
    }
    else if (strcasecmp(conf.c_str(), "powersave") == 0){
        return &(ark_config.powersave);
    }
    else if (strcasecmp(conf.c_str(), "launcher") == 0){
        return &(ark_config.launcher);
    }
    else if (strcasecmp(conf.c_str(), "disablepause") == 0){
        return &(ark_config.disablepause);
    }
    else if (strcasecmp(conf.c_str(), "highmem") == 0){
        return &(ark_config.highmem);
    }
    else if (strcasecmp(conf.c_str(), "mscache") == 0){
        return &(ark_config.mscache);
    }
    else if (strcasecmp(conf.c_str(), "infernocache") == 0){
        return &(ark_config.infernocache);
    }
    else if (strcasecmp(conf.c_str(), "oldplugin") == 0){
        return &(ark_config.oldplugin);
    }
    else if (strcasecmp(conf.c_str(), "region_jp") == 0){
        ark_config.regionchange = REGION_JAPAN;
        return NULL;
    }
    else if (strcasecmp(conf.c_str(), "region_us") == 0){
        ark_config.regionchange = REGION_AMERICA;
        return NULL;
    }
    else if (strcasecmp(conf.c_str(), "region_eu") == 0){
        ark_config.regionchange = REGION_EUROPE;
        return NULL;
    }
    return NULL;
}

static void processConfig(string runlevel, string conf, string enable){
    unsigned char* config_ptr = configConvert(conf);
    unsigned char config = runlevelConvert(runlevel, enable);
    if (config_ptr != NULL){
        *config_ptr = config;
    }
}

static void processLine(string line){
    stringstream ss(line);
    vector<string> result;
    while( ss.good() )
    {
        string substr;
        getline( ss, substr, ',' );
        // trim string
        std::stringstream trimmer;
        trimmer << substr;
        trimmer.clear();
        trimmer >> substr;

        result.push_back(substr);
    }
    
    string runlevel = result[0];
    string conf = result[1];
    string enable = result[2];
    
    processConfig(runlevel, conf, enable);
}

void loadSettings(){
    std::ifstream input("SETTINGS.TXT");
    for( std::string line; getline( input, line ); ){
        if (isComment(line)) continue;
        processLine(line);
    }
    input.close();
}

static string processSetting(string name, unsigned char setting){
    switch (setting){
    case DISABLED:            return "always, "+name+", off";
    case ALWAYS_ON:           return "always, "+name+", on";
    case GAME_ONLY:           return "game, "+name+", on";
    case UMD_ONLY:            return "umd, "+name+", on";
    case HOMEBREW_ONLY:       return "homebrew, "+name+", on";
    case POPS_ONLY:           return "pops, "+name+", on";
    case VSH_ONLY:            return "vsh, "+name+", on";
    }
    return "always, "+name+", off";
}

void saveSettings(){
    std::ofstream output("SETTINGS.TXT");
    output << processSetting("usbcharge", ark_config.usbcharge) << endl;
    output << processSetting("overclock", ark_config.overclock) << endl;
    output << processSetting("powersave", ark_config.powersave) << endl;
    output << processSetting("launcher", ark_config.launcher) << endl;
    output << processSetting("disablepause", ark_config.disablepause) << endl;
    output << processSetting("highmem", ark_config.highmem) << endl;
    output << processSetting("mscache", ark_config.mscache) << endl;
    output << processSetting("infernocache", ark_config.infernocache) << endl;
    output << processSetting("oldplugin", ark_config.oldplugin) << endl;
    
    switch (ark_config.regionchange){
        case REGION_JAPAN:
            output << "vsh, region_jp, on" << endl;
            break;
        case REGION_AMERICA:
            output << "vsh, region_us, on" << endl;
            break;
        case REGION_EUROPE:
            output << "vsh, region_eu, on" << endl;
            break;
    }
    
    output.close();
}
