/* DO NOT INCLUDE THIS FILE ANYWHERE OTHER THAN vshmenu.cpp ! */

/* structure defining a common entry layout */

enum{
    DISABLED,
    ALWAYS_ON,
    GAME_ONLY,
    UMD_ONLY,
    HOMEBREW_ONLY,
    POPS_ONLY,
    VSH_ONLY,
    LAUNCHER_ONLY,
    CUSTOM
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
    unsigned char skiplogos;
    unsigned char regionchange;
    unsigned char vshregion;
    unsigned char hidepics;
    unsigned char hibblock;
    unsigned char hidemac;
    unsigned char hidedlc;
    unsigned char noled;
}ArkConf;

ArkConf ark_config;

#define MAX_ARK_OPTIONS 8
#define ARK_OPTIONS { \
    "Disabled", \
    "Always", \
    "Game", \
    "UMD/ISO", \
    "Homebrew", \
    "PS1", \
    "XMB", \
    "Launcher" \
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
    "Force Extra Memory",
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
} skiplogos = {
    "Skip Sony logos in XMB",
    MAX_ARK_OPTIONS,
    0,
    &(ark_config.skiplogos),
    ARK_OPTIONS
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[MAX_ARK_OPTIONS];
} hidepics = {
    "Hide PIC0 and PIC1 in XMB",
    MAX_ARK_OPTIONS,
    0,
    &(ark_config.hidepics),
    ARK_OPTIONS
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[MAX_ARK_OPTIONS];
} hibblock = {
    "Prevent hibernation deletion on PSP Go",
    MAX_ARK_OPTIONS,
    0,
    &(ark_config.hibblock),
    ARK_OPTIONS
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[MAX_ARK_OPTIONS];
} hidemac = {
    "Hide Mac Address",
    MAX_ARK_OPTIONS,
    0,
    &(ark_config.hidemac),
    ARK_OPTIONS
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[MAX_ARK_OPTIONS];
} hidedlc = {
    "Hide DLC",
    MAX_ARK_OPTIONS,
    0,
    &(ark_config.hidedlc),
    ARK_OPTIONS
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[MAX_ARK_OPTIONS];
} noled = {
    "Turn off LEDs",
    MAX_ARK_OPTIONS,
    0,
    &(ark_config.noled),
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

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[14];
} vshregion = {
    "VSH Region Change",
    14,
    0,
    &(ark_config.vshregion),
    {"Default", "Japan", "America", "Europe", "Korea", "United Kingdom", "Latin America", "Australia", "Hong Kong", "Taiwan", "Russia", "China", "Debug I", "Debug II"}
};

int ark_conf_max_entries = 0;
settings_entry** ark_conf_entries = NULL;

settings_entry* ark_conf_entries_1k[] = {
    (settings_entry*)&overclock,
    (settings_entry*)&powersave,
    (settings_entry*)&launcher,
    (settings_entry*)&mscache,
    (settings_entry*)&infernocache,
    (settings_entry*)&skiplogos,
    (settings_entry*)&hidepics,
    (settings_entry*)&hidemac,
    (settings_entry*)&hidedlc,
    (settings_entry*)&noled,
    (settings_entry*)&regionchange,
    (settings_entry*)&vshregion,
};
#define MAX_ARK_CONF_1K (sizeof(ark_conf_entries_1k)/sizeof(ark_conf_entries_1k[0]))

settings_entry* ark_conf_entries_slim[] = {
    (settings_entry*)&usbcharge,
    (settings_entry*)&overclock,
    (settings_entry*)&powersave,
    (settings_entry*)&launcher,
    (settings_entry*)&highmem,
    (settings_entry*)&mscache,
    (settings_entry*)&infernocache,
    (settings_entry*)&skiplogos,
    (settings_entry*)&hidepics,
    (settings_entry*)&hidemac,
    (settings_entry*)&hidedlc,
    (settings_entry*)&noled,
    (settings_entry*)&regionchange,
    (settings_entry*)&vshregion,
};
#define MAX_ARK_CONF_SLIM (sizeof(ark_conf_entries_slim)/sizeof(ark_conf_entries_slim[0]))

settings_entry* ark_conf_entries_go[] = {
    (settings_entry*)&usbcharge,
    (settings_entry*)&overclock,
    (settings_entry*)&powersave,
    (settings_entry*)&launcher,
    (settings_entry*)&disablepause,
    (settings_entry*)&highmem,
    (settings_entry*)&mscache,
    (settings_entry*)&infernocache,
    (settings_entry*)&oldplugin,
    (settings_entry*)&hibblock,
    (settings_entry*)&skiplogos,
    (settings_entry*)&hidepics,
    (settings_entry*)&hidemac,
    (settings_entry*)&hidedlc,
    (settings_entry*)&noled,
    (settings_entry*)&vshregion,
};
#define MAX_ARK_CONF_GO (sizeof(ark_conf_entries_go)/sizeof(ark_conf_entries_go[0]))

settings_entry* ark_conf_entries_street[] = {
    (settings_entry*)&usbcharge,
    (settings_entry*)&overclock,
    (settings_entry*)&powersave,
    (settings_entry*)&launcher,
    (settings_entry*)&highmem,
    (settings_entry*)&mscache,
    (settings_entry*)&infernocache,
    (settings_entry*)&skiplogos,
    (settings_entry*)&hidepics,
    (settings_entry*)&noled,
    (settings_entry*)&regionchange,
    (settings_entry*)&vshregion,
};
#define MAX_ARK_CONF_STREET (sizeof(ark_conf_entries_street)/sizeof(ark_conf_entries_street[0]))

settings_entry* ark_conf_entries_vita[] = {
    (settings_entry*)&mscache,
    (settings_entry*)&infernocache,
};
#define MAX_ARK_CONF_VITA (sizeof(ark_conf_entries_vita)/sizeof(ark_conf_entries_vita[0]))

settings_entry* ark_conf_entries_adr[] = {
    (settings_entry*)&launcher,
    (settings_entry*)&highmem,
    (settings_entry*)&mscache,
    (settings_entry*)&infernocache,
    (settings_entry*)&skiplogos,
    (settings_entry*)&hidepics,
    (settings_entry*)&hidemac,
    (settings_entry*)&hidedlc,
    (settings_entry*)&vshregion,
};
#define MAX_ARK_CONF_ADR (sizeof(ark_conf_entries_adr)/sizeof(ark_conf_entries_adr[0]))

std::vector<string> custom_config;

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
    else if (strcasecmp(runlevel.c_str(), "umd") == 0 || strcasecmp(runlevel.c_str(), "psp") == 0){
        return UMD_ONLY;
    }
    else if (strcasecmp(runlevel.c_str(), "homebrew") == 0){
        return HOMEBREW_ONLY;
    }
    else if (strcasecmp(runlevel.c_str(), "pops") == 0  || strcasecmp(runlevel.c_str(), "psx") == 0 || strcasecmp(runlevel.c_str(), "ps1") == 0){
        return POPS_ONLY;
    }
    else if (strcasecmp(runlevel.c_str(), "vsh") == 0 || strcasecmp(runlevel.c_str(), "xmb") == 0){
        return VSH_ONLY;
    }
    else if (strcasecmp(runlevel.c_str(), "launcher") == 0){
        return LAUNCHER_ONLY;
    }
    return CUSTOM;
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
    else if (strcasecmp(conf.c_str(), "skiplogos") == 0){
        return &(ark_config.skiplogos);
    }
    else if (strcasecmp(conf.c_str(), "hidepics") == 0){
        return &(ark_config.hidepics);
    }
    else if (strcasecmp(conf.c_str(), "hibblock") == 0){
        return &(ark_config.hibblock);
    }
    else if (strcasecmp(conf.c_str(), "hidemac") == 0){
        return &(ark_config.hidemac);
    }
    else if (strcasecmp(conf.c_str(), "hidedlc") == 0){
        return &(ark_config.hidedlc);
    }
    else if (strcasecmp(conf.c_str(), "noled") == 0){
        return &(ark_config.noled);
    }
    else if (strcasecmp(conf.c_str(), "region_none") == 0){
        ark_config.regionchange = 0;
    }
    else if (strcasecmp(conf.c_str(), "region_jp") == 0){
        ark_config.regionchange = REGION_JAPAN;
    }
    else if (strcasecmp(conf.c_str(), "region_us") == 0){
        ark_config.regionchange = REGION_AMERICA;
    }
    else if (strcasecmp(conf.c_str(), "region_eu") == 0){
        ark_config.regionchange = REGION_EUROPE;
    }
    else if (strncasecmp(conf.c_str(), "fakeregion_", 11) == 0){
        int r = atoi(conf.c_str()+11);
        ark_config.vshregion = r;
    }
    return NULL;
}

static void processConfig(string line, string runlevel, string conf, string enable){
    unsigned char* config_ptr = configConvert(conf);
    unsigned char config = runlevelConvert(runlevel, enable);
    if (config == CUSTOM){
        custom_config.push_back(line);
    }
    else if (config_ptr != NULL){
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

    if (result.size() != 3){
        custom_config.push_back(line);
        return;
    }

    string runlevel = result[0];
    string conf = result[1];
    string enable = result[2];
    
    processConfig(line, runlevel, conf, enable);
}

void loadSettings(){

    ARKConfig* ark_config = common::getArkConfig();

    if (IS_VITA(ark_config)){
        if (IS_VITA_ADR(ark_config)){
            ark_conf_entries = ark_conf_entries_adr;
            ark_conf_max_entries = MAX_ARK_CONF_ADR;
        }
        else{
            ark_conf_entries = ark_conf_entries_vita;
            ark_conf_max_entries = MAX_ARK_CONF_VITA;
        }
    }
    else{
        int psp_model = common::getPspModel();
        if (psp_model == PSP_1000){
            ark_conf_entries = ark_conf_entries_1k;
            ark_conf_max_entries = MAX_ARK_CONF_1K;
        }
        else if (psp_model == PSP_GO){
            ark_conf_entries = ark_conf_entries_go;
            ark_conf_max_entries = MAX_ARK_CONF_GO;
        }
        else if (psp_model == PSP_11000){
            ark_conf_entries = ark_conf_entries_street;
            ark_conf_max_entries = MAX_ARK_CONF_STREET;
        }
        else{
            ark_conf_entries = ark_conf_entries_slim;
            ark_conf_max_entries = MAX_ARK_CONF_SLIM;
        }
    }

    

    std::ifstream input("SETTINGS.TXT");
    for( std::string line; getline( input, line ); ){
        if (isComment(line)){
            custom_config.push_back(line);
            continue;
        };
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
    case LAUNCHER_ONLY:       return "launcher, "+name+", on";
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
    output << processSetting("skiplogos", ark_config.skiplogos) << endl;
    output << processSetting("hidepics", ark_config.hidepics) << endl;
    output << processSetting("hibblock", ark_config.hibblock) << endl;
    output << processSetting("hidemac", ark_config.hidemac) << endl;
    output << processSetting("hidedlc", ark_config.hidedlc) << endl;
    output << processSetting("noled", ark_config.noled) << endl;
    
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

    if (ark_config.vshregion > 0){
        char tmp[10];
        snprintf(tmp, 10, "%d", ark_config.vshregion);
        output << "vsh, fakeregion_" << tmp << ", on" << endl;
    }

    for (int i=0; i<custom_config.size(); i++){
        output << custom_config[i] << endl;
    }
    
    output.close();
}
