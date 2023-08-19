#pragma once

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

void cpu_enable_interrupts(void);
unsigned int cpu_get_status(void);
void cpu_set_status(unsigned int status);

void cpu_dcache_wb_inv_all(void);
void cpu_icache_inv_all(void);

#ifdef __cplusplus
}
#endif //__cplusplus
