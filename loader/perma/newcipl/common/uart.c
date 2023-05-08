#include <stdint.h>
#include "sysreg.h"

// PrimeCell UART (PL011)

#define USE_HP

#define UART_IO_BASE	0xBE400000

#define DBG_UART4_BASE	(UART_IO_BASE + 3 * 0x40000)
#define HP_REMOTE_BASE	(UART_IO_BASE + 4 * 0x40000)
#define IRDA_BASE 	(UART_IO_BASE + 5 * 0x40000)

#define UART_DR_REG		0x00
#define UART_RSR_REG		0x04 // or UART_ECR_REG
#define UART_FR_REG		0x18
#define 	UART_FR_CTS		(1 << 0)
#define 	UART_FR_DSR		(1 << 1)
#define 	UART_FR_DCD		(1 << 2)
#define 	UART_FR_BUSY		(1 << 3)
#define 	UART_FR_RXFE		(1 << 4)
#define 	UART_FR_TXFF		(1 << 5)
#define 	UART_FR_RXFF		(1 << 6)
#define 	UART_FR_TXFE		(1 << 7)
#define 	UART_FR_RI		(1 << 8)
#define UART_ILPR_REG		0x20
#define UART_IBRD_REG		0x24
#define UART_FBRD_REG		0x28
#define UART_LCR_H_REG		0x2C
#define 	UART_LCR_H_BRK		(1 << 0)
#define 	UART_LCR_H_PEN		(1 << 1)
#define 	UART_LCR_H_EPS		(1 << 2)
#define 	UART_LCR_H_STP2	(1 << 3)
#define 	UART_LCR_H_FEN		(1 << 4)
#define 	UART_LCR_H_WLEN_MASK	((1 << 5) | (1 << 6))
#define 	UART_LCR_H_WLEN_5BIT	0
#define 	UART_LCR_H_WLEN_6BIT	(1 << 5) // TODO: Check
#define 	UART_LCR_H_WLEN_7BIT	(1 << 6) // TODO: Check
#define 	UART_LCR_H_WLEN_8BIT	((1 << 5) | (1 << 6))
#define 	UART_LCR_H_SPS		(1 << 7)
#define UART_CR_REG		0x30
#define 	UART_CR_UARTEN		(1 << 0)
#define 	UART_CR_SIREN		(1 << 1)
#define 	UART_CR_SIRLP		(1 << 2)
#define 	UART_CR_LBE		(1 << 7)
#define 	UART_CR_TXE		(1 << 8)
#define 	UART_CR_RXE		(1 << 9)
#define 	UART_CR_DTR		(1 << 10)
#define 	UART_CR_RTS		(1 << 11)
#define 	UART_CR_Out1		(1 << 12)
#define 	UART_CR_Out2		(1 << 13)
#define 	UART_CR_RTSEn		(1 << 14)
#define 	UART_CR_CTSEn		(1 << 15)
#define UART_IFLS_REG		0x34
#define UART_IMSC_REG		0x38
#define UART_RIS_REG		0x3C
#define UART_MIS_REG		0x40
#define UART_ICR_REG		0x44
#define 	UART_ICR_RIMIC		(1 << 0)
#define 	UART_ICR_CTSMIC	(1 << 1)
#define 	UART_ICR_DCDMIC	(1 << 2)
#define 	UART_ICR_DSRMIC	(1 << 3)
#define 	UART_ICR_RXIC		(1 << 4)
#define 	UART_ICR_TXIC		(1 << 5)
#define 	UART_ICR_RTIC		(1 << 6)
#define 	UART_ICR_FEIC		(1 << 7)
#define 	UART_ICR_PEIC		(1 << 8)
#define 	UART_ICR_BEIC		(1 << 9)
#define 	UART_ICR_OEIC		(1 << 10)
#define UART_DMACR_REG		0x48

#ifdef USE_HP
#define DEBUG_BASE HP_REMOTE_BASE
#else
#define DEBUG_BASE DBG_UART4_BASE
#endif

void uart_init()
{
	SysregBusclk((1 << 14), 1);
#ifdef USE_HP
	sceSysregSpiClkEnable(10);
	SYSREG_IO_ENABLE_REG |= (1 << 20); // UART5
#else
	sceSysregSpiClkEnable(9);
	SYSREG_IO_ENABLE_REG |= (1 << 19); // UART4
#endif

	uint32_t val = 96000000 / 115200;

	*(vu32 *)(DEBUG_BASE + UART_IBRD_REG) = val >> 6;
	*(vu32 *)(DEBUG_BASE + UART_FBRD_REG) = val & 0x3F;
	
	*(vu32 *)(DEBUG_BASE + UART_LCR_H_REG) = UART_LCR_H_WLEN_8BIT;
	*(vu32 *)(DEBUG_BASE + UART_CR_REG) |= UART_CR_RXE | UART_CR_TXE | UART_CR_UARTEN;
	
	*(vu32 *)(DEBUG_BASE + UART_LCR_H_REG) |= UART_LCR_H_FEN;
	*(vu32 *)(DEBUG_BASE + UART_IFLS_REG) = 0;

	*(vu32 *)(DEBUG_BASE + UART_ICR_REG) |= UART_ICR_RIMIC | UART_ICR_CTSMIC | UART_ICR_DCDMIC | UART_ICR_DSRMIC | UART_ICR_RXIC | UART_ICR_TXIC | UART_ICR_RTIC | UART_ICR_FEIC | UART_ICR_PEIC | UART_ICR_BEIC | UART_ICR_OEIC;
}

char _getchar()
{
	while ((*(vu32 *)(DEBUG_BASE + UART_FR_REG) & UART_FR_RXFE) != 0);

	return *(vu32 *)(DEBUG_BASE + UART_DR_REG);
}

void _putchar(char c)
{
	//if (c == '\n')
	//	_putchar('\r');

	while ((*(vu32 *)(DEBUG_BASE + UART_FR_REG) & UART_FR_TXFF) != 0);

	*(vu32 *)(DEBUG_BASE + UART_DR_REG) = c;
}
