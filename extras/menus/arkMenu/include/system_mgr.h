#ifndef SYSTEM_H
#define SYSTEM_H

#include "system_entry.h"

namespace SystemMgr{
    extern void initMenu(SystemEntry**, int);
    extern void startMenu();
    extern void flushFiles();
    extern void stopMenu();
    extern void endMenu();
    extern void pauseDraw();
    extern void resumeDraw();
    extern void enterFullScreen();
    extern void exitFullScreen();
    extern SystemEntry* getSystemEntry(unsigned index);
};

#endif
