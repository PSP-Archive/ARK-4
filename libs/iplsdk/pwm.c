#include "pwm.h"

#include <sysreg.h>

#include <stdint.h>

#define REG32(addr)                 ((volatile uint32_t *)(addr))
#define PWM_MMIO_BASE               (0xBE300000)

#define PWM_UNK00_REG(channel)      REG32(PWM_MMIO_BASE + 0x00 + channel*0x20)
#define UNK00_BIT15                 (1 << 15)

#define PWM_UNK04_REG(channel)      REG32(PWM_MMIO_BASE + 0x04 + channel*0x20)
#define UNK04_BIT0                  (1 << 0)

#define PWM_UNK08_REG(channel)      REG32(PWM_MMIO_BASE + 0x08 + channel*0x20)
#define PWM_UNK0C_REG(channel)      REG32(PWM_MMIO_BASE + 0x0C + channel*0x20)
#define PWM_UNK10_REG(channel)      REG32(PWM_MMIO_BASE + 0x10 + channel*0x20)
#define PWM_UNK14_REG(channel)      REG32(PWM_MMIO_BASE + 0x14 + channel*0x20)

#define PWM_MAX_CHANNEL             (3)

uint8_t g_channel_status[PWM_MAX_CHANNEL];

int pwm_query_channel(uint32_t channel, uint16_t *out1, uint16_t *out2, uint32_t *out3)
{
    if (channel >= PWM_MAX_CHANNEL) {
        return -1;
    }

    if (out1) {
        *out1 = *PWM_UNK10_REG(channel);
    }

    if (out2) {
        *out2 = *PWM_UNK14_REG(channel);
    }

    if (out3) {
        *out3 = *PWM_UNK0C_REG(channel);
    }

    int res = g_channel_status[channel];

    if (res == 2) {
        if ((*PWM_UNK04_REG(channel) & UNK04_BIT0) == 0) {
            g_channel_status[channel] = 0;
        }

        res = 0;
    }

    return res;
}

int pwm_disable_channel(uint32_t channel)
{
    if (channel >= PWM_MAX_CHANNEL) {
        return -1;
    }

    uint32_t val = g_channel_status[channel];

    if (val == 2 && (*PWM_UNK04_REG(channel) & UNK04_BIT0) == 0) {
        g_channel_status[channel] = 0;
        val = 0;
    }

    int res = 0;

    if (val) {
        *PWM_UNK04_REG(channel) = 0;
        g_channel_status[channel] = 2;
        res = val;

        if ((*PWM_UNK04_REG(channel) & UNK04_BIT0) == 0) {
            g_channel_status[channel] = 0;
        }
    }

    return res;
}

int pwm_enable_channel(uint32_t channel, uint32_t unk, uint32_t unk2, uint32_t unk3)
{
    if (channel >= PWM_MAX_CHANNEL) {
        return -1;
    }

    while (*PWM_UNK00_REG(channel) & UNK00_BIT15);

    
    uint32_t val = g_channel_status[channel];

    if (val == 2) {
        if (*PWM_UNK04_REG(channel) & UNK04_BIT0) {
            return -1;
        }

        val = 0;
    }

    g_channel_status[channel] = 1;
    *PWM_UNK00_REG(channel) = unk & 0x3F;
    *PWM_UNK10_REG(channel) = unk2 & 0xFFFF;
    *PWM_UNK14_REG(channel) = unk3 & 0xFFFF;
    *PWM_UNK08_REG(channel) = 1;
    *PWM_UNK04_REG(channel) = 1;
    return val;
}

void pwm_init(void)
{
    sysreg_clk2_enable(CLK2_PWM);
    sysreg_io_enable(IO_PWM);

    for (int i = 0; i < PWM_MAX_CHANNEL; ++i) {
        g_channel_status[i] = (*PWM_UNK04_REG(i) & UNK04_BIT0) ? 1 : 0;
    }
}
