#include "uart.h"

#define psp_uart_sts_BSY  0x80
#define psp_uart_sts_TI   0x20
#define psp_uart_sts_RE   0x10
#define psp_uart_sts_BIT3 0x08

struct psp_uart_hw
{
    volatile unsigned int txd;  // +00 : txd
    volatile unsigned int r04;  // +04 
    volatile unsigned int r08;  // +08 : 
    volatile unsigned int r0c;  // +0c ? 0x0000
    volatile unsigned int r10;  // +10 ? 0x0000
    volatile unsigned int r14;  // +14 ? 0x0000
    volatile unsigned int sts;  // +18 : status */
    volatile unsigned int r1c;  // +1c : unk , 0x1ff */
    volatile unsigned int r20;  // +20 ? 0x0000
    volatile unsigned int brgh; // +24 :(96000000 / bps) >> 6
    volatile unsigned int brgl; // +28 :(96000000 / bps) & 0x3f
    volatile unsigned int r2c;  // +2c : unknown , 0x70 = enable , 0x60 = stop?
    volatile unsigned int r30;  // +30 : unk , 0x0301
    volatile unsigned int r34;  // +34 : 0x00=0x00 , 0x08=0x09,0x16=0x12,0x24=0x12,0x27=0x24 */
    volatile unsigned int r38;  // +38 : unk , uart4=00 , uart=0x10
    volatile unsigned int r3c;  // +3c : unk , uart4=00 , uart=0x28d
    volatile unsigned int r40;  // +40 ? 0
    volatile unsigned int r44;  // +44 :  unk , 0x07ff / 0x0000
    volatile unsigned int r48;  // +48 ? 0
    volatile unsigned int r4c;  // +4c ? 0
    volatile unsigned int r50;  // +50 : unk , set bit14 init,resume = 0x4000
    volatile unsigned int r54;  // +54 ? 0
    volatile unsigned int r58;  // +58 : unk , set bit9  init,resume = 0x0020
    volatile unsigned int r5c;  // +5c ?
    volatile unsigned int r60;  // +60
    volatile unsigned int r64;  // +64
    volatile unsigned int r68;  // +68
    volatile unsigned int r6c;  // +6c
    volatile unsigned int r70;  // +70
    volatile unsigned int r74;  // +74
    volatile unsigned int r78;  // +78 : unk , set bit3 init resume , set bit19 sleep
};

#define UART_IO_BASE (0xBE400000)
#define UART_REGS(x)    ((struct psp_uart_hw *)(UART_IO_BASE + (x)*0x40000))

int uart_init(int port)
{
	struct psp_uart_hw *uart = UART_REGS(port);
	unsigned int brg = 96000000 / 921600; // 115200 baud

    if (port == UART_UART4)
    {
        // uart4 init
        uart->brgh = brg>>6; // 24
        uart->brgl = brg&0x3f; // 28
        uart->r2c  = 0x60;
        uart->r30  = 0x0301;
        uart->r2c  |= 0x10;
        uart->r44  |= 0x07ff;
    }

    else if (port == UART_HPREMOTE)
    {
        // hpremote
        uart->r30 = 0x300; // transmit enable, receive enable
        uart->brgh = brg>>6;   // 24 : 4800bps == 0x0138 : 20000
        uart->brgl = brg&0x3f; // 28 : 4800bps == 0x0020
        uart->r2c = 0x070;
        uart->r34 = 0;
        uart->r38 = 0x10;
        uart->r04 = uart->r04;
        uart->r30 = 0x301;
    }

    else if (port == UART_IRDA)
    {
        // irda
        uart->r30 = 0x300;
        uart->brgh = brg>>6;   // 24
        uart->brgl = brg&0x3f; // 28
        uart->r20 = 0x0d;
        uart->r2c = 0x070;
        uart->r1c = 0x280;
        uart->r34 = 0;
        uart->r38 = 0x50;
        uart->r04 = 0xffff;
        uart->r30 = 0x303;
    }

    return 0;
}

int uart_putc(int port, char c)
{
	struct psp_uart_hw *uart = UART_REGS(port);
	while(uart->sts & psp_uart_sts_TI) __asm("sync"::);
	uart->txd = c;
    return 0;
}

int uart_getc(int port)
{
	struct psp_uart_hw *uart = UART_REGS(port);
    while(uart->sts & psp_uart_sts_RE) __asm("sync"::);
    return (uart->txd)&0xFF;
}

int uart_flush_tx(int port)
{
	struct psp_uart_hw *uart = UART_REGS(port);
	while(uart->sts & psp_uart_sts_TI) __asm("sync"::);
    return 0;
}

int uart_flush_rx(int port)
{
    return 0;
}

void uart_puts(int port, const char *s)
{
    while (*s) {
        uart_putc(port, *s);
        if (*s == '\n') {
            uart_flush_tx(port);
        }
        s++;
    }
}
