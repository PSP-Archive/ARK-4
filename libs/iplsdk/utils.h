#pragma once

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

void clear_dcache(void);
void clear_icache(void);

unsigned int delay_us(unsigned int usec);

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
