#ifndef SYSTEM_H
#define SYSTEM_H

#include "system_entry.h"

namespace SystemMgr{
    extern void initMenu();
    extern void startMenu();
    extern void flushFiles();
    extern void endMenu();
    extern void pauseDraw();
    extern void resumeDraw();
    extern SystemEntry* getSystemEntry(unsigned index);
};

#endif
