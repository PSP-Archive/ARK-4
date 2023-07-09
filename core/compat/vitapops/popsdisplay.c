#include <pspsdk.h>
#include <globals.h>
#include <macros.h>
#include <module2.h>
#include <pspdisplay_kernel.h>
#include <pspsysmem_kernel.h>
#include <systemctrl.h>
#include <systemctrl_private.h>
#include <pspiofilemgr.h>
#include <pspgu.h>
#include <functions.h>
#include "popsdisplay.h"

// Internal function
int (* _sceDisplaySetFrameBufferInternal)(int pri, void *topaddr, int width, int format, int sync) = NULL;

// Vram address and config
u16* pops_vram = (u16*)0x490C0000;
POPSVramConfigList* vram_config = (POPSVramConfigList*)0x49FE0000;

// Registered Vram handler
void (*_psxVramHandler)(u32* psp_vram, u16* ps1_vram) = &SoftRelocateVram; // soft render by default

// initialize POPS VRAM
void initVitaPopsVram(){
    memset((void *)pops_vram, 0, 0x3C0000);
    vram_config->counter = 0;
    vram_config->configs[0].x = 0x1F6;
    vram_config->configs[0].y = 0;
    vram_config->configs[0].width = 640*4;
    vram_config->configs[0].height = 240;
    vram_config->configs[0].color_width = sizeof(u16);
    vram_config->configs[0].cur_buffer = 0;
}

// PSP to PSX Color Conversion
static u16 RGBA8888_to_RGBA5551(u32 color)
{
    int r, g, b, a;
    a = (color >> 24) ? 0x8000 : 0;
    b = (color >> 19) & 0x1F;
    g = (color >> 11) & 0x1F;
    r = (color >> 3) & 0x1F;
    return a | r | (g << 5) | (b << 10);
}


static u32 GetPopsVramAddr(u32 framebuffer, int x, int y)
{
    return framebuffer + x * 2 + y * 640 * 4;
}

static u32 GetPspVramAddr(u32 framebuffer, int x, int y)
{
    return framebuffer + x * 4 + y * 512 * 4;
}

// Copy PSP VRAM to PSX VRAM
void SoftRelocateVram(u32* psp_vram, u16* ps1_vram)
{
    if(psp_vram)
    {
        int y;
        if (ps1_vram == NULL) ps1_vram = pops_vram;
        for(y = 0; y < 272; y++)
        {
            int x;
            for(x = 0; x < 480; x++)
            {
                u32 color = *(u32 *)GetPspVramAddr((u32)psp_vram, x, y);
                *(u16 *)GetPopsVramAddr(ps1_vram, x, y) = RGBA8888_to_RGBA5551(color);
            }
        }
    }
}

void copyPSPVram(u32* psp_vram){
    SoftRelocateVram(psp_vram, NULL);
}

// hooked function to copy framebuffer
int sceDisplaySetFrameBufferInternalHook(int pri, void *topaddr,
        int width, int format, int sync){
    if (_psxVramHandler)
        _psxVramHandler(topaddr, pops_vram); // handle vram copy
    if (_sceDisplaySetFrameBufferInternal) // passthrough
        return _sceDisplaySetFrameBufferInternal(pri, topaddr, width, format, sync); 
    return -1;
}

// register custom vram handler
void* registerPSXVramHandler(void (*handler)(u32* psp_vram, u16* ps1_vram)){
    void* prev = _psxVramHandler;
    if (handler) _psxVramHandler = handler;
    return prev;
}
