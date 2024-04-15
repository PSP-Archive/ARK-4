#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

// MMIO
#define REG32(addr) ((volatile uintptr_t *)(addr))

#define PL011_UARTDR_OFS    (0x000)
#define UARTDR_OE           (1 << 11)
#define UARTDR_BE           (1 << 10)
#define UARTDR_PE           (1 << 9)
#define UARTDR_FE           (1 << 8)
#define UARTDR_DATA_MASK    (0xFF)

static uint32_t pl011_uartdr_read(uintptr_t base)
{
    return *REG32(base + PL011_UARTDR_OFS);
}

static void pl011_uartdr_write(uintptr_t base, uint32_t dr)
{
    *REG32(base + PL011_UARTDR_OFS) = dr;
}

#define PL011_UARTRSR_OFS   (0x004)
#define UARTRSR_OE          (1 << 3)
#define UARTRSR_BE          (1 << 2)
#define UARTRSR_PE          (1 << 1)
#define UARTRSR_FE          (1 << 0)

static uint32_t pl011_uartrsr_read(uintptr_t base)
{
    return *REG32(base + PL011_UARTRSR_OFS);
}

#define PL011_UARTECR_OFS   (0x004)

static void pl011_uartecr_write(uintptr_t base)
{
    *REG32(base + PL011_UARTECR_OFS) = 0;
}

#define PL011_UARTFR_OFS    (0x018)
#define UARTFR_RI           (1 << 8)
#define UARTFR_TXFE         (1 << 7)
#define UARTFR_RXFF         (1 << 6)
#define UARTFR_TXFF         (1 << 5)
#define UARTFR_RXFE         (1 << 4)
#define UARTFR_BUSY         (1 << 3)
#define UARTFR_DCD          (1 << 2)
#define UARTFR_DSR          (1 << 1)
#define UARTFR_CTS          (1 << 0)

static uint32_t pl011_uartfr_read(uintptr_t base)
{
    return *REG32(base + PL011_UARTFR_OFS);
}

#define PL011_UARTILPR_OFS  (0x020)

static uint8_t pl011_uartilpr_read(uintptr_t base)
{
    return *REG32(base + PL011_UARTILPR_OFS);
}

static void pl011_uartilpr_write(uintptr_t base, uint8_t ilpdvsr)
{
    *REG32(base + PL011_UARTILPR_OFS) = ilpdvsr;
}

#define PL011_UARTIBRD_OFS  (0x024)

static uint16_t pl011_uartibrd_read(uintptr_t base)
{
    return *REG32(base + PL011_UARTIBRD_OFS);
}

static void pl011_uartibrd_write(uintptr_t base, uint16_t bauddivint)
{
    *REG32(base + PL011_UARTIBRD_OFS) = bauddivint;
}

#define PL011_UARTFBRD_OFS  (0x028)

static uint8_t pl011_uartfbrd_read(uintptr_t base)
{
    return *REG32(base + PL011_UARTFBRD_OFS);
}

static void pl011_uartfbrd_write(uintptr_t base, uint8_t bauddivfrac)
{
    *REG32(base + PL011_UARTFBRD_OFS) = bauddivfrac;
}

#define PL011_UARTLCR_H_OFS (0x02C)
#define UARTLCR_H_SPS       (1 << 7)
#define UARTLCR_H_WLEN_8BIT (0b11 << 5)
#define UARTLCR_H_WLEN_7BIT (0b10 << 5)
#define UARTLCR_H_WLEN_6BIT (0b01 << 5)
#define UARTLCR_H_WLEN_5BIT (0b00 << 5)
#define UARTLCR_H_WLEN_MASK (0b11 << 5)
#define UARTLCR_H_FEN       (1 << 4)
#define UARTLCR_H_STP2      (1 << 3)
#define UARTLCR_H_EPS       (1 << 2)
#define UARTLCR_H_PEN       (1 << 1)
#define UARTLCR_H_BRK       (1 << 0)

static uint32_t pl011_uartlcr_h_read(uintptr_t base)
{
    return *REG32(base + PL011_UARTLCR_H_OFS);
}

static void pl011_uartlcr_h_write(uintptr_t base, uint32_t lcr_h)
{
    *REG32(base + PL011_UARTLCR_H_OFS) = lcr_h;
}

#define PL011_UARTCR_OFS    (0x30)
#define UARTCR_CTSEn        (1 << 15)
#define UARTCR_RTSEn        (1 << 14)
#define UARTCR_Out2         (1 << 13)
#define UARTCR_Out1         (1 << 12)
#define UARTCR_RTS          (1 << 11)
#define UARTCR_DTR          (1 << 10)
#define UARTCR_RXE          (1 << 9)
#define UARTCR_TXE          (1 << 8)
#define UARTCR_LBE          (1 << 7)
#define UARTCR_SIRLP        (1 << 2)
#define UARTCR_SIREN        (1 << 1)
#define UARTCR_UARTEN       (1 << 0)

static uint32_t pl011_uartcr_read(uintptr_t base)
{
    return *REG32(base + PL011_UARTCR_OFS);
}

static void pl011_uartcr_write(uintptr_t base, uint32_t cr)
{
    *REG32(base + PL011_UARTCR_OFS) = cr;
}

#define PL011_UARTIFLS_OFS      (0x34)
#define UARTIFLS_RXIFLSEL_MASK  (0b111 << 3)
#define UARTIFLS_TXIFLSEL_MASK  (0b111 << 0)

static uint32_t pl011_uartifls_read(uintptr_t base)
{
    return *REG32(base + PL011_UARTIFLS_OFS);
}

static void pl011_uartifls_write(uintptr_t base, uint32_t ifls)
{
    *REG32(base + PL011_UARTIFLS_OFS) = ifls;
}

#define PL011_UARTIMSC_OFS  (0x38)
#define UARTIMSC_OEIM       (1 << 10)
#define UARTIMSC_BEIM       (1 << 9)
#define UARTIMSC_PEIM       (1 << 8)
#define UARTIMSC_FEIM       (1 << 7)
#define UARTIMSC_RTIM       (1 << 6)
#define UARTIMSC_TXIM       (1 << 5)
#define UARTIMSC_RXIM       (1 << 4)
#define UARTIMSC_DSRMIM     (1 << 3)
#define UARTIMSC_DCDMIM     (1 << 2)
#define UARTIMSC_CTSMIM     (1 << 1)
#define UARTIMSC_RIMIM      (1 << 0)

static uint32_t pl011_uartimsc_read(uintptr_t base)
{
    return *REG32(base + PL011_UARTIMSC_OFS);
}

static void pl011_uartimsc_write(uintptr_t base, uint32_t imsc)
{
    *REG32(base + PL011_UARTIMSC_OFS) = imsc;
}

#define PL011_UARTRIS_OFS   (0x3C)
#define UARTRIS_OERIS       (1 << 10)
#define UARTRIS_BERIS       (1 << 9)
#define UARTRIS_PERIS       (1 << 8)
#define UARTRIS_FERIS       (1 << 7)
#define UARTRIS_RTRIS       (1 << 6)
#define UARTRIS_TXRIS       (1 << 5)
#define UARTRIS_RXRIS       (1 << 4)
#define UARTRIS_DSRRMIS     (1 << 3)
#define UARTRIS_DCDRMIS     (1 << 2)
#define UARTRIS_CTSRMIS     (1 << 1)
#define UARTRIS_RIRMIS      (1 << 0)

static uint32_t pl011_uartris_read(uintptr_t base)
{
    return *REG32(base + PL011_UARTRIS_OFS);
}

#define PL011_UARTMIS_OFS   (0x40)
#define UARTRIS_OEMIS       (1 << 10)
#define UARTRIS_BEMIS       (1 << 9)
#define UARTRIS_PEMIS       (1 << 8)
#define UARTRIS_FEMIS       (1 << 7)
#define UARTRIS_RTMIS       (1 << 6)
#define UARTRIS_TXMIS       (1 << 5)
#define UARTRIS_RXMIS       (1 << 4)
#define UARTRIS_DSMMMIS     (1 << 3)
#define UARTRIS_DCDMMIS     (1 << 2)
#define UARTRIS_CTSMMIS     (1 << 1)
#define UARTRIS_RIMMIS      (1 << 0)

static uint32_t pl011_uartmis_read(uintptr_t base)
{
    return *REG32(base + PL011_UARTMIS_OFS);
}

#define PL011_UARTICR_OFS   (0x44)
#define UARTICR_OEIC        (1 << 10)
#define UARTICR_BEIC        (1 << 9)
#define UARTICR_PEIC        (1 << 8)
#define UARTICR_FEIC        (1 << 7)
#define UARTICR_RTIC        (1 << 6)
#define UARTICR_TXIC        (1 << 5)
#define UARTICR_RXIC        (1 << 4)
#define UARTICR_DSRMIC      (1 << 3)
#define UARTICR_DCDMIC      (1 << 2)
#define UARTICR_CTSMIC      (1 << 1)
#define UARTICR_RIMIC       (1 << 0)
#define UARTICR_ALL         (UARTICR_OEIC | UARTICR_BEIC | UARTICR_PEIC | UARTICR_FEIC | UARTICR_RTIC | UARTICR_TXIC | UARTICR_RXIC | UARTICR_DSRMIC | UARTICR_DCDMIC | UARTICR_CTSMIC | UARTICR_RIMIC)

static void pl011_uarticr_write(uintptr_t base, uint32_t icr)
{
    *REG32(base + PL011_UARTICR_OFS) = icr;
}

#define PL011_UARTDMACR_OFS (0x48)
#define UARTDMACR_DMAONERR  (1 << 2)
#define UARTDMACR_TXDMAE    (1 << 1)
#define UARTDMACR_RXDMAE    (1 << 0)

static uint32_t pl011_uartdmacr_read(uintptr_t base)
{
    return *REG32(base + PL011_UARTDMACR_OFS);
}

static void pl011_uartdmacr_write(uintptr_t base, uint32_t dmacr)
{
    *REG32(base + PL011_UARTDMACR_OFS) = dmacr;
}

#ifdef __cplusplus
}
#endif //__cplusplus
