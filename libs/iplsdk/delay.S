.set noreorder
.set noat

.text
.global util_delay_cpu222_us

.ent util_delay_cpu222_us
util_delay_cpu222_us:
    li $v0, 83 # the main loop is 3 cycles. assume 4ns per cycle
    mult $v0, $a0 # translate micro seconds to number of loops
    mflo $v0

delay_loop:
    bnez $v0, delay_loop
    addiu $v0, $v0, -1

    jr $ra
    nop
.end util_delay_cpu222_us
