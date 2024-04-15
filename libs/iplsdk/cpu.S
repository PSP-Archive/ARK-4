.set noreorder
.set noat

#include "cop0.h"

.text
.global cpu_enable_interrupts
.global cpu_suspend_interrupts
.global cpu_resume_interrupts
.global cpu_resume_interrupts_with_sync
.global cpu_get_status
.global cpu_set_status

.ent cpu_enable_interrupts
cpu_enable_interrupts:
    li $v0, 1
    mtic $v0, $0
    jr $ra
    nop
.end cpu_enable_interrupts

.ent cpu_suspend_interrupts
cpu_suspend_interrupts:
    mfic    $v0, $0
    mtic    $0, $0
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    jr      $ra
    nop
.end cpu_suspend_interrupts

.ent cpu_resume_interrupts
cpu_resume_interrupts:
    mtic    $a0, $0
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    jr      $ra
    nop
.end cpu_resume_interrupts

.ent cpu_resume_interrupts_with_sync
cpu_resume_interrupts_with_sync:
    sync
    nop
    sync
    nop
    mtic    $a0, $0
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    jr      $ra
    nop
.end cpu_resume_interrupts_with_sync

.ent cpu_get_status
cpu_get_status:
    mfc0 $v0, CPU_COP0_STATUS
    jr $ra
    nop
.end cpu_get_status

.ent cpu_set_status
cpu_set_status:
    mtc0 $a0, CPU_COP0_STATUS
    jr $ra
    nop
.end cpu_set_status
