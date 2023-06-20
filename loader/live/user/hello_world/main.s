	.file	1 "main.c"
	.section .mdebug.eabi32
	.previous
	.section .gcc_compiled_long32
	.previous
	.text
	.align	2
	.globl	InitGlobals
	.set	nomips16
	.ent	InitGlobals
InitGlobals:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	lui	$7,%hi(colours)
	li	$2,8912896			# 0x880000
	addiu	$4,$7,%lo(colours)
	ori	$2,$2,0x8888
	sw	$2,36($4)
	li	$5,9			# 0x9
	lui	$2,%hi(color)
	sw	$5,%lo(color)($2)
	li	$2,255			# 0xff
	li	$6,16711680			# 0xff0000
	li	$3,34816			# 0x8800
	sw	$2,12($4)
	li	$2,65535			# 0xffff
	ori	$11,$6,0xff88
	ori	$8,$6,0xffff
	ori	$9,$6,0xff
	ori	$10,$6,0xff00
	sw	$3,%lo(colours)($7)
	sw	$2,24($4)
	li	$3,65280			# 0xff00
	lui	$2,%hi(i)
	sw	$8,4($4)
	sw	$9,8($4)
	sw	$3,20($4)
	sw	$10,28($4)
	sw	$11,32($4)
	sw	$0,%lo(i)($2)
	sw	$0,40($4)
	j	$31
	sw	$6,16($4)

	.set	macro
	.set	reorder
	.end	InitGlobals
	.size	InitGlobals, .-InitGlobals
	.align	2
	.globl	drawPixel
	.set	nomips16
	.ent	drawPixel
drawPixel:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	lui	$9,%hi(pixel)
	addiu	$2,$9,%lo(pixel)
	move	$3,$4
	move	$10,$5
	addu	$7,$7,$5
	addu	$6,$6,$4
	move	$5,$8
	move	$4,$2
	sw	$3,%lo(pixel)($9)
	sw	$7,12($2)
	sw	$6,4($2)
	j	FillRect
	sw	$10,8($2)

	.set	macro
	.set	reorder
	.end	drawPixel
	.size	drawPixel, .-drawPixel
	.section	.rodata.str1.4,"aMS",@progbits,1
	.align	2
$LC0:
	.ascii	"ms0:/screen.bmp\000"
	.text
	.align	2
	.globl	ProcessKeys
	.set	nomips16
	.ent	ProcessKeys
ProcessKeys:
	.frame	$sp,16,$31		# vars= 0, regs= 4/0, args= 0, gp= 0
	.mask	0x80070000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-16
	andi	$2,$4,0x30
	sw	$18,8($sp)
	sw	$31,12($sp)
	sw	$17,4($sp)
	sw	$16,0($sp)
	beq	$2,$0,$L6
	move	$18,$4

	lui	$4,%hi(color)
	lw	$3,%lo(color)($4)
	slt	$2,$3,10
	bne	$2,$0,$L7
	addiu	$2,$3,1

	sw	$0,%lo(color)($4)
$L6:
	andi	$2,$18,0xc0
	beq	$2,$0,$L18
	ext	$2,$18,16,1

	lui	$3,%hi(color)
	lw	$2,%lo(color)($3)
	bltz	$2,$L16
	addiu	$2,$2,-1

	sw	$2,%lo(color)($3)
$L8:
	ext	$2,$18,16,1
$L18:
	beq	$2,$0,$L19
	andi	$2,$18,0x1000

	lui	$16,%hi(i)
	li	$2,16711680			# 0xff0000
	sw	$0,%lo(i)($16)
	ori	$17,$2,0xffff
$L11:
	jal	Fillvram
	move	$4,$17

	jal	changeBuffer
	nop

	jal	Sleep
	li	$4,10			# 0xa

	lw	$3,%lo(i)($16)
	addiu	$3,$3,1
	slt	$2,$3,278
	bne	$2,$0,$L11
	sw	$3,%lo(i)($16)

	jal	changeBuffer
	nop

	jal	fadeIn
	nop

	jal	fadeOut
	nop

	jal	sceKernelExitGame
	nop

	andi	$2,$18,0x1000
$L19:
	bne	$2,$0,$L17
	lw	$31,12($sp)

	lw	$18,8($sp)
	lw	$17,4($sp)
	lw	$16,0($sp)
	j	$31
	addiu	$sp,$sp,16

$L7:
	j	$L6
	sw	$2,%lo(color)($4)

$L17:
	lui	$4,%hi($LC0)
	lw	$18,8($sp)
	lw	$17,4($sp)
	lw	$16,0($sp)
	addiu	$4,$4,%lo($LC0)
	j	createScreenshot
	addiu	$sp,$sp,16

$L16:
	li	$2,10			# 0xa
	j	$L8
	sw	$2,%lo(color)($3)

	.set	macro
	.set	reorder
	.end	ProcessKeys
	.size	ProcessKeys, .-ProcessKeys
	.section	.rodata.str1.4
	.align	2
$LC1:
	.ascii	" - Hello World! -\000"
	.align	2
$LC2:
	.ascii	"Hello World by Team OILIX\000"
	.section	.text.start,"ax",@progbits
	.align	2
	.globl	_start
	.set	nomips16
	.ent	_start
_start:
	.frame	$sp,24,$31		# vars= 0, regs= 6/0, args= 0, gp= 0
	.mask	0x801f0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$31,20($sp)
	sw	$20,16($sp)
	sw	$19,12($sp)
	sw	$18,8($sp)
	sw	$17,4($sp)
	jal	sceKernelDcacheWritebackAll
	sw	$16,0($sp)

	jal	InitGlobals
	lui	$16,%hi(snowflake)

	jal	initScreenAndCtrl
	addiu	$20,$16,%lo(snowflake)

	jal	fadeIn
	nop

	jal	fadeOut
	nop

	lui	$2,%hi(colours)
	addiu	$18,$2,%lo(colours)
	lw	$6,20($18)
	lui	$7,%hi($LC1)
	addiu	$7,$7,%lo($LC1)
	li	$5,15			# 0xf
	jal	Print
	li	$4,22			# 0x16

	jal	changeBuffer
	nop

	jal	Sleep
	li	$4,1000			# 0x3e8

	jal	changeBuffer
	nop

	jal	fadeIn
	nop

	jal	fadeOut
	nop

	jal	initSnow
	addiu	$4,$16,%lo(snowflake)

	lui	$2,%hi(gpaddata)
	addiu	$17,$2,%lo(gpaddata)
	lui	$2,%hi($LC2)
	addiu	$19,$2,%lo($LC2)
	lui	$16,%hi(color)
	li	$5,1			# 0x1
$L23:
	move	$4,$17
	jal	sceCtrlReadBufferPositive
	sw	$0,4($17)

	jal	ProcessKeys
	lw	$4,4($17)

	jal	Fillvram
	lw	$4,40($18)

	lw	$6,%lo(color)($16)
	move	$4,$20
	jal	updateSnow
	move	$5,$18

	lw	$8,36($18)
	move	$4,$0
	li	$5,247			# 0xf7
	li	$6,480			# 0x1e0
	jal	drawPixel
	li	$7,272			# 0x110

	lw	$6,36($18)
	move	$4,$0
	move	$5,$0
	jal	Print
	move	$7,$19

	jal	changeBuffer
	nop

	j	$L23
	li	$5,1			# 0x1

	.set	macro
	.set	reorder
	.end	_start
	.size	_start, .-_start

	.comm	drawframe,4,4

	.comm	gcursor,8,4

	.comm	gpaddata,16,4

	.comm	Paused,4,4

	.comm	colours,44,4

	.comm	color,4,4

	.comm	i,4,4

	.comm	pixel,16,4

	.comm	snowflake,1200,4
	.ident	"GCC: (GNU) 4.3.2"
