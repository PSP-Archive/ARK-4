#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

// reset device bits
#define RESET_SC        (1 << 1)
#define RESET_ME        (1 << 2)
#define RESET_AW        (1 << 3)
#define RESET_VME       (1 << 4)
#define RESET_AVC       (1 << 5)
#define RESET_USB       (1 << 6)
#define RESET_ATA       (1 << 7)
#define RESET_MSPRO0    (1 << 8)
#define RESET_MSPRO1    (1 << 9)
#define RESET_KIRK      (1 << 10)
#define RESET_ATAHDD    (1 << 11)
#define RESET_USBHOST   (1 << 12)

void sysreg_reset_enable(uint32_t devices);
void sysreg_reset_disable(uint32_t devices);

// bus clock device bits
#define BUSCLK_ME       (1 << 0)
#define BUSCLK_AW_REGA  (1 << 1)
#define BUSCLK_AW_REGB  (1 << 2)
#define BUSCLK_AW_EDRAM (1 << 3)
#define BUSCLK_DMACPLUS (1 << 4)
#define BUSCLK_DMAC0    (1 << 5)
#define BUSCLK_DMAC1    (1 << 6)
#define BUSCLK_KIRK     (1 << 7)
#define BUSCLK_ATA      (1 << 8)
#define BUSCLK_USB      (1 << 9)
#define BUSCLK_MSPRO0   (1 << 10)
#define BUSCLK_MSPRO1   (1 << 11)
#define BUSCLK_EMCDDR   (1 << 12)
#define BUSCLK_EMCSM    (1 << 13)
#define BUSCLK_APB      (1 << 14)
#define BUSCLK_AUDIO0   (1 << 15)
#define BUSCLK_AUDIO1   (1 << 16)
#define BUSCLK_ATAHDD   (1 << 17)
#define BUSCLK_USBHOST  (1 << 18)

void sysreg_busclk_enable(uint32_t devices);
void sysreg_busclk_disable(uint32_t devices);

// device clock bits
#define CLK1_ATA         (1 << 0)
#define CLK1_ATAHDD      (1 << 1)
#define CLK1_USB         (1 << 2)
#define CLK1_MSPRO0      (1 << 8)
#define CLK1_MSPRO1      (1 << 9)
#define CLK1_EMCDDR      (1 << 12)
#define CLK1_USBHOST     (1 << 16)
#define CLK2_SPI0        (1 << 0)
#define CLK2_SPI1        (1 << 1)
#define CLK2_SPI2        (1 << 2)
#define CLK2_SPI3        (1 << 3)
#define CLK2_SPI4        (1 << 4)
#define CLK2_SPI5        (1 << 5)
#define CLK2_UART0       (1 << 6)
#define CLK2_UART1       (1 << 7)
#define CLK2_UART2       (1 << 8)
#define CLK2_UART3       (1 << 9)
#define CLK2_UART4       (1 << 10)
#define CLK2_UART5       (1 << 11)
#define CLK2_APB_TIMER0  (1 << 12)
#define CLK2_APB_TIMER1  (1 << 13)
#define CLK2_APB_TIMER2  (1 << 14)
#define CLK2_APB_TIMER3  (1 << 15)
#define CLK2_AUDIO0      (1 << 16)
#define CLK2_AUDIO1      (1 << 17)
#define CLK2_LCDCTRL     (1 << 18)
#define CLK2_PWM         (1 << 19)
#define CLK2_I2C         (1 << 21)
#define CLK2_SIRCS       (1 << 22)
#define CLK2_GPIO        (1 << 23)
#define CLK2_AUDIOCLKOUT (1 << 24)

void sysreg_clk1_enable(uint32_t devices);
void sysreg_clk1_disable(uint32_t devices);
void sysreg_clk2_enable(uint32_t devices);
void sysreg_clk2_disable(uint32_t devices);

// device IO bits
#define IO_EMCSM        (1 << 1)
#define IO_USB          (1 << 2)
#define IO_ATA          (1 << 3)
#define IO_MSIF0        (1 << 4)
#define IO_MSIF1        (1 << 5)
#define IO_LCDC         (1 << 6)
#define IO_AUDIO0       (1 << 7)
#define IO_AUDIO1       (1 << 8)
#define IO_I2C          (1 << 9)
#define IO_SIRCS        (1 << 10)
#define IO_AUDIOCLKOUT  (1 << 11)
#define IO_UNK_KEYPAD   (1 << 12)
#define IO_PWM          (1 << 13)
#define IO_ATAHDD       (1 << 14)
#define IO_UART0        (1 << 16)
#define IO_UART1        (1 << 17)
#define IO_UART2        (1 << 18)
#define IO_UART3        (1 << 19)
#define IO_UART4        (1 << 20)
#define IO_UART5        (1 << 21)
#define IO_SPI0         (1 << 24)
#define IO_SPI1         (1 << 25)
#define IO_SPI2         (1 << 26)
#define IO_SPI3         (1 << 27)
#define IO_SPI4         (1 << 28)
#define IO_SPI5         (1 << 29)

void sysreg_io_enable(uint32_t devices);
void sysreg_io_disable(uint32_t devices);

// UART
void sysreg_clock_enable_uart_bus(void);
void sysreg_clock_enable_uart(int port);
void sysreg_io_enable_uart(int port);

// APB
void sysreg_clock_enable_apb_bus(void);

// EMC SM
void sysreg_clock_enable_emc_sm_bus(void);
void sysreg_io_enable_emc_sm(void);

// EMC DDR
void sysreg_clock_enable_emc_ddr_bus(void);

// GPIO
void sysreg_io_enable_gpio(void);
void sysreg_io_enable_gpio_port(int port);

// I2C
void sysreg_clock_enable_i2c(void);
void sysreg_io_enable_i2c(void);

// PWM
void sysreg_clock_enable_pwm(void);
void sysreg_io_enable_pwm(void);

// LCDC
void sysreg_clock_enable_lcdc(void);
void sysreg_io_enable_lcdc(void);

// USB
void sysreg_clock_enable_usb_bus(void);
void sysreg_io_enable_usb(void);

// SPI
void sysreg_io_enable_spi(unsigned int port);
void sysreg_clock_enable_spi(unsigned int port);
void sysreg_clock_select_spi(unsigned int port, unsigned int clk);

// KIRK
static inline void sysreg_reset_enable_kirk(void)
{
    sysreg_reset_enable(RESET_KIRK);
}

static inline void sysreg_reset_disable_kirk(void)
{
    sysreg_reset_disable(RESET_KIRK);
}

static inline void sysreg_busclk_enable_kirk(void)
{
    sysreg_busclk_enable(BUSCLK_KIRK);
}

static inline void sysreg_busclk_disable_kirk(void)
{
    sysreg_busclk_disable(BUSCLK_KIRK);
}

uint32_t sysreg_get_tachyon_version(void);

#ifdef __cplusplus
}
#endif //__cplusplus
