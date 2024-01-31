#include <pspsdk.h>
#include <pspdebug.h>
#include <pspiofilemgr.h>
#include <pspctrl.h>
#include <pspinit.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <systemctrl.h>
#include <systemctrl_se.h>

// Default Start Path
#define START_PATH "ms0:/"
#define START_PATH_GO "ef0:/"
#define stricmp strcasecmp

// Current Path
static char cwd[1024];

enum {
    MODE_HOMEBREW = 0,
    MODE_ISO = 1,
    MODE_POPS = 2,
    MODE_MAX = 3,
};

// Runlevel Definition
#define HOMEBREW_RUNLEVEL 0x141
#define HOMEBREW_RUNLEVEL_GO 0x152
#define ISO_RUNLEVEL 0x123
#define POPS_RUNLEVEL 0x144
#define ISO_RUNLEVEL_GO 0x125
#define POPS_RUNLEVEL_GO 0x155

// Copy Move Origin
static char copysource[1024];

// Copy Flags
#define COPY_FOLDER_RECURSIVE 2
#define COPY_DELETE_ON_FINISH 1
#define COPY_KEEP_ON_FINISH 0
#define NOTHING_TO_COPY -1

// Copy Mode
// -1 Nothing
//  0 Copy
//  1 Move
int copymode = NOTHING_TO_COPY;

// Generic Definitions
#define CLEAR 1
#define KEEP 0

// GUI Definitions
#define FONT_SIZE 7
#define PADDING 3
#define FONT_COLOR 0xFFFFFF
#define FONT_SELECT_COLOR 0xFFD800
#define BACK_COLOR 0xBCAD94
#define CENTER_X(s) (240 - ((strlen(s) * FONT_SIZE) >> 1))
#define RIGHT_X(s) (480 - (strlen(s) * FONT_SIZE))
#define CENTER_Y 132
#define FILES_PER_PAGE 18

// Button Test Macro
#define PRESSED(b, a, m) (((b & m) == 0) && ((a & m) == m))
#define NELEMS(x) (sizeof(x)/sizeof(x[0]))

#define EBOOT_MAGIC 0x50425000
#define ELF_MAGIC 0x464C457F

#define PS1_CAT 0x454D
#define PSN_CAT 0x4745
#define HMB_CAT 0x474D

// File Information Structure
typedef struct File
{
    // Next Item
    struct File * next;
    
    // Folder Flag
    int isFolder;
    
    // File Name
    char name[256];
} File;

typedef struct  __attribute__((packed)) {
	u32 signature;
	u32 version;
	u32 fields_table_offs;
	u32 values_table_offs;
	int nitems;
} SFOHeader;

typedef struct __attribute__((packed)) {
	u16 field_offs;
	u8  unk;
	u8  type; // 0x2 -> string, 0x4 -> number
	u32 unk2;
	u32 unk3;
	u16 val_offs;
	u16 unk4;
} SFODir;

typedef struct SfoInfo {
    char title[128];
    char gameid[10];
}SfoInfo;

typedef struct
{
    u32 magic;
    u32 version;
    u32 param_offset;
    u32 icon0_offset;
    u32 icon1_offset;
    u32 pic0_offset;
    u32 pic1_offset;
    u32 snd0_offset;
    u32 elf_offset;
    u32 psar_offset;
} PBPHeader;

// Prototypes
void printoob(char * text, int x, int y, unsigned int color);
void updateList(int clearindex);
void paintList(int withclear);
void recursiveFree(File * node);
void start(void);
void pluginInstall(File *file);
int delete(void);
int navigate(void);
void copy(int flag);
int paste(void);
File * findindex(int index);
int delete_folder_recursive(char * path);
int copy_file(char * a, char * b);
int copy_folder_recursive(char * a, char * b);
int isGameISO(const char * path);

// Menu Position
int position = 0;

// Number of Files
int filecount = 0;

// File List
File * files = NULL;

extern int is_launcher_mode;

// Entry Point
int proshell_main()
{

    // Set Start Path
    strcpy(cwd, START_PATH_GO);
    
    // Initialize Screen Output
    pspDebugScreenClear();
    
    // Update List
    updateList(CLEAR);
    
    // Paint List
    paintList(CLEAR);
    
    // Last Buttons
    unsigned int lastbuttons = 0;

    int working = 1;
    
    // Input Loop
    while (working)
    {
        // Button Data
        SceCtrlData data;
        
        // Clear Memory
        memset(&data, 0, sizeof(data));
        
        // Read Button Data
        sceCtrlSetSamplingCycle(0);
        sceCtrlSetSamplingMode(1);
        sceCtrlReadBufferPositive(&data, 1);
		File * file = findindex(position);
        
        // Other Commands
        if(filecount > 0)
        {
            // Start File
            if(PRESSED(lastbuttons, data.Buttons, PSP_CTRL_START))
            {
                // Start File
                start();
                // Paint List
                paintList(CLEAR);
            }
            
            // Delete File
            else if(PRESSED(lastbuttons, data.Buttons, PSP_CTRL_SELECT))
            {
                // Delete File
                if(delete() == 0)
                {
                    // Update List
                    updateList(KEEP);
                    
                    // Paint List
                    paintList(CLEAR);
                }
            }
            
            // Position Decrement
            else if(PRESSED(lastbuttons, data.Buttons, PSP_CTRL_UP))
            {
                // Decrease Position
                if(position > 0) position--;
                
                // Rewind Pointer
                else position = filecount - 1;
                
                // Paint List
                paintList(KEEP);
            }
            
            // Position Increment
            else if(PRESSED(lastbuttons, data.Buttons, PSP_CTRL_DOWN))
            {
                // Increase Position
                if(position < (filecount - 1)) position++;
                
                // Rewind Pointer
                else position = 0;
                
                // Paint List
                paintList(KEEP);
            }
            
            // Navigate to Folder
            else if(PRESSED(lastbuttons, data.Buttons, PSP_CTRL_CROSS))
            {
                // Attempt to navigate to Target
                if(navigate() == 0)
                {
                    // Update List
                    updateList(CLEAR);
                    
                    // Paint List
                    paintList(CLEAR);
                }
            }
            
            // Copy
            else if(PRESSED(lastbuttons, data.Buttons, PSP_CTRL_SQUARE))
            {
                // Copy File
                copy(COPY_KEEP_ON_FINISH);
                
                // Paint List
                paintList(KEEP);
            }
            
            // Cut
            else if(PRESSED(lastbuttons, data.Buttons, PSP_CTRL_TRIANGLE))
            {
                // Copy File
                copy(COPY_DELETE_ON_FINISH);
                
                // Paint List
                paintList(KEEP);
            }
            
            // Paste
            else if(PRESSED(lastbuttons, data.Buttons, PSP_CTRL_CIRCLE))
            {
                // Paste File
                if(paste() == 0)
                {
                    // Update List
                    updateList(KEEP);
                    
                    // Paint List
                    paintList(CLEAR);
                }
            }
        }

        // change device
        if(PRESSED(lastbuttons, data.Buttons, PSP_CTRL_RTRIGGER))
        {
            // change device
            if (cwd[0] == 'm') strcpy(cwd, START_PATH_GO);
            else if (cwd[0] == 'e') strcpy(cwd, START_PATH);

            // Update List
            updateList(CLEAR);
                
            // Paint List
            paintList(CLEAR);
        }

        // finish
        else if(PRESSED(lastbuttons, data.Buttons, PSP_CTRL_LTRIGGER))
        {
            working = 0;
        }
        
        // Copy Buttons to Memory
        lastbuttons = data.Buttons;
        
        // Delay Thread (~100FPS are enough)
        sceKernelDelayThread(10000);
    }

    // Exit Function
    return 0;
}

// Print Out of Bounds Text
void printoob(char * text, int x, int y, unsigned int color)
{
    // Iterate Characters
    int i = 0; for(; i < strlen(text); i++)
    {
        // Print Character
        pspDebugScreenPutChar(x + i * FONT_SIZE, y, color, (unsigned char)text[i]);
    }
}

// Update File List
void updateList(int clearindex)
{
    // Clear List
    recursiveFree(files);
    files = NULL;
    filecount = 0;
    
    // Open Working Directory
    int directory = sceIoDopen(cwd);

    if (directory < 0) {
        // retry other device
        if (strcmp(cwd, START_PATH) == 0) strcpy(cwd, START_PATH_GO);
        else if (strcmp(cwd, START_PATH_GO) == 0) strcpy(cwd, START_PATH);
        directory = sceIoDopen(cwd);
    }
    
    // Opened Directory
    if(directory >= 0)
    {
        // File Info Read Result
        int dreadresult = 1;
        
        // Iterate Files
        while(dreadresult > 0)
        {
            // File Info
            SceIoDirent info;
            
            // Clear Memory
            memset(&info, 0, sizeof(info));
            
            // Read File Data
            dreadresult = sceIoDread(directory, &info);
            
            // Read Success
            if(dreadresult > 0)
            {
                // Ingore null filename
                if(info.d_name[0] == '\0') continue;

                // Ignore "." in all Directories
                if(strcmp(info.d_name, ".") == 0) continue;
                
                // Ignore ".." in Root Directory
                if( (strcmp(cwd, START_PATH) == 0 || strcmp(cwd, START_PATH_GO) == 0) && strcmp(info.d_name, "..") == 0) continue;
                
                // Allocate Memory
                File * item = (File *)malloc(sizeof(File));
                
                // Clear Memory
                memset(item, 0, sizeof(File));
                
                // Copy File Name
                strcpy(item->name, info.d_name);
                
                // Set Folder Flag
                item->isFolder = FIO_S_ISDIR(info.d_stat.st_mode);
                
                // New List
                if(files == NULL) files = item;
                
                // Existing List
                else
                {
                    // Iterator Variable
                    File * list = files;
                    
                    // Append to List
                    while(list->next != NULL) list = list->next;
                    
                    // Link Item
                    list->next = item;
                }
                
                // Increase File Count
                filecount++;
            }
        }
        
        // Close Directory
        sceIoDclose(directory);
    }
    
    // Attempt to keep Index
    if(!clearindex)
    {
        // Fix Position
        if(position >= filecount) position = filecount - 1;
    }
    
    // Reset Position
    else position = 0;
}

// Paint Picker List
void paintList(int withclear)
{
    // Clear Screen
    if(withclear) pspDebugScreenClear();
    
    // Paint Current Path
    printoob(cwd, 10, 10, FONT_COLOR);
    
    // Paint Serial Number
    char serialnumber[64];
    printoob(serialnumber, 10, 242, FONT_COLOR);
    
    // Paint Controls
    printoob("[ UP & DOWN ]    File Selection", 10, 252, FONT_COLOR);
    printoob("[ START ]  Run File", 330, 252, FONT_COLOR);
    printoob("[ SELECT ] Delete", 330, 262, FONT_COLOR);
    printoob("SQUARE    Copy", 345, 120, FONT_COLOR);
    printoob("TRIANGLE  Cut", 345, 130, FONT_COLOR);
    printoob("CIRCLE    Paste", 345, 140, FONT_COLOR);
    printoob("CROSS     Navigate", 345, 150, FONT_COLOR);
    printoob("RTRIGGER  Device", 345, 160, FONT_COLOR);
    printoob("LTRIGGER  Exit", 345, 170, FONT_COLOR);
    
    // Copy Paste in Progress
    if(copymode != NOTHING_TO_COPY)
    {
        // Get Cache Source Filename
        char * filename = NULL; int i = 0; for(; i < strlen(copysource); i++) if(copysource[i] == '/') filename = copysource + i + 1;
        
        // Paint to Screen
        printoob(filename, 10, 242, FONT_SELECT_COLOR);
    }
    
    // File Iterator Variable
    int i = 0;
    
    // Print Counter
    int printed = 0;
    
    // Paint File List
    File * file = files;
    for(; file != NULL; file = file->next)
    {
        // Printed enough already
        if(printed == FILES_PER_PAGE) break;
        
        // Interesting File
        if(position < FILES_PER_PAGE || i > (position - FILES_PER_PAGE))
        {
            // Default Font Color
            unsigned int color = FONT_COLOR;
            
            // Selected File Font Color
            if(i == position) color = FONT_SELECT_COLOR;
            
            // Print Node Type
            printoob((file->isFolder) ? ("D") : ("F"), 10, 30 + (FONT_SIZE + PADDING) * printed, FONT_COLOR);

            char buf[64];

            strncpy(buf, file->name, sizeof(buf));
            buf[sizeof(buf) - 1] = '\0';
            int len = strlen(buf);
            len = 40 - len;

            while(len -- > 0)
            {
                strcat(buf, " ");
            }
            
            // Print Filename
            printoob(buf, 20, 30 + (FONT_SIZE + PADDING) * printed, color);
            
            // Increase Print Counter
            printed++;
        }
        
        // Increase Counter
        i++;
    }
}

// Free Heap Memory
void recursiveFree(File * node)
{
    // not recursive anymore
    while (node){
        File* aux = node;
        node = node->next;
        free(aux);
    }
}

// Game ISO File Check
int isGameISO(const char * path)
{
    const char *ext;
    const char* known[] = {
        ".iso", ".img", ".cso", ".zso", ".jso", ".dax"
    };

    ext = path + strlen(path) - 4;

    if (ext > path)
    {
        //check extension
        for (int i=0; i<5; i++){
            if (stricmp(ext, known[i]) == 0)
            {
                return 1;
            }
        }
    }

    return 0;
}

// Game ISO File Check
int isEboot(const char * path)
{
    const char *ext;

    ext = path + strlen(path) - 4;

    if (ext > path)
    {
        //check extension
        if (stricmp(ext, ".pbp") == 0)
        {
            return 1;
        }
    }

    return 0;
}

// Game ISO File Check
int isPlugin(const char * path)
{
    const char *ext;

    ext = path + strlen(path) - 4;

    if (ext > path)
    {
        //check extension
        if (stricmp(ext, ".prx") == 0)
        {
            return 1;
        }
    }

    return 0;
}

int getSfoParam(unsigned char* sfo_buffer, int buf_size, char* param_name, unsigned char* var, int* var_size){
    SFOHeader *header = (SFOHeader *)sfo_buffer;
	SFODir *entries = (SFODir *)(sfo_buffer + sizeof(SFOHeader));
    int res = 0;
	int i;
	for (i = 0; i < header->nitems; i++) {
		if (strcmp((char*)sfo_buffer + header->fields_table_offs + entries[i].field_offs, param_name) == 0) {
			memcpy(var, sfo_buffer + header->values_table_offs + entries[i].val_offs, *var_size);
            res = 1;
			break;
		}
	}
    return res;
}

int getEbootRunlevel(const char* path){
    PBPHeader header;

    int fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
    if (fd < 0) return -1;

    sceIoRead(fd, &header, sizeof(header));

    if (header.magic == ELF_MAGIC){ // 1.50 homebrew
        sceIoClose(fd);
        return MODE_HOMEBREW;
    }
    else if (header.magic != EBOOT_MAGIC){
        sceIoClose(fd);
        return -1;
    }
    
    int mode = -1;
    
    u32 size = header.icon0_offset - header.param_offset;
    
    if (size == 0){
        sceIoClose(fd);
        return -1;
    }

    void* buf = malloc(size);
    sceIoLseek32(fd, header.param_offset, SEEK_SET);
    sceIoRead(fd, buf, size);
    sceIoClose(fd);

    u16 categoryType = 0;
    int value_size = sizeof(categoryType);
    int success = getSfoParam(buf, size, "CATEGORY", (unsigned char*)(&categoryType), &value_size);

    if (!success) return -1;

    switch(categoryType){
        case HMB_CAT:      mode = MODE_HOMEBREW;    break;
        case PSN_CAT:      mode = MODE_ISO;         break;
        case PS1_CAT:      mode = MODE_POPS;        break;
        default:                                    break;
    }

    return mode;
}

int getRunlevelMode(int mode)
{
    switch(mode)
    {
        case MODE_HOMEBREW:
            return HOMEBREW_RUNLEVEL;
        case MODE_ISO:
            return (cwd[0] == 'e')? ISO_RUNLEVEL_GO : ISO_RUNLEVEL;
        case MODE_POPS:
            return (cwd[0] == 'e')? POPS_RUNLEVEL_GO : POPS_RUNLEVEL;
    }

    return -1;
}


struct Items {
	int mode;
	int offset;
	char text[64];
} items[] = {
	{ 0, 40, "Cancel" },
	{ 1, 50, "Always" },
	{ 2, 60, "Game" },
	{ 3, 70, "POPS (PS1)" },
	{ 4, 80, "VSH (XMB)" },
	{ 5, 90, "UMD/ISO" },
	{ 6, 100, "Homebrew" },
    { 7, 110, "<LoadStart>" },
	{ 8, 0, "-> " }
};

char* plugin_runlevels[] = {"always", "game", "ps1", "xmb", "umd", "homebrew"};

void printPluginInstall(char* plugin_name, int cur){
    char title[64];

    pspDebugScreenClear();
	snprintf(title, sizeof(title), "Install Plugin %s?", plugin_name);
	printoob(title, 50, 15, FONT_COLOR);

    for (int i=0; i<NELEMS(items)-1; i++){
        if(i==cur) {
            char buf[128];
            snprintf(buf, sizeof(buf), "%s%s", items[8].text, items[i].text);
            printoob(buf, 175, items[i].offset, FONT_SELECT_COLOR);
        }
        else {
            printoob(items[i].text, 175, items[i].offset, FONT_COLOR);
		}
    }
}

void pluginInstall(File *file) {

	SceCtrlData pad;
    int cur = 0;

    printPluginInstall(file->name, cur);
    sceKernelDelayThread(500000);

	while(1){
		sceCtrlReadBufferPositive(&pad, 1);
		if ((pad.Buttons&PSP_CTRL_DOWN) == PSP_CTRL_DOWN){
            if (cur < NELEMS(items)-1) cur++;
            printPluginInstall(file->name, cur);
            sceKernelDelayThread(500000);
        }
        else if ((pad.Buttons&PSP_CTRL_UP) == PSP_CTRL_UP){
            if (cur > 0) cur--;
            printPluginInstall(file->name, cur);
            sceKernelDelayThread(500000);
        }
        else if ((pad.Buttons&PSP_CTRL_CROSS) == PSP_CTRL_CROSS){
            sceKernelDelayThread(500000);
            break;
        }
	}

    if (!cur) return;

    char buf[256];
    if (cur == 7){
        snprintf(buf, sizeof(buf), "%s%s", cwd, file->name);
        int uid = kuKernelLoadModule(buf, 0, NULL);
        int res = sceKernelStartModule(uid, strlen(buf) + 1, (void*)buf, NULL, NULL);
        sceKernelDelayThread(500000);
        pspDebugScreenClear();
    }
    else {
        char* txt_path = (cwd[0] == 'e')? "ef0:/SEPLUGINS/PLUGINS.TXT" : "ms0:/SEPLUGINS/PLUGINS.TXT";
        snprintf(buf, sizeof(buf), "\n%s, %s%s, on\n", plugin_runlevels[cur-1], cwd, file->name);

        int fd = sceIoOpen(txt_path, PSP_O_WRONLY|PSP_O_APPEND|PSP_O_CREAT, 0777);
        sceIoWrite(fd, buf, strlen(buf));
        sceIoClose(fd);

        pspDebugScreenClear();
        printoob("Plugin installed!", 125, 115, FONT_COLOR);
        sceKernelDelayThread(2000000);
        pspDebugScreenClear();
    }
}

// Start Application
void start(void)
{

    // Find File
    File * file = findindex(position);
    
    // Not a valid file
    if(file == NULL || file->isFolder) return;

	if(isPlugin(file->name)) {
		pluginInstall(file);
		return;
	}

    // Load Execute Parameter
    struct SceKernelLoadExecVSHParam param;
    
    // Clear Memory
    memset(&param, 0, sizeof(param));
    
    // Set Common Parameters
    param.size = sizeof(param);
    
    // Boot Path Buffer
    char bootpath[1024];
    
    // Create Boot Path
    strcpy(bootpath, cwd);
    strcpy(bootpath + strlen(bootpath), file->name);
    
    int mode;

    // ISO File Check
    if(isGameISO(bootpath))
    {
        // Correct Runlevel Setting
        mode = MODE_ISO;
    }
    else if (isEboot(bootpath)){
        mode = getEbootRunlevel(bootpath);
    }
    else {
        return;
    }
    
    // Homebrew Runlevel
    if(mode == MODE_HOMEBREW)
    {
        // Prepare Homebrew Reboot
        param.args = strlen(bootpath) + 1;
        param.argp = bootpath;
        param.key = "game";
    }

    if(mode == MODE_POPS)
    {
        // Prepare Homebrew Reboot
        param.args = strlen(bootpath) + 1;
        param.argp = bootpath;
        param.key = "pops";
    }
    
    // ISO Runlevel
    else if(mode == MODE_ISO)
    {
        // EBOOT Path
        static char * ebootpath = "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN";
        
        // Prepare ISO Reboot
        param.args = strlen(ebootpath) + 1;
        param.argp = ebootpath;
        param.key = "umdemu";
        
        // PSN EBOOT Support
        if(isEboot(bootpath))
        {
            // Switch to NP9660
            sctrlSESetBootConfFileIndex(MODE_NP9660);
            sctrlSESetUmdFile("");
        }
        
        // Normal ISO
        else
        {
            // Switch to Inferno
            sctrlSESetBootConfFileIndex(MODE_INFERNO);
            sctrlSESetUmdFile(bootpath);
        }
    }
    
    int runlevel = getRunlevelMode(mode);

    // Trigger Reboot
    sctrlKernelLoadExecVSHWithApitype(runlevel, bootpath, &param);
}

// Delete Application
int delete(void)
{
    // Find File
    File * file = findindex(position);
    
    // Not found
    if(file == NULL) return -1;

    if(strcmp(file->name, "..") == 0) return -2;

    char title[64];
	SceCtrlData pad;
    int choice = 1;
    int display = 1;
    sceKernelDelayThread(1000000);
    while (1){
        if (display){
            pspDebugScreenClear();
            snprintf(title, sizeof(title), "Delete %s?", file->name);
            printoob(title, 125, 115, FONT_COLOR);
            if (choice){
                printoob("   Yes", 130, 140, FONT_COLOR);
                printoob("-> No", 265+strlen(title), 140, FONT_SELECT_COLOR);
            }
            else {
                printoob("-> Yes", 130, 140, FONT_SELECT_COLOR);
                printoob("   No", 265+strlen(title), 140, FONT_COLOR);
            }
            display = 0;
        }
        sceCtrlReadBufferPositive(&pad, 1);
        if ((pad.Buttons&PSP_CTRL_CROSS) == PSP_CTRL_CROSS){
            sceKernelDelayThread(500000);
            if (choice) return 0;
            break;
        }
        else if ((pad.Buttons & PSP_CTRL_LEFT) == PSP_CTRL_LEFT){
            if (choice) choice = 0;
            display = 1;
            sceKernelDelayThread(500000);
        }
        else if ((pad.Buttons & PSP_CTRL_RIGHT) == PSP_CTRL_RIGHT){
            if (!choice) choice = 1;
            display = 1;
            sceKernelDelayThread(500000);
        }
    }

    // File Path
    char path[1024];
    
    // Puzzle Path
    strcpy(path, cwd);
    strcpy(path + strlen(path), file->name);
    
    // Delete Folder
    if(file->isFolder)
    {
        // Add Trailing Slash
        path[strlen(path) + 1] = 0;
        path[strlen(path)] = '/';
        
        // Delete Folder
        return delete_folder_recursive(path);
    }
    
    // Delete File
    else return sceIoRemove(path);
}

// Navigate to Folder
int navigate(void)
{
    // Find File
    File * file = findindex(position);
    
    // Not a Folder
    if(file == NULL || !file->isFolder) return -1;
    
    // Special Case ".."
    if(strcmp(file->name, "..") == 0)
    {
        // Slash Pointer
        char * slash = NULL;
        
        // Find Last '/' in Working Directory
        int i = strlen(cwd) - 2; for(; i >= 0; i--)
        {
            // Slash discovered
            if(cwd[i] == '/')
            {
                // Save Pointer
                slash = cwd + i + 1;
                
                // Stop Search
                break;
            }
        }
        
        // Terminate Working Directory
        slash[0] = 0;
    }

    
    // Normal Folder
    else
    {
        // Append Folder to Working Directory
        strcpy(cwd + strlen(cwd), file->name);
        cwd[strlen(cwd) + 1] = 0;
        cwd[strlen(cwd)] = '/';
    }
    
    // Return Success
    return 0;
}

// Copy File or Folder
void copy(int flag)
{
    // Find File
    File * file = findindex(position);
    
    // Not found
    if(file == NULL) return;
    
    // Copy File Source
    strcpy(copysource, cwd);
    strcpy(copysource + strlen(copysource), file->name);
    
    // Add Recursive Folder Flag
    if(file->isFolder) flag |= COPY_FOLDER_RECURSIVE;
    
    // Set Copy Flags
    copymode = flag;
}

// Paste File or Folder
int paste(void)
{
    // No Copy Source
    if(copymode == NOTHING_TO_COPY) return -1;
    
    // Source and Target Folder are identical
    char * lastslash = NULL; int i = 0; for(; i < strlen(copysource); i++) if(copysource[i] == '/') lastslash = copysource + i;
    char backup = lastslash[1];
    lastslash[1] = 0;
    int identical = strcmp(copysource, cwd) == 0;
    lastslash[1] = backup;
    if(identical) return -2;

    char title[64];
	SceCtrlData pad;
    int choice = 1;
    int display = 1;
    sceKernelDelayThread(1000000);
    while (1){
        if (display){
            pspDebugScreenClear();
            snprintf(title, sizeof(title), "Paste %s?", copysource);
            printoob(title, 125, 115, FONT_COLOR);
            if (choice){
                printoob("   Yes", 130, 140, FONT_COLOR);
                printoob("-> No", 265+strlen(title), 140, FONT_SELECT_COLOR);
            }
            else {
                printoob("-> Yes", 130, 140, FONT_SELECT_COLOR);
                printoob("   No", 265+strlen(title), 140, FONT_COLOR);
            }
            display = 0;
        }
        sceCtrlReadBufferPositive(&pad, 1);
        if ((pad.Buttons&PSP_CTRL_CROSS) == PSP_CTRL_CROSS){
            sceKernelDelayThread(500000);
            if (choice) return 0;
            break;
        }
        else if ((pad.Buttons & PSP_CTRL_LEFT) == PSP_CTRL_LEFT){
            if (choice) choice = 0;
            display = 1;
            sceKernelDelayThread(500000);
        }
        else if ((pad.Buttons & PSP_CTRL_RIGHT) == PSP_CTRL_RIGHT){
            if (!choice) choice = 1;
            display = 1;
            sceKernelDelayThread(500000);
        }
    }
    
    // Source Filename
    char * filename = lastslash + 1;
    
    // Required Target Path Buffer Size
    int requiredlength = strlen(cwd) + strlen(filename) + 1;
    
    // Allocate Target Path Buffer
    char * copytarget = (char *)malloc(requiredlength);
    
    // Puzzle Target Path
    strcpy(copytarget, cwd);
    strcpy(copytarget + strlen(copytarget), filename);
    
    // Return Result
    int result = -3;
    
    // Recursive Folder Copy
    if((copymode & COPY_FOLDER_RECURSIVE) == COPY_FOLDER_RECURSIVE)
    {
        // Check Files in current Folder
        File * node = files; for(; node != NULL; node = node->next)
        {
            // Found a file matching the name (folder = ok, file = not)
            if(strcmp(filename, node->name) == 0 && !node->isFolder)
            {
                // Error out
                return -4;
            }
        }
        
        // Copy Folder recursively
        result = copy_folder_recursive(copysource, copytarget);
        
        // Source Delete
        if(result == 0 && (copymode & COPY_DELETE_ON_FINISH) == COPY_DELETE_ON_FINISH)
        {
            // Append Trailing Slash (for recursion to work)
            copysource[strlen(copysource) + 1] = 0;
            copysource[strlen(copysource)] = '/';
            
            // Delete Source
            delete_folder_recursive(copysource);
        }
    }
    
    // Simple File Copy
    else
    {
        // Copy File
        result = copy_file(copysource, copytarget);
        
        // Source Delete
        if(result == 0 && (copymode & COPY_DELETE_ON_FINISH) == COPY_DELETE_ON_FINISH)
        {
            // Delete File
            sceIoRemove(copysource);
        }
    }
    
    // Paste Success
    if(result == 0)
    {
        // Erase Cache Data
        memset(copysource, 0, sizeof(copysource));
        copymode = NOTHING_TO_COPY;
    }
    
    // Free Target Path Buffer
    free(copytarget);
    
    // Return Result
    return result;
}

// Find File Information by Index
File * findindex(int index)
{
    // File Iterator Variable
    int i = 0;
    
    // Find File Item
    File * file = files; for(; file != NULL && i != index; file = file->next) i++;
    
    // Return File
    return file;
}

// Delete Folder (recursively)
int delete_folder_recursive(char * path)
{
    // Internal File List
    File * filelist = NULL;
    
    // Open Working Directory
    int directory = sceIoDopen(path);
    
    // Opened Directory
    if(directory >= 0)
    {
        // File Info Read Result
        int dreadresult = 1;
        
        // Iterate Files
        while(dreadresult > 0)
        {
            // File Info
            SceIoDirent info;
            
            // Clear Memory
            memset(&info, 0, sizeof(info));
            
            // Read File Data
            dreadresult = sceIoDread(directory, &info);
            
            // Read Success
            if(dreadresult >= 0)
            {
                // Valid Filename
                if(strlen(info.d_name) > 0)
                {
                    if(strcmp(info.d_name, ".") == 0 || strcmp(info.d_name, "..") == 0)
                    {
                        continue;
                    }
                    
                    // Allocate Memory
                    File * item = (File *)malloc(sizeof(File));
                    
                    // Clear Memory
                    memset(item, 0, sizeof(File));
                    
                    // Copy File Name
                    strcpy(item->name, info.d_name);
                    
                    // Set Folder Flag
                    item->isFolder = FIO_S_ISDIR(info.d_stat.st_mode);
                    
                    // New List
                    if(filelist == NULL) filelist = item;
                    
                    // Existing List
                    else
                    {
                        // Iterator Variable
                        File * list = filelist;
                        
                        // Append to List
                        while(list->next != NULL) list = list->next;
                        
                        // Link Item
                        list->next = item;
                    }
                }
            }
        }
        
        // Close Directory
        sceIoDclose(directory);
    }
    
    // List Node
    File * node = filelist;
    
    // Iterate Files
    for(; node != NULL; node = node->next)
    {
        // Directory
        if(node->isFolder)
        {
            // Required Buffer Size
            int size = strlen(path) + strlen(node->name) + 2;
            
            // Allocate Buffer
            char * buffer = (char *)malloc(size);
            
            // Combine Path
            strcpy(buffer, path);
            strcpy(buffer + strlen(buffer), node->name);
            buffer[strlen(buffer) + 1] = 0;
            buffer[strlen(buffer)] = '/';

            // Recursion Delete
            delete_folder_recursive(buffer);
            
            // Free Memory
            free(buffer);
        }
        
        // File
        else
        {
            // Required Buffer Size
            int size = strlen(path) + strlen(node->name) + 1;
            
            // Allocate Buffer
            char * buffer = (char *)malloc(size);
            
            // Combine Path
            strcpy(buffer, path);
            strcpy(buffer + strlen(buffer), node->name);

            // Delete File
            sceIoRemove(buffer);
            
            // Free Memory
            free(buffer);
        }
    }
    
    // Free temporary List
    recursiveFree(filelist);

    // Delete now empty Folder
    return sceIoRmdir(path);
}

// Copy File from A to B
int copy_file(char * a, char * b)
{
    // Chunk Size
    int chunksize = 512 * 1024;
    
    // Reading Buffer
    char * buffer = (char *)malloc(chunksize);
    
    // Accumulated Writing
    int totalwrite = 0;
    
    // Accumulated Reading
    int totalread = 0;
    
    // Result
    int result = 0;
    
    // Open File for Reading
    int in = sceIoOpen(a, PSP_O_RDONLY, 0777);
    
    // Opened File for Reading
    if(in >= 0)
    {
        // Delete Output File (if existing)
        sceIoRemove(b);
        
        // Open File for Writing
        int out = sceIoOpen(b, PSP_O_WRONLY | PSP_O_CREAT, 0777);
        
        // Opened File for Writing
        if(out >= 0)
        {
            // Read Byte Count
            int read = 0;
            
            // Copy Loop (512KB at a time)
            while((read = sceIoRead(in, buffer, chunksize)) > 0)
            {
                // Accumulate Read Data
                totalread += read;
                
                // Write Data
                totalwrite += sceIoWrite(out, buffer, read);
            }
            
            // Close Output File
            sceIoClose(out);
            
            // Insufficient Copy
            if(totalread != totalwrite) result = -3;
        }
        
        // Output Open Error
        else result = -2;
        
        // Close Input File
        sceIoClose(in);
    }
    
    // Input Open Error
    else result = -1;
    
    // Free Memory
    free(buffer);
    
    // Return Result
    return result;
}

// Copy Folder from A to B
int copy_folder_recursive(char * a, char * b)
{
    // Open Working Directory
    int directory = sceIoDopen(a);
    
    // Opened Directory
    if(directory >= 0)
    {
        // Create Output Directory (is allowed to fail, we can merge folders after all)
        sceIoMkdir(b, 0777);
        
        // File Info Read Result
        int dreadresult = 1;
        
        // Iterate Files
        while(dreadresult > 0)
        {
            // File Info
            SceIoDirent info;
            
            // Clear Memory
            memset(&info, 0, sizeof(info));
            
            // Read File Data
            dreadresult = sceIoDread(directory, &info);
            
            // Read Success
            if(dreadresult >= 0)
            {
                // Valid Filename
                if(strlen(info.d_name) > 0)
                {
                    // Calculate Buffer Size
                    int insize = strlen(a) + strlen(info.d_name) + 2;
                    int outsize = strlen(b) + strlen(info.d_name) + 2;
                    
                    // Allocate Buffer
                    char * inbuffer = (char *)malloc(insize);
                    char * outbuffer = (char *)malloc(outsize);
                    
                    // Puzzle Input Path
                    strcpy(inbuffer, a);
                    inbuffer[strlen(inbuffer) + 1] = 0;
                    inbuffer[strlen(inbuffer)] = '/';
                    strcpy(inbuffer + strlen(inbuffer), info.d_name);
                    
                    // Puzzle Output Path
                    strcpy(outbuffer, b);
                    outbuffer[strlen(outbuffer) + 1] = 0;
                    outbuffer[strlen(outbuffer)] = '/';
                    strcpy(outbuffer + strlen(outbuffer), info.d_name);
                    
                    // Another Folder
                    if(FIO_S_ISDIR(info.d_stat.st_mode))
                    {
                        // Copy Folder (via recursion)
                        copy_folder_recursive(inbuffer, outbuffer);
                    }
                    
                    // Simple File
                    else
                    {
                        // Copy File
                        copy_file(inbuffer, outbuffer);
                    }
                    
                    // Free Buffer
                    free(inbuffer);
                    free(outbuffer);
                }
            }
        }
        
        // Close Directory
        sceIoDclose(directory);
        
        // Return Success
        return 0;
    }
    
    // Open Error
    else return -1;
}

