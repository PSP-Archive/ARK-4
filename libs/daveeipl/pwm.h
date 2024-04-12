#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

void pwm_init(void);
int pwm_enable_channel(uint32_t channel, uint32_t unk, uint32_t unk2, uint32_t unk3);
int pwm_query_channel(uint32_t channel, uint16_t *out1, uint16_t *out2, uint32_t *out3);
int pwm_disable_channel(uint32_t channel);

#ifdef __cplusplus
}
#endif //__cplusplus
