/* structure defining a common entry layout */

/* Fastboot entry */
static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[2];
} fast_gameboot = {
    "Skip gameboot",
    2,
    0,
    &(common::getConf()->fast_gameboot),
    {"Disabled", "Enabled"}
};

/* Language entry */
static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[1];
} language = {
    "Language",
    1,
    0,
    &(common::getConf()->language),
    {"English"}
};

/* Font entry */
static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[19];
} font = {
    "Font style",
    19,
    0,
    &(common::getConf()->font),
    {
        "Custom",
        "Latin 0",
        "Latin 1",
        "Latin 2",
        "Latin 3",
        "Latin 4",
        "Latin 5",
        "Latin 6",
        "Latin 7",
        "Latin 8",
        "Latin 9",
        "Latin 19",
        "Latin 11",
        "Latin 12",
        "Latin 13",
        "Latin 14",
        "Latin 15",
        "Japanese",
        "Korean"
    }
};

/* Scan savedata */
static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[2];
} scan_save = {
    "Scan savedata entries",
    2,
    0,
    &(common::getConf()->scan_save),
    {"Disabled", "Enabled"}
};

/* Scan categories */
static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[2];
} scan_cat = {
    "Scan category entries",
    2,
    0,
    &(common::getConf()->scan_cat),
    {"Disabled", "Enabled"}
};

/* Button swap */
static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[2];
} swap_buttons = {
    "Swap X and O buttons",
    2,
    0,
    &(common::getConf()->swap_buttons),
    {"Disabled", "Enabled"}
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[9];
} animations = {
    "Menu animation",
    9,
    0,
    &(common::getConf()->animation),
    {"Default", "Waves", "Sprites", "Fire", "Tetris", "Matrix", "Snow", "Game of Life", "None"}
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[9];
} main_menu = {
    "Default Menu",
    2,
    0,
    &(common::getConf()->main_menu),
    {"Games", "Files"}
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[9];
} sort_entries = {
    "Sort Entries by Name",
    2,
    0,
    &(common::getConf()->sort_entries),
    {"Disabled", "Enabled"}
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[9];
} recovery_menu = {
    "Show Recovery Menu",
    2,
    0,
    &(common::getConf()->show_recovery),
    {"Disabled", "Enabled"}
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[9];
} show_fps = {
    "Show FPS",
    2,
    0,
    &(common::getConf()->show_fps),
    {"Disabled", "Enabled"}
};

settings_entry* settings_entries[] = {
    (settings_entry*)&fast_gameboot,
    (settings_entry*)&language,
    (settings_entry*)&font,
    (settings_entry*)&scan_save,
    (settings_entry*)&scan_cat,
    (settings_entry*)&swap_buttons,
    (settings_entry*)&animations,
    (settings_entry*)&main_menu,
    (settings_entry*)&sort_entries,
    (settings_entry*)&recovery_menu,
    (settings_entry*)&show_fps,
};

#define MAX_SETTINGS_OPTIONS (sizeof(settings_entries)/sizeof(settings_entries[0]))
