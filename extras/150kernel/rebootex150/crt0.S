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
    j _arkReboot
    nop
.end _start

.set reorder
