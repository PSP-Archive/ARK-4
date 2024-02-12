#ifndef COMMON_H
#define COMMON_H

#include "debug.h"
#include <pspgu.h>
#include <pspdisplay.h>
#include <pspkernel.h>
#include <string>
#include "graphics.h"
#include "../../arkMenu/include/conf.h"

#define THREAD_DELAY 1000

namespace common{

    static int argc;
    static char** argv;
    static Image* background;
    static Image* noicon;

    extern void setArgs(int c, char** v);
    extern bool fileExists(const std::string &path);
    extern void loadData();
    extern void deleteData();
    extern Image* getBG();
    extern Image* getNoIcon();
    extern void printText(float x, float y, const char *text, u32 color = WHITE_COLOR);
    extern void flip();
	extern void saveConf();
    extern t_conf* getConf();
	extern void resetConf();
	extern void loadConf();
}

#endif
