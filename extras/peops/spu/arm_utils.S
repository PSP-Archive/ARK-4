/*
 * (C) Gražvydas "notaz" Ignotas, 2011
 *
 * This work is licensed under the terms of any of these licenses
 * (at your option):
 *  - GNU GPL, version 2 or later.
 *  - GNU LGPL, version 2.1 or later.
 * See the COPYING file in the top-level directory.
 */


.text
.align 2

.macro load_varadr reg var
#if defined(__ARM_ARCH_7A__) && !defined(__PIC__)
	movw	\reg, #:lower16:\var
	movt	\reg, #:upper16:\var
#else
	ldr	\reg, =\var
#endif
.endm

#ifdef __ARM_NEON__

.global mix_chan @ (int start, int count, int lv, int rv)
mix_chan:
    vmov.32     d14[0], r2
    vmov.32     d14[1], r3             @ multipliers
    mov         r12, r0
    load_varadr r0, ChanBuf
    load_varadr r2, SSumLR
    add         r0, r12, lsl #2
    add         r2, r12, lsl #3
0:
    vldmia      r0!, {d0-d1}
    vldmia      r2, {d2-d5}
    vmul.s32    d10, d14, d0[0]
    vmul.s32    d11, d14, d0[1]
    vmul.s32    d12, d14, d1[0]
    vmul.s32    d13, d14, d1[1]
    vsra.s32    q1, q5, #14
    vsra.s32    q2, q6, #14
    subs        r1, #4
    blt         mc_finish
    vstmia      r2!, {d2-d5}
    bgt         0b
    nop
    bxeq        lr

mc_finish:
    vstmia      r2!, {d2}
    cmp         r1, #-2
    vstmiage    r2!, {d3}
    cmp         r1, #-1
    vstmiage    r2!, {d4}
    bx          lr


.global mix_chan_rvb @ (int start, int count, int lv, int rv)
mix_chan_rvb:
    vmov.32     d14[0], r2
    vmov.32     d14[1], r3             @ multipliers
    mov         r12, r0
    load_varadr r0, ChanBuf
    load_varadr r3, sRVBStart
    load_varadr r2, SSumLR
    ldr         r3, [r3]
    add         r0, r12, lsl #2
    add         r2, r12, lsl #3
    add         r3, r12, lsl #3
0:
    vldmia      r0!, {d0-d1}
    vldmia      r2, {d2-d5}
    vldmia      r3, {d6-d9}
    vmul.s32    d10, d14, d0[0]
    vmul.s32    d11, d14, d0[1]
    vmul.s32    d12, d14, d1[0]
    vmul.s32    d13, d14, d1[1]
    vsra.s32    q1, q5, #14
    vsra.s32    q2, q6, #14
    vsra.s32    q3, q5, #14
    vsra.s32    q4, q6, #14
    subs        r1, #4
    blt         mcr_finish
    vstmia      r2!, {d2-d5}
    vstmia      r3!, {d6-d9}
    bgt         0b
    nop
    bxeq        lr

mcr_finish:
    vstmia      r2!, {d2}
    vstmia      r3!, {d6}
    cmp         r1, #-2
    vstmiage    r2!, {d3}
    vstmiage    r3!, {d7}
    cmp         r1, #-1
    vstmiage    r2!, {d4}
    vstmiage    r3!, {d8}
    bx          lr

#else

.global mix_chan @ (int start, int count, int lv, int rv)
mix_chan:
    stmfd       sp!, {r4-r8,lr}
    orr         r3, r2, r3, lsl #16
    lsl         r3, #1                 @ packed multipliers << 1
    mov         r12, r0
    load_varadr r0, ChanBuf
    load_varadr r2, SSumLR
    add         r0, r12, lsl #2
    add         r2, r12, lsl #3
0:
    ldmia       r0!, {r4,r5}
    ldmia       r2, {r6-r8,lr}
    lsl         r4, #1                 @ adjust for mul
    lsl         r5, #1
    smlawb      r6, r4, r3, r6
    smlawt      r7, r4, r3, r7
    smlawb      r8, r5, r3, r8
    smlawt      lr, r5, r3, lr
    subs        r1, #2
    blt         mc_finish
    stmia       r2!, {r6-r8,lr}
    bgt         0b
    ldmeqfd     sp!, {r4-r8,pc}

mc_finish:
    stmia       r2!, {r6,r7}
    ldmfd       sp!, {r4-r8,pc}


.global mix_chan_rvb @ (int start, int count, int lv, int rv)
mix_chan_rvb:
    stmfd       sp!, {r4-r8,lr}
    orr         lr, r2, r3, lsl #16
    lsl         lr, #1
    load_varadr r3, sRVBStart
    load_varadr r2, SSumLR
    load_varadr r4, ChanBuf
    ldr         r3, [r3]
    add         r2, r2, r0, lsl #3
    add         r3, r3, r0, lsl #3
    add         r0, r4, r0, lsl #2
0:
    ldr         r4, [r0], #4
    ldmia       r2, {r6,r7}
    ldmia       r3, {r8,r12}
    lsl         r4, #1
    smlawb      r6, r4, lr, r6         @ supposedly takes single cycle?
    smlawt      r7, r4, lr, r7
    smlawb      r8, r4, lr, r8
    smlawt      r12,r4, lr, r12
    subs        r1, #1
    stmia       r2!, {r6,r7}
    stmia       r3!, {r8,r12}
    bgt         0b
    ldmfd       sp!, {r4-r8,pc}

#endif

@ vim:filetype=armasm
