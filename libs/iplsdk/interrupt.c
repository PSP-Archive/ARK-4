#include "interrupt.h"

#include <cpu.h>
#include <exception.h>

#include <stddef.h>
#include <stdint.h>

#define REG32(addr)                         ((volatile uintptr_t *)(addr))
#define INTERRUPT_MMIO_BASE                 (0xBC300000)

#define INTERRUPT_CTRL0_UNMASKED_FLAGS_REG  (REG32(INTERRUPT_MMIO_BASE + 0x00))
#define INTERRUPT_CTRL0_FLAGS_REG           (REG32(INTERRUPT_MMIO_BASE + 0x04))
#define INTERRUPT_CTRL0_MASK_REG            (REG32(INTERRUPT_MMIO_BASE + 0x08))

#define INTERRUPT_CTRL1_UNMASKED_FLAGS_REG  (REG32(INTERRUPT_MMIO_BASE + 0x10))
#define INTERRUPT_CTRL1_FLAGS_REG           (REG32(INTERRUPT_MMIO_BASE + 0x14))
#define INTERRUPT_CTRL1_MASK_REG            (REG32(INTERRUPT_MMIO_BASE + 0x18))

#define INTERRUPT_CTRL2_UNMASKED_FLAGS_REG  (REG32(INTERRUPT_MMIO_BASE + 0x20))
#define INTERRUPT_CTRL2_FLAGS_REG           (REG32(INTERRUPT_MMIO_BASE + 0x24))
#define INTERRUPT_CTRL2_MASK_REG            (REG32(INTERRUPT_MMIO_BASE + 0x28))

#define INTR_MASK(x)                        (1 << (x % 32))

#define ALWAYS_ENABLED_INTERRUPTS           (INTR_MASK(IRQ_ALL_UART) | INTR_MASK(IRQ_ALL_SPI) | INTR_MASK(IRQ_ALL_TIMERS) | INTR_MASK(IRQ_ALL_USB) | INTR_MASK(IRQ_UNK29))

#define NUM_IRQ_HANDLERS                    (16)

#define IP0 (1 << 0)
#define IP1 (1 << 1)
#define IP2 (1 << 2)
#define IP3 (1 << 3)
#define IP4 (1 << 4)
#define IP5 (1 << 5)
#define IP6 (1 << 6)
#define IP7 (1 << 7)

#define ALLEGREX_EXTERNAL_INTERRUPT IP2
#define ALLEGREX_COUNT_INTERRUPT    IP7

extern EXCEPTION_HANDLER _irq;

struct AllegrexIframe 
{
    uint32_t at;
    uint32_t v0;
    uint32_t v1;
    uint32_t a0;
    uint32_t a1;
    uint32_t a2;
    uint32_t a3;
    uint32_t t0;
    uint32_t t1;
    uint32_t t2;
    uint32_t t3;
    uint32_t t4;
    uint32_t t5;
    uint32_t t6;
    uint32_t t7;
    uint32_t s0;
    uint32_t s1;
    uint32_t s2;
    uint32_t s3;
    uint32_t s4;
    uint32_t s5;
    uint32_t s6;
    uint32_t s7;
    uint32_t t8;
    uint32_t t9;
    uint32_t k0;
    uint32_t k1;
    uint32_t gp;
    uint32_t sp;
    uint32_t fp;
    uint32_t ra;
    uint32_t lo;
    uint32_t hi;
    uint32_t status;
    uint32_t cause;
    uint32_t epc;
};

static IrqHandlerFunction g_irq_handlers[IRQ_MAXIMUM_COUNT][NUM_IRQ_HANDLERS] = { 0 };
static RescheduleHookFunction g_reschedule_hook = NULL;

static inline uint32_t *enabled_interrupts(void)
{
    static uint32_t masks[2] = { 0 };
    return &masks[0];
}

static inline int is_ctrl2_interrupt(enum InterruptType interrupt)
{
    return interrupt == IRQ_UNK38 || interrupt == IRQ_UNK39;
}

static inline int is_ctrl1_interrupt(enum InterruptType interrupt)
{
    return interrupt >= 32 && !is_ctrl2_interrupt(interrupt);
}

static inline int is_ctrl0_interrupt(enum InterruptType interrupt)
{
    return interrupt < 32;
}

static inline void set_interrupt_masks(uint32_t *interrupts)
{
    *INTERRUPT_CTRL0_MASK_REG = interrupts[0] | ALWAYS_ENABLED_INTERRUPTS;
    *INTERRUPT_CTRL1_MASK_REG = interrupts[1] & ~(INTR_MASK(IRQ_UNK38) | INTR_MASK(IRQ_UNK39));
    *INTERRUPT_CTRL2_MASK_REG = (interrupts[1] & (INTR_MASK(IRQ_UNK38) | INTR_MASK(IRQ_UNK39))) >> 6;
    cpu_sync();
}

static enum IrqHandleStatus despatch_irq_handlers(enum InterruptType irq)
{
    enum IrqHandleStatus res = IRQ_HANDLE_NO_RESCHEDULE;

    if (!g_irq_handlers[irq][0]) {
        return res;
    }

    for (size_t handler = 0; handler < NUM_IRQ_HANDLERS && g_irq_handlers[irq][handler]; ++handler) {
        if (g_irq_handlers[irq][handler]() == IRQ_HANDLE_RESCHEDULE) {
            res = IRQ_HANDLE_RESCHEDULE;
        }
    }

    return res;
}

//#include <lk/debug.h>


static enum IrqHandleStatus on_ext_irq(struct AllegrexIframe *iframe)
{
    // dprintf(INFO, "got ext IRQ\n");
    uint32_t *irqs = enabled_interrupts();
    uint32_t ctrl0_irqs = *INTERRUPT_CTRL0_UNMASKED_FLAGS_REG & irqs[0];
    
    // TODO: represent this better. what a fucking mess
    uint32_t ctrl1_irqs = ((*INTERRUPT_CTRL1_UNMASKED_FLAGS_REG & ~(INTR_MASK(IRQ_UNK38) | INTR_MASK(IRQ_UNK39))) | ((*INTERRUPT_CTRL2_UNMASKED_FLAGS_REG << 6) & (INTR_MASK(IRQ_UNK38) | INTR_MASK(IRQ_UNK39)))) * irqs[1];

    unsigned int irq_num = 0;

    if (ctrl0_irqs) {
        irq_num = cpu_clz(cpu_bitrev(ctrl0_irqs));
    }
    else if (ctrl1_irqs) {
        irq_num = cpu_clz(cpu_bitrev(ctrl1_irqs)) + 32;
    }
    else {
        // not our interrupt?
    // dprintf(INFO, "BAD, UNKNOWN INTR\n");
        return IRQ_HANDLE_NO_RESCHEDULE;
    }

    // dprintf(INFO, "despatching handler for IRQ num %i\n", irq_num);
    enum IrqHandleStatus res = despatch_irq_handlers(irq_num);
    interrupt_clear(irq_num);
    return res;
}

void on_irq(struct AllegrexIframe *iframe)
{
    // disable interrupts, clear EXL
    cpu_set_status(cpu_get_status() & 0xFFFFFE0);

    unsigned int irq_num = (iframe->cause >> 8) & 0xFF;
    // dprintf(INFO, "received IRQ %u\n", irq_num);
    // dprintf(INFO, "0x%08X - CAUSE\n", iframe->cause);
    // dprintf(INFO, "0x%08X - STATUS\n", cpu_get_status());
    enum IrqHandleStatus res = IRQ_HANDLE_NO_RESCHEDULE;

    if (irq_num & ALLEGREX_COUNT_INTERRUPT) {
        res = despatch_irq_handlers(IRQ_COUNT);
    }

    else if (irq_num & ALLEGREX_EXTERNAL_INTERRUPT) {
        res = on_ext_irq(iframe);
    }

    else {
        //dprintf(INFO,  "unknown irq state. cause: 0x%08X\n", iframe->cause);
    }

    if (res == IRQ_HANDLE_RESCHEDULE && g_reschedule_hook) {
        g_reschedule_hook();
    }
}

void interrupt_init(void)
{
    uint32_t mask = cpu_suspend_interrupts();
    cpu_set_status((cpu_get_status() & 0xFFFF00FF) | 0x400);
    uint32_t *interrupts = enabled_interrupts();
    interrupts[0] = interrupts[1] = 0;
    set_interrupt_masks(interrupts);
    cpu_resume_interrupts(mask);

    exception_register_handler(EXCEPTION_IRQ, &_irq);
    cpu_enable_interrupts();
}

void interrupt_enable(enum InterruptType interrupt)
{
    uint32_t mask = cpu_suspend_interrupts();
    uint32_t *interrupts = enabled_interrupts();
    interrupts[interrupt/32] = (1 << (interrupt % 32));
    set_interrupt_masks(interrupts);
    cpu_resume_interrupts(mask);
}

int interrupt_occured(enum InterruptType interrupt)
{
    if (is_ctrl0_interrupt(interrupt)) {
        return *INTERRUPT_CTRL0_UNMASKED_FLAGS_REG & INTR_MASK(interrupt);
    }

    else if (is_ctrl1_interrupt(interrupt)) {
        return *INTERRUPT_CTRL1_UNMASKED_FLAGS_REG & INTR_MASK(interrupt);
    }
    
    else { // if (is_ctrl2_interrupt(interrupt)) {
        return *INTERRUPT_CTRL2_UNMASKED_FLAGS_REG & (INTR_MASK(interrupt) >> 6);
    }
}

void interrupt_clear(enum InterruptType interrupt)
{
    if (is_ctrl0_interrupt(interrupt)) {
        *INTERRUPT_CTRL0_UNMASKED_FLAGS_REG = INTR_MASK(interrupt);
    }

    else if (is_ctrl1_interrupt(interrupt)) {
        *INTERRUPT_CTRL1_UNMASKED_FLAGS_REG = INTR_MASK(interrupt);
    }
    
    else { // if (is_ctrl2_interrupt(interrupt)) {
        *INTERRUPT_CTRL2_UNMASKED_FLAGS_REG = (INTR_MASK(interrupt) >> 6);
    }
}

void interrupt_set_handler(enum InterruptType irq, IrqHandlerFunction handler)
{
    if ((unsigned int)irq < IRQ_MAXIMUM_COUNT) {
        for (size_t i = 0; i < NUM_IRQ_HANDLERS; ++i) {
            if (!g_irq_handlers[irq][i]) {
                g_irq_handlers[irq][i] = handler;
                break;
            }
        }
    }
}

void interrupt_set_reschedule_hook(RescheduleHookFunction function)
{
    g_reschedule_hook = function;
}