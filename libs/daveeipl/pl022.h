#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

// MMIO
#define REG32(addr) ((volatile uintptr_t *)(addr))

// SSPCR0 Control Register 0
#define PL022_CR0_OFFSET            (0x000)
#define CR0_SCR                     (1 << 8)
#define CR0_SPH                     (1 << 7)
#define CR0_SPO                     (1 << 6)
#define CR0_FRF_MOTOROLA            (0b00 << 4)
#define CR0_FRF_TEXAS_INSTRUMENTS   (0b01 << 4)
#define CR0_FRF_MICROWIRE           (0b10 << 4)
#define CR0_DSS_16BIT               (15)
#define CR0_DSS_15BIT               (14)
#define CR0_DSS_14BIT               (13)
#define CR0_DSS_13BIT               (12)
#define CR0_DSS_12BIT               (11)
#define CR0_DSS_11BIT               (10)
#define CR0_DSS_10BIT               (9)
#define CR0_DSS_9BIT                (8)
#define CR0_DSS_8BIT                (7)
#define CR0_DSS_7BIT                (6)
#define CR0_DSS_6BIT                (5)
#define CR0_DSS_5BIT                (4)
#define CR0_DSS_4BIT                (3)

static void pl022_sspcr0_write(uintptr_t base, uint32_t cr0)
{
    *REG32(base + PL022_CR0_OFFSET) = cr0;
}

// SSPCR1 Control Register 1
#define PL022_CR1_OFFSET            (0x004)
#define CR1_SOD                     (1 << 3)
#define CR1_MS                      (1 << 2)
#define CR1_SSE                     (1 << 1)
#define CR1_LBM                     (1 << 0)

static uint32_t pl022_sspcr1_read(uintptr_t base)
{
    return *REG32(base + PL022_CR1_OFFSET);
}

static void pl022_sspcr1_write(uintptr_t base, uint32_t cr1)
{
    *REG32(base + PL022_CR1_OFFSET) = cr1;
}

// SSPDR Data Register
#define PL022_DR_OFFSET             (0x008)

static uint16_t pl022_sspdr_read(uintptr_t base)
{
    return *REG32(base + PL022_DR_OFFSET);
}

static void pl022_sspdr_write(uintptr_t base, uint16_t data)
{
    *REG32(base + PL022_DR_OFFSET) = data;
}

// SSPSR Status Register
#define PL022_SR_OFFSET             (0x00C)
#define SR_BSY                      (1 << 4)
#define SR_RFF                      (1 << 3)
#define SR_RNE                      (1 << 2)
#define SR_TNF                      (1 << 1)
#define SR_TFE                      (1 << 0)

static uint32_t pl022_sspsr_read(uintptr_t base)
{
    return *REG32(base + PL022_SR_OFFSET);
}

// SSPCPSR Clock Prescale Register
#define PL022_CP_OFFSET             (0x010)

static void pl022_sspcpsr_write(uintptr_t base, uint32_t cp)
{
    *REG32(base + PL022_CP_OFFSET) = cp;
}

// SSPICR Interrupt Clear Register
#define PL022_ICR_OFFSET            (0x020)
#define ICR_RTIC                    (1 << 1)
#define ICR_RORIC                   (1 << 0)

static void pl022_sspicr_write(uintptr_t base, uint32_t icr)
{
    *REG32(base + PL022_ICR_OFFSET) = icr;
}

enum Status
{
    TFE,
    TNF,
    RNE,
    RFF,
    BSY
};

// Interrupt Mask Set Clear
enum InterruptMaskSetClear
{
    RORIM,
    RTIM,
    RXIM,
    TXIM
};

// Raw Interrupt Status
#define TXRIS   (3)
#define RXRIS   (2)
#define RTRIS   (1)
#define RORRIS  (0)

// Masked Interrupt Status
#define TXMIS   (3)
#define RXMIS   (2)
#define RTMIS   (1)
#define RORMIS  (0)

// Interrupt Clear
#define RTIC    (1)
#define RORIC   (0)

// DMA Control
#define TXDMAE  (1)
#define RXDMAE  (0)

#ifdef __cplusplus
}
#endif //__cplusplus
