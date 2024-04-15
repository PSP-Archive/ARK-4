#include "phatlcd.h"

#include <gpio.h>
#include <cpu.h>
#include <dmacplus.h>
#include <pwm.h>

#include <stdint.h>
#include <string.h>

enum DisplayState
{
    DISPLAY_OFF,
    DISPLAY_ON
};

enum BacklightActivation
{
    BACKLIGHT_NO_ACTION = 0,
    BACKLIGHT_ACTIVATE_VSYNC,
    BACKLIGHT_ACTIVATE_NEXT_VSYNC
};

typedef struct
{
    enum DisplayState active_display_state;
    enum DisplayState requested_display_state;
    unsigned int vsync_defer;
    unsigned int active_brightness;
    unsigned int requested_brightness;
    enum BacklightActivation backlight_state;
} PhatLcdState;

static PhatLcdState *lcd_state(void)
{
    static PhatLcdState s_lcd_state;
    return &s_lcd_state;
}

static void enable_display(void)
{
    gpio_set(GPIO_PORT_LCD_RESET);
}

static void disable_display(void)
{
    gpio_clear(GPIO_PORT_LCD_RESET);
}

static void enable_backlight(void)
{
    pwm_enable_channel(1, 0x18, 0x19, 0x19);
}

static void update_display_state(PhatLcdState *state)
{
    if (state->active_display_state != state->requested_display_state) {
        switch (state->requested_display_state) {
            case DISPLAY_OFF:
                state->active_display_state = DISPLAY_OFF;
                disable_display();
                break;

            case DISPLAY_ON:
                {
                unsigned int intr = cpu_suspend_interrupts();
                state->active_display_state = DISPLAY_ON;
                state->vsync_defer = 30;
                enable_display();
                dmacplus_lcdc_enable();
                cpu_resume_interrupts(intr);
                }
                break;
        }
    }
}

static void update_brightness(PhatLcdState *state, int brightness)
{
    // rescale to 0 -> 25
    uint32_t scaled_brightness = brightness/4;

    // when the brightness is non-zero we expect a minimum value of 1
    // so as not to turn the display off
    if (scaled_brightness == 0 && brightness != 0) {
        scaled_brightness = 1;
    }

    // we have different operations to do depending on the current state
    // and the desired state.
    // here we assume the hardware is not initialised and the desired state
    // is to activate it and set the brightness
    if (!state->active_brightness && brightness)
    {
        state->active_brightness = brightness;
        pwm_enable_channel(0, 0x18, 0x19, scaled_brightness & 0xFFFF);
        state->backlight_state = BACKLIGHT_ACTIVATE_NEXT_VSYNC;
    }

    // here we have the hardware initialised and only wish to change the
    // current brightness level
    else if (state->active_brightness && brightness)
    {
        state->active_brightness = brightness;
        // TODO: implement
        // pwm_clear(0, 0x19, scaled_brightness & 0xFFFF);
    }

    // the final state we can action is when we have initialised output
    // but the desired state is to turn off the display (or brightness == 0)
    else if (state->active_brightness && !brightness)
    {
        state->active_brightness = 0;
        state->backlight_state = BACKLIGHT_NO_ACTION;
        pwm_disable_channel(1);
        pwm_disable_channel(0);
    }
}

LcdDriverResult lcd_set_mode(int mode, unsigned int xres, unsigned int yres, unsigned int stride, unsigned int format)
{
    return LCD_DRIVER_ERR_NOT_IMPL;
}

LcdDriverResult lcd_set_backlight_brightness(unsigned int brightness)
{
    lcd_state()->requested_brightness = brightness;
    return LCD_DRIVER_RES_OK;
}

LcdDriverResult lcd_enable_display(void)
{
    lcd_state()->requested_display_state = DISPLAY_ON;
    return LCD_DRIVER_RES_OK;
}

LcdDriverResult lcd_disable_display(void)
{
    lcd_state()->requested_display_state = DISPLAY_OFF;
    return LCD_DRIVER_RES_OK;
}

LcdDriverResult lcd_init_backlight(void)
{
    return LCD_DRIVER_RES_OK;
}

LcdDriverResult lcd_on_interrupt(void)
{
    PhatLcdState *state = lcd_state();
    update_display_state(state);

    // reduce vsync deferral counter
    if (state->vsync_defer) {
        state->vsync_defer -= 1;
    }

    // set the backlight if the deferral has passed and the display is on
    if (state->active_display_state == DISPLAY_ON && state->vsync_defer == 0) {
        // set the brightness if we have a new value
        if (state->active_brightness != state->requested_brightness) {
            update_brightness(state, state->requested_brightness);
        }

        // we defer our backlight activation another vsync cycle
        if (state->backlight_state == BACKLIGHT_ACTIVATE_NEXT_VSYNC) {
            state->backlight_state = BACKLIGHT_ACTIVATE_VSYNC;
        }

        // otherwise check if we're activating the backlight on
        // this vsync and issue the call to pwm to instruct
        else if (state->backlight_state == BACKLIGHT_ACTIVATE_VSYNC) {
            state->backlight_state = BACKLIGHT_NO_ACTION;
            enable_backlight();
        }
    }

    return LCD_DRIVER_RES_OK;
}

LcdDriverResult lcd_init(void)
{
    PhatLcdState *state = lcd_state();
    memset(state, 0, sizeof(*state));
    state->active_display_state = DISPLAY_OFF;
    state->requested_display_state = DISPLAY_OFF;
    state->vsync_defer = 0;
    state->active_brightness = 0;
    state->requested_brightness = 0;
    state->backlight_state = BACKLIGHT_NO_ACTION;

    gpio_set_port_mode(GPIO_PORT_LCD_RESET, GPIO_MODE_OUTPUT);
    return LCD_DRIVER_RES_OK;
}

const LcdDriver g_phat_lcd_driver = 
{
    .init = lcd_init,
    .set_mode = lcd_set_mode,
    .set_backlight_brightness = lcd_set_backlight_brightness,
    .enable_display = lcd_enable_display,
    .disable_display = lcd_disable_display,
    .on_interrupt = lcd_on_interrupt,
};
