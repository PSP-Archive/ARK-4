.set noreorder
.global start
.ent    start

start:

	j main
	nop

.end start

.global Dcache
.ent    Dcache
Dcache:
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
.end Dcache

.global Icache
.ent    Icache
Icache:
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
.end Icache
