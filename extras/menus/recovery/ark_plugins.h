typedef struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[2];
} plugin_t;

// variable length array
vsh_entry** ark_plugin_entries = NULL;
int ark_plugins_count = 0;
int ark_plugins_max = 0;
#define MAX_INITIAL_PLUGINS 8

static void addPlugin(plugin_t* plugin){
    if (ark_plugin_entries == NULL){ // create initial table
        ark_plugin_entries = (vsh_entry**)malloc(MAX_INITIAL_PLUGINS * sizeof(vsh_entry*));
        ark_plugins_max = MAX_INITIAL_PLUGINS;
        ark_plugins_count = 0;
    }
    if (ark_plugins_count >= ark_plugins_max){ // resize table
        vsh_entry** new_table = (vsh_entry**)malloc(2 * ark_plugins_max * sizeof(vsh_entry*));
        for (int i=0; i<ark_plugins_count; i++) new_table[i] = ark_plugin_entries[i];
        free(ark_plugin_entries);
        ark_plugin_entries = new_table;
        ark_plugins_max *= 2;
    }
    ark_plugin_entries[ark_plugins_count++] = (vsh_entry*)plugin;
}

static plugin_t* createPlugin(const char* description, unsigned char enable){
    plugin_t* plugin = (plugin_t*)malloc(sizeof(plugin_t));
    plugin->description = strdup(description);
    plugin->max_options = 2;
    plugin->selection = enable;
    plugin->config_ptr = &(plugin->selection);
    plugin->options[0] = "Disable";
    plugin->options[1] = "Enable";
    return plugin;
}

void loadPlugins(){
    std::ifstream input("PLUGINS.TXT");
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
        
        plugin_t* plugin = createPlugin(description.c_str(), isRunlevelEnabled(enabled)?1:0);
        addPlugin(plugin);
    }
    input.close();
}

void savePlugins(){
    std::ofstream output("PLUGINS.TXT");
    for (int i=0; i<ark_plugins_count; i++){
        plugin_t* plugin = (plugin_t*)(ark_plugin_entries[i]);
        output << plugin->description << ", " << ((plugin->selection)? "on":"off") << endl;
    }
    output.close();
}
