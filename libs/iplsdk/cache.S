.set noreorder
.set noat

#include "cop0.h"

.text
.global cpu_dcache_wb_all
.global cpu_dcache_inv_all
.global cpu_dcache_wb_inv_all
.global cpu_icache_inv_all

#define INSTRUCTION_INDEX_INVALIDATE    (0x04)
#define DATA_LOAD_BOTH_WAYS             (0x10)
#define DATA_INDEX_INVALIDATE1          (0x11)
#define DATA_INDEX_INVALIDATE2          (0x13)
#define DATA_INDEX_WRITEBACK_INVALIDATE (0x14)
#define DATA_HIT_WRITEBACK              (0x1A)

.ent cpu_dcache_wb_all
cpu_dcache_wb_all:
    // disable all interrupts and grab the cache config
    mfic    $v0, $0
    mtic    $0, $0
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
    li      $t6, -1

cpu_dcache_wb_all_clr_line:
    // theres not been a lot of research into what this command does. i (davee)
    // suspect that since the DATA_HIT_WRITEBACK command requires an address in
    // order to perform the command, this instruction loads both data cache tags
    // into TagLo and TagHi.
    cache   DATA_LOAD_BOTH_WAYS, 0($t0)
    mfc0    $t2, CPU_COP0_TAGLO
    mfc0    $t3, CPU_COP0_TAGHI
    move    $t4, $t2
    xor     $t5, $t3, $t6

    // load VTagB. If its all zeroes then switch A and B
    ext     $t5, $t5, 0, 19
    movz    $t2, $t3, $t5
    movz    $t3, $t4, $t5

    // if TagA is not dirty then move onto tag B
    ext     $t4, $t2, 20, 1
    beqz    $t4, cpu_dcache_wb_all_check_tagb
    ext     $t4, $t3, 20, 1
    sll     $t5, $t2, 13
    or      $t5, $t5, $t0
    cache   DATA_HIT_WRITEBACK, 0($t5)
    move    $t6, $t2

cpu_dcache_wb_all_check_tagb:
    // if TagB is not dirty then move onto next cache line
    beqz    $t4, cpu_dcache_wb_all_next
    sll     $t5, $t3, 13
    or      $t5, $t5, $t0
    cache   DATA_HIT_WRITEBACK, 0($t5)
    move    $t6, $t3

cpu_dcache_wb_all_next:
    addiu $t0, $t0, 64
    bne $t0, $t1, cpu_dcache_wb_all_clr_line
    nop
    
    // restore interrupts, sync and return
    mtic    $v0, $0
    jr      $ra
    sync
.end cpu_dcache_wb_all

.ent cpu_dcache_inv_all
cpu_dcache_inv_all:
    // grab the cache config
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

    // disable interrupts and clear cache
    mfic    $v0, $0
    mtic    $0, $0
    mtc0    $0, CPU_COP0_TAGLO
    mtc0    $0, CPU_COP0_TAGHI
    li      $t0, 0

cpu_dcache_inv_all_clr_line:
    addiu $t0, $t0, 64
    cache DATA_INDEX_INVALIDATE1, -64($t0)
    bne $t0, $t1, cpu_dcache_inv_all_exit
    cache DATA_INDEX_INVALIDATE2, -64($t0)

cpu_dcache_inv_all_exit:
    // restore interrupts, sync and return
    mtic    $v0, $0
    jr      $ra
    nop
.end cpu_dcache_inv_all

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
