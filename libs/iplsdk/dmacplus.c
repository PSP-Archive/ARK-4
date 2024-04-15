#include "dmacplus.h"

#include <sysreg.h>
#include <cpu.h>

#include <stdint.h>
#include <string.h>

#define REG32(addr)                 ((volatile uint32_t *)(addr))
#define DMACPLUS_LCDC_MMIO_BASE     (0xBC800100)

#define LCDC_FRAMEBUFFER_ADDR_REG   (REG32(DMACPLUS_LCDC_MMIO_BASE + 0x00))
#define LCDC_PIXEL_FORMAT_REG       (REG32(DMACPLUS_LCDC_MMIO_BASE + 0x04))

#define LCDC_FRAMEBUFFER_WIDTH_REG  (REG32(DMACPLUS_LCDC_MMIO_BASE + 0x08))
#define LCDC_FRAMEBUFFER_STRIDE_REG (REG32(DMACPLUS_LCDC_MMIO_BASE + 0x0C))
#define LCDC_FRAMEBUFFER_CFG_REG    (REG32(DMACPLUS_LCDC_MMIO_BASE + 0x10))
#define CONFIG_SCANOUT_ENABLED      (1 << 0)
#define CONFIG_UNK_BIT1             (1 << 1)

enum LcdcBaseAddrState
{
    FRAMEBUFFER_NEEDS_INIT = -1,
    FRAMEBUFFER_ASSIGNED = 0,
    FRAMEBUFFER_INTERNAL = 1,
};

typedef struct
{
    uintptr_t framebuffer;
    unsigned int format;
    unsigned int stride;
    unsigned int width;
    enum LcdcBaseAddrState state;
    int enabled;
    int has_error;
} LcdcConfig;

static LcdcConfig g_lcdc_config;
static uint8_t g_empty_framebuffer[0x1000 + 64];
static uintptr_t g_empty_framebuffer_ptr;

int dmacplus_lcdc_set_base_addr(uintptr_t addr)
{
    // require 16 byte alignment
    if (addr % 16) {
        return -1;
    }

    // TODO: some addresses aren't valid. figure out and
    // implement a whitelist

    unsigned int mask = cpu_suspend_interrupts();

    if (!addr) {
        if (g_lcdc_config.state != FRAMEBUFFER_INTERNAL) {
            uint32_t cfg = *LCDC_FRAMEBUFFER_CFG_REG;
            *LCDC_FRAMEBUFFER_CFG_REG = cfg & ~CONFIG_SCANOUT_ENABLED;
            *LCDC_FRAMEBUFFER_ADDR_REG = (uintptr_t)g_empty_framebuffer_ptr & 0x1FFFFFFF;
            *LCDC_FRAMEBUFFER_STRIDE_REG = 0;
            *LCDC_FRAMEBUFFER_CFG_REG = cfg;
            g_lcdc_config.state = FRAMEBUFFER_INTERNAL;
        }
    }
    else {
        // technically there is support for the CPU scratchpad? not in
        // this implementation.
        if (g_lcdc_config.state != FRAMEBUFFER_ASSIGNED) {
            uint32_t cfg = *LCDC_FRAMEBUFFER_CFG_REG;
            *LCDC_FRAMEBUFFER_CFG_REG = cfg & ~CONFIG_SCANOUT_ENABLED;
            *LCDC_FRAMEBUFFER_ADDR_REG = addr & 0x1FFFFFFF;
            *LCDC_FRAMEBUFFER_STRIDE_REG = g_lcdc_config.stride;
            *LCDC_FRAMEBUFFER_CFG_REG = cfg;
            g_lcdc_config.state = FRAMEBUFFER_ASSIGNED;
        }

        else {
            // we can just update the framebuffer address only
            *LCDC_FRAMEBUFFER_ADDR_REG = addr & 0x1FFFFFFF;
        }
    }

    if (g_lcdc_config.has_error && g_lcdc_config.enabled) {
        *LCDC_FRAMEBUFFER_CFG_REG |= CONFIG_SCANOUT_ENABLED;
    }

    g_lcdc_config.framebuffer = addr;
    cpu_resume_interrupts_with_sync(mask);
    return 0;
}

int dmacplus_lcdc_set_format(unsigned int width, unsigned int stride, enum PixelFormat format)
{
    if (width % 8) {
        return -1;
    }

    if (stride % 64) {
        return -2;
    }

    // the format can only be set when the device is disabled
    if (!g_lcdc_config.has_error && g_lcdc_config.enabled) {
        return -3;
    }

    *LCDC_FRAMEBUFFER_WIDTH_REG = width;
    *LCDC_FRAMEBUFFER_STRIDE_REG = stride;
    *LCDC_PIXEL_FORMAT_REG = format;
    g_lcdc_config.width = width;
    g_lcdc_config.stride = stride;
    g_lcdc_config.format = format;
    return 0;
}

void dmacplus_lcdc_enable(void)
{
    unsigned int mask = cpu_suspend_interrupts();

    if (!g_lcdc_config.enabled || g_lcdc_config.has_error) {
        g_lcdc_config.has_error = 0;

        // TODO: check for blacklisted framebuffers...
        if (!g_lcdc_config.framebuffer) {
            *LCDC_FRAMEBUFFER_ADDR_REG = (uintptr_t)g_empty_framebuffer_ptr & 0x1FFFFFFF;
            *LCDC_FRAMEBUFFER_STRIDE_REG = 0;
        }
        else {
            *LCDC_FRAMEBUFFER_ADDR_REG = g_lcdc_config.framebuffer & 0x1FFFFFFF;
            *LCDC_FRAMEBUFFER_STRIDE_REG = g_lcdc_config.stride;
        }

        *LCDC_FRAMEBUFFER_CFG_REG |= CONFIG_SCANOUT_ENABLED;
        g_lcdc_config.enabled = 1;
    }

    cpu_resume_interrupts_with_sync(mask);
}

void dmacplus_lcdc_disable(void)
{
    unsigned int mask = cpu_suspend_interrupts();

    if (g_lcdc_config.enabled) {
        *LCDC_FRAMEBUFFER_CFG_REG &= ~CONFIG_SCANOUT_ENABLED;
        g_lcdc_config.enabled = 0;
    }

    cpu_resume_interrupts_with_sync(mask);
}

void dmacplus_lcdc_init(void)
{
    memset(g_empty_framebuffer, 0x00, sizeof(g_empty_framebuffer));
    g_empty_framebuffer_ptr = (uintptr_t)&g_empty_framebuffer[0];

    while (g_empty_framebuffer_ptr % 64) {
        g_empty_framebuffer_ptr += 1;
    }

    unsigned int mask = cpu_suspend_interrupts();

    g_lcdc_config.state = FRAMEBUFFER_NEEDS_INIT;
    g_lcdc_config.enabled = 0;

    // set the default framebuffer to the one blank one internally
    uintptr_t framebuffer_addr = 0;

    // if the hardware is already configured we can copy the settings
    if (*LCDC_FRAMEBUFFER_CFG_REG & CONFIG_SCANOUT_ENABLED) {
        g_lcdc_config.stride = *LCDC_FRAMEBUFFER_STRIDE_REG;
        g_lcdc_config.width = *LCDC_FRAMEBUFFER_WIDTH_REG;
        g_lcdc_config.format = *LCDC_PIXEL_FORMAT_REG;

        // if there is a stride set then we can clone the framebuffer
        if (g_lcdc_config.stride) {
            framebuffer_addr = *LCDC_FRAMEBUFFER_ADDR_REG;
        }
        
        g_lcdc_config.enabled = 1;
    }

    // apply the framebuffer settings. this functions sets
    // g_lcdc_config.framebuffer for us.
    dmacplus_lcdc_set_base_addr(framebuffer_addr);

    // probably some enable or init?
    *LCDC_FRAMEBUFFER_CFG_REG |= CONFIG_UNK_BIT1;

    cpu_resume_interrupts_with_sync(mask);
    dmacplus_lcdc_enable();
}

void dmacplus_init(void)
{
    sysreg_busclk_enable(BUSCLK_DMACPLUS);
    dmacplus_lcdc_init();
}
