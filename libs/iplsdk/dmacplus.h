#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

enum PixelFormat
{
    PIXEL_FORMAT_RGBA8888 = 0,
    PIXEL_FORMAT_RGB565,
    PIXEL_FORMAT_RGBA5551,
    PIXEL_FORMAT_RGBA4444,
};

void dmacplus_init(void);
void dmacplus_lcdc_init(void);
void dmacplus_lcdc_enable(void);
void dmacplus_lcdc_disable(void);
int dmacplus_lcdc_set_base_addr(uintptr_t addr);
int dmacplus_lcdc_set_format(unsigned int width, unsigned int stride, enum PixelFormat format);

#ifdef __cplusplus
}
#endif //__cplusplus
