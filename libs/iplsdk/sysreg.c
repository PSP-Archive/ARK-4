#include "sysreg.h"
#include "cpu.h"

#include <stdint.h>

#define REG32(addr) ((volatile unsigned int *)(addr))

void sysreg_clock_enable_uart_bus(void)
{
    *REG32(0xbc100050) |= 0x4000;
}

void sysreg_clock_enable_uart(int port)
{
    *REG32(0xbc100058) |= (0x40 << port);
}

void sysreg_io_enable_uart(int port)
{
    *REG32(0xbc100078) |= (0x00010000 << port);
}

void sysreg_clock_enable_apb_bus(void)
{
    *REG32(0xbc100050) |= 0x4000;
}

void sysreg_clock_enable_emc_sm_bus(void)
{
    *REG32(0xbc100050) |= 0x2000;
}

void sysreg_io_enable_emc_sm(void)
{
    *REG32(0xbc100078) |= 2;
}

void sysreg_clock_enable_emc_ddr_bus(void)
{
    *REG32(0xbc100050) |= 0x1000;
}

void sysreg_io_enable_gpio(void)
{
    *REG32(0xbc100058) |= 0x800000;
}

void sysreg_io_enable_gpio_port(int port)
{
    *REG32(0xbc10007c) |= (1 << port);
}

void sysreg_clock_enable_i2c(void)
{
    *REG32(0xbc100058) |= 0x20;
}

void sysreg_io_enable_i2c(void)
{
    *REG32(0xbc100078) |= 0x200;
}

void sysreg_clock_enable_pwm(void)
{
    *REG32(0xbc100058) |= 0x8;
}

void sysreg_io_enable_pwm(void)
{
    *REG32(0xbc100078) |= 0x2000;
}

void sysreg_clock_enable_lcdc(void)
{
    *REG32(0xbc100058) |= 0x4;
}

void sysreg_io_enable_lcdc(void)
{
    *REG32(0xbc100078) |= 0x40;
}

void sysreg_clock_enable_usb_bus(void)
{
    *REG32(0xbc100050) |= 0x200;
}

void sysreg_io_enable_usb(void)
{
    *REG32(0xbc100078) |= 0x4;
}

void sysreg_io_enable_spi(unsigned int port)
{
    *REG32(0xbc100078) |= (0x01000000 << port);
}

void sysreg_clock_enable_spi(unsigned int port)
{
    *REG32(0xbc100058) |= (1 << port);
}

void sysreg_clock_select_spi(unsigned int port, unsigned int clk)
{
    unsigned int clear_mask = ~(0b111 << (port * 4));
    unsigned int set_bits = (clk << (port * 4));

    unsigned int cur_bits = *REG32(0xbc100064);
    cur_bits &= clear_mask;
    cur_bits |= set_bits;
    *REG32(0xbc100064) = cur_bits;
}

enum ResetDevice
{
    SC = 1,

    KIRK = 10
};

#define SYSREG_CONFIG_REG   (REG32(0xBC100040))
#define SYSREG_RESET_REG    (REG32(0xBC10004C))
#define SYSREG_BUSCLK_REG   (REG32(0xBC100050))
#define SYSREG_CLK1_REG     (REG32(0xBC100054))
#define SYSREG_CLK2_REG     (REG32(0xBC100058))
#define SYSREG_IOEN_REG     (REG32(0xBC100078))

void sysreg_reset_enable(uint32_t devices)
{
    *SYSREG_RESET_REG |= devices;
}

void sysreg_reset_disable(uint32_t devices)
{
    *SYSREG_RESET_REG &= ~devices;
}

void sysreg_busclk_enable(uint32_t devices)
{
    *SYSREG_BUSCLK_REG |= devices;
}

void sysreg_busclk_disable(uint32_t devices)
{
    *SYSREG_BUSCLK_REG &= ~devices;
}

void sysreg_clk1_enable(uint32_t devices)
{
    *SYSREG_CLK1_REG |= devices;
}

void sysreg_clk1_disable(uint32_t devices)
{
    *SYSREG_CLK1_REG &= ~devices;
}

void sysreg_clk2_enable(uint32_t devices)
{
    *SYSREG_CLK2_REG |= devices;
}

void sysreg_clk2_disable(uint32_t devices)
{
    *SYSREG_CLK2_REG &= ~devices;
}

void sysreg_io_enable(uint32_t devices)
{
    *SYSREG_IOEN_REG |= devices;
}

void sysreg_io_disable(uint32_t devices)
{
    *SYSREG_IOEN_REG &= ~devices;
}

uint32_t sysreg_get_tachyon_version(void)
{
    static uint32_t s_tachyon_version = -1;

    if (s_tachyon_version == -1) {
        uint32_t mask = cpu_suspend_interrupts();
        uint32_t cfg = *SYSREG_CONFIG_REG;

        if ((cfg & 0xFF000000) != 0) {
            s_tachyon_version = (cfg & 0xFF000000) >> 8;
        }
        else {
            s_tachyon_version = 0x100000;
        }

        cpu_resume_interrupts(mask);
    }

    return s_tachyon_version;
}
