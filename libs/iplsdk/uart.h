#pragma once

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#define UART_UART4      (3)
#define UART_HPREMOTE   (4)
#define UART_IRDA       (5)

int uart_init(int port);
int uart_putc(int port, char c);
int uart_getc(int port);
int uart_flush_tx(int port);
int uart_flush_rx(int port);
void uart_puts(int port, const char *s);

#ifdef __cplusplus
}
#endif //__cplusplus
