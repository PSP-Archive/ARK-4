#ifndef COLORDEBUGGER_H
#define COLORDEBUGGER_H

#include <stdlib.h>
#include <psptypes.h>

#define MAX_VRAM_CONFIGS 2
typedef struct POPSVramConfig{
	short x;
	short y;
	short width;
	short height;
	unsigned char color_width;
	unsigned char cur_buffer;
}POPSVramConfig;

typedef struct POPSVramConfigVLA{
	POPSVramConfig configs[MAX_VRAM_CONFIGS];
	unsigned char counter;
}POPSVramConfigVLA;

extern POPSVramConfigVLA* vram_config;
extern u32* g_vram_base;
extern u16* ps1_vram;

// variable to set vita pops state
int isVitaPops();
void setIsVitaPops(int is);

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
