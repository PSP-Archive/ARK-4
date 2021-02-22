#ifndef DEBUG_H
#define DEBUG_H

#include <cstdio>
#include <cstring>

extern "C"{
#include "../graphics/graphics.h"
}

void debugScreen(const char* text);
void debugFile(const char* text);

#endif
