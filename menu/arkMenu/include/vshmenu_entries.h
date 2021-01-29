/* DO NOT INCLUDE THIS FILE ANYWHERE OTHER THAN vshmenu.cpp ! */

#define MAX_OPTIONS 10

/* structure defining a common entry layout */

typedef struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[];
} vsh_entry;

/* ISO Driver entry */
static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[4];
} iso_driver = {
    "Iso Driver",
    4,
    2,
    &(common::getConf()->iso_driver),
    {"M33 Driver", "Sony NP9660", "Inferno", "ME"}
};

/* Hide game Exploit entry */
static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[2];
} hide_exploit = {
    "Hide game exploit",
    2,
    1,
    &(common::getConf()->hide_exploit),
    {"Disabled", "Enabled"}
};

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
      
/* plugins entry */
static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[2];
} plugins = {
    "Enable plugins",
    2,
    1,
    &(common::getConf()->plugins),
    {"Disabled", "Enabled"}
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

static struct {
    char* description;
    unsigned char max_options;
    unsigned char selection;
    unsigned char* config_ptr;
    char* options[1];
} open_peops_menu = {
    "Configure PEOPS Plugin",
    1,
    0,
    &(open_peops_menu.selection),
    {" "}
};

static vsh_entry* vsh_entries[MAX_OPTIONS] = {
    (vsh_entry*)&iso_driver,
    (vsh_entry*)&hide_exploit,
    (vsh_entry*)&fast_gameboot,
    (vsh_entry*)&language,
    (vsh_entry*)&font,
    (vsh_entry*)&plugins,
    (vsh_entry*)&scan_save,
    (vsh_entry*)&swap_buttons,
    (vsh_entry*)&animations,
    (vsh_entry*)&open_peops_menu,
};
