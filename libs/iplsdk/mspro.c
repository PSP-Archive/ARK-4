#include "mspro.h"
#include "sysreg.h"

#include <stddef.h>
#include <string.h>

#define REG32(addr)             ((volatile uintptr_t *)(addr))
#define MSPRO_IF_MMIO_BASE      (0xBD200000)

// Command register
#define MS_CR_OFS               (0x030)
#define MSPRO_COMMAND_REG       REG32(MSPRO_IF_MMIO_BASE + MS_CR_OFS)
#define CR_TPC_RD_LDATA         (0x2 << 12)
#define CR_TPC_RD_SDATA         (0x3 << 12)
#define CR_TPC_RD_REG           (0x4 << 12)
#define CR_TPC_GET_INT          (0x7 << 12)
#define CR_TPC_WR_LDATA         (0xD << 12)
#define CR_TPC_WR_SDATA         (0xC << 12)
#define CR_TPC_WR_REG           (0xB << 12)
#define CR_TPC_SET_RW_REG_ADRS  (0x8 << 12)
#define CR_TPC_SET_CMD          (0xE << 12)
#define CR_TPC_EX_SET_CMD       (0x9 << 12)
#define CR_DSZ_MASK             (0x3FF)

// Data register
#define MS_DATA_REG_OFS         (0x034)
#define MSPRO_DATA_REG          REG32(MSPRO_IF_MMIO_BASE + MS_DATA_REG_OFS)
#define DATA_RX_SIZE            (0x08)

// Status register
#define MS_STAT_REG_OFS         (0x038)
#define MSPRO_STATUS_REG        REG32(MSPRO_IF_MMIO_BASE + MS_STAT_REG_OFS)
#define STAT_DRQ                (1 << 14)
#define STAT_MSINT              (1 << 13)
#define STAT_RDY                (1 << 12)
#define STAT_CRC                (1 << 9)
#define STAT_TOE                (1 << 8)
#define STAT_EMP                (1 << 5)
#define STAT_FUL                (1 << 4)
#define STAT_CED                (1 << 3)
#define STAT_ERR                (1 << 2)
#define STAT_BRQ                (1 << 1)
#define STAT_CNK                (1 << 0)

// System register
#define MS_SYS_REG_OFS          (0x03C)
#define MSPRO_SYSTEM_REG        REG32(MSPRO_IF_MMIO_BASE + MS_SYS_REG_OFS)
#define SYS_RST                 (1 << 15)
#define SYS_SRAC                (1 << 14)
#define SYS_INTEN               (1 << 13)
#define SYS_NCRC                (1 << 12)
#define SYS_ICLR                (1 << 11)
#define SYS_MSIEN               (1 << 10)
#define SYS_FCLR                (1 << 9)
#define SYS_FDIR                (1 << 8)
#define SYS_DAM                 (1 << 7)
#define SYS_DRM                 (1 << 6)
#define SYS_DRSL                (1 << 5)
#define SYS_REI                 (1 << 4)
#define SYS_REO                 (1 << 3)
#define SYS_BSY_MASK            (0b111 << 0)

// EX_SET_CMD command list
#define MS_EX_SET_CMD_DATA_SIZE (0x07)
#define EX_SET_CMD_READ_DATA    (0x20)
#define EX_SET_CMD_WRITE_DATA   (0x21)
#define EX_SET_CMD_READ_INFO    (0x22)
#define EX_SET_CMD_WRITE_INFO   (0x23)
#define EX_SET_CMD_READ_ATRB    (0x24)
#define EX_SET_CMD_STOP         (0x25)
#define EX_SET_CMD_ERASE        (0x26)
#define EX_SET_CMD_CHG_CLASS    (0x27)
#define EX_SET_CMD_FORMAT       (0x10)
#define EX_SET_CMD_SLEEP        (0x11)

// Interrupt registers
#define INT_REG_CED             (0x80)
#define INT_REG_ERR             (0x40)
#define INT_REG_BREQ            (0x20)
#define INT_REG_CMDNK           (0x01)

static int is_ready(void)
{
    return (*MSPRO_STATUS_REG & STAT_RDY) != 0;
}

static int is_fifo_empty(void)
{
    return (*MSPRO_STATUS_REG & STAT_EMP) != 0;
}

static int is_dma_requested(void)
{
    return (*MSPRO_STATUS_REG & STAT_DRQ) != 0;
}

static int is_interrupt_activated(void)
{
    return (*MSPRO_STATUS_REG & STAT_MSINT) != 0;
}

static void mspro_wait_for_ready(void)
{
    while (!is_ready());
}

static void mspro_wait_for_empty(void)
{
    while(!is_fifo_empty());
}

static void mspro_wait_for_dma_req(void)
{
    while(!is_dma_requested());
}

static void send_tpc(uint32_t tpc, size_t size)
{
    mspro_wait_for_ready();
    mspro_wait_for_empty();
    *MSPRO_COMMAND_REG = tpc | (size & CR_DSZ_MASK);
}

static void read_data(void *dst, size_t len)
{
    size_t num_rx = (len / DATA_RX_SIZE) + ((len % DATA_RX_SIZE) != 0 ? (1) : (0));

    for (size_t i = 0; i < num_rx; ++i)
    {
        mspro_wait_for_dma_req();

        for (unsigned int j = 0; j < 2; ++j)
        {
            size_t copylen = len > 4 ? 4 : len;
            uint32_t data = *MSPRO_DATA_REG;

            if (copylen > 0) {
                memcpy(dst, &data, copylen);
                len -= copylen;
                dst += copylen;
            }
        }
    }
}

static void write_data(const void *src, size_t len)
{
    while (len > 0)
    {
        uint32_t data[DATA_RX_SIZE/sizeof(*MSPRO_DATA_REG)] = { 0, 0 };

        for (unsigned int i = 0; i < sizeof(data)/sizeof(*data); ++i)
        {
            size_t copylen = len > sizeof(*MSPRO_DATA_REG) ? sizeof(*MSPRO_DATA_REG) : len;

            if (copylen > 0) {
                memcpy(&data[i], src, copylen);
                len -= copylen;
                src = (void *)((uintptr_t)src + copylen);
            }
        }

        mspro_wait_for_dma_req();
        *MSPRO_DATA_REG = data[0];
        *MSPRO_DATA_REG = data[1];
    }
}

static int mspro_tpc_get_int(void)
{
    unsigned char int_val = 0;
    send_tpc(CR_TPC_GET_INT, sizeof(int_val));
    read_data(&int_val, sizeof(int_val));
    mspro_wait_for_ready();
    return int_val;
}

static void mspro_tpc_set_rw_reg_adrs(uint32_t reg_addr)
{
    send_tpc(CR_TPC_SET_RW_REG_ADRS, sizeof(reg_addr));
    write_data(&reg_addr, sizeof(reg_addr));
    mspro_wait_for_ready();
}

static void mspro_tpc_ex_set_cmd(uint32_t excmd, uint32_t start_sector, uint16_t num_sector)
{
    uint8_t ex_cmd_data[MS_EX_SET_CMD_DATA_SIZE];

    // this command expects start_sector and num_sector as big endian
    ex_cmd_data[0] = excmd;
    ex_cmd_data[1] = (uint8_t)(num_sector >> 8);
    ex_cmd_data[2] = (uint8_t)(num_sector >> 0);
    ex_cmd_data[3] = (uint8_t)(start_sector >> 24);
    ex_cmd_data[4] = (uint8_t)(start_sector >> 16);
    ex_cmd_data[5] = (uint8_t)(start_sector >> 8);
    ex_cmd_data[6] = (uint8_t)(start_sector >> 0);

    // send the data to the MS controller and await completion
    send_tpc(CR_TPC_EX_SET_CMD, sizeof(ex_cmd_data));
    write_data(&ex_cmd_data, sizeof(ex_cmd_data));
    mspro_wait_for_ready();
}

static void mspro_tpc_read_long_data(void *dst)
{
    send_tpc(CR_TPC_RD_LDATA, MS_SECTOR_SIZE);
    read_data(dst, MS_SECTOR_SIZE);
    mspro_wait_for_ready();
}

static void mspro_tpc_write_long_data(const void *src)
{
    send_tpc(CR_TPC_WR_LDATA, MS_SECTOR_SIZE);
    write_data(src, MS_SECTOR_SIZE);
    mspro_wait_for_ready();
}

static void mspro_reset(void)
{
    // reset the controller. it will automatically
    // clear after reset is completed
    *MSPRO_SYSTEM_REG = SYS_RST;
    while ((*MSPRO_SYSTEM_REG & SYS_RST) != 0);
}

int mspro_init(void)
{
    // enable the memory card hardware
    sysreg_reset_enable(RESET_MSPRO0);
    sysreg_busclk_enable(BUSCLK_MSPRO0);
    sysreg_clk1_enable(CLK1_MSPRO0);
    sysreg_io_enable(IO_MSIF0);
    sysreg_reset_disable(RESET_MSPRO0);

    // reset the controller
    mspro_reset();
    mspro_wait_for_ready();
    return 0;
}

static int wait_interrupt_satifised(uint32_t interrupt)
{
    // just poll the device until the interrupt is activated
    while (!is_interrupt_activated());

    int intreg = 0;
    
    // get the interrupt 
    do {
        intreg = mspro_tpc_get_int();

        // we couldn't read the register interrupt so cannot
        // proceed any further. the caller can decide on an
        // action to take
        if (intreg < 0) {
            return intreg;
        }
    } while ((intreg & interrupt) == 0);

    // if the error bit is set then something went wrong
    // and we cannot continue
    if (intreg & INT_REG_ERR) {
        return -1;
    }

    return 0;
}

int mspro_read_sector(uint32_t sector, void *data)
{
    // issue a command for the memory card interface to read a sector.
    // we don't have a fancy driver here so serial only.
    mspro_tpc_ex_set_cmd(EX_SET_CMD_READ_DATA, sector, 1);

    // wait for the data buffer request to be satifised
    int res = wait_interrupt_satifised(INT_REG_BREQ);

    // if there is an error requesting the sector then we cannot continue
    if (res < 0) {
        return res;
    }

    // read the sector data into the provided buffer
    mspro_tpc_read_long_data(data);

    // wait for the command to complete
    res = wait_interrupt_satifised(INT_REG_CED);

    // if there is an error reading the sector then we cannot continue
    if (res < 0) {
        return res;
    }

    return 0;
}

int mspro_write_sector(uint32_t sector, const void *data)
{
    // issue a command for the memory card interface to read a sector.
    // we don't have a fancy driver here so serial only.
    mspro_tpc_ex_set_cmd(EX_SET_CMD_WRITE_DATA, sector, 1);
    
    // wait for the data buffer request to be satifised
    int res = wait_interrupt_satifised(INT_REG_BREQ);

    // if there is an error requesting the sector then we cannot continue
    if (res < 0) {
        return res;
    }

    // write the sector data to the device
    mspro_tpc_write_long_data(data);

    // wait for the command to complete
    res = wait_interrupt_satifised(INT_REG_CED);

    // if there is an error reading the sector then we cannot continue
    if (res < 0) {
        return res;
    }

    return 0;
}
