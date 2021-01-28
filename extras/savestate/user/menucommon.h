#ifndef COMMON_H
#define COMMON_H

#include <pspgu.h>
#include <pspdisplay.h>
#include <pspkernel.h>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <malloc.h>
#include "gfx.h"

#define THREAD_DELAY 1000

// Colors
enum colors {
	RED =	0xFF0000FF,
	GREEN =	0xFF00FF00,
	BLUE =	0xFFFF0000,
	WHITE =	0xFFFFFFFF,
	LITEGRAY = 0xFFBFBFBF,
	GRAY =  0xFF7F7F7F,
	DARKGRAY = 0xFF3F3F3F,		
	BLACK = 0xFF000000,
	YELLOW = 0xFF00FFFF,
	WEIRD_ORANGE = 0xFF2AB1FF
};

enum images {
	IMAGE_BG,
	IMAGE_BOX,
	IMAGE_LOAD,
	IMAGE_SAVE,
	IMAGE_EXIT,
	IMAGE_NOSCREEN,
	IMAGE_WAITICON,
};

#define MAX_IMAGES 7

#define SIZE_LITTLE 0.51f
#define SIZE_MEDIUM 0.6f
#define SIZE_BIG 0.7f

#define PKG_PATH "ms0:/SAVESTATE/SAVE.PKG"
#define FONT_PATH "ms0:/SAVESTATE/FONT.PGF"
#define CONFIG_PATH "CONFIG.BIN"

namespace common{

	extern int getArgc();
	extern char** getArgv();
	extern bool has_suffix(const std::string &str, const std::string &suffix);
	SceOff findPkgOffset(const char* filename, unsigned* size = NULL);
	extern void* readFromPKG(const char* filename, unsigned* size = NULL);
	extern u32 getMagic(const char* filename, unsigned int offset);
	extern void loadData(int ac, char** av);
	extern void deleteData();
	extern bool fileExists(const std::string &path);
	extern bool folderExists(const std::string &path);
	extern Image* getImage(int which);
	extern bool isSharedImage(Image* img);
	extern intraFont* getFont();
	extern void printText(float x, float y, const char *text, u32 color=GRAY_COLOR, float size=SIZE_LITTLE, int glow=0, int scroll=0);
	extern void clearScreen(u32 color);
	extern void drawBorder();
	extern void flipScreen();
	extern void upperString(char* text);
	extern int maxString(string* strings, int n_strings);
	extern std::string getExtension(std::string path);

}

#endif
