#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

void lcdc_init(void);
int lcdc_set_mode(unsigned int id, unsigned int x_res, unsigned int y_res, unsigned int format);

#ifdef __cplusplus
}
#endif //__cplusplus
