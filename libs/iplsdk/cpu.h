#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#define __STRINGIFY(x)                      #x
#define __XSTRINGIFY(x)                     __STRINGIFY(x)

#define CPU_DATA_CREATE_DIRTY_EXCLUSIVE     (0x18)
#define CPU_DATA_HIT_INVALIDATE             (0x19)
#define CPU_DATA_HIT_WRITEBACK_INVALIDATE   (0x1B)
#define CPU_DCACHE_LINE_SIZE                (64)

#define CPU_SEGMENT_MASK                    (0xE0000000)
#define CPU_KUSEG_VADDR                     (0x00000000)
#define CPU_KSEG0_VADDR                     (0x80000000)
#define CPU_KSSEG_VADDR                     (0xC0000000)
#define CPU_KSEG3_VADDR                     (0xE0000000)

static inline uint32_t cpu_bitrev(uint32_t x)
{
    uint32_t res;
    __asm("bitrev %0, %1" : "=r" (res) : "r"(x));
    return res;
}

static inline uint32_t cpu_clz(uint32_t x)
{
    uint32_t res;
    __asm("clz %0, %1" : "=r" (res) : "r"(x));
    return res;
}

static inline void cpu_sync(void)
{
    __asm("sync");
}

void cpu_enable_interrupts(void);
unsigned int cpu_suspend_interrupts(void);
void cpu_resume_interrupts(unsigned int mask);
void cpu_resume_interrupts_with_sync(unsigned int mask);
unsigned int cpu_get_status(void);
void cpu_set_status(unsigned int status);

void cpu_dcache_wb_all(void);
void cpu_dcache_inv_all(void);
void cpu_dcache_wb_inv_all(void);
void cpu_icache_inv_all(void);

static inline void cpu_dcache_hit_inv_line(const volatile void *address)
{
    __asm("cache " __XSTRINGIFY(CPU_DATA_HIT_INVALIDATE) ", 0(%0)" :: "r"(address));
}

static inline void cpu_dcache_create_dirty_exl_line(const volatile void *address)
{
    __asm("cache " __XSTRINGIFY(CPU_DATA_CREATE_DIRTY_EXCLUSIVE) ", 0(%0)" :: "r"(address));
}

static inline void cpu_dcache_hit_wb_inv_line(const volatile void *address)
{
    __asm("cache " __XSTRINGIFY(CPU_DATA_HIT_WRITEBACK_INVALIDATE) ", 0(%0)" :: "r"(address));
}

static inline int cpu_vaddr_is_kuseg(const void *vaddr)
{
    return (((uintptr_t)vaddr) & CPU_SEGMENT_MASK) == CPU_KUSEG_VADDR;
}

static inline int cpu_vaddr_is_kseg0(const void *vaddr)
{
    return (((uintptr_t)vaddr) & CPU_SEGMENT_MASK) == CPU_KSEG0_VADDR;
}

static inline int cpu_vaddr_is_ksseg(const void *vaddr)
{
    return (((uintptr_t)vaddr) & CPU_SEGMENT_MASK) == CPU_KSSEG_VADDR;
}

static inline int cpu_vaddr_is_kseg3(const void *vaddr)
{
    return (((uintptr_t)vaddr) & CPU_SEGMENT_MASK) == CPU_KSEG3_VADDR;
}

#ifdef __cplusplus
}
#endif //__cplusplus
