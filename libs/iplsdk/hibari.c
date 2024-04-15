#include "hibari.h"

#include <model.h>
#include <pwm.h>
#include <spi.h>
#include <sysreg.h>
#include <gpio.h>
#include <cpu.h>
#include <utils.h>

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

typedef enum
{
    SEQ_CMD_HIBARI_SINGLE_REG,
    SEQ_CMD_SAKURA_SINGLE_REG,
    SEQ_CMD_BOTH_SINGLE_REG,
    SEQ_CMD_WAIT_DELAY,
    SEQ_CMD_SET_GAMMA_TABLE,
    SEQ_CMD_WRITE_OS_PARAMS,
    SEQ_CMD_TERM = 0xFF
} CommandType;

typedef struct
{
    CommandType cmd;
    unsigned char reg;
    unsigned char value;
} SequenceCommand;

typedef struct 
{
    enum DisplayState active_display_state;
    enum DisplayState requested_display_state;
    unsigned int vsync_defer;
    unsigned int active_brightness;
    unsigned int requested_brightness;
    enum BacklightActivation backlight_state;
    unsigned int active_hibari_reg;
    unsigned int active_hibari_sub_reg;
    unsigned int active_sekura_reg;
    const SequenceCommand *sequence;
} HibariState;

typedef struct
{
    unsigned char val1;
    unsigned char val2;
} GammaTableValues;

typedef struct
{
    unsigned char params[0x80];
} GammaTable;

typedef enum {
    HIBARI_RES_OK,
    HIBARI_ERR_UNSUPPORED_MODEL,
    HIBARI_UNKNOWN_VERSION,
} HibariResult;

#define INVALID_REG_VALUE       (0xFFFFFFFF)

SequenceCommand g_seq_init[] =
{
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x12, 0x02 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x13, 0x10 },
    { SEQ_CMD_BOTH_SINGLE_REG,      0x14, 0x04 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x15, 0x0D },
    { SEQ_CMD_SAKURA_SINGLE_REG,    0x15, 0x2B },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x16, 0x88 },
    { SEQ_CMD_SAKURA_SINGLE_REG,    0x16, 0x44 },
    { SEQ_CMD_BOTH_SINGLE_REG,      0x17, 0x78 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x1C, 0x5A },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x40, 0x10 },
    { SEQ_CMD_TERM,                 0, 0 }
};

SequenceCommand g_seq_os_init[] =
{
    { SEQ_CMD_SAKURA_SINGLE_REG,    0x12, 0x02 },
    { SEQ_CMD_SAKURA_SINGLE_REG,    0x13, 0x00 },
    { SEQ_CMD_SAKURA_SINGLE_REG,    0x18, 0x0B },
    { SEQ_CMD_SAKURA_SINGLE_REG,    0x19, 0x00 },
    { SEQ_CMD_SAKURA_SINGLE_REG,    0x1B, 0x0B },
    { SEQ_CMD_SAKURA_SINGLE_REG,    0x1C, 0x64 },
    { SEQ_CMD_SAKURA_SINGLE_REG,    0x1A, 0x00 },
    { SEQ_CMD_TERM,                 0, 0 }
};

SequenceCommand g_seq_timing_init[] =
{
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x42, 0x40 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x43, 0x46 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x44, 0x42 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x45, 0x20 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x46, 0x03 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x47, 0x04 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x48, 0x43 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x49, 0x3F },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x4A, 0x14 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x4B, 0x20 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x4C, 0x26 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x4D, 0x27 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x4E, 0x1E },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x4F, 0x42 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x50, 0x9E },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x51, 0x02 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x52, 0x90 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x53, 0x00 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x54, 0x14 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x57, 0x3F },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x58, 0x19 },
    { SEQ_CMD_TERM,                 0, 0 }
};

SequenceCommand g_seq_counter_init[] =
{
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x56, 0x60 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x3E, 0x01 },
    { SEQ_CMD_TERM,                 0, 0 }
};

SequenceCommand g_seq_step_up_freq[] =
{
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x81, 0x81 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x82, 0x44 },
    { SEQ_CMD_TERM,                 0, 0 }
};

SequenceCommand g_seq_bias_current[] =
{
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x55, 0x88 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x87, 0x36 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x84, 0x01 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x85, 0x0F },
    { SEQ_CMD_TERM,                 0, 0 }
};

SequenceCommand g_seq_display_on[] =
{
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x11, 0x00 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0xD0, 0x00 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x36, 0x00 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x34, 0x13 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x3C, 0x01 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x83, 0x10 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x80, 0x01 },
    { SEQ_CMD_WRITE_OS_PARAMS,      0x00, 0x00 },
    { SEQ_CMD_WAIT_DELAY,           0x01, 0x00 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x80, 0x03 },
    { SEQ_CMD_SET_GAMMA_TABLE,      0x00, 0x00 },
    { SEQ_CMD_WAIT_DELAY,           0x01, 0x00 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x80, 0x07 },
    { SEQ_CMD_WAIT_DELAY,           0x01, 0x00 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x83, 0x00 },
    { SEQ_CMD_WAIT_DELAY,           0x01, 0x00 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x89, 0x02 },
    { SEQ_CMD_WAIT_DELAY,           0x01, 0x00 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x80, 0x27 },
    { SEQ_CMD_WAIT_DELAY,           0x01, 0x00 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x80, 0x37 },
    { SEQ_CMD_WAIT_DELAY,           0x01, 0x00 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x80, 0x77 },
    { SEQ_CMD_WAIT_DELAY,           0x01, 0x00 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x80, 0xF7 },
    { SEQ_CMD_WAIT_DELAY,           0x01, 0x00 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x83, 0x03 },
    { SEQ_CMD_WAIT_DELAY,           0x01, 0x00 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x33, 0x04 },
    { SEQ_CMD_WAIT_DELAY,           0x01, 0x00 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x33, 0x05 },
    { SEQ_CMD_WAIT_DELAY,           0x01, 0x00 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x35, 0x00 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x30, 0x02 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x3D, 0x07 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x31, 0x01 },
    { SEQ_CMD_WAIT_DELAY,           0x01, 0x00 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x3D, 0x03 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x31, 0x01 },
    { SEQ_CMD_WAIT_DELAY,           0x01, 0x00 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x3D, 0x01 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x31, 0x01 },
    { SEQ_CMD_WAIT_DELAY,           0x01, 0x00 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x10, 0x00 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0xD0, 0x01 },
    { SEQ_CMD_WAIT_DELAY,           0x01, 0x00 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x30, 0x03 },
    { SEQ_CMD_WAIT_DELAY,           0x01, 0x00 },
    { SEQ_CMD_TERM,                 0x00, 0x00 }
};

SequenceCommand g_seq_display_off[] =
{
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x34, 0x13 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x30, 0x02 },
    { SEQ_CMD_WAIT_DELAY,           0x02, 0x00 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x35, 0x01 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x31, 0x01 },
    { SEQ_CMD_WAIT_DELAY,           0x01, 0x00 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x30, 0x00 },
    { SEQ_CMD_WAIT_DELAY,           0x01, 0x00 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x33, 0x00 },
    { SEQ_CMD_WAIT_DELAY,           0x01, 0x00 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x80, 0xB7 },
    { SEQ_CMD_WAIT_DELAY,           0x02, 0x00 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x80, 0x37 },
    { SEQ_CMD_WAIT_DELAY,           0x02, 0x00 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x80, 0x07 },
    { SEQ_CMD_WAIT_DELAY,           0x01, 0x00 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x80, 0x03 },
    { SEQ_CMD_WAIT_DELAY,           0x02, 0x00 },
    { SEQ_CMD_HIBARI_SINGLE_REG,    0x80, 0x00 },
    { SEQ_CMD_WAIT_DELAY,           0x02, 0x00 },
    { SEQ_CMD_TERM,                 0x00, 0x00 }
};

#define GAMMA_TABLE_COUNT   (4)

static unsigned char g_gamma_set[GAMMA_TABLE_COUNT] = {
    0x10, 0x10, 0x10, 0x10
};

static GammaTableValues g_gamma_defaults[GAMMA_TABLE_COUNT] = {
    { 0x00, 0xE8 },
    { 0x00, 0x9F },
    { 0x00, 0x47 },
    { 0x01, 0x90 },
};

static GammaTable g_gamma_params[GAMMA_TABLE_COUNT] = {
    { .params = { 0x00, 0x09, 0x08, 0x08, 0x07, 0x06, 0x05, 0x05, 0x04, 0x05, 0x05, 0x04, 0x05, 0x04, 0x05, 0x04, 0x04, 0x05, 0x04, 0x04, 0x04, 0x04, 0x03, 0x04, 0x04, 0x02, 0x04, 0x03, 0x03, 0x04, 0x03, 0x03, 0x02, 0x03, 0x02, 0x04, 0x02, 0x03, 0x02, 0x02, 0x02, 0x03, 0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x03, 0x01, 0x03, 0x01, 0x02, 0x02, 0x02, 0x02, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x03, 0x01, 0x03, 0x01, 0x01, 0x02, 0x01, 0x02, 0x01, 0x01, 0x02, 0x01, 0x01, 0x02, 0x01, 0x02, 0x01, 0x02, 0x01, 0x02, 0x01, 0x01, 0x02, 0x01, 0x02, 0x01, 0x01, 0x02, 0x01, 0x01, 0x02, 0x01, 0x02, 0x01, 0x02, 0x01, 0x02, 0x01, 0x02, 0x01, 0x01, 0x02, 0x01, 0x02, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00 }},
    { .params = { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x02, 0x00, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x02, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x02, 0x01, 0x01, 0x01, 0x01, 0x02, 0x01, 0x02, 0x03, 0x03, 0x04, 0x04, 0x06, 0x07, 0x11, 0x14, 0x18, 0x17, 0x07 }},
    { .params = { 0x00, 0x09, 0x08, 0x09, 0x06, 0x06, 0x05, 0x05, 0x04, 0x05, 0x05, 0x04, 0x05, 0x04, 0x05, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x05, 0x03, 0x04, 0x03, 0x03, 0x03, 0x04, 0x03, 0x03, 0x03, 0x03, 0x04, 0x02, 0x03, 0x02, 0x03, 0x02, 0x02, 0x02, 0x03, 0x02, 0x02, 0x03, 0x02, 0x02, 0x02, 0x02, 0x01, 0x03, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x02, 0x02, 0x02, 0x02, 0x01, 0x03, 0x01, 0x03, 0x01, 0x02, 0x02, 0x01, 0x02, 0x01, 0x01, 0x02, 0x01, 0x02, 0x01, 0x01, 0x02, 0x01, 0x02, 0x01, 0x02, 0x01, 0x01, 0x02, 0x01, 0x02, 0x01, 0x01, 0x02, 0x01, 0x02, 0x01, 0x01, 0x02, 0x01, 0x02, 0x01, 0x02, 0x01, 0x02, 0x01, 0x01, 0x02, 0x01, 0x02, 0x01, 0x01, 0x02, 0x01, 0x02, 0x01, 0x01, 0x01, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x01 }},
    { .params = { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x02, 0x00, 0x02, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x02, 0x00, 0x02, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x02, 0x01, 0x01, 0x02, 0x01, 0x02, 0x02, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x10, 0x14, 0x19, 0x1E, 0x10 }},
};

static unsigned char g_os_params[] = {
    0x00, 0x43, 0x78, 0x97, 0xB1, 0xC6, 0xE1, 0xF7,
    0xFF, 0x00, 0x20, 0x50, 0x75, 0x9B, 0xBA, 0xD7,
    0xF4, 0xFF, 0x00, 0x12, 0x40, 0x69, 0x8F, 0xB4,
    0xD3, 0xEF, 0xFF, 0x00, 0x0F, 0x34, 0x60, 0x88,
    0xAD, 0xCF, 0xEE, 0xFF, 0x00, 0x0C, 0x29, 0x56,
    0x80, 0xA3, 0xC8, 0xE8, 0xFF, 0x00, 0x08, 0x22,
    0x4E, 0x77, 0xA0, 0xC5, 0xE7, 0xFF, 0x00, 0x08,
    0x14, 0x43, 0x70, 0x9A, 0xC0, 0xE1, 0xFF, 0x00,
    0x05, 0x10, 0x37, 0x62, 0x90, 0xBA, 0xE0, 0xFF,
    0x00, 0x04, 0x0C, 0x30, 0x53, 0x82, 0xAE, 0xDC,
    0xFF
};

static HibariState *hibari_state(void)
{
    static HibariState state;
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

static void enable_device(void)
{
    sysreg_clk2_enable(CLK2_SPI1);

    while (spi_is_data_available(SPI_HIBARI)) {
        (void)spi_read(SPI_HIBARI);
    }

    spi_enable_ssp(SPI_HIBARI);
    cpu_sync();
}

static void disable_device(void)
{
    while(spi_is_busy(SPI_HIBARI));
    spi_disable_ssp(SPI_HIBARI);
    cpu_sync();
    sysreg_clk2_disable(CLK2_SPI1);
}

void write_hibari_continuous_reg(HibariState *state, unsigned int reg, unsigned int sub_reg, unsigned char *data, unsigned int num_values)
{
    enable_device();

    if (state->active_hibari_reg != reg && sub_reg != 0xEF) {
        // wait until the TX fifo has some space so we can set the register
        while (spi_is_transmit_fifo_full(SPI_HIBARI));
        spi_write(SPI_HIBARI, 0xEF);
        while (spi_is_transmit_fifo_full(SPI_HIBARI));
        spi_write(SPI_HIBARI, reg | 0x100);
    
        state->active_hibari_reg = reg;
        state->active_hibari_sub_reg = INVALID_REG_VALUE;
    }

    if (state->active_hibari_sub_reg != sub_reg) {
        while (spi_is_transmit_fifo_full(SPI_HIBARI));
        spi_write(SPI_HIBARI, sub_reg);
    }

    for (size_t i = 0; i < num_values; ++i, ++sub_reg) {
        if (sub_reg == 0xEF) {
            state->active_hibari_reg = data[i];
        }

        while (spi_is_transmit_fifo_full(SPI_HIBARI));
        spi_write(SPI_HIBARI, data[i] | 0x100);
    }

    state->active_hibari_sub_reg = sub_reg;
    state->active_sekura_reg = INVALID_REG_VALUE;

    disable_device();
}

static void set_active_reg(HibariState *state, unsigned int reg)
{
    while (spi_is_transmit_fifo_full(SPI_HIBARI));
    spi_write(SPI_HIBARI, 0xEF);
    while (spi_is_transmit_fifo_full(SPI_HIBARI));
    spi_write(SPI_HIBARI, reg | 0x100);
    state->active_hibari_reg = reg;
}

unsigned int read_hibari_reg(HibariState *state, unsigned int reg, unsigned int sub_reg)
{
    enable_device();

    if (state->active_hibari_reg != reg) {
        set_active_reg(state, reg);
    }

    while (spi_is_busy(SPI_HIBARI));
    while (spi_is_data_available(SPI_HIBARI)) {
        (void)spi_read(SPI_HIBARI);
    }
    while (spi_is_transmit_fifo_full(SPI_HIBARI));
    spi_write(SPI_HIBARI, sub_reg);
    while (!spi_is_data_available(SPI_HIBARI));
    (void)spi_read(SPI_HIBARI);
    while (spi_is_transmit_fifo_full(SPI_HIBARI));
    spi_write(SPI_HIBARI, 0x2000);
    while (!spi_is_data_available(SPI_HIBARI));
    unsigned char reg_value = spi_read(SPI_HIBARI);
    state->active_hibari_sub_reg = sub_reg;

    disable_device();
    return reg_value & 0xFF;
}

unsigned int write_both_single_reg(HibariState *state, unsigned int reg, unsigned int sub_reg)
{
    enable_device();

    while (spi_is_transmit_fifo_full(SPI_HIBARI));
    spi_write(SPI_HIBARI, 0xC000 | reg);

    while (spi_is_transmit_fifo_full(SPI_HIBARI));
    spi_write(SPI_HIBARI, 0xC100 | sub_reg);

    state->active_sekura_reg = INVALID_REG_VALUE;
    state->active_hibari_sub_reg = INVALID_REG_VALUE;

    disable_device();
    return 0;
}

void write_hibari_single_reg(HibariState *state, int reg, unsigned int sub_reg, unsigned int value)
{
    unsigned char val = value;
    write_hibari_continuous_reg(state, reg, sub_reg, &val, 1);
}

void write_sakura_continuous_reg(HibariState *state, unsigned int reg, unsigned char *data, size_t len)
{
    enable_device();

    if (state->active_sekura_reg != reg) {
        while (spi_is_transmit_fifo_full(SPI_HIBARI));
        spi_write(SPI_HIBARI, 0x8000 | reg);
    }

    for (size_t i = 0; i < len; ++i, ++reg) {
        while (spi_is_transmit_fifo_full(SPI_HIBARI));
        spi_write(SPI_HIBARI, 0x8100 | data[i]);
    }

    state->active_sekura_reg = reg;
    state->active_hibari_sub_reg = INVALID_REG_VALUE;

    disable_device();
}

void write_sakura_single_reg(HibariState *state, unsigned int reg, unsigned int val)
{
    unsigned char value = val;
    write_sakura_continuous_reg(state, reg, &value, 1);
}

void write_hibari_select_bank(HibariState *state, unsigned int bank)
{
    unsigned char bank_byte = bank;
    write_hibari_continuous_reg(state, 0, 0xEF, &bank_byte, 1);
}

void set_gamma_table(HibariState *state, unsigned char *data, GammaTableValues *values, GammaTable *tables)
{
    for (size_t i = 0; i < GAMMA_TABLE_COUNT; ++i) {
        write_hibari_single_reg(state, i + 1, 0xD1, data[i]);

        if (i < 2) {
            write_hibari_single_reg(state, i + 1, 0xD2, values[i].val2);
            write_hibari_single_reg(state, i + 1, 0xD3, values[i].val1);
        } else {
            write_hibari_single_reg(state, i + 1, 0xD4, values[i].val2);
            write_hibari_single_reg(state, i + 1, 0xD5, values[i].val1);
        }

        write_hibari_continuous_reg(state, i + 1, 0, tables[i].params, sizeof(tables[i].params));
    }

    write_hibari_select_bank(state, 0);
}

static void sequence_process(HibariState *state)
{
    if (!state->sequence) {
        return;
    }

    for (const SequenceCommand *sequence = state->sequence; sequence->cmd != SEQ_CMD_TERM; ++sequence)
    {
        switch (sequence->cmd)
        {
            case SEQ_CMD_HIBARI_SINGLE_REG:
                write_hibari_single_reg(state, 0, sequence->reg, sequence->value);
                break;

            case SEQ_CMD_SAKURA_SINGLE_REG:
                write_sakura_single_reg(state, sequence->reg, sequence->value);
                break;
            
            case SEQ_CMD_BOTH_SINGLE_REG:
                write_both_single_reg(state, sequence->reg, sequence->value);
                break;

            case SEQ_CMD_WAIT_DELAY:
                for (size_t i = 0; i < sequence->reg; ++i) {
                    util_delay_cpu222_us(16666);
                }
                break;
            
            case SEQ_CMD_SET_GAMMA_TABLE:
                set_gamma_table(state, g_gamma_set, g_gamma_defaults, g_gamma_params);
                break;
            
            case SEQ_CMD_WRITE_OS_PARAMS:
                write_sakura_continuous_reg(state, 0x20, g_os_params, sizeof(g_os_params));
                break;

            case SEQ_CMD_TERM:
                // this shouldn't be possible, but if it happens just exit
                state->sequence = NULL;
                return;
        }
    }

    state->sequence = NULL;
}

static void sequence_execute(HibariState *state, SequenceCommand *sequence)
{
    state->sequence = sequence;
    sequence_process(state);
}

unsigned int get_version(HibariState *state)
{
    return read_hibari_reg(state, 0, 0);
}

HibariResult reset_sequence(HibariState *state)
{
    reset_disable();

    util_delay_cpu222_us(10*1000);

    write_both_single_reg(state, 0xC0, 1);

    util_delay_cpu222_us(50);

    sequence_execute(state, g_seq_init);
    sequence_execute(state, g_seq_os_init);
    sequence_execute(state, g_seq_timing_init);
    sequence_execute(state, g_seq_counter_init);
    sequence_execute(state, g_seq_step_up_freq);
    sequence_execute(state, g_seq_bias_current);

    return HIBARI_RES_OK;
}

static void enable_backlight(void)
{
    pwm_enable_channel(1, 0x18, 0x19, 0x19);
}

static void update_display_state(HibariState *state)
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
                state->vsync_defer = 30;
                break;
        }
    }
}

static void update_brightness(HibariState *state, unsigned int brightness)
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

LcdDriverResult hibari_init(void)
{
    if (model_get_identity()->model != PSP_MODEL_02G) {
        return HIBARI_ERR_UNSUPPORED_MODEL;
    }

    sysreg_clock_select_spi(1, 3);
    sysreg_clk2_enable(CLK2_SPI1);
    sysreg_io_enable(IO_SPI1);

    spi_init(SPI_HIBARI);
    cpu_sync();

    sysreg_clk2_disable(CLK2_SPI1);
    gpio_set_port_mode(GPIO_PORT_LCD_RESET, GPIO_MODE_OUTPUT);

    HibariState *state = hibari_state();
    memset(state, 0, sizeof(*state));
    state->active_brightness = 0;
    state->requested_brightness = 0;
    state->active_display_state = DISPLAY_OFF;
    state->requested_display_state = DISPLAY_OFF;
    state->vsync_defer = 0;
    state->backlight_state = BACKLIGHT_NO_ACTION;
    state->active_hibari_reg = INVALID_REG_VALUE;
    state->active_hibari_sub_reg = INVALID_REG_VALUE;
    state->active_sekura_reg = INVALID_REG_VALUE;

    unsigned int version;
    if (!is_resetted()) {
        version = get_version(state);
    } else {
        reset_disable();
        util_delay_cpu222_us(10*1000);
        version = get_version(state);
        reset_enable();
    }

    if (version != 0xA0) {
        return HIBARI_UNKNOWN_VERSION;
    }

    reset_sequence(state);
    return LCD_DRIVER_RES_OK;
}

LcdDriverResult hibari_set_mode(int mode, unsigned int xres, unsigned int yres, unsigned int stride, unsigned int format)
{
    return LCD_DRIVER_ERR_NOT_IMPL;
}

LcdDriverResult hibari_set_backlight_brightness(unsigned int brightness)
{
    hibari_state()->requested_brightness = brightness;
    return LCD_DRIVER_RES_OK;
}

LcdDriverResult hibari_enable_display(void)
{
    hibari_state()->requested_display_state = DISPLAY_ON;
    return LCD_DRIVER_RES_OK;
}

LcdDriverResult hibari_disable_display(void)
{
    hibari_state()->requested_display_state = DISPLAY_OFF;
    return LCD_DRIVER_RES_OK;
}

LcdDriverResult hibari_on_interrupt(void)
{
    HibariState *state = hibari_state();
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

const LcdDriver g_hibari_driver = 
{
    .init = hibari_init,
    .set_mode = hibari_set_mode,
    .set_backlight_brightness = hibari_set_backlight_brightness,
    .enable_display = hibari_enable_display,
    .disable_display = hibari_disable_display,
    .on_interrupt = hibari_on_interrupt,
};
