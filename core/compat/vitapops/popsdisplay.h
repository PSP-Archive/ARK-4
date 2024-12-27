#ifndef POPS_DISPLAY
#define POPS_DISPLAY

// TN-X patches for PSX exploits on Vita

#define PSP_SCREEN_WIDTH 480
#define PSP_SCREEN_HEIGHT 272
#define PSP_SCREEN_LINE 512
#define SCE_PSPEMU_FRAMEBUFFER_SIZE 0x88000

#define MAX_VRAM_BUFFERS 2
typedef struct POPSFrameBufferConfig{
    short x;
    short y;
    short width;
    short height;
    unsigned char color_width;
    unsigned char cur_buffer;
}POPSFrameBufferConfig;

typedef struct POPSVramConfig{
    POPSFrameBufferConfig configs[MAX_VRAM_BUFFERS];
    unsigned char counter;
}POPSVramConfig;

extern POPSVramConfig* vram_config;
extern u16* pops_vram;

// Initialize PSX Vram
void initVitaPopsVram();

void copyPSPVram(u32* psp_vram);

// Software render PSP Vram into PSX Vram
void SoftRelocateVram(u32* psp_vram, u16* ps1_vram);

// register custom vram handler
void* registerPSXVramHandler(void (*handler)(u32* psp_vram, u16* ps1_vram));

#endif
