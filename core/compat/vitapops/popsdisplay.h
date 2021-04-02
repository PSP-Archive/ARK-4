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

// Initialize PSX Vram
void initVitaPopsVram();

// Software render PSP Vram into PSX Vram
void SoftRelocateVram(u32* psp_vram, u16* ps1_vram);

// register custom vram handler
void* registerPSXVramHandler(void (*handler)(u32* psp_vram, u16* ps1_vram));

// original sony function
extern int (* _sceDisplaySetFrameBufferInternal)(int pri, void *topaddr, int width, int format, int sync);

// hooked function to copy framebuffer
int sceDisplaySetFrameBufferInternalHook(int pri, void *topaddr,
        int width, int format, int sync);

extern void* sctrlHENSetPSXVramHandler(void (*handler)(u32* psp_vram, u16* ps1_vram));

#endif
