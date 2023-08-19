#include "gpio.h"
#include "sysreg.h"

#define REG32(addr) ((volatile unsigned int *)(addr))

void gpio_set_port_mode(int port, enum PortMode mode)
{
    switch (mode)
    {
    case GPIO_MODE_OUTPUT:
        *REG32(0xBE240000) |= (1 << port);
        *REG32(0xbe240040) &= ~(1 << port);
        break;
    case GPIO_MODE_INPUT:
        *REG32(0xBE240000) &= ~(1 << port);
        *REG32(0xbe240040) |= (1 << port);
        break;
    }
}

void gpio_set(int port)
{
    *REG32(0xbe240008) = (1 << port);
}

void gpio_clear(int port)
{
    *REG32(0xbe24000C) = (1 << port);
}

unsigned int gpio_read(void)
{
    return *REG32(0xbe240004);
}

int gpio_query_interrupt(unsigned int port)
{
    return *REG32(0xbe240020) & (1 << port);
}

void gpio_acquire_interrupt(unsigned int port)
{
    *REG32(0xbe240024) = (1 << port);
}

void gpio_unmask_interrupt(unsigned int port)
{
    *REG32(0xbe240010) &= ~(1 << port);
}

void gpio_clear_unk14(unsigned int port)
{
    *REG32(0xbe240014) &= ~(1 << port);
}

void gpio_set_unk18(unsigned int port)
{
    *REG32(0xbe240018) |= (1 << port);
}
