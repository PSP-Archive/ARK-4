#ifndef COMMON_H
#define COMMON_H

#include "debug.h"
#include <pspgu.h>
#include <pspdisplay.h>
#include <pspkernel.h>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <malloc.h>
#include <time.h>
#include <globals.h>
#include "gfx.h"
#include "mp3.h"
#include "conf.h"

#define THREAD_DELAY 1000

// Colors
enum colors {
    RED =    0xFF0000FF,
    GREEN =    0xFF00FF00,
    BLUE =    0xFFFF0000,
    WHITE =    0xFFFFFFFF,
    LITEGRAY = 0xFFBFBFBF,
    GRAY =  0xFF7F7F7F,
    DARKGRAY = 0xFF3F3F3F,        
    BLACK = 0xFF000000,
    YELLOW = 0xFF00FFFF,
    WEIRD_ORANGE = 0xFF2AB1FF
};

enum images {
    IMAGE_BG,
    IMAGE_NOICON,
    IMAGE_WAITICON,
    IMAGE_GAME,
    IMAGE_SETTINGS,
    IMAGE_BROWSER,
    IMAGE_FTP,
    IMAGE_DIALOG,
    IMAGE_LOADING,
    IMAGE_SPRITE,
    IMAGE_EXIT,
    IMAGE_PLUGINS,
    MAX_IMAGES
};

enum {
    FOLDER,
    FILE_BIN,
    FILE_TXT,
    FILE_PBP,
    FILE_PRX,
    FILE_ISO,
    FILE_ZIP,
    FILE_MUSIC,
    FILE_PICTURE,
    MAX_FILE_TYPES,
};

#define SIZE_TINY 0.4f
#define SIZE_LITTLE 0.51f
#define SIZE_MEDIUM 0.6f
#define SIZE_BIG 0.7f
#define SIZE_HUGE 1.5f

#define THEME_NAME "THEME.ARK"

#define MS0_PATH 0x3A30736D // 'ms0:' as u32
#define EF0_PATH 0x3A306665 // 'ef0:' as u32

namespace common{

    extern ARKConfig* getArkConfig();
    extern int getArgc();
    extern char** getArgv();
    extern int getPspModel();
    extern struct tm getDateTime();
    extern bool has_suffix(const std::string &str, const std::string &suffix);
    SceOff findPkgOffset(const char* filename, unsigned* size = NULL);
    extern void* readFromPKG(const char* filename, unsigned* size = NULL);
    extern u32 getMagic(const char* filename, unsigned int offset);
    extern void loadData(int ac, char** av);
    extern void deleteData();
    extern void loadTheme();
    extern void deleteTheme();
    extern void setThemePath(char* path = NULL);
    extern bool fileExists(const std::string &path);
    extern bool folderExists(const std::string &path);
    extern long fileSize(const std::string &path);
    extern u64 deviceSize(const std::string path);
    extern string beautifySize(u64 size);
    extern Image* getImage(int which);
    extern Image* getIcon(int which);
    extern Image* getCheckbox(int which);
    extern bool isSharedImage(Image* img);
    extern intraFont* getFont();
    extern MP3* getMP3Sound();
    extern void saveConf();
    extern t_conf* getConf();
    extern void resetConf();
    extern void playMenuSound();
    extern void printText(float x, float y, const char *text, u32 color=GRAY_COLOR, float size=SIZE_LITTLE, int glow=0, int scroll=0);
    extern int calcTextWidth(const char* text, float size=SIZE_LITTLE);
    extern void clearScreen(u32 color = CLEAR_COLOR);
    extern void drawBorder();
    extern void drawScreen();
    extern bool canDrawBackground();
    extern void flipScreen();
    extern void upperString(char* text);
    extern int maxString(string* strings, int n_strings);
    extern std::string getExtension(std::string path);
    extern void launchRecovery();
}

#endif
