/////////////////////////////////////////////////////////
// status/debug logging (saves to "err.txt")

//void my_print_init();
//void my_print(const char* str);
//void my_printn(const char* str1, int val, const char* str2);

/////////////////////////////////////////////////////////
// simple picker

#define MAX_PICK 5
    // tight fit because we use the very big font
    // 32 for title + 5 * (8 gap + 32 + 8 fineprint) = 272 exactly

#define MAX_PICK_TITLE 80
#define MAX_PICK_MAINSTR 13
#define MAX_PICK_FINEPRINT 45

typedef struct
{
    char szBig[MAX_PICK_MAINSTR+1]; // big font
    char szFinePrint[MAX_PICK_FINEPRINT+1]; // small font
    u32 userData;
} PICKER_INFO;

typedef struct
{
    char szTitle[MAX_PICK_TITLE+1];
    int pick_count;
    PICKER_INFO picks[MAX_PICK];
    int pick_start; // -1 for none, 0->pick_count-1 for selection
} PICKER;

int my_picker(const PICKER* pickerP);
    // returns new pick or -1 if cancelled

void my_initpicker(PICKER* pickerP, const char* szTitle);
bool my_addpick(PICKER* pickerP, const char* szBig, const char* szFinePrint, u32 userData);

int my_loadpicks_fromfile(PICKER* pickerP, const char* szFile, bool bParseIP);
    // returns number of entries read

extern char *my_dirname(char *filename);


/////////////////////////////////////////////////////////
