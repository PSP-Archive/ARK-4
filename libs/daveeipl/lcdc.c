#include "lcdc.h"

#include <sysreg.h>
#include <cpu.h>
#include <model.h>

#include <stdint.h>
#include <stddef.h>
#include <string.h>

typedef struct
{
    uint16_t back_porch;
    uint16_t sync_width;
    uint16_t front_porch;
    uint16_t resolution;
} Line;

typedef struct
{
    int id;
    float pixel_freq;
    Line x, y;
    uint16_t sync_diff;
    uint8_t clk[2];
} Mode;

typedef struct
{
    unsigned int x;
    unsigned int y;
} Resolution;

typedef struct
{
    uint32_t cycles_per_pixel;
    uintptr_t mmio_base;
    Resolution resolution;
    Resolution shift_resolution;
    uint32_t unk_rate;
    uint32_t mode_id;
    uint32_t pixel_format;
    Mode mode;
} LcdcConfiguration;

static Mode g_displayModes[] = {
    {
        .id = 0,
        .pixel_freq = 9000000.0,
        .clk = { 0, 3 },
        .sync_diff = 0,
        .x = {
            .front_porch = 2,
            .back_porch = 41,
            .sync_width = 2,
            .resolution = 480
        },
        .y = {
            .front_porch = 10,
            .back_porch = 2,
            .sync_width = 2,
            .resolution = 272
        }
    },
    {
        .id = 1,
        .pixel_freq = 27000000.0,
        .clk = { 0, 1 },
        .sync_diff = 1,
        .x = {
            .front_porch = 16,
            .back_porch = 61,
            .sync_width = 61,
            .resolution = 720
        },
        .y = {
            .front_porch = 9,
            .back_porch = 6,
            .sync_width = 30,
            .resolution = 480
        }
    },
    {
        .id = 2,
        .pixel_freq = 25175000.0,
        .clk = { 1, 1 },
        .sync_diff = 1,
        .x = {
            .front_porch = 32,
            .back_porch = 96,
            .sync_width = 32,
            .resolution = 640
        },
        .y = {
            .front_porch = 10,
            .back_porch = 2,
            .sync_width = 33,
            .resolution = 480
        }
    },
    {
        .id = 3,
        .pixel_freq = 27000000.0,
        .clk = { 0, 1 },
        .sync_diff = 1,
        .x = {
            .front_porch = 61,
            .back_porch = 96,
            .sync_width = 61,
            .resolution = 640
        },
        .y = {
            .front_porch = 10,
            .back_porch = 2,
            .sync_width = 33,
            .resolution = 480
        }
    },
    {
        .id = 4,
        .pixel_freq = 13500000.0,
        .clk = { 0, 2 },
        .sync_diff = 1,
        .x = {
            .front_porch = 16,
            .back_porch = 61,
            .sync_width = 61,
            .resolution = 720
        },
        .y = {
            .front_porch = 5,
            .back_porch = 3,
            .sync_width = 14,
            .resolution = 240
        }
    },
    {
        .id = 5,
        .pixel_freq = 13500000.0,
        .clk = { 0, 2 },
        .sync_diff = 1,
        .x = {
            .front_porch = 16,
            .back_porch = 61,
            .sync_width = 61,
            .resolution = 720
        },
        .y = {
            .front_porch = 6,
            .back_porch = 3,
            .sync_width = 14,
            .resolution = 240
        }
    },
    {
        .id = 6,
        .pixel_freq = 13500000.0,
        .clk = { 0, 2 },
        .sync_diff = 1,
        .x = {
            .front_porch = 16,
            .back_porch = 61,
            .sync_width = 61,
            .resolution = 720
        },
        .y = {
            .front_porch = 3,
            .back_porch = 3,
            .sync_width = 14,
            .resolution = 505
        }
    },
    {
        .id = 11,
        .pixel_freq = 9000000.0,
        .clk = { 0, 3 },
        .sync_diff = 0,
        .x = {
            .front_porch = 3,
            .back_porch = 41,
            .sync_width = 2,
            .resolution = 480
        },
        .y = {
            .front_porch = 10,
            .back_porch = 2,
            .sync_width = 2,
            .resolution = 272
        }
    },
    {
        .id = 12,
        .pixel_freq = 27000000.0,
        .clk = { 0, 1 },
        .sync_diff = 1,
        .x = {
            .front_porch = 16,
            .back_porch = 61,
            .sync_width = 61,
            .resolution = 720
        },
        .y = {
            .front_porch = 10,
            .back_porch = 6,
            .sync_width = 30,
            .resolution = 480
        }
    },
    {
        .id = 14,
        .pixel_freq = 9000000.0,
        .clk = { 0, 2 },
        .sync_diff = 0,
        .x = {
            .front_porch = 10,
            .back_porch = 51,
            .sync_width = 4,
            .resolution = 320
        },
        .y = {
            .front_porch = 5,
            .back_porch = 5,
            .sync_width = 5,
            .resolution = 180
        }
    },
    {
        .id = 15,
        .pixel_freq = 18000000.0,
        .clk = { 1, 1 },
        .sync_diff = 0,
        .x = {
            .front_porch = 2,
            .back_porch = 41,
            .sync_width = 2,
            .resolution = 480
        },
        .y = {
            .front_porch = 10,
            .back_porch = 2,
            .sync_width = 2,
            .resolution = 272
        }
    },
    {
        .id = 16,
        .pixel_freq = 18000000.0,
        .clk = { 1, 1 },
        .sync_diff = 0,
        .x = {
            .front_porch = 3,
            .back_porch = 41,
            .sync_width = 2,
            .resolution = 480
        },
        .y = {
            .front_porch = 10,
            .back_porch = 2,
            .sync_width = 2,
            .resolution = 272
        }
    },
    {
        .id = 17,
        .pixel_freq = 13500000.0,
        .clk = { 2, 4 },
        .sync_diff = 1793,
        .x = {
            .front_porch = 16,
            .back_porch = 61,
            .sync_width = 61,
            .resolution = 720
        },
        .y = {
            .front_porch = 10,
            .back_porch = 6,
            .sync_width = 30,
            .resolution = 479
        }
    }
};

#define MAX_DISPLAY_MODE_COUNT  (sizeof(g_displayModes)/sizeof(Mode))

LcdcConfiguration g_lcdc_config;

#define REG32(addr)                 ((volatile uint32_t *)(addr))
#define LCDC_MMIO_BASE              (0xBE140000)

#define LCDC_CONTROL_REG(x)         REG32(x + 0x00)
#define LCDC_SYNC_DIFF_REG(x)       REG32(x + 0x04)
#define LCDC_PIXEL_FORMAT_REG(x)    REG32(x + 0x08)
#define LCDC_X_BACK_PORCH_REG(x)    REG32(x + 0x10)
#define LCDC_X_SYNC_WIDTH_REG(x)    REG32(x + 0x14)
#define LCDC_X_FRONT_PORCH_REG(x)   REG32(x + 0x18)
#define LCDC_X_RESOLUTION_REG(x)    REG32(x + 0x1C)
#define LCDC_Y_BACK_PORCH_REG(x)    REG32(x + 0x20)
#define LCDC_Y_SYNC_WIDTH_REG(x)    REG32(x + 0x24)
#define LCDC_Y_FRONT_PORCH_REG(x)   REG32(x + 0x28)
#define LCDC_Y_RESOLUTION_REG(x)    REG32(x + 0x2C)
#define LCDC_SHIFT_Y_RES_REG(x)     REG32(x + 0x40)
#define LCDC_SHIFT_X_RES_REG(x)     REG32(x + 0x44)
#define LCDC_SCALED_X_RES_REG(x)    REG32(x + 0x48)
#define LCDC_SCALED_Y_RES_REG(x)    REG32(x + 0x4C)
#define LCDC_UNK70_REG(x)           REG32(x + 0x70)

#define SYSREG_CLK_SELECT_REG       (REG32(0xBC100060))

void sysreg_clk_select_lcdc(uint32_t clk1, uint32_t clk2)
{
    uint32_t clk_sel = *SYSREG_CLK_SELECT_REG;
    clk_sel &= ~(0b111 << 20);
    clk_sel |= clk1 << 22;
    clk_sel |= clk2 << 20;
    *SYSREG_CLK_SELECT_REG = clk_sel;
}

void lcdc_init(void)
{
    memset(&g_lcdc_config, 0, sizeof(g_lcdc_config));

    sysreg_clk2_disable(CLK2_LCDCTRL);
    sysreg_clk_select_lcdc(0, 1);
    sysreg_clk2_enable(CLK2_LCDCTRL);
    sysreg_io_enable(IO_LCDC);

    // TODO: later than 01g support
    switch (model_get_identity()->model) {
        case PSP_MODEL_01G:
            g_lcdc_config.cycles_per_pixel = 1;
            g_lcdc_config.mmio_base = LCDC_MMIO_BASE;
            break;

        case PSP_MODEL_02G:
        case PSP_MODEL_03G:
        case PSP_MODEL_04G:
        case PSP_MODEL_05G:
        case PSP_MODEL_07G:
        case PSP_MODEL_09G:
        case PSP_MODEL_11G:
            g_displayModes[0].clk[1] = 1;
            g_displayModes[1].clk[0] = 1;
            g_displayModes[1].clk[1] = 1;
            g_displayModes[4].clk[0] = 1;
            g_displayModes[4].clk[1] = 1;
            g_displayModes[5].clk[0] = 1;
            g_displayModes[5].clk[1] = 1;
            g_displayModes[6].clk[0] = 1;
            g_displayModes[6].clk[1] = 1;
            g_displayModes[7].clk[1] = 1;
            g_displayModes[8].clk[0] = 1;
            g_displayModes[8].clk[1] = 1;
            g_lcdc_config.cycles_per_pixel = 3;
            g_lcdc_config.mmio_base = LCDC_MMIO_BASE;
            *LCDC_UNK70_REG(g_lcdc_config.mmio_base) = 1;
            break;
    }


    g_lcdc_config.mode.x.back_porch = *LCDC_X_BACK_PORCH_REG(g_lcdc_config.mmio_base);
    g_lcdc_config.mode.x.sync_width = *LCDC_X_SYNC_WIDTH_REG(g_lcdc_config.mmio_base);
    g_lcdc_config.mode.x.front_porch = *LCDC_X_FRONT_PORCH_REG(g_lcdc_config.mmio_base);
    g_lcdc_config.mode.x.resolution = *LCDC_X_RESOLUTION_REG(g_lcdc_config.mmio_base);
    g_lcdc_config.mode.y.back_porch = *LCDC_Y_BACK_PORCH_REG(g_lcdc_config.mmio_base);
    g_lcdc_config.mode.y.sync_width = *LCDC_Y_SYNC_WIDTH_REG(g_lcdc_config.mmio_base);
    g_lcdc_config.mode.y.front_porch = *LCDC_Y_FRONT_PORCH_REG(g_lcdc_config.mmio_base);
    g_lcdc_config.mode.y.resolution = *LCDC_Y_RESOLUTION_REG(g_lcdc_config.mmio_base);
    g_lcdc_config.mode.pixel_freq = 9000000.0;
    g_lcdc_config.mode.sync_diff = *LCDC_SYNC_DIFF_REG(g_lcdc_config.mmio_base);

    if (g_lcdc_config.mode.sync_diff) {
        g_lcdc_config.resolution.x = *LCDC_SCALED_X_RES_REG(g_lcdc_config.mmio_base);
        g_lcdc_config.resolution.y = *LCDC_SCALED_Y_RES_REG(g_lcdc_config.mmio_base);
        g_lcdc_config.shift_resolution.x = *LCDC_SHIFT_X_RES_REG(g_lcdc_config.mmio_base);
        g_lcdc_config.shift_resolution.y = *LCDC_SHIFT_Y_RES_REG(g_lcdc_config.mmio_base);
    }
    else {
        g_lcdc_config.resolution.x = *LCDC_X_RESOLUTION_REG(g_lcdc_config.mmio_base);
        g_lcdc_config.resolution.y = *LCDC_Y_RESOLUTION_REG(g_lcdc_config.mmio_base);
    }

    for (int i = 0; i < MAX_DISPLAY_MODE_COUNT; ++i) {
        Line *x = &g_lcdc_config.mode.x;
        Line *x_disp = &g_displayModes[i].x;
        uint32_t x_len_cfg = x->front_porch + x->back_porch + x->sync_width + x->resolution;
        uint32_t x_len_disp = x_disp->front_porch + x_disp->back_porch + x_disp->sync_width + (x_disp->resolution * g_lcdc_config.cycles_per_pixel);

        if (x_len_cfg == x_len_disp) {
            Line *y = &g_lcdc_config.mode.y;
            Line *y_disp = &g_displayModes[i].y;
            uint32_t y_len_cfg = y->front_porch + y->back_porch + y->sync_width + y->resolution;
            uint32_t y_len_disp = y_disp->front_porch + y_disp->back_porch + y_disp->sync_width + y_disp->resolution;

            if (y_len_cfg == y_len_disp) {
                g_lcdc_config.mode.id = g_displayModes[i].id;
                g_lcdc_config.mode.pixel_freq = g_displayModes[i].pixel_freq;

                if (g_lcdc_config.mode.id != 0 && g_lcdc_config.mode.sync_diff) {
                    g_lcdc_config.unk_rate = g_lcdc_config.mode.x.sync_width / g_lcdc_config.cycles_per_pixel - g_displayModes[i].x.sync_width;
                }

                break;
            }
        }
    }

    sysreg_clk2_disable(CLK2_LCDCTRL);
    unsigned char *clk = g_displayModes[g_lcdc_config.mode.id].clk;
    sysreg_clk_select_lcdc(clk[0], clk[1]);
    sysreg_clk2_enable(CLK2_LCDCTRL);

    *LCDC_CONTROL_REG(g_lcdc_config.mmio_base) |= 3;
}

int lcdc_set_mode(unsigned int id, unsigned int x_res, unsigned int y_res, unsigned int format)
{
    if (id != 0) {
        return -1;
    }

    Mode *mode = &g_displayModes[id];
    unsigned int mask = cpu_suspend_interrupts();

    g_lcdc_config.mode_id = id;
    g_lcdc_config.resolution.x = x_res * g_lcdc_config.cycles_per_pixel;
    g_lcdc_config.resolution.y = y_res;
    g_lcdc_config.pixel_format = format;

    if (g_lcdc_config.mode.clk[0] != mode->clk[0] || g_lcdc_config.mode.clk[1] != mode->clk[1]) {
        sysreg_clk2_disable(CLK2_LCDCTRL);
        sysreg_clk_select_lcdc(mode->clk[0], mode->clk[1]);
        sysreg_clk2_enable(CLK2_LCDCTRL);
    }

    // copy the mode over to our active state
    g_lcdc_config.mode = *mode;

    if (g_lcdc_config.cycles_per_pixel != 1) {
        g_lcdc_config.mode.x.resolution *= g_lcdc_config.cycles_per_pixel;
        g_lcdc_config.mode.x.sync_width *= g_lcdc_config.cycles_per_pixel;
        g_lcdc_config.mode.x.front_porch *= g_lcdc_config.cycles_per_pixel;
        g_lcdc_config.mode.x.back_porch *= g_lcdc_config.cycles_per_pixel;
    }

    if (g_lcdc_config.mode.sync_diff == 1 && (g_lcdc_config.mode.x.resolution == g_lcdc_config.resolution.x)) {
        // do nothing?
    }

    if (g_lcdc_config.mode.sync_diff == 0) {

    }

    *LCDC_SYNC_DIFF_REG(g_lcdc_config.mmio_base) = g_lcdc_config.mode.sync_diff;
    *LCDC_PIXEL_FORMAT_REG(g_lcdc_config.mmio_base) = g_lcdc_config.pixel_format;
    *LCDC_X_BACK_PORCH_REG(g_lcdc_config.mmio_base) = g_lcdc_config.mode.x.back_porch;
    *LCDC_X_SYNC_WIDTH_REG(g_lcdc_config.mmio_base) = g_lcdc_config.mode.x.sync_width;
    *LCDC_X_FRONT_PORCH_REG(g_lcdc_config.mmio_base) = g_lcdc_config.mode.x.front_porch;
    *LCDC_X_RESOLUTION_REG(g_lcdc_config.mmio_base) = g_lcdc_config.mode.x.resolution;
    *LCDC_Y_BACK_PORCH_REG(g_lcdc_config.mmio_base) = g_lcdc_config.mode.y.back_porch;
    *LCDC_Y_SYNC_WIDTH_REG(g_lcdc_config.mmio_base) = g_lcdc_config.mode.y.sync_width;
    *LCDC_Y_FRONT_PORCH_REG(g_lcdc_config.mmio_base) = g_lcdc_config.mode.y.front_porch;
    *LCDC_Y_RESOLUTION_REG(g_lcdc_config.mmio_base) = g_lcdc_config.mode.y.resolution;
    *LCDC_SHIFT_Y_RES_REG(g_lcdc_config.mmio_base) = g_lcdc_config.shift_resolution.y;
    *LCDC_SHIFT_X_RES_REG(g_lcdc_config.mmio_base) = g_lcdc_config.shift_resolution.x;
    *LCDC_SCALED_X_RES_REG(g_lcdc_config.mmio_base) = g_lcdc_config.resolution.x;
    *LCDC_SCALED_Y_RES_REG(g_lcdc_config.mmio_base) = g_lcdc_config.resolution.y;

    cpu_resume_interrupts_with_sync(mask);
    return 0;
}
