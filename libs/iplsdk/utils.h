#pragma once

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

void util_delay_cpu222_us(unsigned int usec);

#define _wsbh(x) __extension__({ \
    unsigned int __x = (x), __v; \
    __asm ("wsbh %0,%1" \
             : "=d" (__v) \
             : "d" (__x)); \
    __v; \
})

#ifdef __cplusplus
}
#endif //__cplusplus
