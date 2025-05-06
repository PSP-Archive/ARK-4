/* DO NOT INCLUDE THIS FILE ANYWHERE OTHER THAN vshmenu.cpp ! */

/* structure defining a common entry layout */

#define FIX_BOOLEAN(c) {c = (c)?1:0;}


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

enum{
    CLOCK_AUTO,
    OVERCLOCK,
    DEFAULTCLOCK,
    POWERSAVE
};

typedef struct {
    unsigned char usbcharge;
    unsigned char clock_game;
    unsigned char clock_vsh;
    unsigned char wpa2;
    unsigned char launcher;
    unsigned char highmem;
    unsigned char mscache;
    unsigned char infernocache;
    unsigned char disablepause;
    unsigned char oldplugin;
    unsigned char hibblock;
    unsigned char skiplogos;
    unsigned char regionchange;
    unsigned char vshregion;
    unsigned char hidepics;
    unsigned char hidemac;
    unsigned char hidedlc;
    unsigned char noled;
    unsigned char noumd;
    unsigned char noanalog;
    unsigned char qaflags;
}CfwConf;

CfwConf cfw_config;

#define MAX_BOOLEAN_OPTIONS 2
#define BOOLEAN_OPTIONS {"Off", "On"}
#define BOOLEAN_2_OPTIONS {"Auto", "Forced"}

#define MAX_CLOCK_OPTIONS 4
#define CLOCK_OPTIONS { \
    "Auto", \
    "OverClock", \
    "Balanced", \
    "PowerSave", \
}

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[MAX_BOOLEAN_OPTIONS];
} usbcharge = {
    "USB Charge",
    MAX_BOOLEAN_OPTIONS,
    0,
    &(cfw_config.usbcharge),
    BOOLEAN_OPTIONS
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[MAX_CLOCK_OPTIONS];
} clock_game = {
    "CPU Clock in Game",
    MAX_CLOCK_OPTIONS,
    0,
    &(cfw_config.clock_game),
    CLOCK_OPTIONS
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[MAX_CLOCK_OPTIONS];
} clock_vsh = {
    "CPU Clock in XMB",
    MAX_CLOCK_OPTIONS,
    0,
    &(cfw_config.clock_vsh),
    CLOCK_OPTIONS
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[MAX_BOOLEAN_OPTIONS];
} launcher = {
    "Autoboot Launcher",
    MAX_BOOLEAN_OPTIONS,
    0,
    &(cfw_config.launcher),
    BOOLEAN_OPTIONS
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[MAX_BOOLEAN_OPTIONS];
} highmem = {
    "Use Extra Memory",
    MAX_BOOLEAN_OPTIONS,
    0,
    &(cfw_config.highmem),
    BOOLEAN_2_OPTIONS
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[MAX_BOOLEAN_OPTIONS];
} mscache = {
    "Memory Stick Speedup",
    MAX_BOOLEAN_OPTIONS,
    0,
    &(cfw_config.mscache),
    BOOLEAN_OPTIONS
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[3];
} infernocache = {
    "Inferno Cache",
    3,
    0,
    &(cfw_config.infernocache),
    {"Off", "LRU", "RR"}
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[MAX_BOOLEAN_OPTIONS];
} oldplugin = {
    "Old Plugins on ef0",
    MAX_BOOLEAN_OPTIONS,
    0,
    &(cfw_config.oldplugin),
    BOOLEAN_OPTIONS
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[MAX_BOOLEAN_OPTIONS];
} disablepause = {
    "Disable PSP Go Pause",
    MAX_BOOLEAN_OPTIONS,
    0,
    &(cfw_config.disablepause),
    BOOLEAN_2_OPTIONS
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[MAX_BOOLEAN_OPTIONS];
} hibblock = {
    "Prevent hibernation deletion on PSP Go",
    MAX_BOOLEAN_OPTIONS,
    0,
    &(cfw_config.hibblock),
    BOOLEAN_OPTIONS
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[4];
} skiplogos = {
    "Skip Sony logos",
    4,
    0,
    &(cfw_config.skiplogos),
    {"Off", "All", "GameBoot", "ColdBoot"}
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[4];
} hidepics = {
    "Hide PIC0 and PIC1",
    4,
    0,
    &(cfw_config.hidepics),
    {"Off", "All", "PIC0", "PIC1"}
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[MAX_BOOLEAN_OPTIONS];
} hidemac = {
    "Hide Mac Address",
    MAX_BOOLEAN_OPTIONS,
    0,
    &(cfw_config.hidemac),
    BOOLEAN_OPTIONS
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[MAX_BOOLEAN_OPTIONS];
} hidedlc = {
    "Hide DLC",
    MAX_BOOLEAN_OPTIONS,
    0,
    &(cfw_config.hidedlc),
    BOOLEAN_OPTIONS
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[MAX_BOOLEAN_OPTIONS];
} noled = {
    "Turn off LEDs",
    MAX_BOOLEAN_OPTIONS,
    0,
    &(cfw_config.noled),
    BOOLEAN_OPTIONS
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[MAX_BOOLEAN_OPTIONS];
} noumd = {
    "Disable UMD Drive",
    MAX_BOOLEAN_OPTIONS,
    0,
    &(cfw_config.noumd),
    BOOLEAN_OPTIONS
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[MAX_BOOLEAN_OPTIONS];
} noanalog = {
    "Disable Analog Stick",
    MAX_BOOLEAN_OPTIONS,
    0,
    &(cfw_config.noanalog),
    BOOLEAN_OPTIONS
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[MAX_BOOLEAN_OPTIONS];
} qaflags = {
    "QA Flags",
    MAX_BOOLEAN_OPTIONS,
    0,
    &(cfw_config.qaflags),
    BOOLEAN_OPTIONS
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[MAX_BOOLEAN_OPTIONS];
} wpa2 = {
    "WPA2 Support",
    MAX_BOOLEAN_OPTIONS,
    0,
    &(cfw_config.wpa2),
    BOOLEAN_OPTIONS
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[4];
} regionchange = {
    "UMD Region",
    4,
    0,
    &(cfw_config.regionchange),
    {"Default", "Japan", "America", "Europe"}
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[14];
} vshregion = {
    "VSH Region",
    14,
    0,
    &(cfw_config.vshregion),
    {
        "Default", "Japan", "America", "Europe", "Korea",
        "United Kingdom", "Latin America", "Australia", "Hong Kong",
        "Taiwan", "Russia", "China", "Debug I", "Debug II"
    }
};

int ark_conf_max_entries = 0;
settings_entry** ark_conf_entries = NULL;

settings_entry* ark_conf_entries_1k[] = {
    (settings_entry*)&clock_game,
    (settings_entry*)&clock_vsh,
    (settings_entry*)&wpa2,
    (settings_entry*)&launcher,
    (settings_entry*)&mscache,
    (settings_entry*)&infernocache,
    (settings_entry*)&skiplogos,
    (settings_entry*)&hidepics,
    (settings_entry*)&hidemac,
    (settings_entry*)&hidedlc,
    (settings_entry*)&noled,
    (settings_entry*)&noumd,
    (settings_entry*)&noanalog,
    (settings_entry*)&qaflags,
    (settings_entry*)&regionchange,
    (settings_entry*)&vshregion,
};
#define MAX_ARK_CONF_1K (sizeof(ark_conf_entries_1k)/sizeof(ark_conf_entries_1k[0]))

settings_entry* ark_conf_entries_slim[] = {
    (settings_entry*)&usbcharge,
    (settings_entry*)&clock_game,
    (settings_entry*)&clock_vsh,
    (settings_entry*)&wpa2,
    (settings_entry*)&launcher,
    (settings_entry*)&highmem,
    (settings_entry*)&mscache,
    (settings_entry*)&infernocache,
    (settings_entry*)&skiplogos,
    (settings_entry*)&hidepics,
    (settings_entry*)&hidemac,
    (settings_entry*)&hidedlc,
    (settings_entry*)&noled,
    (settings_entry*)&noumd,
    (settings_entry*)&noanalog,
    (settings_entry*)&qaflags,
    (settings_entry*)&regionchange,
    (settings_entry*)&vshregion,
};
#define MAX_ARK_CONF_SLIM (sizeof(ark_conf_entries_slim)/sizeof(ark_conf_entries_slim[0]))

settings_entry* ark_conf_entries_go[] = {
    (settings_entry*)&usbcharge,
    (settings_entry*)&clock_game,
    (settings_entry*)&clock_vsh,
    (settings_entry*)&wpa2,
    (settings_entry*)&launcher,
    (settings_entry*)&highmem,
    (settings_entry*)&mscache,
    (settings_entry*)&infernocache,
    (settings_entry*)&disablepause,
    (settings_entry*)&oldplugin,
    (settings_entry*)&hibblock,
    (settings_entry*)&skiplogos,
    (settings_entry*)&hidepics,
    (settings_entry*)&hidemac,
    (settings_entry*)&hidedlc,
    (settings_entry*)&noled,
    (settings_entry*)&noanalog,
    (settings_entry*)&qaflags,
    (settings_entry*)&vshregion,
};
#define MAX_ARK_CONF_GO (sizeof(ark_conf_entries_go)/sizeof(ark_conf_entries_go[0]))

settings_entry* ark_conf_entries_street[] = {
    (settings_entry*)&usbcharge,
    (settings_entry*)&clock_game,
    (settings_entry*)&clock_vsh,
    (settings_entry*)&launcher,
    (settings_entry*)&highmem,
    (settings_entry*)&mscache,
    (settings_entry*)&infernocache,
    (settings_entry*)&skiplogos,
    (settings_entry*)&hidepics,
    (settings_entry*)&noled,
    (settings_entry*)&noumd,
    (settings_entry*)&noanalog,
    (settings_entry*)&qaflags,
    (settings_entry*)&regionchange,
    (settings_entry*)&vshregion,
};
#define MAX_ARK_CONF_STREET (sizeof(ark_conf_entries_street)/sizeof(ark_conf_entries_street[0]))

settings_entry* ark_conf_entries_vita[] = {
    (settings_entry*)&clock_game,
    (settings_entry*)&highmem,
    (settings_entry*)&mscache,
    (settings_entry*)&infernocache,
    (settings_entry*)&oldplugin,
};
#define MAX_ARK_CONF_VITA (sizeof(ark_conf_entries_vita)/sizeof(ark_conf_entries_vita[0]))

settings_entry* ark_conf_entries_adr[] = {
    (settings_entry*)&clock_game,
    (settings_entry*)&clock_vsh,
    (settings_entry*)&launcher,
    (settings_entry*)&highmem,
    (settings_entry*)&mscache,
    (settings_entry*)&infernocache,
    (settings_entry*)&skiplogos,
    (settings_entry*)&hidepics,
    (settings_entry*)&hidemac,
    (settings_entry*)&hidedlc,
    (settings_entry*)&noanalog,
    (settings_entry*)&qaflags,
    (settings_entry*)&vshregion,
};
#define MAX_ARK_CONF_ADR (sizeof(ark_conf_entries_adr)/sizeof(ark_conf_entries_adr[0]))

std::vector<string> custom_config;

void cleanupSettings(){
    custom_config.clear();
}

bool isComment(string line){
    return (line[0] == '#' || line[0] == ';' || (line[0]=='/'&&line[1]=='/'));
}

bool isRunlevelEnabled(string line){
    return (line == "on" || line == "1" || line == "enabled" || line == "true");
}

static unsigned char runlevelConvert(string runlevel, string enable){
    int enabled = isRunlevelEnabled(enable);
    
    if (strcasecmp(runlevel.c_str(), "always") == 0 || strcasecmp(runlevel.c_str(), "all") == 0){
        return (enabled)?ALWAYS_ON:DISABLED;
    }
    else if (strcasecmp(runlevel.c_str(), "game") == 0){
        return (enabled)?GAME_ONLY:DISABLED;
    }
    else if (strcasecmp(runlevel.c_str(), "umd") == 0 || strcasecmp(runlevel.c_str(), "psp") == 0){
        return (enabled)?UMD_ONLY:DISABLED;
    }
    else if (strcasecmp(runlevel.c_str(), "homebrew") == 0){
        return (enabled)?HOMEBREW_ONLY:DISABLED;
    }
    else if (strcasecmp(runlevel.c_str(), "pops") == 0  || strcasecmp(runlevel.c_str(), "psx") == 0 || strcasecmp(runlevel.c_str(), "ps1") == 0){
        return (enabled)?POPS_ONLY:DISABLED;
    }
    else if (strcasecmp(runlevel.c_str(), "vsh") == 0 || strcasecmp(runlevel.c_str(), "xmb") == 0){
        return (enabled)?VSH_ONLY:DISABLED;
    }
    else if (strcasecmp(runlevel.c_str(), "launcher") == 0){
        return (enabled)?LAUNCHER_ONLY:DISABLED;
    }
    return CUSTOM;
}

static unsigned char* configConvert(string conf){
    if (strcasecmp(conf.c_str(), "usbcharge") == 0){
        return &(cfw_config.usbcharge);
    }
    else if (strcasecmp(conf.c_str(), "wpa2") == 0){
        return &(cfw_config.wpa2);
    }
    else if (strcasecmp(conf.c_str(), "launcher") == 0){
        return &(cfw_config.launcher);
    }
    else if (strcasecmp(conf.c_str(), "disablepause") == 0){
        return &(cfw_config.disablepause);
    }
    else if (strcasecmp(conf.c_str(), "highmem") == 0){
        return &(cfw_config.highmem);
    }
    else if (strcasecmp(conf.c_str(), "mscache") == 0){
        return &(cfw_config.mscache);
    }
    else if (strncasecmp(conf.c_str(), "infernocache", 12) == 0){
        return &(cfw_config.infernocache);
    }
    else if (strcasecmp(conf.c_str(), "oldplugin") == 0){
        return &(cfw_config.oldplugin);
    }
    else if (strncasecmp(conf.c_str(), "skiplogos", 9) == 0){
        return &(cfw_config.skiplogos);
    }
    else if (strncasecmp(conf.c_str(), "hidepics", 8) == 0){
        return &(cfw_config.hidepics);
    }
    else if (strcasecmp(conf.c_str(), "hibblock") == 0){
        return &(cfw_config.hibblock);
    }
    else if (strcasecmp(conf.c_str(), "hidemac") == 0){
        return &(cfw_config.hidemac);
    }
    else if (strcasecmp(conf.c_str(), "hidedlc") == 0){
        return &(cfw_config.hidedlc);
    }
    else if (strcasecmp(conf.c_str(), "noled") == 0){
        return &(cfw_config.noled);
    }
    else if (strcasecmp(conf.c_str(), "noumd") == 0){
        return &(cfw_config.noumd);
    }
    else if (strcasecmp(conf.c_str(), "noanalog") == 0){
        return &(cfw_config.noanalog);
    }
    else if (strcasecmp(conf.c_str(), "qaflags") == 0){
        return &(cfw_config.qaflags);
    }
    else if (strncasecmp(conf.c_str(), "region_", 7) == 0){
        char* c = strchr(conf.c_str(), '_')+1;
        if (strcasecmp(c, "jp") == 0){
            cfw_config.regionchange = REGION_JAPAN;
        }
        else if (strcasecmp(c, "us") == 0){
            cfw_config.regionchange = REGION_AMERICA;
        }
        else if (strcasecmp(c, "eu") == 0){
            cfw_config.regionchange = REGION_EUROPE;
        }
    }
    else if (strncasecmp(conf.c_str(), "fakeregion_", 11) == 0){
        int r = atoi(conf.c_str()+11);
        cfw_config.vshregion = r;
    }
    return NULL;
}

static void convertClockConfig(unsigned char config, unsigned char value){
    // XMB only
    if (config == VSH_ONLY){
        cfw_config.clock_vsh = value;
    }
    // XMB and Game
    else if (config == ALWAYS_ON){
        cfw_config.clock_game = value;
        cfw_config.clock_vsh = value;
    }
    // Game only
    else if (config != DISABLED){
        cfw_config.clock_game = value;
    }
}

static void processConfig(string line, string runlevel, string conf, string enable){
    unsigned char* config_ptr = configConvert(conf);
    unsigned char config = runlevelConvert(runlevel, enable);
    if (config == CUSTOM){
        custom_config.push_back(line);
    }
    else if (config_ptr != NULL){
        *config_ptr = config;
        if (strncasecmp(conf.c_str(), "infernocache", 12) == 0){
            char* c = strchr(conf.c_str(), ':');
            FIX_BOOLEAN(config);
            cfw_config.infernocache = config;
            if (config && c){
                if (strcasecmp(c+1, "lru") == 0) cfw_config.infernocache = 1;
                else if (strcasecmp(c+1, "rr") == 0) cfw_config.infernocache = 2;
            }
        }
        else if (strncasecmp(conf.c_str(), "skiplogos", 9) == 0){
            char* c = strchr(conf.c_str(), ':');
            FIX_BOOLEAN(config);
            cfw_config.skiplogos = config;
            if (config && c){
                if (strcasecmp(c+1, "gameboot") == 0) cfw_config.skiplogos = 2;
                else if (strcasecmp(c+1, "coldboot") == 0) cfw_config.skiplogos = 3;
            }
        }
        else if (strncasecmp(conf.c_str(), "hidepics", 8) == 0){
            char* c = strchr(conf.c_str(), ':');
            FIX_BOOLEAN(config);
            cfw_config.hidepics = config;
            if (config && c){
                if (strcasecmp(c+1, "pic0") == 0) cfw_config.hidepics = 2;
                else if (strcasecmp(c+1, "pic1") == 0) cfw_config.hidepics = 3;
            }
        }
    }
    else {
        if (strcasecmp(conf.c_str(), "overclock") == 0){
            convertClockConfig(config, OVERCLOCK);
        }
        else if (strcasecmp(conf.c_str(), "powersave") == 0){
            convertClockConfig(config, POWERSAVE);
        }
        else if (strcasecmp(conf.c_str(), "defaultclock") == 0){
            convertClockConfig(config, DEFAULTCLOCK);
        }
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

    cleanupSettings(); 

    std::ifstream input((string(ark_config->arkpath)+ARK_SETTINGS).c_str());
    for( std::string line; getline( input, line ); ){
        if (isComment(line)){
            custom_config.push_back(line);
            continue;
        };
        processLine(line);
    }
    input.close();

    FIX_BOOLEAN(cfw_config.usbcharge);
    FIX_BOOLEAN(cfw_config.launcher);
    FIX_BOOLEAN(cfw_config.highmem);
    FIX_BOOLEAN(cfw_config.mscache);
    FIX_BOOLEAN(cfw_config.disablepause);
    FIX_BOOLEAN(cfw_config.oldplugin);
    FIX_BOOLEAN(cfw_config.hibblock);
    FIX_BOOLEAN(cfw_config.hidemac);
    FIX_BOOLEAN(cfw_config.hidedlc);
    FIX_BOOLEAN(cfw_config.noumd);
    FIX_BOOLEAN(cfw_config.noanalog);
    FIX_BOOLEAN(cfw_config.noled);
    FIX_BOOLEAN(cfw_config.qaflags);
    FIX_BOOLEAN(cfw_config.wpa2);
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

static void saveClockSetting(std::ofstream& output, string category, unsigned char config){
    if (config){
        output << category << ", ";
        switch (config){
            case 1: output << "overclock, on"; break;
            case 2: output << "defaultclock, on"; break;
            case 3: output << "powersave, on"; break;
            default: output << "overclock, off"; break;
        }
        output << endl;
    }
}

void saveSettings(){
    ARKConfig* ark_config = common::getArkConfig();
    std::ofstream output((string(ark_config->arkpath)+ARK_SETTINGS).c_str());
    output << processSetting("usbcharge", cfw_config.usbcharge) << endl;

    if (cfw_config.clock_game == cfw_config.clock_vsh){
        // save both at once
        saveClockSetting(output, "always", cfw_config.clock_game);
    }
    else {
        // Save CPU Clock in-game
        saveClockSetting(output, "game", cfw_config.clock_game);
        // Save CPU Clock in-vsh
        saveClockSetting(output, "vsh", cfw_config.clock_vsh);
    }

    output << processSetting("wpa2", cfw_config.wpa2) << endl;
    output << processSetting("launcher", cfw_config.launcher) << endl;
    output << processSetting("highmem", cfw_config.highmem) << endl;
    output << processSetting("mscache", cfw_config.mscache) << endl;
    switch (cfw_config.infernocache){
        case 0: output << processSetting("infernocache", 0) << endl; break;
        case 1: output << processSetting("infernocache:lru", 1) << endl; break;
        case 2: output << processSetting("infernocache:rr", 1) << endl; break;
    }
    output << processSetting("disablepause", cfw_config.disablepause) << endl;
    output << processSetting("oldplugin", cfw_config.oldplugin) << endl;
    output << processSetting("hibblock", cfw_config.hibblock) << endl;
    switch (cfw_config.skiplogos){
        case 0: output << processSetting("skiplogos", 0) << endl; break;
        case 1: output << processSetting("skiplogos", 1) << endl; break;
        case 2: output << processSetting("skiplogos:gameboot", 1) << endl; break;
        case 3: output << processSetting("skiplogos:coldboot", 1) << endl; break;
    }
    switch (cfw_config.hidepics){
        case 0: output << processSetting("hidepics", 0) << endl; break;
        case 1: output << processSetting("hidepics", 1) << endl; break;
        case 2: output << processSetting("hidepics:pic0", 1) << endl; break;
        case 3: output << processSetting("hidepics:pic1", 1) << endl; break;
    }
    output << processSetting("hidemac", cfw_config.hidemac) << endl;
    output << processSetting("hidedlc", cfw_config.hidedlc) << endl;
    output << processSetting("noled", cfw_config.noled) << endl;
    output << processSetting("noumd", cfw_config.noumd) << endl;
    output << processSetting("noanalog", cfw_config.noanalog) << endl;
    output << processSetting("qaflags", cfw_config.qaflags) << endl;
    
    switch (cfw_config.regionchange){
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

    if (cfw_config.vshregion > 0){
        char tmp[10];
        snprintf(tmp, 10, "%d", cfw_config.vshregion);
        output << "vsh, fakeregion_" << tmp << ", on" << endl;
    }

    for (int i=0; i<custom_config.size(); i++){
        output << custom_config[i] << endl;
    }

    output.close();
}

void resetCfwSettings() {
    memset(&cfw_config, 0, sizeof(cfw_config));
    cfw_config.usbcharge = 1;
    cfw_config.clock_game = OVERCLOCK;
    cfw_config.clock_vsh = OVERCLOCK;
    cfw_config.wpa2 = 1;
    cfw_config.mscache = 1;
    cfw_config.infernocache = 1;
    cfw_config.oldplugin = 1;
    cfw_config.hibblock = 1;
    cfw_config.hidemac = 1;
    cfw_config.hidedlc = 1;
    cfw_config.qaflags = 1;

    custom_config.clear();
    custom_config.push_back("\n");
    custom_config.push_back("# The following games don't like Inferno Cache\n");
    custom_config.push_back("# Luxor - The Wrath of Set (the other Luxor game works fine)\n");
    custom_config.push_back("ULUS10201, infernocache, off\n");
    custom_config.push_back("# Flat-Out Head On (both US and EU)\n");
    custom_config.push_back("ULUS10328 ULES00968, infernocache, off\n");
    custom_config.push_back("\n");
    custom_config.push_back("# Enable Extra RAM on GTA LCS and VCS for CheatDeviceRemastered\n");
    custom_config.push_back("ULUS10041 ULUS10160 ULES00151 ULES00502, highmem, on\n");
}


