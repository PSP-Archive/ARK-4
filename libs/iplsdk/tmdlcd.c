#include "tmdlcd.h"

#include <model.h>
#include <pwm.h>
#include <spi.h>
#include <sysreg.h>
#include <gpio.h>
#include <cpu.h>
#include <utils.h>

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

typedef enum
{
    SEQ_CMD_CONTROLLER_SINGLE_REG,
    SEQ_CMD_DRIVER_SINGLE_REG,
    SEQ_CMD_MODIFY_FLAG,
    SEQ_CMD_WAIT_DELAY,
    SEQ_CMD_TERM = 0xFF
} CommandType;

typedef struct
{
    CommandType cmd;
    unsigned char reg;
    unsigned char value;
} SequenceCommand;

enum TmdLcdDevice {
    TMDLCD_DEV_CONTROLLER,
    TMDLCD_DEV_DRIVER,
};

typedef struct 
{
    enum DisplayState active_display_state;
    enum DisplayState requested_display_state;
    unsigned int vsync_defer;
    unsigned int active_brightness;
    unsigned int requested_brightness;
    enum BacklightActivation backlight_state;
    unsigned int active_bank;
    enum TmdLcdDevice active_device;
    int is_enabled_without_backlight;
    const SequenceCommand *sequence;
    unsigned char controller_flag;
} TmdLcdState;

#define CONTROLLER_LOCKED   (1 << 4)

typedef enum {
    HIBARI_RES_OK,
    HIBARI_ERR_UNSUPPORED_MODEL,
    HIBARI_UNKNOWN_VERSION,
} HibariResult;

#define INVALID_REG_VALUE       (0xFFFFFFFF)

static SequenceCommand g_seq_init[] = {
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x02, 0x00 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x02, 0x00 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x2D, 0x00 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x05, 0x00 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x06, 0x00 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x07, 0xAC },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x08, 0x00 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x09, 0x22 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x0A, 0xFF },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x0B, 0x00 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x0C, 0x00 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x0D, 0xAC },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x0E, 0x56 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x10, 0x00 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x11, 0x00 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x12, 0x0F },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x13, 0x08 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x14, 0x00 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x15, 0x31 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x16, 0x45 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x17, 0x00 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x18, 0x02 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x19, 0x31 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x1A, 0x35 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x1B, 0x11 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x1C, 0x11 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x1D, 0x12 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x1E, 0x0F },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x1F, 0x07 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x20, 0x00 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x21, 0x00 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x22, 0x40 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x23, 0x80 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x24, 0x10 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x25, 0x01 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x26, 0x01 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x27, 0x22 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x28, 0x06 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x29, 0x00 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x2A, 0x00 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x2B, 0x15 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x2C, 0x00 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x05, 0x01 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x06, 0x01 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x07, 0x31 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x08, 0x3B },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x09, 0x12 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x0A, 0x85 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x0B, 0x18 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x0C, 0x24 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x0D, 0x3B },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x0E, 0x13 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x0F, 0x53 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x10, 0x15 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x11, 0x1C },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x12, 0x3B },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x13, 0x13 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x14, 0x54 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x15, 0x15 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x16, 0x02 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x19, 0x0B },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x1A, 0x22 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x1B, 0x06 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x1C, 0x00 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x1D, 0xFE },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x1E, 0x03 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x1F, 0x3F },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x20, 0x09 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x21, 0x01 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x22, 0x7F },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x23, 0x3F },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x24, 0x01 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x25, 0x15 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x26, 0x60 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x27, 0xC7 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x28, 0x4F },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x29, 0x00 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x2A, 0x00 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x2B, 0x00 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x2C, 0x00 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x2D, 0x00 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x2E, 0x00 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x2F, 0x20 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x30, 0x28 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x31, 0x28 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x32, 0x00 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x33, 0x00 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x34, 0x00 },
    { SEQ_CMD_TERM,                      0x00, 0x00 }
};

static SequenceCommand g_seq_tcomdc[] = {
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x26, 0x6B },
    { SEQ_CMD_TERM,                      0x00, 0x00 }
};

static SequenceCommand g_seq_display_on[] = {
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x2C, 0x00 },
    { SEQ_CMD_WAIT_DELAY,                0x02, 0x00 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x2E, 0x00 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x2C, 0x02 },
    { SEQ_CMD_WAIT_DELAY,                0x02, 0x00 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x04, 0x01 },
    { SEQ_CMD_WAIT_DELAY,                0x03, 0x00 },
    { SEQ_CMD_DRIVER_SINGLE_REG,         0x04, 0x01 },
    { SEQ_CMD_WAIT_DELAY,                0x04, 0x00 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x0F, 0x01 },
    { SEQ_CMD_CONTROLLER_SINGLE_REG,     0x2C, 0x00 },
    { SEQ_CMD_WAIT_DELAY,                0x02, 0x00 },
    { SEQ_CMD_MODIFY_FLAG,               0x02, 0x00 },
    { SEQ_CMD_TERM,                      0x00, 0x32 }
};

static SequenceCommand g_seq_backlight_on[] = {
{ SEQ_CMD_MODIFY_FLAG,               0x00, 0x00 },
{ SEQ_CMD_WAIT_DELAY,                0x01, 0x00 },
{ SEQ_CMD_CONTROLLER_SINGLE_REG,     0x32, 0x8F },
{ SEQ_CMD_CONTROLLER_SINGLE_REG,     0x2F, 0x00 },
{ SEQ_CMD_CONTROLLER_SINGLE_REG,     0x30, 0x00 },
{ SEQ_CMD_CONTROLLER_SINGLE_REG,     0x31, 0x00 },
{ SEQ_CMD_CONTROLLER_SINGLE_REG,     0x33, 0x43 },
{ SEQ_CMD_CONTROLLER_SINGLE_REG,     0x34, 0x01 },
{ SEQ_CMD_CONTROLLER_SINGLE_REG,     0x2E, 0x01 },
{ SEQ_CMD_WAIT_DELAY,                0x01, 0x00 },
{ SEQ_CMD_CONTROLLER_SINGLE_REG,     0x2D, 0x01 },
{ SEQ_CMD_TERM,                      0x00, 0x00 }
};

static TmdLcdState *tmdlcd_state(void)
{
    static TmdLcdState state;
    return &state;
}

static int is_resetted(void)
{
    return (gpio_read() & GPIO_PORT_LCD_RESET) == 0;
}

static void reset_disable(void)
{
    gpio_set(GPIO_PORT_LCD_RESET);
}

static void reset_enable(void)
{
    gpio_clear(GPIO_PORT_LCD_RESET);
}

static void enable_backlight(void)
{    
    while (pwm_enable_channel(1, 0x18, 0x19, 0x19) < 0);
    while (1) {
        uint32_t unk;
        pwm_query_channel(1, NULL, NULL, &unk);

        if (unk != 0) {
            break;
        }
    }
}

static void disable_backlight(void)
{
    pwm_disable_channel(1);
    while (1) {
        uint32_t unk;
        pwm_query_channel(1, NULL, NULL, &unk);

        if (unk == 0) {
            break;
        }
    }
}

static void enable_device(TmdLcdState *state, int do_bl_disable)
{
    if (do_bl_disable) {
        state->is_enabled_without_backlight = 1;
        disable_backlight();
    }

    sysreg_clk2_enable(CLK2_SPI1);

    while (spi_is_data_available(SPI_HIBARI)) {
        (void)spi_read(SPI_HIBARI);
    }

    spi_enable_ssp(SPI_HIBARI);
    cpu_sync();
}

static void disable_device(TmdLcdState *state)
{
    while(spi_is_busy(SPI_HIBARI));
    spi_disable_ssp(SPI_HIBARI);
    cpu_sync();
    sysreg_clk2_disable(CLK2_SPI1);

    if (state->is_enabled_without_backlight) {
        state->is_enabled_without_backlight = 0;
        enable_backlight();
    }
}

static unsigned char read_controller_register(TmdLcdState *state, unsigned int bank, unsigned int reg)
{
    enable_device(state, 0);

    if (state->active_device != TMDLCD_DEV_CONTROLLER && reg != 0xCF) {
        while (spi_is_transmit_fifo_full(SPI_HIBARI));
        spi_write(SPI_HIBARI, 0xCF00);
        state->active_device = TMDLCD_DEV_CONTROLLER;
    }

    if (state->active_bank != bank && reg != 0xEF) {
        while (spi_is_transmit_fifo_full(SPI_HIBARI));
        spi_write(SPI_HIBARI, 0xEF00 | bank);
        state->active_bank = bank;
    }

    while (spi_is_busy(SPI_HIBARI));
    while (spi_is_data_available(SPI_HIBARI)) {
        (void)spi_read(SPI_HIBARI);
    }
    while (spi_is_transmit_fifo_full(SPI_HIBARI));
    spi_write(SPI_HIBARI, 0xC100 | reg);
    while (!spi_is_data_available(SPI_HIBARI));
    (void)spi_read(SPI_HIBARI);
    while (spi_is_transmit_fifo_full(SPI_HIBARI));
    spi_write(SPI_HIBARI, 0xC200);
    while (!spi_is_data_available(SPI_HIBARI));
    unsigned char reg_value = spi_read(SPI_HIBARI);

    disable_device(state);
    return reg_value & 0xFF;
}

static void write_driver_single_register(TmdLcdState *state, unsigned int reg, unsigned int val)
{
    enable_device(state, 0);

    if (state->active_device != TMDLCD_DEV_DRIVER && reg != 0xCF) {
        while (spi_is_transmit_fifo_full(SPI_HIBARI));
        spi_write(SPI_HIBARI, 0xCF01);
        state->active_device = TMDLCD_DEV_DRIVER;
    }

    while (spi_is_transmit_fifo_full(SPI_HIBARI));
    spi_write(SPI_HIBARI, (reg << 8) | val);

    disable_device(state);
}

static void write_controller_single_register(TmdLcdState *state, unsigned int bank, unsigned int reg, unsigned int val)
{
    enable_device(state, 0);

    if (state->active_device != TMDLCD_DEV_CONTROLLER && reg != 0xCF) {
        while (spi_is_transmit_fifo_full(SPI_HIBARI));
        spi_write(SPI_HIBARI, 0xCF00);
        state->active_device = TMDLCD_DEV_CONTROLLER;
    }

    if (state->active_bank != bank && reg != 0xC0) {
        while (spi_is_transmit_fifo_full(SPI_HIBARI));
        spi_write(SPI_HIBARI, 0xC000 | bank);
        state->active_bank = bank;
    }

    while (spi_is_transmit_fifo_full(SPI_HIBARI));
    spi_write(SPI_HIBARI, (reg << 8) | val);

    if (reg == 0xC0) {
        state->active_bank = val;
    }

    disable_device(state);
}

static void sequence_process(TmdLcdState *state)
{
    if (!state->sequence) {
        return;
    }

    for (const SequenceCommand *sequence = state->sequence; sequence->cmd != SEQ_CMD_TERM; ++sequence)
    {
        switch (sequence->cmd)
        {
            case SEQ_CMD_CONTROLLER_SINGLE_REG:
                write_controller_single_register(state, 0, sequence->reg, sequence->value);
                break;
            
            case SEQ_CMD_DRIVER_SINGLE_REG:
                write_driver_single_register(state, sequence->reg, sequence->value);
                break;

            case SEQ_CMD_MODIFY_FLAG:
                if (sequence->value == 0) {
                    state->controller_flag &= ~(1 << sequence->reg);
                }
                else if (sequence->value == 1) {
                    state->controller_flag |= 1 << sequence->reg;
                }

                switch (state->controller_flag) {
                    case 0:
                        write_controller_single_register(state, 0, 0x2C, 8);
                        break;

                    case 1:
                        write_controller_single_register(state, 0, 0x2C, 2);
                        break;

                    case 2:
                        write_controller_single_register(state, 0, 0x2C, 9);
                        break;

                    case 3:
                        write_controller_single_register(state, 0, 0x2C, 0xF);
                        break;

                    default:
                        write_controller_single_register(state, 0, 0x2C, 0);
                        break;

                }
                
                break;

            case SEQ_CMD_WAIT_DELAY:
                for (size_t i = 0; i < sequence->reg; ++i) {
                    util_delay_cpu222_us(16666);
                }
                break;

            case SEQ_CMD_TERM:
                // this shouldn't be possible, but if it happens just exit
                state->sequence = NULL;
                return;
        }
    }

    state->sequence = NULL;
}

static void sequence_execute(TmdLcdState *state, SequenceCommand *sequence)
{
    state->sequence = sequence;
    sequence_process(state);
}

static unsigned int get_version(TmdLcdState *state)
{
    return read_controller_register(state, 0, 0);
}

static HibariResult reset_sequence(TmdLcdState *state)
{
    reset_disable();

    util_delay_cpu222_us(10*1000);

    write_controller_single_register(state, 0, 2, 0);

    util_delay_cpu222_us(50);

    sequence_execute(state, g_seq_init);
    sequence_execute(state, g_seq_tcomdc);

    return HIBARI_RES_OK;
}

static void update_display_state(TmdLcdState *state)
{
    if (state->active_display_state != state->requested_display_state) {
        switch (state->requested_display_state) {
            case DISPLAY_OFF:
                // TODO: add display off sequence?
                state->active_display_state = DISPLAY_OFF;
                break;

            case DISPLAY_ON:
                sequence_execute(state, g_seq_display_on);
                state->active_display_state = DISPLAY_ON;
                state->active_brightness = 0;
                state->vsync_defer = 30;
                break;
        }
    }
}

LcdDriverResult tmdlcd_init(void)
{
    switch (model_get_identity()->model) {
        case PSP_MODEL_07G:
            break;

        default:
            return LCD_DRIVER_ERR_BAD_MODEL;
    }

    enable_backlight();

    sysreg_clock_select_spi(1, 3);
    sysreg_clk2_enable(CLK2_SPI1);
    sysreg_io_enable(IO_SPI1);

    spi_init(SPI_HIBARI);
    cpu_sync();

    sysreg_clk2_disable(CLK2_SPI1);
    gpio_set_port_mode(GPIO_PORT_LCD_RESET, GPIO_MODE_OUTPUT);

    TmdLcdState *state = tmdlcd_state();
    state->active_brightness = 0;
    state->requested_brightness = 0;
    state->active_display_state = DISPLAY_OFF;
    state->requested_display_state = DISPLAY_OFF;
    state->vsync_defer = 0;
    state->backlight_state = BACKLIGHT_NO_ACTION;
    state->controller_flag = 0;
    state->active_bank = INVALID_REG_VALUE;
    state->active_device = TMDLCD_DEV_CONTROLLER;

    unsigned int version;
    if (!is_resetted()) {
        version = get_version(state);
    } else {
        reset_disable();
        util_delay_cpu222_us(10*1000);
        version = get_version(state);
        reset_enable();
    }

    if (version != 7) {
        return HIBARI_UNKNOWN_VERSION;
    }

    reset_sequence(state);
    return LCD_DRIVER_RES_OK;
}

LcdDriverResult tmdlcd_set_mode(int mode, unsigned int xres, unsigned int yres, unsigned int stride, unsigned int format)
{
    return LCD_DRIVER_ERR_NOT_IMPL;
}

LcdDriverResult tmdlcd_set_brightness(unsigned int brightness)
{
    tmdlcd_state()->requested_brightness = brightness;
    return LCD_DRIVER_RES_OK;
}

LcdDriverResult tmdlcd_enable_display(void)
{
    tmdlcd_state()->requested_display_state = DISPLAY_ON;
    return LCD_DRIVER_RES_OK;
}

LcdDriverResult tmdlcd_disable_display(void)
{
    tmdlcd_state()->requested_display_state = DISPLAY_OFF;
    return LCD_DRIVER_RES_OK;
}

LcdDriverResult tmdlcd_on_interrupt(void)
{
    TmdLcdState *state = tmdlcd_state();
    update_display_state(state);

    // reduce vsync deferral counter
    if (state->vsync_defer) {
        state->vsync_defer -= 1;
    }

    // set the backlight if the deferral has passed and the display is on
    if (state->active_display_state == DISPLAY_ON && state->vsync_defer == 0) {
        // set the brightness if we have a new value
        if (state->active_brightness != state->requested_brightness) {
            sequence_execute(state, g_seq_backlight_on);
            state->active_brightness = state->requested_brightness;
        }
    }

    return LCD_DRIVER_RES_OK;
}

const LcdDriver g_tmdlcd_driver = 
{
    .init = tmdlcd_init,
    .set_mode = tmdlcd_set_mode,
    .set_backlight_brightness = tmdlcd_set_brightness,
    .enable_display = tmdlcd_enable_display,
    .disable_display = tmdlcd_disable_display,
    .on_interrupt = tmdlcd_on_interrupt,
};
