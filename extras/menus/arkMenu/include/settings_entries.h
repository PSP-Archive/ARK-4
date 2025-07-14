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
unsigned char language_selection = 0;
static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[19];
} language = {
    "Language",
    19,
    0,
    &language_selection,
    {
        "Default",
        "English",
        "Español",
        "Deutsch",
        "Français",
        "Português",
        "Italiano",
        "Nederlands",
        "Русский",
        "Yкраїнська",
        "Română",
        "Latin",
        "Japanese",
        "Korean",
        "Chinese (Trad.)",
        "Chinese (Simp.)",
        "Polski",
        "Ellhnika",
        "Türk"
        //"Thai",
    }
};

/* Font entry */
static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[17];
} font = {
    "Font style",
    17,
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
        "Latin 10",
        "Latin 11",
        "Latin 12",
        "Latin 13",
        "Latin 14",
        "Latin 15",
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

/* Show DLC */
static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[2];
} show_dlc = {
    "Show DLC files",
    2,
    0,
    &(common::getConf()->show_dlc),
    {"Disabled", "Enabled"}
};

/* Show Hidden files/folders */
static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[2];
} show_hidden = {
    "Show Hidden",
    2,
    0,
    &(common::getConf()->show_hidden),
    {"Disabled", "Enabled"}
};

/* Show Hidden files/folders */
static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[2];
} show_size = {
    "Show File Size",
    2,
    0,
    &(common::getConf()->show_size),
    {"Disabled", "Enabled"}
};

/* Show game device path on GO (whether on ef0: or ms0:) */
static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[2];
} show_path = {
    "Show Game Path",
    2,
    0,
    &(common::getConf()->show_path),
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
    char* options[11];
} animations = {
    "Menu animation",
    11,
    0,
    &(common::getConf()->animation),
    {"Default", "Waves", "Sprites", "Fire", "Tetris", "Matrix", "Hacker", "BSoD", "Snow", "Game of Life", "None"}
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[2];
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
    char* options[2];
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
    char* options[2];
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
    char* options[2];
} show_fps = {
    "Show FPS",
    2,
    0,
    &(common::getConf()->show_fps),
    {"Disabled", "Enabled"}
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[4];
} text_glow = {
    "Text Glow",
    4,
    0,
    &(common::getConf()->text_glow),
    {"Disabled", "1", "2", "3"}
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[6];
} screensaver = {
    "Screensaver time",
    6,
    0,
    &(common::getConf()->screensaver),
    {"Disabled", "5s", "10s", "20s", "30s", "1m"}
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[2];
} redirect_ms0 = {
    "Redirect ms0 to ef0",
    2,
    0,
    &(common::getConf()->redirect_ms0),
    {"Disabled", "Enabled"}
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[2];
} force_update = {
    "Force Update",
    2,
    0,
    &(common::getConf()->force_update),
    {"Disabled", "Enabled"}
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[2];
} battery_percent = {
    "Display Battery Percent",
    2,
    0,
    &(common::getConf()->battery_percent),
    {"Disabled", "Enabled"}
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[3];
} startbtn = {
    "Start Button Behavior",
    3,
    0,
    &(common::getConf()->startbtn),
    {"Current", "Last Game", "Random Game"}
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[2];
} app_autoboot = {
    "AutoBoot last game",
    2,
    0,
    &(common::getConf()->app_autoboot),
    {"Disabled", "Enabled"}
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[4];
} menusize = {
    "System Menu Size",
    4,
    0,
    &(common::getConf()->menusize),
    {"Default", "Small", "Medium", "Large"}
};

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[2];
} browser_icon0 = {
    "ICON0 in Browser",
    2,
    0,
    &(common::getConf()->browser_icon0),
    {"Disabled", "Enabled"}
};

settings_entry* settings_entries[] = {
    (settings_entry*)&language,
    (settings_entry*)&fast_gameboot,
    (settings_entry*)&font,
    (settings_entry*)&scan_save,
    (settings_entry*)&scan_cat,
    (settings_entry*)&show_dlc,
    (settings_entry*)&show_hidden,
    (settings_entry*)&show_size,
    (settings_entry*)&swap_buttons,
    (settings_entry*)&animations,
    (settings_entry*)&main_menu,
    (settings_entry*)&sort_entries,
    (settings_entry*)&recovery_menu,
    (settings_entry*)&show_fps,
    (settings_entry*)&text_glow,
    (settings_entry*)&screensaver,
    (settings_entry*)&force_update,
    (settings_entry*)&battery_percent,
    (settings_entry*)&startbtn,
    (settings_entry*)&app_autoboot,
    (settings_entry*)&menusize,
    (settings_entry*)&browser_icon0,
    (settings_entry*)&redirect_ms0,
    (settings_entry*)&show_path,
};

#define MAX_SETTINGS_OPTIONS (sizeof(settings_entries)/sizeof(settings_entries[0]))
