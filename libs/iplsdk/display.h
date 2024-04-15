#pragma once

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#pragma once

void display_init(void);
void display_set_framebuffer(const void *framebuffer);
void display_set_mode(int mode, unsigned int xres, unsigned yres, unsigned stride, unsigned int format);

#ifdef __cplusplus
}
#endif //__cplusplus
