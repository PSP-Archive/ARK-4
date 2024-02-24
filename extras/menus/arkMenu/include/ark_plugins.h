enum{
    PLACE_ARK_SAVE,
    PLACE_MS0_SEPLUGINS,
    PLACE_EF0_SEPLUGINS,
    MAX_PLUGINS_PLACES
};

enum{
    PLUGIN_OFF,
    PLUGIN_ON,
    PLUGIN_REMOVED,
};

typedef struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[3];
    unsigned char place;
} plugin_t;

typedef struct {
    string line;
    plugin_t* plugin;
} plugin_line;

char* plugins_path[] = {
    "PLUGINS.TXT",
    "ms0:/SEPLUGINS/PLUGINS.TXT",
    "ef0:/SEPLUGINS/PLUGINS.TXT"
};

// variable length array
std::vector<plugin_line> plugin_lines[MAX_PLUGINS_PLACES];

SettingsTable plugins_table = {NULL, 0};
int ark_plugins_max = 0;

#define MAX_INITIAL_PLUGINS 8

static void addPlugin(plugin_t* plugin){
    if (plugins_table.settings_entries == NULL){ // create initial table
        plugins_table.settings_entries = (settings_entry**)malloc(MAX_INITIAL_PLUGINS * sizeof(settings_entry*));
        ark_plugins_max = MAX_INITIAL_PLUGINS;
        plugins_table.max_options = 0;
    }
    if (plugins_table.max_options >= ark_plugins_max){ // resize table
        settings_entry** new_table = (settings_entry**)malloc(2 * ark_plugins_max * sizeof(settings_entry*));
        for (int i=0; i<plugins_table.max_options; i++) new_table[i] = plugins_table.settings_entries[i];
        free(plugins_table.settings_entries);
        plugins_table.settings_entries = new_table;
        ark_plugins_max *= 2;
    }
    plugins_table.settings_entries[plugins_table.max_options++] = (settings_entry*)plugin;
}

static void removePlugin(std::vector<plugin_line>& pl, int pi){
    plugin_t* plugin = pl[pi].plugin;
    pl.erase(pl.begin()+pi);

    SystemMgr::pauseDraw();

    for (int i=0; i<plugins_table.max_options; i++){
        if ((void*)(plugins_table.settings_entries[i]) == (void*)plugin){
            for (int j=i; j<plugins_table.max_options-1; j++){
                plugins_table.settings_entries[j] = plugins_table.settings_entries[j+1];
            }
            plugins_table.max_options--;
            break;
        }
    }

    free(plugin->description);
    free(plugin);

    SystemMgr::resumeDraw();
}

static plugin_t* createPlugin(const char* description, unsigned char enable, unsigned char place){
    plugin_t* plugin = (plugin_t*)malloc(sizeof(plugin_t));
    plugin->description = strdup(description);
    plugin->max_options = 3;
    plugin->selection = enable;
    plugin->config_ptr = &(plugin->selection);
    plugin->options[0] = "Disabled";
    plugin->options[1] = "Enabled";
    plugin->options[2] = "Remove";
    plugin->place = place;
    return plugin;
}

static void loadPluginsFile(unsigned char place){
    std::ifstream input(plugins_path[place]);
    for( std::string line; getline( input, line ); ){
        plugin_line pl = { line, NULL };
        if (!isComment(line)){
            string description;
            string enabled;
            int pos = line.rfind(',');
            if (pos != string::npos){
                description = line.substr(0, pos);
                enabled = line.substr(pos+1, line.size());
                
                // trim string
                std::stringstream trimmer;
                trimmer << enabled;
                trimmer.clear();
                trimmer >> enabled;
                
                pl.plugin = createPlugin(description.c_str(), isRunlevelEnabled(enabled)?1:0, place);
                addPlugin(pl.plugin);
            }
        }
        plugin_lines[place].push_back(pl);
    }
    input.close();
}

void loadPlugins(){
    loadPluginsFile(PLACE_ARK_SAVE);
    loadPluginsFile(PLACE_MS0_SEPLUGINS);
    loadPluginsFile(PLACE_EF0_SEPLUGINS);
}

void savePlugins(){
    std::ofstream output[MAX_PLUGINS_PLACES];

    for (int i=0; i<MAX_PLUGINS_PLACES; i++){
        output[i].open(plugins_path[i]);
        for (int j=0; j<plugin_lines[i].size(); j++){
            plugin_line pl = plugin_lines[i][j];
            if (pl.plugin != NULL){
                if (pl.plugin->selection != PLUGIN_REMOVED)
                    output[i] << pl.plugin->description << ", " << ((pl.plugin->selection)? "on":"off") << endl;
                else{
                    removePlugin(plugin_lines[i], j);
                    j--;
                }
            }
            else{
                output[i] << pl.line << endl;
            }
        }
        output[i].close();
    }
}

void cleanupPlugins(){
    plugins_table.max_options = 0;
    for (int i=0; i<MAX_PLUGINS_PLACES; i++){
        for (int j=0; j<plugin_lines[i].size(); j++){
            plugin_line pl = plugin_lines[i][j];
            plugin_t* plugin = pl.plugin;
            if (plugin){
                free(plugin->description);
                free(plugin);
            }
        }
        plugin_lines[i].clear();
    }
}
