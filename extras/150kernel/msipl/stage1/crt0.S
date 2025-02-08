.set noreorder

.section .text.vector

#include "as_reg_compat.h"

.global _start

.ent _start
_start:
# Clear bss
    la $t3, __bss_start
    la $t4, __bss_end
clear_bss_loop:
    sltu $t5, $t3, $t4
    addiu $t3, $t3, 4
    bnel $t5, $0, clear_bss_loop
    sw $0, -4($t3)
    j entry
    nop
.end _start

.global DcacheClear
.ent    DcacheClear
DcacheClear:
    .word 0x40088000
    .word 0x24090800
    .word 0x7D081180
    .word 0x01094804
    .word 0x00004021
    .word 0xBD100000
    .word 0x400AE000
    .word 0x400BE800
    .word 0x7D4C0500
    .word 0x11800004
    .word 0x7D6C0500
    .word 0x000A5340
    .word 0x01485025
    .word 0xBD5A0000
    .word 0x11800003
    .word 0x000B5B40
    .word 0x01685825
    .word 0xBD7A0000
    .word 0x25080040
    .word 0x1509FFF1
    .word 0x00000000
    .word 0x03E00008
    .word 0x0000000F
.end DcacheClear

.global IcacheClear
.ent    IcacheClear
IcacheClear:
    .word 0x40088000
    .word 0x24091000
    .word 0x7D081240
    .word 0x01094804
    .word 0x4080E000
    .word 0x4080E800
    .word 0x00004021
    .word 0xBD010000
    .word 0xBD030000
    .word 0x25080040
    .word 0x1509FFFC
    .word 0x00000000
    .word 0x03E00008
    .word 0x00000000
.end IcacheClear

