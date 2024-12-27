#ifndef COLORDEBUGGER_H
#define COLORDEBUGGER_H

#include <stdlib.h>
#include <psptypes.h>

extern u32* g_vram_base;

// fill vram with the given color
void colorDebug(u32 color);

// set screen handler
void setScreenHandler(void (*handler)(u32 vram));

// fill vram with an infinite loop of colors
void doBreakpoint(void);

// Set a colorLoop call at a specified address
void setBreakpoint(u32 addr);

#endif
