#include "display.h"

#include <lcd.h>
#include <lcdc.h>
#include <dmacplus.h>
#include <interrupt.h>
#include <sysreg.h>
#include <gpio.h>
#include <cpu.h>
#include <model.h>

#include <stdint.h>

#define REG32(addr)                 ((volatile uint32_t *)(addr))

typedef struct
{
    const LcdDriver *lcd_driver;
    int desired_brightness;
    int mode, xres, yres, stride, format;
    int do_update_mode_on_sync;
    const void *current_framebuffer;
    const void *desired_framebuffer;
} DisplayState;

static inline DisplayState *display_state(void)
{
    static DisplayState state = {0};
    return &state;
}

void set_mode(DisplayState *state, int mode, unsigned int xres, unsigned int yres, unsigned int stride, unsigned int format)
{
    // state->lcd_driver->set_mode(mode, xres, yres, stride, format);
    lcdc_set_mode(mode, xres, yres, format);
    dmacplus_lcdc_disable();
    dmacplus_lcdc_set_format(xres, stride, format);
    dmacplus_lcdc_enable();
}

void display_set_mode(int mode, unsigned int xres, unsigned yres, unsigned stride, unsigned int format)
{
    DisplayState *state = display_state();
    unsigned int mask = cpu_suspend_interrupts();
    state->mode = mode;
    state->xres = xres;
    state->yres = yres;
    state->stride = stride;
    state->format = format;
    state->do_update_mode_on_sync = 1;
    cpu_resume_interrupts_with_sync(mask);
}

void display_set_framebuffer(const void *framebuffer)
{
    DisplayState *state = display_state();
    unsigned int mask = cpu_suspend_interrupts();
    state->desired_framebuffer = framebuffer;
    cpu_resume_interrupts_with_sync(mask);
}

static void set_backlight_brightness(DisplayState *state, int brightness)
{
    state->lcd_driver->set_backlight_brightness(brightness);
}

static enum IrqHandleStatus on_vsync(void)
{
    DisplayState *state = display_state();

    if (state->do_update_mode_on_sync) {
        set_mode(state, state->mode, state->xres, state->yres, state->stride, state->format);
        state->do_update_mode_on_sync = 0;
    }

    if (state->current_framebuffer != state->desired_framebuffer) {
        dmacplus_lcdc_set_base_addr((uintptr_t)state->desired_framebuffer);
        state->current_framebuffer = state->desired_framebuffer;
    }

    state->lcd_driver->on_interrupt();
    return IRQ_HANDLE_NO_RESCHEDULE;
}

#define APB_CLK_SELECT_APBTIMER0    (0)
#define APB_CLK_SELECT_APBTIMER1    (1)
#define APB_CLK_SELECT_APBTIMER2    (2)
#define APB_CLK_SELECT_APBTIMER3    (3)

#define SYSREG_CLK_SELECT_REG       (REG32(0xBC100060))

void sysreg_clk_select_apb_timer(uint32_t timer, uint32_t val)
{
    uint32_t clk_sel = *SYSREG_CLK_SELECT_REG;
    clk_sel &= ~(7 << (timer * 4));
    clk_sel |= val << (timer * 4);
    *SYSREG_CLK_SELECT_REG = clk_sel;
}

void setup_vblank_irq(void)
{
    unsigned int mask = cpu_suspend_interrupts();

    sysreg_clk2_disable(CLK2_APB_TIMER1);
    sysreg_clk_select_apb_timer(APB_CLK_SELECT_APBTIMER1, 4);
    sysreg_clk2_enable(CLK2_APB_TIMER1);

    *REG32(0xbc10003c) |= 1;

    while ((*REG32(0xBE740020) & 4) != 0);
    *REG32(0xBE740000) = 0;
    while ((*REG32(0xBE740020) & 4) != 0);
    while ((*REG32(0xBE740020) & 2) != 0);
    *REG32(0xBE740004) = 0;
    while ((*REG32(0xBE740020) & 8) != 0);
    *REG32(0xBE74000C) = 0xFFFFFFFF;
    while ((*REG32(0xBE740020) & 0x10) != 0);
    *REG32(0xBE740010) = 0;
    while ((*REG32(0xBE740020) & 0x20) != 0);
    *REG32(0xBE740014) = 0;
    while ((*REG32(0xBE740020) & 1) != 0);
    *REG32(0xBE740000) = 0x20201;
    while ((*REG32(0xBE740020) & 4) != 0);

    sysreg_clk2_disable(CLK2_APB_TIMER1);
    sysreg_clk_select_apb_timer(APB_CLK_SELECT_APBTIMER1, 7);
    sysreg_clk2_enable(CLK2_APB_TIMER1);
    *REG32(0xBE740024) = 1;
    cpu_resume_interrupts_with_sync(mask);

    interrupt_set_handler(IRQ_VSYNC, on_vsync);
    interrupt_enable(IRQ_VSYNC);
}

void display_init(void)
{
    DisplayState *state = display_state();
    state->lcd_driver = lcd_driver();
    state->lcd_driver->init();

    dmacplus_lcdc_set_base_addr(0);
    display_set_mode(0, 480, 272, 0, PIXEL_FORMAT_RGBA8888);

    set_backlight_brightness(state, 10);
    setup_vblank_irq();

    state->lcd_driver->enable_display();
}
