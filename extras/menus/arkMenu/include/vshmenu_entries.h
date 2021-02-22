/* DO NOT INCLUDE THIS FILE ANYWHERE OTHER THAN vshmenu.cpp ! */

/* structure defining a common entry layout */

typedef struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[];
} vsh_entry;

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

/* Button swap */
static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[2];
} swap_buttons = {
    "Swap X/O buttons",
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
    char* options[7];
} animations = {
    "Menu animation",
    7,
    0,
    &(common::getConf()->animation),
    {"Default", "Waves", "Sprites", "Fire", "Tetris", "Matrix", "None"}
};

static vsh_entry* vsh_entries[] = {
    (vsh_entry*)&fast_gameboot,
    (vsh_entry*)&language,
    (vsh_entry*)&font,
    (vsh_entry*)&scan_save,
    (vsh_entry*)&swap_buttons,
    (vsh_entry*)&animations,
};

#define MAX_OPTIONS 6
