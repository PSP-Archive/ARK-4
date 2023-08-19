#include "spi.h"
#include "pl022.h"

#include <stdint.h>

#define SPI_IO_BASE             (0xBE580000)
#define DEV_TO_PL022_MMIO(x)    (SPI_IO_BASE + (uintptr_t)x * 0x1000)

void spi_init(enum SpiDevice device)
{
    // by default only support the standard configuration for syscon.
    // if for whatever reason we want to support more SPI configurations
    // then we can extend this functionality
    uintptr_t pl022_mmio = DEV_TO_PL022_MMIO(device);

    // enable SPH, SPO, Motorola framing and 16 bit transfers
    pl022_sspcr0_write(pl022_mmio, CR0_SPH | CR0_SPO | CR0_FRF_MOTOROLA | CR0_DSS_16BIT);

    // syscon operates as master for SPI communications so configure this
    // spi device to operate as slave
    pl022_sspcr1_write(pl022_mmio, CR1_MS);
}

int spi_is_data_available(enum SpiDevice device)
{
    uintptr_t pl022_mmio = DEV_TO_PL022_MMIO(device);
    return (pl022_sspsr_read(pl022_mmio) & SR_RNE) != 0;
}

uint16_t spi_read(enum SpiDevice device)
{
    uintptr_t pl022_mmio = DEV_TO_PL022_MMIO(device);
    return pl022_sspdr_read(pl022_mmio);
}

void spi_write(enum SpiDevice device, uint16_t data)
{
    uintptr_t pl022_mmio = DEV_TO_PL022_MMIO(device);
    return pl022_sspdr_write(pl022_mmio, data);
}

void spi_clear_interrupts(enum SpiDevice device)
{
    uintptr_t pl022_mmio = DEV_TO_PL022_MMIO(device);
    pl022_sspicr_write(pl022_mmio, ICR_RTIC);
}

void spi_enable_ssp(enum SpiDevice device)
{
    uintptr_t pl022_mmio = DEV_TO_PL022_MMIO(device);
    pl022_sspcr1_write(pl022_mmio, pl022_sspcr1_read(pl022_mmio) | CR1_SSE);
}
void spi_disable_ssp(enum SpiDevice device)
{
    uintptr_t pl022_mmio = DEV_TO_PL022_MMIO(device);
    pl022_sspcr1_write(pl022_mmio, pl022_sspcr1_read(pl022_mmio) & (~CR1_SSE));
}