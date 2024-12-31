#ifndef REBOOT661_H
#define REBOOT661_H

#include "rebootbin.h"
#include <psptypes.h>

u8 reboot661[0x12E18] __attribute__((aligned(16))) = 
{
    REBOOT661_HEADER
};
 
#endif
