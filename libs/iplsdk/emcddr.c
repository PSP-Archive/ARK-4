#include "emcddr.h"

#include <sysreg.h>
#include <syscon.h>
#include <cpu.h>

#include <stddef.h>
#include <string.h>

#define REG32(addr)                 ((volatile uint32_t *)(addr))
#define EMCDDR_MMIO_BASE            (0xBD000000)

#define EMCDDR_UNK20_REG            REG32(EMCDDR_MMIO_BASE + 0x20)
#define UNK20_BIT16                 (1 << 16)
#define UNK20_BIT15                 (1 << 15)

#define EMCDDR_UNK24_REG            REG32(EMCDDR_MMIO_BASE + 0x24)

#define EMCDDR_POWER_DOWN_CTR_REG   REG32(EMCDDR_MMIO_BASE + 0x2C)

#define EMCDDR_UNK30_REG            REG32(EMCDDR_MMIO_BASE + 0x30)
#define UNK30_BIT5                  (1 << 5)
#define UNK30_BIT4                  (1 << 4)
#define UNK30_BIT0                  (1 << 0)

#define EMCDDR_UNK34_REG            REG32(EMCDDR_MMIO_BASE + 0x34)
#define EMCDDR_UNK38_REG            REG32(EMCDDR_MMIO_BASE + 0x38)
#define EMCDDR_UNK40_REG            REG32(EMCDDR_MMIO_BASE + 0x40)
#define EMCDDR_UNK44_REG            REG32(EMCDDR_MMIO_BASE + 0x44)

extern void emcddr_power_down_counter(int unk);
extern void emcddr_set_params2(int, int);
extern void emcddr_set_params3(int, int);
extern void emcddr_flush(int);

typedef struct
{
    uint32_t unk0;
    uint32_t unk4;
} DdrParams;

static const DdrParams *params_for_type(enum DdrType type)
{
    static const DdrParams g_params_32mb =
    {
        .unk0 = UNK30_BIT4,
        .unk4 = 0
    };

    static const DdrParams g_params_64mb =
    {
        .unk0 = UNK30_BIT0 | UNK30_BIT4,
        .unk4 = 0
    };

    switch (type) {
        case DDR_TYPE_32MB:
            return &g_params_32mb;

        case DDR_TYPE_64MB:
            return &g_params_64mb;
    }

    // TODO: error handling?
    return NULL;
}


static unsigned int g_ddr_self_test[16] __attribute__ ((aligned (64))) = {
    0x00000000, 0xFFFFFFFF, 0x55555555, 0xAAAAAAAA,
    0x00000000, 0xFFFFFFFF, 0x55555555, 0xAAAAAAAA,
    0x00000000, 0xFFFFFFFF, 0x55555555, 0xAAAAAAAA,
    0x00000000, 0xFFFFFFFF, 0x55555555, 0xAAAAAAAA,
};

static void set_ddr_params(enum DdrType type)
{
    const DdrParams *params = params_for_type(type);
    *EMCDDR_UNK30_REG = (*EMCDDR_UNK30_REG & ~(UNK30_BIT5 | UNK30_BIT4 | UNK30_BIT0)) | params->unk0;
    *EMCDDR_UNK38_REG = params->unk4;
    cpu_sync();
}

static void exec_cmd(uint32_t cmd, uint32_t unk, uint32_t param)
{
    while (*EMCDDR_UNK20_REG & UNK20_BIT16);
    *EMCDDR_UNK24_REG = ((cmd & 1) << 10) | (unk << 16) | (param & ~(0x8400));
    cpu_sync();
    *EMCDDR_UNK20_REG = cmd | UNK20_BIT15;
    cpu_sync();
    while (*EMCDDR_UNK20_REG & UNK20_BIT16);
}

static void reset_device(int dev, int unk)
{
    exec_cmd(0x25, 0, 0);
    exec_cmd(0x22, 0, 0);
    exec_cmd(0x22, 0, 0);
    exec_cmd(0x20, 0, (dev << 4) | 1);
    exec_cmd(0x20, 2, unk << 5);
}

static unsigned int get_fuse_id_based_code(void)
{
    // unused
    unsigned int sys_flag = *(volatile unsigned int *)0xBC100040;
    (void)sys_flag;

    unsigned int fuse_config = *(volatile unsigned int *)0xBC100098;

    switch (fuse_config & 0x700) {
        case 0x000:
            return 0x700;
        case 0x100:
            return 0x600;
        case 0x200:
            return 0x500;
        case 0x300:
            return 0x400;
        case 0x400:
            return 0x300;
        case 0x500:
            return 0x200;
        case 0x600:
            return 0x100;
        case 0x700:
            return 0x000;
    }

    return 0;
}

extern unsigned char __emcddr_support_code_start;
extern unsigned char __emcddr_support_code_end;

static void ddr_related_reconfiguration(unsigned int unk)
{
    // no idea, some power feature?
    syscon_ctrl_voltage(1, get_fuse_id_based_code());

    // again not sure. some GE edram settings?
    *(volatile unsigned int *)0xBD500020 = 0x510;

    cpu_dcache_wb_inv_all();

    // protect some memory?
    unsigned int range_prot = *(volatile unsigned int *)0xBC000044;
    *(volatile unsigned int *)0xBC000044 = range_prot & 0xC000ffff;

    // no-op read
    unsigned int unused = *(volatile unsigned int *)0xBC100068;
    (void)unused;

    memcpy((void *)0xBFC00C00, &__emcddr_support_code_start, &__emcddr_support_code_end - &__emcddr_support_code_start);
    cpu_dcache_wb_inv_all();

    unsigned flag = (*(volatile unsigned int *)0xBC10004C & 4) ? 5 : 0xF;
    emcddr_flush(flag);

    typedef void (* DDR_INIT2)(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3, unsigned int t0, unsigned int *t1);
    DDR_INIT2 init2 = (DDR_INIT2)0xBFC00C00;
    init2(unk, 3, -1, 8, 10, &g_ddr_self_test[0]);
    *(volatile unsigned int *)0xBC000044 = range_prot;

    // no-op read
    unsigned int unused2 = *(volatile unsigned int *)0xBC100068;
    (void)unused2;
    cpu_sync();
}

void emcddr_init(enum DdrType type)
{
    sysreg_busclk_enable(BUSCLK_EMCDDR);
    set_ddr_params(type);
    emcddr_set_params2(3, 3);
    emcddr_set_params3(8, 10);
    emcddr_power_down_counter(4);
    reset_device(3, 1);
    ddr_related_reconfiguration(5);
}
