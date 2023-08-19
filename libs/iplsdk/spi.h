#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

enum SpiDevice
{
    SPI_SYSCON
};

void spi_init(enum SpiDevice device);
int spi_is_data_available(enum SpiDevice device);
uint16_t spi_read(enum SpiDevice device);
void spi_write(enum SpiDevice device, uint16_t data);
void spi_clear_interrupts(enum SpiDevice device);
void spi_enable_ssp(enum SpiDevice device);
void spi_disable_ssp(enum SpiDevice device);

#ifdef __cplusplus
}
#endif //__cplusplus
