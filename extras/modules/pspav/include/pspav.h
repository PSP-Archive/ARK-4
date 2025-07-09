#ifndef PSPAV_H
#define PSPAV_H

#include "pspav_entry.h"

unsigned char pspavPlayGamePSPAV(PSPAVEntry* e, PSPAVCallbacks* callbacks, int x, int y);
void pspavPlayVideoFile(const char* path, PSPAVCallbacks* callbacks);

#endif
