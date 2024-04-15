#include "emcsm.h"
#include "chip.h"
#include "state.h"
#include "ecc.h"
#include "config.h"
#include "scramble.h"

#include <sysreg.h>
#include <cpu.h>
#include <model.h>

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define REG32(addr)                 ((volatile uint32_t *)(addr))
#define EMCSM_MMIO_BASE             (0xBD101000)

#define EMCSM_CONTROL_REG           REG32(EMCSM_MMIO_BASE + 0x000)
#define EMCSM_STATUS_REG            REG32(EMCSM_MMIO_BASE + 0x004)
#define STATUS_READY                (1 << 0)
#define STATUS_WRITE_PROTECT_DISABLE        (1 << 7)

#define EMCSM_CMD_REG               REG32(EMCSM_MMIO_BASE + 0x008)
#define CMD_READ_EXTRA              (0x50)
#define CMD_ERASE_BLOCK_SETUP       (0x60)
#define CMD_READ_STATUS             (0x70)
#define CMD_READ_DEVICE_ID          (0x90)
#define CMD_PAGE_PROGRAM_CONFIRM    (0xD0)
#define CMD_RESET                   (0xFF)

// 0 indicates that the last operation was successful, 1 for failure.
#define STATUS_CMD_OP_FAIL          (1 << 0)
// 0 indicates that writes are allowed, 1 means disallowed (protected)
#define STATUS_CMD_WRITE_PROTECT    (1 << 1)
// 0 indicates that the device is busy, 1 if ready
#define STATUS_CMD_READY            (1 << 7)


#define EMCSM_ADDRESS_REG           REG32(EMCSM_MMIO_BASE + 0x00C)
#define EMCSM_RESET_REG             REG32(EMCSM_MMIO_BASE + 0x014)
#define EMCSM_DMA_ADDRESS_REG       REG32(EMCSM_MMIO_BASE + 0x020)
#define EMCSM_DMA_CONTROL_REG       REG32(EMCSM_MMIO_BASE + 0x024)
#define DMA_CONTROL_STOP            (0 << 0)
#define DMA_CONTROL_START           (1 << 0)
#define DMA_CONTROL_READ_FROM_NAND  (0 << 1)
#define DMA_CONTROL_WRITE_TO_NAND   (1 << 1)
#define DMA_CONTROL_TX_PAGE_DATA    (1 << 8)
#define DMA_CONTROL_TX_SPARE_DATA   (1 << 9)

#define EMCSM_DMA_STATUS_REG        REG32(EMCSM_MMIO_BASE + 0x028)

#define EMCSM_DMA_INTERRUPT_REG     REG32(EMCSM_MMIO_BASE + 0x038)
#define UNK38_BIT0                  (1 << 0)
#define UNK38_INIT_SEQUENCE         (0x303)

#define EMCSM_UNK200_REG            REG32(EMCSM_MMIO_BASE + 0x200)
#define UNK200_START_01G            (0xB040205)
#define UNK200_START_02GPLUS        (0xB060309)

#define EMCSM_SERIAL_DATA_REG       REG32(EMCSM_MMIO_BASE + 0x300)

#define EMCSM_DMA_BUFFER_MMIO_BASE  (0xBFF00000)
#define EMCSM_DMA_BUFFER_ADDR       (EMCSM_DMA_BUFFER_MMIO_BASE + 0x000)
#define EMCSM_DMA_BUFFER_SIZE       (0x200)
#define EMCSM_DMA_ECC_CALC_REG      REG32(EMCSM_DMA_BUFFER_MMIO_BASE + 0x800)
#define EMCSM_DMA_SPARE_ADDR        (EMCSM_DMA_BUFFER_MMIO_BASE + 0x900)
#define EMCSM_DMA_SPARE_SIZE        (0x10)

#define VADDR_TO_PADDR(vaddr)       ((uintptr_t)vaddr & ~CPU_SEGMENT_MASK)
#define MAKE_KSEG0_VADDR(paddr)     ((uintptr_t)paddr | CPU_KSEG0_VADDR)
#define IS_ALIGNED4(x)              (((uintptr_t)x & 0x3) == 0)
#define IS_ALIGNED64(x)             (((uintptr_t)x & 0x3F) == 0)

// flags for read/write
#define USER_ECC_IN_SPARE           (1 << 0)
#define NO_AUTO_USER_ECC            (1 << 4)
#define NO_AUTO_SPARE_ECC           (1 << 5)

#define EMCSM_METADATA_IS_VALID     (0xFF)

static inline int is_write_protected(void)
{
    return (*EMCSM_STATUS_REG & STATUS_WRITE_PROTECT_DISABLE) == 0;
}

static inline int is_ready(void)
{
    return *EMCSM_STATUS_REG & STATUS_READY;
}

static inline void wait_ready(void)
{
    while (!is_ready());
}

void read_id(uint8_t *dst, size_t len)
{
    *EMCSM_CMD_REG = CMD_READ_DEVICE_ID;
    *EMCSM_ADDRESS_REG = 0;

    for (size_t i = 0; i < len; ++i) {
        uint32_t value = *EMCSM_SERIAL_DATA_REG;

        if (dst) {
            dst[i] = (uint8_t)value;
        }
    }

    *EMCSM_RESET_REG = 1;
}

int emcsm_erase_block(size_t ppn)
{
    wait_ready();
    *EMCSM_CMD_REG = CMD_ERASE_BLOCK_SETUP;
    *EMCSM_ADDRESS_REG = ppn << 10;
    *EMCSM_DMA_INTERRUPT_REG &= ~1;
    *EMCSM_CMD_REG = CMD_PAGE_PROGRAM_CONFIRM;

    wait_ready();
    *EMCSM_CMD_REG = CMD_READ_STATUS;
    uint32_t status = *EMCSM_SERIAL_DATA_REG;
    *EMCSM_RESET_REG = 1;

    if ((status & STATUS_CMD_READY) == 0) {
        return -1;
    }

    if (status & STATUS_CMD_OP_FAIL) {
        return -2;
    }

    return 0;
}

int emcsm_erase_block_with_retry(size_t ppn)
{
    // physical page number must be the first in the block
    if (ppn % g_emcsm_state.num_pages_per_block) {
        return -1;
    }

    int res = 0;

    for (size_t i = 0; i < MAX_RETRY; ++i) {
        res = emcsm_erase_block(ppn);

        if (res >= 0) {
            return 0;
        }
    }

    return res;
}

static int dma_start_write(int flags, size_t ppn, const void *user_src, const void *spare_src)
{
    volatile uint32_t *dma_buffer = (volatile uint32_t *)MAKE_KSEG0_VADDR(VADDR_TO_PADDR(EMCSM_DMA_BUFFER_ADDR));
    cpu_dcache_create_dirty_exl_line((uint8_t *)dma_buffer + CPU_DCACHE_LINE_SIZE*0);
    cpu_dcache_create_dirty_exl_line((uint8_t *)dma_buffer + CPU_DCACHE_LINE_SIZE*1);
    cpu_dcache_create_dirty_exl_line((uint8_t *)dma_buffer + CPU_DCACHE_LINE_SIZE*2);
    cpu_dcache_create_dirty_exl_line((uint8_t *)dma_buffer + CPU_DCACHE_LINE_SIZE*3);
    cpu_dcache_create_dirty_exl_line((uint8_t *)dma_buffer + CPU_DCACHE_LINE_SIZE*4);
    cpu_dcache_create_dirty_exl_line((uint8_t *)dma_buffer + CPU_DCACHE_LINE_SIZE*5);
    cpu_dcache_create_dirty_exl_line((uint8_t *)dma_buffer + CPU_DCACHE_LINE_SIZE*6);
    cpu_dcache_create_dirty_exl_line((uint8_t *)dma_buffer + CPU_DCACHE_LINE_SIZE*7);

    if (user_src) {
        if (g_emcsm_state.scramble_code) {
            const uint32_t *user_src32 = (const uint32_t *)user_src;
            scramble(dma_buffer, user_src32, EMCSM_DMA_BUFFER_SIZE, ppn);
        }
        else {
            const uint32_t *user_src32 = (const uint32_t *)user_src;
            for (size_t i = 0; i < EMCSM_DMA_BUFFER_SIZE/4; ++i) {
                dma_buffer[i] = user_src32[i];
            }
        }
    }
    else {
        for (size_t i = 0; i < EMCSM_DMA_BUFFER_SIZE/4; ++i) {
            dma_buffer[i] = -1;
        }
    }

    cpu_dcache_hit_wb_inv_line((uint8_t *)dma_buffer + CPU_DCACHE_LINE_SIZE*0);
    cpu_dcache_hit_wb_inv_line((uint8_t *)dma_buffer + CPU_DCACHE_LINE_SIZE*1);
    cpu_dcache_hit_wb_inv_line((uint8_t *)dma_buffer + CPU_DCACHE_LINE_SIZE*2);
    cpu_dcache_hit_wb_inv_line((uint8_t *)dma_buffer + CPU_DCACHE_LINE_SIZE*3);
    cpu_dcache_hit_wb_inv_line((uint8_t *)dma_buffer + CPU_DCACHE_LINE_SIZE*4);
    cpu_dcache_hit_wb_inv_line((uint8_t *)dma_buffer + CPU_DCACHE_LINE_SIZE*5);
    cpu_dcache_hit_wb_inv_line((uint8_t *)dma_buffer + CPU_DCACHE_LINE_SIZE*6);
    cpu_dcache_hit_wb_inv_line((uint8_t *)dma_buffer + CPU_DCACHE_LINE_SIZE*7);

    if (flags & NO_AUTO_USER_ECC) {
        uint32_t ecc = 0xFFFFFFFF;
        if (flags & USER_ECC_IN_SPARE) {
            uint8_t *spare_src8 = (uint8_t *)spare_src;
            ecc = ((uint32_t)spare_src8[2] << 16) | ((uint32_t)spare_src8[1] << 8) | (uint32_t)spare_src8[0];
        }

        // TODO: constants!
        *EMCSM_DMA_ECC_CALC_REG = ecc;
        *EMCSM_CONTROL_REG &= ~0x20000;
    }
    else {
        *EMCSM_CONTROL_REG |= 0x20000;
    }

    if (spare_src) {
        uint32_t *spare_src32 = (uint32_t *)spare_src;

        // if we have the ECC value we need to adjust our pointer
        // to correctly point at the data sitting beyond the ECC value
        if (flags & USER_ECC_IN_SPARE) {
            spare_src32 = (uint32_t *)((uintptr_t)spare_src + 4);
        }

        uint32_t user_ecc = spare_src32[2];

        // if we don't want ECC to be automatically calculated then we
        // must do it ourselves
        if ((flags & NO_AUTO_SPARE_ECC) == 0) {
            user_ecc = 0xFFFFF000 | calculate_ecc(spare_src32);
        }

        *REG32(EMCSM_DMA_SPARE_ADDR + 0x00) = spare_src32[0];
        *REG32(EMCSM_DMA_SPARE_ADDR + 0x04) = spare_src32[1];
        *REG32(EMCSM_DMA_SPARE_ADDR + 0x08) = user_ecc;
    }
    else {
        *REG32(EMCSM_DMA_SPARE_ADDR + 0x00) = 0xFFFFFFFF;
        *REG32(EMCSM_DMA_SPARE_ADDR + 0x04) = 0xFFFFFFFF;
        *REG32(EMCSM_DMA_SPARE_ADDR + 0x08) = 0xFFFFFFFF;
    }

    *EMCSM_DMA_ADDRESS_REG = ppn << 10;
    *EMCSM_DMA_INTERRUPT_REG &= 0x103;
    *EMCSM_DMA_CONTROL_REG = 0x303;
    return 0;
}

#define READ_ERR_USER_ECC   (1 << 0)
#define READ_ERR_SPARE_ECC  (1 << 1)

static int dma_finish_read(int flags, size_t ppn, void *user_dst, void *spare_dst)
{
    if (user_dst) {
        volatile uint32_t *dma_buffer = (volatile uint32_t *)MAKE_KSEG0_VADDR(VADDR_TO_PADDR(EMCSM_DMA_BUFFER_ADDR));
        cpu_dcache_hit_inv_line((uint8_t *)dma_buffer + CPU_DCACHE_LINE_SIZE*0);
        cpu_dcache_hit_inv_line((uint8_t *)dma_buffer + CPU_DCACHE_LINE_SIZE*1);
        cpu_dcache_hit_inv_line((uint8_t *)dma_buffer + CPU_DCACHE_LINE_SIZE*2);
        cpu_dcache_hit_inv_line((uint8_t *)dma_buffer + CPU_DCACHE_LINE_SIZE*3);
        cpu_dcache_hit_inv_line((uint8_t *)dma_buffer + CPU_DCACHE_LINE_SIZE*4);
        cpu_dcache_hit_inv_line((uint8_t *)dma_buffer + CPU_DCACHE_LINE_SIZE*5);
        cpu_dcache_hit_inv_line((uint8_t *)dma_buffer + CPU_DCACHE_LINE_SIZE*6);
        cpu_dcache_hit_inv_line((uint8_t *)dma_buffer + CPU_DCACHE_LINE_SIZE*7);

        if (IS_ALIGNED64(user_dst)) {
            cpu_dcache_create_dirty_exl_line(user_dst + CPU_DCACHE_LINE_SIZE*0);
        }

        cpu_dcache_create_dirty_exl_line(user_dst + CPU_DCACHE_LINE_SIZE*1);
        cpu_dcache_create_dirty_exl_line(user_dst + CPU_DCACHE_LINE_SIZE*2);
        cpu_dcache_create_dirty_exl_line(user_dst + CPU_DCACHE_LINE_SIZE*3);
        cpu_dcache_create_dirty_exl_line(user_dst + CPU_DCACHE_LINE_SIZE*4);
        cpu_dcache_create_dirty_exl_line(user_dst + CPU_DCACHE_LINE_SIZE*5);
        cpu_dcache_create_dirty_exl_line(user_dst + CPU_DCACHE_LINE_SIZE*6);

        if (IS_ALIGNED64(user_dst)) {
            cpu_dcache_create_dirty_exl_line(user_dst + CPU_DCACHE_LINE_SIZE*7);
        }

        if (g_emcsm_state.scramble_code) {
            descramble(user_dst, EMCSM_DMA_BUFFER_ADDR, EMCSM_DMA_BUFFER_SIZE, ppn);
        }
        else {
            uint32_t *user_dst32 = (uint32_t *)user_dst;
            for (size_t i = 0; i < EMCSM_DMA_BUFFER_SIZE/4; ++i) {
                user_dst32[i] = dma_buffer[i];
            }
        }
    }

    int result = 0;

    if ((flags & NO_AUTO_USER_ECC) == 0 && *EMCSM_DMA_STATUS_REG != 0) {
        result = READ_ERR_USER_ECC;
    } 

    uint32_t calculated_ecc = *EMCSM_DMA_ECC_CALC_REG;
    uint32_t spare_data[2];
    spare_data[0] = *REG32(EMCSM_DMA_SPARE_ADDR + 0x00);
    spare_data[1] = *REG32(EMCSM_DMA_SPARE_ADDR + 0x04);
    uint32_t user_ecc = *REG32(EMCSM_DMA_SPARE_ADDR + 0x08);

    if ((flags & NO_AUTO_SPARE_ECC) == 0) {
        // TODO: add defines for the error types?
        if (correct_ecc(&spare_data, user_ecc & 0xFFF) == ECC_ERR_UNCORRECTABLE) {
            result |= READ_ERR_SPARE_ECC;
        }
    }

    if (spare_dst) {
        uint32_t *spare_dst32 = (uint32_t *)spare_dst;

        if (flags & USER_ECC_IN_SPARE) {
            spare_dst32[0] = calculated_ecc;
            spare_dst32[1] = spare_data[0];
            spare_dst32[2] = spare_data[1];
            spare_dst32[3] = user_ecc;
        }

        else {
            spare_dst32[0] = spare_data[0];
            spare_dst32[1] = spare_data[1];
            spare_dst32[2] = user_ecc;
        }
    }

    return result;
}

static int on_interrupt(void)
{
    // TODO: clearly bits 0-2, and 8-10 are some linked states.
    // explore this more
    uint32_t intr = *EMCSM_DMA_INTERRUPT_REG;
    *EMCSM_DMA_INTERRUPT_REG = (intr & ~3) | ((intr & (intr >> 8)) & 3) | 0x300;
    cpu_sync();

    switch (g_emcsm_state.state) {
        case PROCESS_STATE_READ_READY:
            g_emcsm_state.access_state.status |= dma_finish_read(g_emcsm_state.access_state.flags, g_emcsm_state.access_state.ppn, g_emcsm_state.access_state.user_dst, g_emcsm_state.access_state.spare_dst);

            // TODO: this is the same process as write. refactor?
            g_emcsm_state.access_state.ppn += 1;

            if (g_emcsm_state.access_state.user_dst) {
                g_emcsm_state.access_state.user_dst = (void *)((uintptr_t)g_emcsm_state.access_state.user_dst + EMCSM_PAGE_SIZE);
            }

            if (g_emcsm_state.access_state.spare_dst) {
                // TODO: name these constants!
                if (g_emcsm_state.access_state.flags & USER_ECC_IN_SPARE) {
                    g_emcsm_state.access_state.spare_dst = (void *)((uintptr_t)g_emcsm_state.access_state.spare_dst + 0x10);
                }
                else {
                    g_emcsm_state.access_state.spare_dst = (void *)((uintptr_t)g_emcsm_state.access_state.spare_dst + 0xC);
                }
            }

            g_emcsm_state.access_state.len -= 1;
            if (g_emcsm_state.access_state.len == 0) {
                break;
            }

            // TODO: this pattern shows up a lot. maybe refactor into a new
            // function?
            // TODO: constants!
            if (!is_ready()) {
                *EMCSM_DMA_INTERRUPT_REG &= 0x203;
                g_emcsm_state.state = PROCESS_STATE_READ_BUSY;

                if (!is_ready()) {
                    break;
                }

                *EMCSM_DMA_INTERRUPT_REG = (*EMCSM_DMA_INTERRUPT_REG & ~1) | 0x202;
            }

            // NOTE: purposefully dropping into UNK1 state (since we're already ready)
        
        case PROCESS_STATE_READ_BUSY:
            if (g_emcsm_state.access_state.flags & NO_AUTO_USER_ECC) {
                *EMCSM_CONTROL_REG &= ~0x10000;
            }
            else {
                *EMCSM_CONTROL_REG |= 0x10000;
            }

            g_emcsm_state.state = PROCESS_STATE_READ_READY;
            *EMCSM_DMA_ADDRESS_REG = g_emcsm_state.access_state.ppn << 10;
            *EMCSM_DMA_INTERRUPT_REG &= ~0x103;
            *EMCSM_DMA_CONTROL_REG = DMA_CONTROL_START | DMA_CONTROL_READ_FROM_NAND | DMA_CONTROL_TX_PAGE_DATA | DMA_CONTROL_TX_SPARE_DATA;
            break;

        case PROCESS_STATE_WRITE_READY:
            g_emcsm_state.access_state.status |= *EMCSM_DMA_STATUS_REG ? 1 : 0;
            g_emcsm_state.access_state.ppn += 1;

            if (g_emcsm_state.access_state.user_dst) {
                g_emcsm_state.access_state.user_dst = (void *)((uintptr_t)g_emcsm_state.access_state.user_dst + EMCSM_PAGE_SIZE);
            }

            if (g_emcsm_state.access_state.spare_dst) {
                // TODO: name these constants!
                if (g_emcsm_state.access_state.flags & USER_ECC_IN_SPARE) {
                    g_emcsm_state.access_state.spare_dst = (void *)((uintptr_t)g_emcsm_state.access_state.spare_dst + 0x10);
                }
                else {
                    g_emcsm_state.access_state.spare_dst = (void *)((uintptr_t)g_emcsm_state.access_state.spare_dst + 0xC);
                }
            }

            g_emcsm_state.access_state.len -= 1;
            if (g_emcsm_state.access_state.len == 0) {
                break;
            }

            // TODO: this pattern shows up a lot. maybe refactor into a new
            // function?
            // TODO: constants!
            if (!is_ready()) {
                *EMCSM_DMA_INTERRUPT_REG &= 0x203;
                g_emcsm_state.state = PROCESS_STATE_WRITE_BUSY;

                if (!is_ready()) {
                    break;
                }

                *EMCSM_DMA_INTERRUPT_REG = (*EMCSM_DMA_INTERRUPT_REG & ~1) | 0x202;
            }

            // NOTE: purposely dropping into state 2
        case PROCESS_STATE_WRITE_BUSY:
            g_emcsm_state.state = PROCESS_STATE_WRITE_READY;
            // TODO: error handling??
            dma_start_write(g_emcsm_state.access_state.flags, g_emcsm_state.access_state.ppn, g_emcsm_state.access_state.user_dst, g_emcsm_state.access_state.spare_dst);
            break;

        default:
            // TODO: probably do something?
            break;
    }

    if (g_emcsm_state.access_state.len == 0) {
        g_emcsm_state.state = PROCESS_STATE_INACTIVE;
    }

    return 0;
}

static enum EmcSmError read_access(size_t ppn, void *user, void *spare, size_t len, unsigned int flags)
{
    if (len > EMCSM_NUM_PAGES_PER_BLOCK) {
        return EMCSM_ERR_TOO_MANY_PAGES;
    }

    if ((ppn % EMCSM_NUM_PAGES_PER_BLOCK) + len > EMCSM_NUM_PAGES_PER_BLOCK) {
        return EMCSM_ERR_MULTIPLE_BLOCKS;
    }

    if (!IS_ALIGNED4(user) || !IS_ALIGNED4(spare)) {
        return EMCSM_ERR_UNALIGNED;
    }

    uint32_t mask = cpu_suspend_interrupts();

    g_emcsm_state.access_state.status = 0;
    g_emcsm_state.access_state.len = len;
    g_emcsm_state.access_state.ppn = ppn;
    g_emcsm_state.access_state.user_dst = user;
    g_emcsm_state.access_state.spare_dst = spare;
    g_emcsm_state.access_state.flags = flags;

    if (!is_ready()) {
        // TODO: cleanup unexplained constants
        *EMCSM_DMA_INTERRUPT_REG &= ~0x203;

        if (!is_ready()) {
            g_emcsm_state.state = PROCESS_STATE_READ_BUSY;
            goto poll_state;
        }
        else {
            *EMCSM_DMA_INTERRUPT_REG = (*EMCSM_DMA_INTERRUPT_REG & ~1) | 0x202;
        }
    }

    g_emcsm_state.state = PROCESS_STATE_READ_READY;

    if (flags & NO_AUTO_USER_ECC) {
        *EMCSM_CONTROL_REG &= ~0x10000;
    }
    else {
        *EMCSM_CONTROL_REG |= 0x10000;
    }

    *EMCSM_DMA_ADDRESS_REG = ppn << 10;
    *EMCSM_DMA_INTERRUPT_REG &= ~0x103;
    *EMCSM_DMA_CONTROL_REG = DMA_CONTROL_START | DMA_CONTROL_READ_FROM_NAND | DMA_CONTROL_TX_PAGE_DATA | DMA_CONTROL_TX_SPARE_DATA;

poll_state:
    cpu_resume_interrupts_with_sync(mask);

    while (g_emcsm_state.state != PROCESS_STATE_INACTIVE) {
        uint32_t intr = *EMCSM_DMA_INTERRUPT_REG;

        // presumably this is some completion state check
        // TODO: look into this a bit more
        if (((intr >> 8) & intr & 3) != 0) {
            on_interrupt();
        }
    }

    g_emcsm_state.state = PROCESS_STATE_INACTIVE;

    if (g_emcsm_state.access_state.status & READ_ERR_USER_ECC) {
        return EMCSM_ERR_DATA_ECC;
    }

    if (g_emcsm_state.access_state.status & READ_ERR_SPARE_ECC) {
        return EMCSM_ERR_SPARE_ECC;
    }

    return EMCSM_ERR_NONE;
}

int emcsm_read_pages(size_t ppn, void *user, void *spare, size_t num_pages)
{
    // TODO this is anything but robust error handling
    enum EmcSmError err = read_access(ppn, user, spare, num_pages, 0);
    if (err == EMCSM_ERR_SPARE_ECC && !spare) {
        err = EMCSM_ERR_NONE;
    }

    return emcsm_err_to_int(err);
}

int emcsm_read_pages_raw_all(size_t ppn, void *user, void *spare, size_t num_pages)
{
    return emcsm_err_to_int(read_access(ppn, user, spare, num_pages, USER_ECC_IN_SPARE | NO_AUTO_USER_ECC | NO_AUTO_SPARE_ECC));
}

int emcsm_read_block_with_retry(size_t ppn, void *user, void *spare)
{
    int res = 0;
    for (size_t attempt = 0; attempt < MAX_RETRY; ++attempt) {
        res = emcsm_read_pages(ppn, user, spare, g_emcsm_state.num_pages_per_block);

        if (res >= 0) {
            return emcsm_err_to_int(EMCSM_ERR_NONE);
        }
    }

    return res;
}

int emcsm_read_extra_only(size_t ppn, void *spare, size_t num_pages)
{
    // TODO: error handling
    // TODO: constants
    if (num_pages > 32) {
        return -1;
    }

    // make sure that this range does not exceed a single block
    if ((ppn % 32) + num_pages > 32) {
        return -2;
    }

    if (!spare) {
        return -3;
    }

    uint8_t *spare_dst = (uint8_t *)spare;

    for (size_t i = 0; i < num_pages; ++i) {
        wait_ready();

        *EMCSM_CMD_REG = CMD_READ_EXTRA;
        *EMCSM_ADDRESS_REG = (ppn + i) << 10;

        for (size_t j = 0; j < 4; ++j) {
            uint32_t data = *EMCSM_SERIAL_DATA_REG;
            *spare_dst++ = (uint8_t)(data >> 0);
            *spare_dst++ = (uint8_t)(data >> 8);
            *spare_dst++ = (uint8_t)(data >> 16);
            *spare_dst++ = (uint8_t)(data >> 24);
        }
    }

    *EMCSM_RESET_REG = 1;
    return 0;
}

int emcsm_is_bad_block(size_t ppn)
{
    // physical page provided must be at the start of a block
    if ((ppn % g_emcsm_state.num_pages_per_block) != 0) {
        return -1;
    }

    int result = -2;

    // read the block upto MAX_RETRY count
    for (size_t i = 0; i < MAX_RETRY; ++i) {
        EmcSmBlockMetadataWithEcc spare;
        result = emcsm_read_pages_raw_all(ppn, NULL, &spare, 1);

        if (result >= 0) {
            return spare.metadata.validity != EMCSM_METADATA_IS_VALID;
        }
    }

    return result;
}

static int write_access(size_t ppn, const void *user, const void *spare, size_t num_pages, unsigned int flags)
{
    // TODO: constants
    if (num_pages > 32) {
        return -1;
    }

    // make sure that this range does not exceed a single block
    if ((ppn % 32) + num_pages > 32) {
        return -2;
    }

    if (!IS_ALIGNED4(user) || !IS_ALIGNED4(spare)) {
        return -3;
    }

    if (!user && (flags & NO_AUTO_USER_ECC) == 0) {
        return -4;
    }

    if (!spare && (flags & NO_AUTO_SPARE_ECC) == 0) {
        return -5;
    }

    wait_ready();

    uint32_t mask = cpu_suspend_interrupts();

    g_emcsm_state.access_state.status = 0;
    g_emcsm_state.access_state.len = num_pages;
    g_emcsm_state.access_state.ppn = ppn;
    g_emcsm_state.access_state.user_dst = (void *)user;
    g_emcsm_state.access_state.spare_dst = (void *)spare;
    g_emcsm_state.access_state.flags = flags;

    if (!is_ready()) {
        // TODO: cleanup unexplained constants
        *EMCSM_DMA_INTERRUPT_REG &= ~0x203;

        if (!is_ready()) {
            g_emcsm_state.state = PROCESS_STATE_WRITE_BUSY;
            goto poll_state;
        }
        else {
            *EMCSM_DMA_INTERRUPT_REG = (*EMCSM_DMA_INTERRUPT_REG & ~1) | 0x202;
        }
    }

    g_emcsm_state.state = PROCESS_STATE_WRITE_READY;
    dma_start_write(flags, ppn, user, spare);

poll_state:
    cpu_resume_interrupts_with_sync(mask);

    while (g_emcsm_state.state != PROCESS_STATE_INACTIVE) {
        uint32_t intr = *EMCSM_DMA_INTERRUPT_REG;

        // presumably this is some completion state check
        // TODO: look into this a bit more
        if (((intr >> 8) & intr & 3) != 0) {
            on_interrupt();
        }
    }

    g_emcsm_state.state = PROCESS_STATE_INACTIVE;

    if (g_emcsm_state.access_state.status != 0) {
        return -6;
    }

    return 0;
}

int emcsm_write_pages(size_t ppn, const void *user, const void *spare, size_t num_pages)
{
    unsigned int flags = 0;

    if (!user) {
        flags |= NO_AUTO_USER_ECC;
    }

    if (!spare) {
        flags |= NO_AUTO_SPARE_ECC;
    }

    return write_access(ppn, user, spare, num_pages, flags);
}

int emcsm_write_pages_raw_extra(size_t ppn, const void *user, const void *spare, size_t num_pages)
{
    unsigned int flags = NO_AUTO_SPARE_ECC;

    if (!user) {
        flags |= NO_AUTO_USER_ECC;
    }

    return write_access(ppn, user, spare, num_pages, flags);
}

int emcsm_write_pages_raw_all(size_t ppn, const void *user, const void *spare, size_t num_pages)
{
    return write_access(ppn, user, spare, num_pages, NO_AUTO_SPARE_ECC | NO_AUTO_USER_ECC | USER_ECC_IN_SPARE);
}

int emc_verify_block_with_retry(size_t ppn, const void *user, const void *spare)
{
    static uint8_t page_data[EMCSM_PAGE_SIZE];
    static EmcSmBlockMetadata spare_data;

    for (size_t page_offset = 0; page_offset < g_emcsm_state.num_pages_per_block; ++page_offset) {
        int res = 0;

        for (size_t attempt = 0; attempt < MAX_RETRY; ++attempt) {
            res = emcsm_read_pages(ppn + page_offset, page_data, &spare_data, 1);

            if (res < 0) {
                continue;
            }

            if (user) {
                uint8_t *user8 = (uint8_t *)user + page_offset * g_emcsm_state.page_size;
                if (memcmp(page_data, user8, g_emcsm_state.page_size) != 0) {
                    return -2;
                }
            }

            if (spare) {
                // only check the spare data up to the ECC value
                uint8_t *spare8 = (uint8_t *)spare + page_offset * sizeof(EmcSmBlockMetadata);
                if (memcmp(&spare_data, spare8, sizeof(EmcSmBlockMetadata) - 4) != 0) {
                    return -3;
                }
            }
        }

        if (res < 0) {
            return res;
        }
    }

    return 0;
}

int emcsm_write_block_with_verify(size_t ppn, const void *user, const void *spare)
{
    int res = -1;

    for (size_t attempt = 0; attempt < MAX_RETRY; ++attempt) {
        res = emcsm_erase_block_with_retry(ppn);

        if (res < 0) {
            return res;
        }

        res = emcsm_write_pages(ppn, user, spare, g_emcsm_state.num_pages_per_block);

        if (res < 0) {
            return res;
        }

        res = emc_verify_block_with_retry(ppn, user, spare);

        if (res >= 0) {
            return 0;
        }
    }

    return res;
}

void emcsm_set_write_protect(enum EmcSmWriteProtect wp)
{
    if (wp == EMCSM_DISABLE_WRITE_PROTECT) {
        *EMCSM_STATUS_REG |= STATUS_WRITE_PROTECT_DISABLE;
        while (is_write_protected());
    }
    else {
        *EMCSM_STATUS_REG &= ~STATUS_WRITE_PROTECT_DISABLE;
    }
}

void emcsm_lock(enum EmcSmWriteProtect wp)
{
    if (!g_emcsm_state.is_inited) {
        sysreg_busclk_enable(BUSCLK_EMCSM);
        g_emcsm_state.is_inited = 1;
    }

    emcsm_set_write_protect(wp);
}

void emcsm_unlock(void)
{
    emcsm_set_write_protect(EMCSM_DISABLE_WRITE_PROTECT);
    sysreg_busclk_disable(BUSCLK_EMCSM);
    g_emcsm_state.is_inited = 0;
}

int emcsm_reset(uint32_t *status_out)
{
    *EMCSM_DMA_INTERRUPT_REG &= ~UNK38_BIT0;
    *EMCSM_CMD_REG = CMD_RESET;
    wait_ready();
    *EMCSM_CMD_REG = CMD_READ_STATUS;
    uint32_t status = *EMCSM_SERIAL_DATA_REG;

    if (status_out) {
        *status_out = status;
    }

    *EMCSM_RESET_REG = 1;

    if (status & STATUS_CMD_OP_FAIL) {
        return -1;
    }

    return 0;
}

void emcsm_init(void)
{
    memset(&g_emcsm_state, 0, sizeof(g_emcsm_state));
    g_emcsm_state.state = PROCESS_STATE_INACTIVE;

    sysreg_busclk_enable(BUSCLK_EMCSM);
    sysreg_io_enable(IO_EMCSM);

    *EMCSM_DMA_INTERRUPT_REG = UNK38_INIT_SEQUENCE;

    if (model_get_identity()->model == PSP_MODEL_01G) {
        *EMCSM_UNK200_REG = UNK200_START_01G;
    }
    else {
        *EMCSM_UNK200_REG = UNK200_START_02GPLUS;
    }

    // emcsm_lock(DISABLE_WRITE_PROTECT);

    if (is_ready()) {
        emcsm_reset(NULL);
        detect_chip();
    }

    // emcsm_unlock();
}
