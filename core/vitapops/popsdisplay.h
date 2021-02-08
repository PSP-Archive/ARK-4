#ifndef POPS_DISPLAY
#define POPS_DISPLAY

// TN-X patches for PSX exploits on Vita

#define MAX_VRAM_CONFIGS 2
typedef struct POPSVramConfig{
    short x;
    short y;
    short width;
    short height;
    unsigned char color_width;
    unsigned char cur_buffer;
}POPSVramConfig;

typedef struct POPSVramConfigList{
    POPSVramConfig configs[MAX_VRAM_CONFIGS];
    unsigned char counter;
}POPSVramConfigList;

extern POPSVramConfigList* vram_config;
extern u16* pops_vram;

void patchVitaPopsDisplay();
void* sctrlHENSetPSXVramHandler(void (*handler)(u32* psp_vram, u16* ps1_vram));

#endif
