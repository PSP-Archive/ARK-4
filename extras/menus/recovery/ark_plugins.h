enum{
    PLACE_ARK_SAVE,
    PLACE_MS0_SEPLUGINS,
    PLACE_EF0_SEPLUGINS
};

typedef struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[2];
    unsigned char place;
} plugin_t;

// variable length array
settings_entry** ark_plugin_entries = NULL;
int ark_plugins_count = 0;
int ark_plugins_max = 0;
#define MAX_INITIAL_PLUGINS 8

static void addPlugin(plugin_t* plugin){
    if (ark_plugin_entries == NULL){ // create initial table
        ark_plugin_entries = (settings_entry**)malloc(MAX_INITIAL_PLUGINS * sizeof(settings_entry*));
        ark_plugins_max = MAX_INITIAL_PLUGINS;
        ark_plugins_count = 0;
    }
    if (ark_plugins_count >= ark_plugins_max){ // resize table
        settings_entry** new_table = (settings_entry**)malloc(2 * ark_plugins_max * sizeof(settings_entry*));
        for (int i=0; i<ark_plugins_count; i++) new_table[i] = ark_plugin_entries[i];
        free(ark_plugin_entries);
        ark_plugin_entries = new_table;
        ark_plugins_max *= 2;
    }
    ark_plugin_entries[ark_plugins_count++] = (settings_entry*)plugin;
}

static plugin_t* createPlugin(const char* description, unsigned char enable, unsigned char place){
    plugin_t* plugin = (plugin_t*)malloc(sizeof(plugin_t));
    plugin->description = strdup(description);
    plugin->max_options = 2;
    plugin->selection = enable;
    plugin->config_ptr = &(plugin->selection);
    plugin->options[0] = "Disable";
    plugin->options[1] = "Enable";
    plugin->place = place;
    return plugin;
}

static void loadPluginsFile(const char* path, unsigned char place){
    std::ifstream input(path);
    for( std::string line; getline( input, line ); ){
        if (isComment(line)) continue;
        
        string description;
        string enabled;
        int pos = line.rfind(',');
        description = line.substr(0, pos);
        enabled = line.substr(pos+1, line.size());
        
        // trim string
        std::stringstream trimmer;
        trimmer << enabled;
        trimmer.clear();
        trimmer >> enabled;
        
        plugin_t* plugin = createPlugin(description.c_str(), isRunlevelEnabled(enabled)?1:0, place);
        addPlugin(plugin);
    }
    input.close();
}

void loadPlugins(){
    loadPluginsFile("PLUGINS.TXT", 0);
    loadPluginsFile("ms0:/SEPLUGINS/PLUGINS.TXT", 1);
    loadPluginsFile("ef0:/SEPLUGINS/PLUGINS.TXT", 2);
}

void savePlugins(){
    std::ofstream output[3];
    char* plugins_path[3] = {
        "PLUGINS.TXT",
        "ms0:/SEPLUGINS/PLUGINS.TXT",
        "ef0:/SEPLUGINS/PLUGINS.TXT"
    };
    for (int i=0; i<ark_plugins_count; i++){
        plugin_t* plugin = (plugin_t*)(ark_plugin_entries[i]);
        int place = plugin->place;
        if (!output[place].is_open()) output[place].open(plugins_path[place]);
        output[place] << plugin->description << ", " << ((plugin->selection)? "on":"off") << endl;
    }
    output[0].close();
    output[1].close();
    output[2].close();
}
