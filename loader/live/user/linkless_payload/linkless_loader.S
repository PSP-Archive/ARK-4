	.set	noreorder

	b	ark_loader
	nop

findSavePath:
	addiu	$sp,$sp,-40
	li	$2,142606336			# 0x8800000
	li	$3,167706624			# 0x9ff0000
	sw	$fp,36($sp)
	sw	$23,32($sp)
	sw	$22,28($sp)
	sw	$21,24($sp)
	sw	$20,20($sp)
	sw	$19,16($sp)
	sw	$18,12($sp)
	sw	$17,8($sp)
	sw	$16,4($sp)
	ori	$5,$2,0x1
	ori	$3,$3,0xff01
	li	$8,18			# 0x12
	li	$9,47			# 0x2f
	li	$11,110			# 0x6e
	li	$12,78			# 0x4e
	li	$13,105			# 0x69
	li	$14,73			# 0x49
	li	$15,98			# 0x62
	li	$24,66			# 0x42
	li	$25,46			# 0x2e
	li	$16,14			# 0xe
	li	$17,104			# 0x68
	li	$18,72			# 0x48
	li	$10,15			# 0xf
	li	$19,97			# 0x61
	li	$20,65			# 0x41
	li	$21,116			# 0x74
	li	$22,84			# 0x54
	li	$23,100			# 0x64
	b	$L53
	li	$fp,68			# 0x44

$L60:
	li	$2,77			# 0x4d
	beql	$4,$2,$L82
	lb	$4,0($5)

	addiu	$5,$5,1
$L83:
	beq	$5,$3,$L52
	move	$6,$0

$L53:
	lb	$4,-1($5)
$L89:
	addiu	$2,$4,-65
	andi	$2,$2,0x00ff
	sltu	$2,$2,26
	bne	$2,$0,$L60
	addiu	$6,$5,-1

	li	$2,109			# 0x6d
	bnel	$4,$2,$L83
	addiu	$5,$5,1

	lb	$4,0($5)
$L82:
	addiu	$2,$4,-65
	andi	$2,$2,0x00ff
	sltu	$2,$2,26
	beq	$2,$0,$L5
	li	$2,115			# 0x73

	li	$2,83			# 0x53
	bnel	$4,$2,$L83
	addiu	$5,$5,1

	lb	$4,1($5)
	addiu	$2,$4,-65
	andi	$2,$2,0x00ff
	sltu	$2,$2,26
	beq	$2,$0,$L84
	li	$2,48			# 0x30

	li	$2,16			# 0x10
$L90:
	bnel	$4,$2,$L83
	addiu	$5,$5,1

	lb	$4,2($5)
	addiu	$2,$4,-65
	andi	$2,$2,0x00ff
	sltu	$2,$2,26
	beq	$2,$0,$L85
	li	$2,58			# 0x3a

	li	$2,26			# 0x1a
$L91:
	bnel	$4,$2,$L83
	addiu	$5,$5,1

	lb	$4,3($5)
	addiu	$2,$4,-65
	andi	$2,$2,0x00ff
	sltu	$2,$2,26
	beq	$2,$0,$L11
	nop

$L63:
	bnel	$4,$10,$L83
	addiu	$5,$5,1

	lb	$4,4($5)
	addiu	$2,$4,-65
	andi	$2,$2,0x00ff
	sltu	$2,$2,26
	beq	$2,$0,$L86
	li	$2,112			# 0x70

	li	$2,80			# 0x50
$L65:
	bnel	$4,$2,$L83
	addiu	$5,$5,1

	lb	$4,5($5)
$L88:
	addiu	$2,$4,-65
	andi	$2,$2,0x00ff
	sltu	$2,$2,26
	beq	$2,$0,$L15
	li	$2,115			# 0x73

	li	$2,83			# 0x53
	bnel	$4,$2,$L83
	addiu	$5,$5,1

	lb	$4,6($5)
$L67:
	addiu	$2,$4,-65
	andi	$2,$2,0x00ff
	sltu	$2,$2,26
	beq	$2,$0,$L17
	li	$2,112			# 0x70

	li	$2,80			# 0x50
	bnel	$4,$2,$L83
	addiu	$5,$5,1

	lb	$4,7($5)
$L66:
	addiu	$2,$4,-65
	andi	$2,$2,0x00ff
	sltu	$2,$2,26
	beq	$2,$0,$L19
	nop

	bnel	$4,$10,$L83
	addiu	$5,$5,1

	lb	$4,8($5)
$L69:
	addiu	$2,$4,-65
	andi	$2,$2,0x00ff
	sltu	$2,$2,26
	beq	$2,$0,$L21
	li	$2,115			# 0x73

	li	$2,83			# 0x53
	bnel	$4,$2,$L83
	addiu	$5,$5,1

	lb	$4,9($5)
$L68:
	addiu	$2,$4,-65
	andi	$2,$2,0x00ff
	sltu	$2,$2,26
	beq	$2,$0,$L23
	nop

	bnel	$4,$20,$L83
	addiu	$5,$5,1

	lb	$4,10($5)
$L70:
	addiu	$2,$4,-65
	andi	$2,$2,0x00ff
	sltu	$2,$2,26
	beq	$2,$0,$L25
	li	$2,118			# 0x76

	li	$2,86			# 0x56
	bnel	$4,$2,$L83
	addiu	$5,$5,1

	lb	$4,11($5)
$L71:
	addiu	$2,$4,-65
	andi	$2,$2,0x00ff
	sltu	$2,$2,26
	beq	$2,$0,$L27
	li	$2,101			# 0x65

	li	$2,69			# 0x45
	bnel	$4,$2,$L83
	addiu	$5,$5,1

	lb	$4,12($5)
$L72:
	addiu	$2,$4,-65
	andi	$2,$2,0x00ff
	sltu	$2,$2,26
	beq	$2,$0,$L29
	nop

	bnel	$4,$fp,$L83
	addiu	$5,$5,1

	lb	$4,13($5)
$L73:
	addiu	$2,$4,-65
	andi	$2,$2,0x00ff
	sltu	$2,$2,26
	beq	$2,$0,$L31
	nop

	bnel	$4,$20,$L83
	addiu	$5,$5,1

	lb	$4,14($5)
$L74:
	addiu	$2,$4,-65
	andi	$2,$2,0x00ff
	sltu	$2,$2,26
	beq	$2,$0,$L33
	nop

	bnel	$4,$22,$L83
	addiu	$5,$5,1

	lb	$4,15($5)
$L81:
	addiu	$2,$4,-65
	andi	$2,$2,0x00ff
	sltu	$2,$2,26
	beq	$2,$0,$L35
	nop

	bnel	$4,$20,$L83
	addiu	$5,$5,1

	lb	$4,16($5)
$L80:
	addiu	$2,$4,-65
	andi	$2,$2,0x00ff
	sltu	$2,$2,26
	beq	$2,$0,$L37
	nop

	bnel	$4,$10,$L83
	addiu	$5,$5,1

	addu	$4,$6,$8
$L75:
	lb	$2,0($4)
	beql	$2,$0,$L87
	addiu	$8,$8,-5

$L41:
	lb	$2,1($4)
	addiu	$8,$8,1
	bne	$2,$0,$L41
	addiu	$4,$4,1

	addiu	$8,$8,-5
$L87:
	addu	$7,$6,$8
	lb	$4,0($7)
	addiu	$2,$4,-65
	andi	$2,$2,0x00ff
	sltu	$2,$2,26
	beq	$2,$0,$L42
	nop

	bnel	$4,$18,$L83
	addiu	$5,$5,1

	lb	$4,1($7)
$L79:
	addiu	$2,$4,-65
	andi	$2,$2,0x00ff
	sltu	$2,$2,26
	beq	$2,$0,$L44
	nop

	bnel	$4,$16,$L83
	addiu	$5,$5,1

	lb	$4,2($7)
$L78:
	addiu	$2,$4,-65
	andi	$2,$2,0x00ff
	sltu	$2,$2,26
	beq	$2,$0,$L46
	nop

	bnel	$4,$24,$L83
	addiu	$5,$5,1

	lb	$4,3($7)
$L77:
	addiu	$2,$4,-65
	andi	$2,$2,0x00ff
	sltu	$2,$2,26
	beq	$2,$0,$L48
	nop

	bnel	$4,$14,$L83
	addiu	$5,$5,1

	lb	$4,4($7)
$L76:
	addiu	$2,$4,-65
	andi	$2,$2,0x00ff
	sltu	$2,$2,26
	beq	$2,$0,$L50
	nop

	bnel	$4,$12,$L83
	addiu	$5,$5,1

	b	$L52
	sb	$0,0($7)

$L13:
$L86:
	beql	$4,$2,$L88
	lb	$4,5($5)

	addiu	$5,$5,1
	bnel	$5,$3,$L89
	lb	$4,-1($5)

	move	$6,$0
$L52:
	move	$2,$6
	lw	$fp,36($sp)
	lw	$23,32($sp)
	lw	$22,28($sp)
	lw	$21,24($sp)
	lw	$20,20($sp)
	lw	$19,16($sp)
	lw	$18,12($sp)
	lw	$17,8($sp)
	lw	$16,4($sp)
	j	$31
	addiu	$sp,$sp,40

$L5:
	bnel	$4,$2,$L83
	addiu	$5,$5,1

	lb	$4,1($5)
	addiu	$2,$4,-65
	andi	$2,$2,0x00ff
	sltu	$2,$2,26
	bne	$2,$0,$L90
	li	$2,16			# 0x10

	li	$2,48			# 0x30
$L84:
	bnel	$4,$2,$L83
	addiu	$5,$5,1

	lb	$4,2($5)
	addiu	$2,$4,-65
	andi	$2,$2,0x00ff
	sltu	$2,$2,26
	bne	$2,$0,$L91
	li	$2,26			# 0x1a

	li	$2,58			# 0x3a
$L85:
	bnel	$4,$2,$L83
	addiu	$5,$5,1

	lb	$4,3($5)
	addiu	$2,$4,-65
	andi	$2,$2,0x00ff
	sltu	$2,$2,26
	bne	$2,$0,$L63
	nop

$L11:
	bnel	$4,$9,$L83
	addiu	$5,$5,1

	lb	$4,4($5)
	addiu	$2,$4,-65
	andi	$2,$2,0x00ff
	sltu	$2,$2,26
	beql	$2,$0,$L13
	li	$2,112			# 0x70

	b	$L65
	li	$2,80			# 0x50

$L17:
	bnel	$4,$2,$L83
	addiu	$5,$5,1

	b	$L66
	lb	$4,7($5)

$L15:
	bnel	$4,$2,$L83
	addiu	$5,$5,1

	b	$L67
	lb	$4,6($5)

$L21:
	bnel	$4,$2,$L83
	addiu	$5,$5,1

	b	$L68
	lb	$4,9($5)

$L19:
	bnel	$4,$9,$L83
	addiu	$5,$5,1

	b	$L69
	lb	$4,8($5)

$L23:
	bnel	$4,$19,$L83
	addiu	$5,$5,1

	b	$L70
	lb	$4,10($5)

$L25:
	bnel	$4,$2,$L83
	addiu	$5,$5,1

	b	$L71
	lb	$4,11($5)

$L27:
	bnel	$4,$2,$L83
	addiu	$5,$5,1

	b	$L72
	lb	$4,12($5)

$L29:
	bnel	$4,$23,$L83
	addiu	$5,$5,1

	b	$L73
	lb	$4,13($5)

$L31:
	bnel	$4,$19,$L83
	addiu	$5,$5,1

	b	$L74
	lb	$4,14($5)

$L37:
	bnel	$4,$9,$L83
	addiu	$5,$5,1

	b	$L75
	addu	$4,$6,$8

$L50:
	bnel	$4,$11,$L83
	addiu	$5,$5,1

	b	$L52
	sb	$0,0($7)

$L48:
	bnel	$4,$13,$L83
	addiu	$5,$5,1

	b	$L76
	lb	$4,4($7)

$L46:
	bnel	$4,$15,$L83
	addiu	$5,$5,1

	b	$L77
	lb	$4,3($7)

$L44:
	bnel	$4,$25,$L83
	addiu	$5,$5,1

	b	$L78
	lb	$4,2($7)

$L42:
	bnel	$4,$17,$L83
	addiu	$5,$5,1

	b	$L79
	lb	$4,1($7)

$L35:
	bnel	$4,$19,$L83
	addiu	$5,$5,1

	b	$L80
	lb	$4,16($5)

$L33:
	bnel	$4,$21,$L83
	addiu	$5,$5,1

	b	$L81
	lb	$4,15($5)

FindImport:
	li	$3,142606336			# 0x8800000
	li	$12,167772160			# 0xa000000
	li	$2,138412032			# 0x8400000
	move	$9,$3
	movn	$9,$2,$6
	move	$10,$4
	movn	$12,$3,$6
	li	$25,-142606336			# 0xfffffffff7800000
	li	$24,25165824			# 0x1800000
	li	$15,-138412032			# 0xfffffffff7c00000
	li	$14,4194304			# 0x400000
	lw	$8,0($9)
$L146:
	beql	$8,$10,$L145
	addiu	$9,$9,4

	beq	$6,$0,$L119
	addu	$2,$8,$25

	addu	$2,$8,$15
	sltu	$2,$2,$14
$L120:
	beql	$2,$0,$L145
	addiu	$9,$9,4

	beq	$6,$0,$L121
	lw	$11,12($9)

	addu	$2,$11,$15
	sltu	$2,$2,$14
$L122:
	beql	$2,$0,$L145
	addiu	$9,$9,4

	beq	$6,$0,$L123
	lw	$13,16($9)

	addu	$2,$13,$15
	sltu	$2,$2,$14
$L124:
	beql	$2,$0,$L145
	addiu	$9,$9,4

	lbu	$2,0($10)
	lbu	$3,0($8)
	bnel	$3,$2,$L145
	addiu	$9,$9,4

	bne	$3,$0,$L141
	move	$7,$0

	lhu	$7,10($9)
$L144:
	beql	$7,$0,$L145
	addiu	$9,$9,4

	lw	$2,0($11)
	beq	$2,$5,$L142
	move	$4,$11

	b	$L131
	move	$3,$0

$L133:
	beq	$2,$5,$L143
	addiu	$4,$4,4

$L131:
	addiu	$3,$3,1
	slt	$2,$3,$7
	bnel	$2,$0,$L133
	lw	$2,4($4)

$L118:
	addiu	$9,$9,4
$L145:
	sltu	$2,$9,$12
	bnel	$2,$0,$L146
	lw	$8,0($9)

	j	$31
	move	$2,$0

$L119:
	b	$L120
	sltu	$2,$2,$24

$L121:
	addu	$2,$11,$25
	b	$L122
	sltu	$2,$2,$24

$L123:
	addu	$2,$13,$25
	b	$L124
	sltu	$2,$2,$24

$L141:
	addu	$2,$10,$7
$L147:
	addu	$3,$8,$7
	lbu	$3,1($3)
	lbu	$4,1($2)
	bne	$3,$4,$L118
	addiu	$7,$7,1

	bne	$3,$0,$L147
	addu	$2,$10,$7

	b	$L144
	lhu	$7,10($9)

$L143:
	sll	$2,$3,3
$L129:
	j	$31
	addu	$2,$13,$2

$L142:
	b	$L129
	move	$2,$0

ark_loader:
	addiu	$sp,$sp,-16416
	sw	$20,16404($sp)
	sw	$31,16412($sp)
	sw	$21,16408($sp)
	sw	$19,16400($sp)
	sw	$18,16396($sp)
	sw	$17,16392($sp)
	bal	findSavePath
	sw	$16,16388($sp)
	lui	$v1,0x0880
	ori	$v1,$v1,0x3800
	move	$20,$v1
	move	$a0,$2
	move	$2,$v1

$move_path:
	lb	$a1,0($a0)
	sb	$a1,0($v1)
	addiu	$a0,$a0,1
	bnez	$a1,$move_path
	addiu	$v1,$v1,1
	lb	$2,0($2)
	beq	$2,$0,$L150
	move	$19,$20

	addiu	$19,$19,1
$L164:
	lb	$2,0($19)
	bnel	$2,$0,$L164
	addiu	$19,$19,1

$L150:
	bal	$LOAD_FILE
	nop
	move	$2,$19
$storepath:
	lb	$3,0($ra)
	sb	$3,0($2)
	addiu	$ra,$ra,1
	bnez	$3,$storepath
	addiu	$2,$2,1
	bal	$LC0
	nop
	move	$16,$ra
	li	$5,278855680			# 0x109f0000
	move	$4,$16
	ori	$5,$5,0x50bc
	bal	FindImport
	move	$6,$0

	li	$5,1784872960			# 0x6a630000
	move	$4,$16
	ori	$5,$5,0x8d83
	move	$6,$0
	bal	FindImport
	move	$17,$2

	li	$5,-2129920000			# 0xffffffff810c0000
	move	$4,$16
	ori	$5,$5,0x4bc3
	move	$6,$0
	bal	FindImport
	move	$18,$2

	move	$4,$20
	li	$5,1			# 0x1
	li	$6,511			# 0x1ff
	jalr	$17
	move	$21,$2

	move	$4,$2
	move	$5,$sp
	bal	$LOAD_SIZE
	nop
	lw	$6,0($ra)
	sb	$0,0($19)
	jalr	$18
	move	$16,$2

	move	$5,$0
	bal	$LOAD_ADDRESS
	nop
	lw	$7,0($ra)
	bal	$LOAD_SIZE
	nop
	lw	$6,0($ra)
$L152:
	addu	$2,$sp,$5
	lbu	$4,0($2)
	addu	$3,$5,$7
	addiu	$5,$5,1
	bne	$5,$6,$L152
	sb	$4,0($3)

	jalr	$21
	move	$4,$16

	bal	$LC1
	nop
	move	$16,$ra
	li	$5,2043740160			# 0x79d10000
	ori	$5,$5,0xc3fa
	move	$4,$16
	bal	FindImport
	move	$6,$0

	beq	$2,$0,$L161
	li	$5,-1271595008			# 0xffffffffb4350000

$L153:
	jalr	$2
	nop

$L163:
	bal	$LOAD_ADDRESS
	nop
	lw	$2,0($ra)
	jalr	$2
	move	$4,$20

	lw	$31,16412($sp)
	lw	$21,16408($sp)
	lw	$20,16404($sp)
	lw	$19,16400($sp)
	lw	$18,16396($sp)
	lw	$17,16392($sp)
	lw	$16,16388($sp)
	j	$31
	addiu	$sp,$sp,16416

$L161:
	ori	$5,$5,0xdec5
	move	$4,$16
	bal	FindImport
	move	$6,$0

	bne	$2,$0,$L153
	li	$5,884539392			# 0x34b90000

	ori	$5,$5,0xfa9e
	move	$4,$16
	bal	FindImport
	move	$6,$0

	beq	$2,$0,$L162
	li	$5,1055064064			# 0x3ee30000

	bal	$LOAD_ADDRESS
	nop
	lw	$4,0($ra)
$L165:
	bal	$LOAD_SIZE
	nop
	jalr	$2
	lw	$5,0($ra)

	b	$L163
	nop

$L162:
	move	$4,$16
	ori	$5,$5,0x821
	bal	FindImport
	move	$6,$0

	bne	$2,$0,$L165
	bal	$LOAD_ADDRESS
	nop
	lw	$4,0($ra)

	b	$L163
	nop

$LC0:
	move	$16,$ra
	jalr	$16
	nop
	.ascii	"IoFileMgrForUser\000000"

$LC1:
	move	$16,$ra
	jalr	$16
	nop
	.ascii	"UtilsForUser\000000"

$LOAD_ADDRESS:
	move	$16,$ra
	jalr	$16
	nop
	.word 0x00010000

$LOAD_SIZE:
	move	$16,$ra
	jalr	$16
	nop
	.word 0x00004000

$LOAD_FILE:
	move	$16,$ra
	jalr	$16
	nop
	.asciz "ARK.BIN"
