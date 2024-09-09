#include "spi.h"
#include "pl022.h"

#include <stddef.h>
#include <stdint.h>

#define SPI_IO_BASE             (0xBE580000)
#define DEV_TO_PL022_MMIO(x)    (SPI_IO_BASE + (uintptr_t)x * 0x40000)

typedef struct
{
    uint32_t mmio;
    uint32_t sspcr0;
    uint32_t sspcr1;
    uint32_t sspcpsr;
} SpiDeviceConfig;

static const SpiDeviceConfig *dev_config(enum SpiDevice device)
{
    switch (device) {
        case SPI_SYSCON:
        {
            static const SpiDeviceConfig s_syscon = {
                .mmio = DEV_TO_PL022_MMIO(0),
                .sspcr0 = CR0_SPH | CR0_SPO | CR0_FRF_MOTOROLA | CR0_DSS_16BIT,
                .sspcr1 = CR1_MS
            };

            return &s_syscon;
        }
        case SPI_HIBARI:
        {
            static const SpiDeviceConfig s_hibari = {
                .mmio = DEV_TO_PL022_MMIO(1),
                .sspcr0 = CR0_FRF_MOTOROLA | CR0_DSS_16BIT,
                .sspcpsr = 2
            };

            return &s_hibari;
        }
    }

    // TODO: some warning, or error? this shouldn't be reachable
    return NULL;
}

void spi_init(enum SpiDevice device)
{
    const SpiDeviceConfig *config = dev_config(device);

    // apply the device configuration for initialisation
    pl022_sspcr0_write(config->mmio, config->sspcr0);
    pl022_sspcr1_write(config->mmio, config->sspcr1);

    // only apply the prescale for settings that have it
    if (config->sspcpsr) {
        pl022_sspcpsr_write(config->mmio, config->sspcpsr);
    }
}

int spi_is_transmit_fifo_full(enum SpiDevice device)
{
    const SpiDeviceConfig *config = dev_config(device);
    return (pl022_sspsr_read(config->mmio) & SR_TNF) == 0;
}

int spi_is_data_available(enum SpiDevice device)
{
    const SpiDeviceConfig *config = dev_config(device);
    return (pl022_sspsr_read(config->mmio) & SR_RNE) != 0;
}

int spi_is_busy(enum SpiDevice device)
{
    const SpiDeviceConfig *config = dev_config(device);
    return (pl022_sspsr_read(config->mmio) & SR_BSY) != 0;
}

uint16_t spi_read(enum SpiDevice device)
{
    const SpiDeviceConfig *config = dev_config(device);
    return pl022_sspdr_read(config->mmio);
}

void spi_write(enum SpiDevice device, uint16_t data)
{
    const SpiDeviceConfig *config = dev_config(device);
    return pl022_sspdr_write(config->mmio, data);
}

void spi_clear_interrupts(enum SpiDevice device)
{
    const SpiDeviceConfig *config = dev_config(device);
    pl022_sspicr_write(config->mmio, ICR_RTIC);
}

void spi_enable_ssp(enum SpiDevice device)
{
    const SpiDeviceConfig *config = dev_config(device);
    pl022_sspcr1_write(config->mmio, pl022_sspcr1_read(config->mmio) | CR1_SSE);
}

void spi_disable_ssp(enum SpiDevice device)
{
    const SpiDeviceConfig *config = dev_config(device);
    pl022_sspcr1_write(config->mmio, pl022_sspcr1_read(config->mmio) & (~CR1_SSE));
}