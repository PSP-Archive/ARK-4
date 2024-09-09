/*
 * Copyright (c) 2015 Travis Geiselbrecht
 * Copyright (c) 2023 Davee Morgan
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 */

.macro iframe_save
    .set    push
    .set    noat

    /* these are saved by the exception handler */
    cfc0    $v0, $6
    cfc0    $v1, $7

    /* save all the register states */
    sw      $at, -0x90($sp)
    sw      $v0, -0x8C($sp)
    sw      $v1, -0x88($sp)
    sw      $a0, -0x84($sp)
    sw      $a1, -0x80($sp)
    sw      $a2, -0x7C($sp)
    sw      $a3, -0x78($sp)
    sw      $t0, -0x74($sp)
    sw      $t1, -0x70($sp)
    sw      $t2, -0x6C($sp)
    sw      $t3, -0x68($sp)
    sw      $t4, -0x64($sp)
    sw      $t5, -0x60($sp)
    sw      $t6, -0x5C($sp)
    sw      $t7, -0x58($sp)
    sw      $s0, -0x54($sp)
    sw      $s1, -0x50($sp)
    sw      $s2, -0x4C($sp)
    sw      $s3, -0x48($sp)
    sw      $s4, -0x44($sp)
    sw      $s5, -0x40($sp)
    sw      $s6, -0x3C($sp)
    sw      $s7, -0x38($sp)
    sw      $t8, -0x34($sp)
    sw      $t9, -0x30($sp)
    sw      $k0, -0x2C($sp)
    sw      $k1, -0x28($sp)
    sw      $gp, -0x24($sp)
    sw      $sp, -0x20($sp)
    sw      $fp, -0x1C($sp)
    sw      $ra, -0x18($sp)

    /* save lo and hi registers */
    mflo    $at
    sw      $at, -0x14($sp)
    mfhi    $at
    sw      $at, -0x10($sp)

    /* save the control registers */
    mfc0    $at, $12 /* status */
    sw      $at, -0x0C($sp)
    mfc0    $at, $13 /* cause */
    sw      $at, -0x08($sp)
    mfc0    $at, $14 /* epc */
    sw      $at, -0x04($sp)

    addiu   $sp, $sp, -0x90
    .set    pop
.endm

.macro iframe_restore
    .set    push
    .set    noat

    addiu   $sp, $sp, 0x90

    /* load lo and hi registers */
    lw      $at, -0x14($sp)
    mtlo    $at
    lw      $at, -0x10($sp)
    mthi    $at

    /* load the control registers */
    lw      $at, -0x0C($sp)
    mtc0    $at, $12 /* status */
    lw      $at, -0x08($sp)
    mtc0    $at, $13 /* cause */
    lw      $at, -0x04($sp)
    mtc0    $at, $14 /* epc */

    /* load all the register states */
    lw      $at, -0x90($sp)
    lw      $v0, -0x8C($sp)
    lw      $v1, -0x88($sp)
    lw      $a0, -0x84($sp)
    lw      $a1, -0x80($sp)
    lw      $a2, -0x7C($sp)
    lw      $a3, -0x78($sp)
    lw      $t0, -0x74($sp)
    lw      $t1, -0x70($sp)
    lw      $t2, -0x6C($sp)
    lw      $t3, -0x68($sp)
    lw      $t4, -0x64($sp)
    lw      $t5, -0x60($sp)
    lw      $t6, -0x5C($sp)
    lw      $t7, -0x58($sp)
    lw      $s0, -0x54($sp)
    lw      $s1, -0x50($sp)
    lw      $s2, -0x4C($sp)
    lw      $s3, -0x48($sp)
    lw      $s4, -0x44($sp)
    lw      $s5, -0x40($sp)
    lw      $s6, -0x3C($sp)
    lw      $s7, -0x38($sp)
    lw      $t8, -0x34($sp)
    lw      $t9, -0x30($sp)
    lw      $k0, -0x2C($sp)
    lw      $k1, -0x28($sp)
    lw      $gp, -0x24($sp)
    lw      $sp, -0x20($sp)
    lw      $fp, -0x1C($sp)
    lw      $ra, -0x18($sp)

    .set    pop
.endm

.global _irq
_irq:
    iframe_save

    move    $a0, $sp
    la      $k0, on_irq
    jalr    $k0
    nop

    iframe_restore

    // EPC does not get set properly without these nops
    nop
    nop
    eret
