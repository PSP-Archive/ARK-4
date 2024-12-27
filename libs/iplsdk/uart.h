#pragma once

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

enum UartDevice
{
    UART_UART1 = 0,
    UART_UART2,
    UART_UART3,
    UART_UART4,
    UART_HPREMOTE,
    UART_IRDA
};

void uart_init(enum UartDevice device, unsigned int baudrate);
void uart_putc(enum UartDevice device, char c);
int uart_getc(enum UartDevice device);
void uart_puts(enum UartDevice device, const char *s);
void uart_flush_tx(enum UartDevice device);

#ifdef __cplusplus
}
#endif //__cplusplus
