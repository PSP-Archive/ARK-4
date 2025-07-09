#ifndef PSPAV_WRAPPER_H
#define PSPAV_WRAPPER_H

#include <pspav.h>
#include "entry.h"

extern PSPAVCallbacks av_callbacks;

PSPAVEntry convertEntry(Entry* e);
int loadstartPSPAV();
int stopunloadPSPAV();

#endif