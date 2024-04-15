#pragma once

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

enum InterruptType
{
    IRQ_ALL_UART = 0,
    IRQ_ALL_SPI,
    IRQ_ALL_TIMERS,
    IRQ_ALL_USB,
    IRQ_GPIO,
    IRQ_ATA,
    IRQ_UMDMAN,
    IRQ_MS,
    IRQ_WLAN,
    IRQ_MG,
    IRQ_AUDIO1,
    IRQ_AUDIO2,
    IRQ_I2C,
    IRQ_KEY,
    IRQ_IRDA,
    IRQ_SYSTIMER0,
    IRQ_SYSTIMER1,
    IRQ_SYSTIMER2,
    IRQ_SYSTIMER3,
    IRQ_THREAD0,
    IRQ_NAND,
    IRQ_DMACPLUS,
    IRQ_DMA0,
    IRQ_DMA1,
    IRQ_KIRK,
    IRQ_GE,
    IRQ_USB,
    IRQ_EFLASH_ATA,
    IRQ_EFLASH_DMA,
    IRQ_UNK29,
    IRQ_VSYNC,
    IRQ_MEDIAENGINE,
    IRQ_UART1,
    IRQ_UART2,
    IRQ_UART3,
    IRQ_UART4,
    IRQ_UART5,
    IRQ_UART6,
    IRQ_UNK38,
    IRQ_UNK39,
    IRQ_SPI1,
    IRQ_SPI2,
    IRQ_SPI3,
    IRQ_SPI4,
    IRQ_SPI5,
    IRQ_SPI6,
    IRQ_UNK46,
    IRQ_UNK47,
    IRQ_APB_TIMER0,
    IRQ_APB_TIMER1,
    IRQ_APB_TIMER2,
    IRQ_APB_TIMER3,
    IRQ_UNK52,
    IRQ_UNK53,
    IRQ_UNK54,
    IRQ_UNK55,
    IRQ_USB_RESUME,
    IRQ_USB_READY,
    IRQ_USB_ATTACH,
    IRQ_USB_DETACH,
    IRQ_MS_INSERT0,
    IRQ_MS_INSERT1,
    IRQ_UNK_WLAN0,
    IRQ_UNK_WLAN1,
    IRQ_COUNT,
    IRQ_MAXIMUM_COUNT
};

enum IrqHandleStatus
{
    IRQ_HANDLE_NO_RESCHEDULE,
    IRQ_HANDLE_RESCHEDULE,
};

typedef void (* RescheduleHookFunction)(void);
typedef enum IrqHandleStatus (* IrqHandlerFunction)(void);

void interrupt_init(void);
void interrupt_enable(enum InterruptType interrupt);
void interrupt_set_handler(enum InterruptType type, IrqHandlerFunction handler);

int interrupt_occured(enum InterruptType interrupt);
void interrupt_clear(enum InterruptType interrupt);

void interrupt_set_reschedule_hook(RescheduleHookFunction function);

#ifdef __cplusplus
}
#endif //__cplusplus
