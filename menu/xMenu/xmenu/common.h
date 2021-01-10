#ifndef COMMON_H
#define COMMON_H

#include "debug.h"
#include <pspgu.h>
#include <pspdisplay.h>
#include <pspkernel.h>
#include <intraFont.h>
#include <string>

extern "C"{
#include "../graphics/graphics.h"
}

#define THREAD_DELAY 1000

namespace common{

	static int argc;
	static char** argv;
	static Image* background;
	static Image* noicon;
	static intraFont* font;

	extern void setArgs(int c, char** v);
	extern bool fileExists(const std::string &path);
	extern void loadData();
	extern void deleteData();
	extern Image* getBG();
	extern Image* getNoIcon();
	extern intraFont* getFont();
	extern void printText(float x, float y, const char *text);
	extern void flip();

}

#endif
