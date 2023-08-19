.set noreorder
.set noat

#include "cop0.h"

.text
.global cpu_dcache_wb_inv_all
.global cpu_icache_inv_all

#define INSTRUCTION_INDEX_INVALIDATE    (0x04)
#define DATA_INDEX_WRITEBACK_INVALIDATE (0x14)

.ent cpu_dcache_wb_inv_all
cpu_dcache_wb_inv_all:
    mfc0    $t0, CPU_COP0_CONFIG

    // data cache size is defined as
    // 2^(12 + cop0.config.dcache_size)
    // this is equivalent to (1 << 12) << sz
    // so $t1 = (1 << 12) = 0x1000
    li      $t1, 0x1000

    // get data cache size from config reg
    // and left shift to calculate the final
    // data cache size in bytes ($t1)
    ext     $t0, $t0, 6, 3
    sllv    $t1, $t1, $t0
    li      $t0, 0

cpu_dcache_wb_inv_all_clr_line:
    // PSP uses a 64 byte two way set associative cache
    // to clear both lines we must issue the command
    // twice
    cache   DATA_INDEX_WRITEBACK_INVALIDATE, 0($t0)
    cache   DATA_INDEX_WRITEBACK_INVALIDATE, 0($t0)
    addiu   $t0, $t0, 64
    bne     $t0, $t1, cpu_dcache_wb_inv_all_clr_line
    nop

    jr $ra
    sync
.end cpu_dcache_wb_inv_all

.ent cpu_icache_inv_all
cpu_icache_inv_all:
    mfc0    $t0, CPU_COP0_CONFIG

    // instruction cache size is defined as
    // 2^(12 + cop0.config.icache_size)
    // this is equivalent to (1 << 12) << sz
    // so $t1 = (1 << 12) = 0x1000
    li      $t1, 0x1000

    // get instruction cache size from config reg
    // and left shift to calculate the final
    // instruction cache size in bytes ($t1)
    ext     $t0, $t0, 9, 3
    sllv    $t1, $t1, $t0
    li      $t0, 0

cpu_icache_inv_all_clr_line:
    // PSP uses a 64 byte two way set associative cache
    // to clear both lines we must issue the command
    // twice
    cache   INSTRUCTION_INDEX_INVALIDATE, 0($t0)
    cache   INSTRUCTION_INDEX_INVALIDATE, 0($t0)
    addiu   $t0, $t0, 64
    bne     $t0, $t1, cpu_icache_inv_all_clr_line
    nop

    jr $ra
    sync
.end cpu_icache_inv_all