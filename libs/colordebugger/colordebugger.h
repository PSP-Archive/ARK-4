#ifndef COLORDEBUGGER_H
#define COLORDEBUGGER_H

#include <stdlib.h>
#include <psptypes.h>

extern u32* g_vram_base;

// variable to set vita pops state
int colorDebugIsVitaPops();
void colorDebugSetIsVitaPops(int is);

// init Vita POPS Vram
void initVitaPopsVram();

// copy PSP Vram to POPS Vram
void copyPSPVram(u32* psp_vram);

// fill vram with the given color
void colorDebug(u32 color);

// fill vram with an infinite loop of colors
void colorLoop(void);

// Set a handler for PSP to PSX Vram copy
void* registerPSXVramHandler(void (*handler)(u32* psp_vram, u16* ps1_vram));

#endif
