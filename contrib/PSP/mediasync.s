
; ==== Section .text - Address 0x00000000 Size 0x00003190 Flags 0x0006

; ======================================================
; Subroutine module_start - Address 0x00000000 
; Exported in syslib
module_start:
	0x00000000: 0x27BDFFF0 '...'' - addiu      $sp, $sp, -16
	0x00000004: 0xAFB20008 '....' - sw         $s2, 8($sp)
	0x00000008: 0x00009021 '!...' - move       $s2, $zr
	0x0000000C: 0xAFB00000 '....' - sw         $s0, 0($sp)
	0x00000010: 0xAFBF000C '....' - sw         $ra, 12($sp)
	0x00000014: 0x0C000C0F '....' - jal        SysMemForKernel_EF29061C
	0x00000018: 0xAFB10004 '....' - sw         $s1, 4($sp)
	0x0000001C: 0x00408021 '!.@.' - move       $s0, $v0
	0x00000020: 0x8C4200D4 '..B.' - lw         $v0, 212($v0)
	0x00000024: 0x24040002 '...$' - li         $a0, 2
	0x00000028: 0x104400A4 '..D.' - beq        $v0, $a0, loc_000002BC
	0x0000002C: 0x00000000 '....' - nop        

loc_00000030:		; Refs: 0x000002C8 0x000002DC 0x000002F0 0x00000304 
	0x00000030: 0x0C000BD3 '....' - jal        sceKernelInitApitype
	0x00000034: 0x00000000 '....' - nop        
	0x00000038: 0x00408021 '!.@.' - move       $s0, $v0
	0x0000003C: 0x0C000BD9 '....' - jal        sceKernelInitFileName
	0x00000040: 0x2610FEF0 '...&' - addiu      $s0, $s0, -272
	0x00000044: 0x2E0A0062 'b...' - sltiu      $t2, $s0, 98
	0x00000048: 0x11400010 '..@.' - beqz       $t2, loc_0000008C
	0x0000004C: 0x00408821 '!.@.' - move       $s1, $v0
; Data ref 0x000041A8 ... 0x0000006C 0x0000006C 0x0000026C 0x0000026C 
	0x00000050: 0x3C0F0000 '...<' - lui        $t7, 0x0
	0x00000054: 0x00106880 '.h..' - sll        $t5, $s0, 2
; Data ref 0x000041A8 ... 0x0000006C 0x0000006C 0x0000026C 0x0000026C 
	0x00000058: 0x25EE41A8 '.A.%' - addiu      $t6, $t7, 16808
	0x0000005C: 0x01AE6021 '!`..' - addu       $t4, $t5, $t6
	0x00000060: 0x8D8B0000 '....' - lw         $t3, 0($t4)
	0x00000064: 0x01600008 '..`.' - jr         $t3
	0x00000068: 0x00000000 '....' - nop        
	0x0000006C: 0x10400007 '..@.' - beqz       $v0, loc_0000008C
	0x00000070: 0x00402021 '! @.' - move       $a0, $v0
; Data ref 0x00003610 "disc"
	0x00000074: 0x3C100000 '...<' - lui        $s0, 0x0
; Data ref 0x00003610 "disc"
	0x00000078: 0x26053610 '.6.&' - addiu      $a1, $s0, 13840
	0x0000007C: 0x0C000C1B '....' - jal        strncmp
	0x00000080: 0x24060004 '...$' - li         $a2, 4
	0x00000084: 0x50400001 '..@P' - beqzl      $v0, loc_0000008C
	0x00000088: 0x24120001 '...$' - li         $s2, 1

loc_0000008C:		; Refs: 0x00000048 0x0000006C 0x00000084 0x000001D8 0x000001E0 0x000001E8 0x000001F0 0x000001F8 0x0000022C 0x0000023C 0x0000026C 0x0000028C 0x00000294 0x000002B4 
	0x0000008C: 0x32440001 '..D2' - andi       $a0, $s2, 0x1

loc_00000090:		; Refs: 0x00000234 0x00000284 0x000002AC 
	0x00000090: 0x1480004B 'K...' - bnez       $a0, loc_000001C0
	0x00000094: 0x00008021 '!...' - move       $s0, $zr

loc_00000098:		; Refs: 0x000001C8 
	0x00000098: 0x32450002 '..E2' - andi       $a1, $s2, 0x2
	0x0000009C: 0x14A00042 'B...' - bnez       $a1, loc_000001A8
	0x000000A0: 0x00000000 '....' - nop        

loc_000000A4:		; Refs: 0x000001B0 
	0x000000A4: 0x32430008 '..C2' - andi       $v1, $s2, 0x8
	0x000000A8: 0x14600039 '9.`.' - bnez       $v1, loc_00000190
	0x000000AC: 0x00000000 '....' - nop        

loc_000000B0:		; Refs: 0x00000198 
	0x000000B0: 0x32490004 '..I2' - andi       $t1, $s2, 0x4
	0x000000B4: 0x32480080 '..H2' - andi       $t0, $s2, 0x80
	0x000000B8: 0x15000031 '1...' - bnez       $t0, loc_00000180
	0x000000BC: 0x0009800B '....' - movn       $s0, $zr, $t1

loc_000000C0:		; Refs: 0x00000188 
	0x000000C0: 0x06000022 '"...' - bltz       $s0, loc_0000014C
	0x000000C4: 0x00000000 '....' - nop        

loc_000000C8:		; Refs: 0x00000168 
	0x000000C8: 0x0C0004D7 '....' - jal        sub_0000135C
	0x000000CC: 0x00000000 '....' - nop        
	0x000000D0: 0x24040034 '4..$' - li         $a0, 52
	0x000000D4: 0x0C000BF3 '....' - jal        KDebugForKernel_86010FCB
	0x000000D8: 0x00408021 '!.@.' - move       $s0, $v0
	0x000000DC: 0x14400014 '..@.' - bnez       $v0, loc_00000130
	0x000000E0: 0x00000000 '....' - nop        

loc_000000E4:		; Refs: 0x0000013C 
	0x000000E4: 0x1200000E '....' - beqz       $s0, loc_00000120
	0x000000E8: 0x00002021 '! ..' - move       $a0, $zr

loc_000000EC:		; Refs: 0x00000128 
	0x000000EC: 0x0C000521 '!...' - jal        sub_00001484
	0x000000F0: 0x00000000 '....' - nop        
	0x000000F4: 0x0C0006B4 '....' - jal        sub_00001AD0
	0x000000F8: 0x00000000 '....' - nop        
	0x000000FC: 0x0C00074F 'O...' - jal        sub_00001D3C
	0x00000100: 0x00000000 '....' - nop        
	0x00000104: 0x24020001 '...$' - li         $v0, 1

loc_00000108:		; Refs: 0x00000334 
	0x00000108: 0x8FBF000C '....' - lw         $ra, 12($sp)
	0x0000010C: 0x8FB20008 '....' - lw         $s2, 8($sp)
	0x00000110: 0x8FB10004 '....' - lw         $s1, 4($sp)
	0x00000114: 0x8FB00000 '....' - lw         $s0, 0($sp)
	0x00000118: 0x03E00008 '....' - jr         $ra
	0x0000011C: 0x27BD0010 '...'' - addiu      $sp, $sp, 16

loc_00000120:		; Refs: 0x000000E4 0x00000144 
	0x00000120: 0x0C000BFB '....' - jal        SysMemForKernel_13EE28DA
	0x00000124: 0x00000000 '....' - nop        
	0x00000128: 0x0800003B ';...' - j          loc_000000EC
	0x0000012C: 0x00000000 '....' - nop        

loc_00000130:		; Refs: 0x000000DC 
; Data ref 0x003B0C39
	0x00000130: 0x0C000C39 '9...' - jal        sceKernelGetGPI
	0x00000134: 0x00000000 '....' - nop        
	0x00000138: 0x7C4B0140 '@.K|' - ext        $t3, $v0, 5, 1
	0x0000013C: 0x1160FFE9 '..`.' - beqz       $t3, loc_000000E4
	0x00000140: 0x24040002 '...$' - li         $a0, 2
	0x00000144: 0x08000048 'H...' - j          loc_00000120
	0x00000148: 0x00000000 '....' - nop        

loc_0000014C:		; Refs: 0x000000C0 0x000001A0 0x000001B8 0x000001D0 
; Data ref 0x00480BCF
	0x0000014C: 0x0C000BCF '....' - jal        sceKernelGetChunk
	0x00000150: 0x00002021 '! ..' - move       $a0, $zr
	0x00000154: 0x14400006 '..@.' - bnez       $v0, loc_00000170
; Data ref 0x00004A5C ... 0x00000000 0x00000000 0x00000000 0x00000000 
	0x00000158: 0x3C120000 '...<' - lui        $s2, 0x0
	0x0000015C: 0x240A0020 ' ..$' - li         $t2, 32
; Data ref 0x00004A5C ... 0x00000000 0x00000000 0x00000000 0x00000000 
	0x00000160: 0xAE4A4A5C '\JJ.' - sw         $t2, 19036($s2)
; Data ref 0x00004A5C ... 0x00000000 0x00000000 0x00000000 0x00000000 
	0x00000164: 0x26444A5C '\JD&' - addiu      $a0, $s2, 19036

loc_00000168:		; Refs: 0x00000178 
	0x00000168: 0x08000032 '2...' - j          loc_000000C8
	0x0000016C: 0xAC900014 '....' - sw         $s0, 20($a0)

loc_00000170:		; Refs: 0x00000154 
; Data ref 0x00320C11
	0x00000170: 0x0C000C11 '....' - jal        SysMemForKernel_F12A62F7
	0x00000174: 0x00402021 '! @.' - move       $a0, $v0
	0x00000178: 0x0800005A 'Z...' - j          loc_00000168
	0x0000017C: 0x00402021 '! @.' - move       $a0, $v0

loc_00000180:		; Refs: 0x000000B8 
; Data ref 0x005A0345
	0x00000180: 0x0C000345 'E...' - jal        sub_00000D14
	0x00000184: 0x00000000 '....' - nop        
	0x00000188: 0x08000030 '0...' - j          loc_000000C0
	0x0000018C: 0x00408021 '!.@.' - move       $s0, $v0

loc_00000190:		; Refs: 0x000000A8 
; Data ref 0x00300299
	0x00000190: 0x0C000299 '....' - jal        sub_00000A64
	0x00000194: 0x00000000 '....' - nop        
	0x00000198: 0x0441FFC5 '..A.' - bgez       $v0, loc_000000B0
	0x0000019C: 0x00408021 '!.@.' - move       $s0, $v0
	0x000001A0: 0x08000053 'S...' - j          loc_0000014C
	0x000001A4: 0x00000000 '....' - nop        

loc_000001A8:		; Refs: 0x0000009C 
; Data ref 0x00530125
	0x000001A8: 0x0C000125 '%...' - jal        sub_00000494
	0x000001AC: 0x00000000 '....' - nop        
	0x000001B0: 0x0441FFBC '..A.' - bgez       $v0, loc_000000A4
	0x000001B4: 0x00408021 '!.@.' - move       $s0, $v0
	0x000001B8: 0x08000053 'S...' - j          loc_0000014C
	0x000001BC: 0x00000000 '....' - nop        

loc_000001C0:		; Refs: 0x00000090 
; Data ref 0x005300CF
	0x000001C0: 0x0C0000CF '....' - jal        sub_0000033C
	0x000001C4: 0x00000000 '....' - nop        
	0x000001C8: 0x0441FFB3 '..A.' - bgez       $v0, loc_00000098
	0x000001CC: 0x00408021 '!.@.' - move       $s0, $v0
	0x000001D0: 0x08000053 'S...' - j          loc_0000014C
	0x000001D4: 0x00000000 '....' - nop        
	0x000001D8: 0x08000023 '#...' - j          loc_0000008C
	0x000001DC: 0x24120003 '...$' - li         $s2, 3
	0x000001E0: 0x08000023 '#...' - j          loc_0000008C
	0x000001E4: 0x24120082 '...$' - li         $s2, 130
	0x000001E8: 0x08000023 '#...' - j          loc_0000008C
	0x000001EC: 0x24120009 '...$' - li         $s2, 9
	0x000001F0: 0x08000023 '#...' - j          loc_0000008C
	0x000001F4: 0x24120088 '...$' - li         $s2, 136
	0x000001F8: 0x1040FFA4 '..@.' - beqz       $v0, loc_0000008C
; Data ref 0x00003618 ... 0x0000736D 0x73616C66 0x00003368 0x00006665 
	0x000001FC: 0x3C180000 '...<' - lui        $t8, 0x0
; Data ref 0x00003618 ... 0x0000736D 0x73616C66 0x00003368 0x00006665 
	0x00000200: 0x27053618 '.6.'' - addiu      $a1, $t8, 13848
	0x00000204: 0x00402021 '! @.' - move       $a0, $v0
; Data ref 0x00230C1B
	0x00000208: 0x0C000C1B '....' - jal        strncmp
	0x0000020C: 0x24060002 '...$' - li         $a2, 2
	0x00000210: 0x02202021 '!  .' - move       $a0, $s1
; Data ref 0x0000361C "flash3"
	0x00000214: 0x3C110000 '...<' - lui        $s1, 0x0
; Data ref 0x0000361C "flash3"
	0x00000218: 0x2625361C '.6%&' - addiu      $a1, $s1, 13852
	0x0000021C: 0x24060006 '...$' - li         $a2, 6
	0x00000220: 0x24030002 '...$' - li         $v1, 2

loc_00000224:		; Refs: 0x00000264 
	0x00000224: 0x0C000C1B '....' - jal        strncmp
	0x00000228: 0x0062900A '..b.' - movz       $s2, $v1, $v0
	0x0000022C: 0x5040FF97 '..@P' - beqzl      $v0, loc_0000008C
	0x00000230: 0x24120004 '...$' - li         $s2, 4
	0x00000234: 0x08000024 '$...' - j          loc_00000090
	0x00000238: 0x32440001 '..D2' - andi       $a0, $s2, 0x1
	0x0000023C: 0x1040FF93 '..@.' - beqz       $v0, loc_0000008C
; Data ref 0x00003624 ... 0x00006665 0x6964656D 0x6E797361 0x3A632E63 
	0x00000240: 0x3C020000 '...<' - lui        $v0, 0x0
; Data ref 0x00003624 ... 0x00006665 0x6964656D 0x6E797361 0x3A632E63 
	0x00000244: 0x24453624 '$6E$' - addiu      $a1, $v0, 13860
	0x00000248: 0x02202021 '!  .' - move       $a0, $s1
; Data ref 0x00240C1B
	0x0000024C: 0x0C000C1B '....' - jal        strncmp
	0x00000250: 0x24060002 '...$' - li         $a2, 2
; Data ref 0x0000361C "flash3"
	0x00000254: 0x3C190000 '...<' - lui        $t9, 0x0
	0x00000258: 0x02202021 '!  .' - move       $a0, $s1
; Data ref 0x0000361C "flash3"
	0x0000025C: 0x2725361C '.6%'' - addiu      $a1, $t9, 13852
	0x00000260: 0x24060006 '...$' - li         $a2, 6
	0x00000264: 0x08000089 '....' - j          loc_00000224
	0x00000268: 0x24030008 '...$' - li         $v1, 8
	0x0000026C: 0x1040FF87 '..@.' - beqz       $v0, loc_0000008C
	0x00000270: 0x00402021 '! @.' - move       $a0, $v0
; Data ref 0x00003610 "disc"
	0x00000274: 0x3C060000 '...<' - lui        $a2, 0x0
; Data ref 0x00003610 "disc"
	0x00000278: 0x24C53610 '.6.$' - addiu      $a1, $a2, 13840
; Data ref 0x00890C1B
	0x0000027C: 0x0C000C1B '....' - jal        strncmp
	0x00000280: 0x24060004 '...$' - li         $a2, 4
	0x00000284: 0x1440FF82 '..@.' - bnez       $v0, loc_00000090
	0x00000288: 0x32440001 '..D2' - andi       $a0, $s2, 0x1
	0x0000028C: 0x08000023 '#...' - j          loc_0000008C
	0x00000290: 0x24120082 '...$' - li         $s2, 130
	0x00000294: 0x1040FF7D '}.@.' - beqz       $v0, loc_0000008C
; Data ref 0x00003610 "disc"
	0x00000298: 0x3C070000 '...<' - lui        $a3, 0x0
	0x0000029C: 0x00402021 '! @.' - move       $a0, $v0
; Data ref 0x00003610 "disc"
	0x000002A0: 0x24E53610 '.6.$' - addiu      $a1, $a3, 13840
; Data ref 0x00230C1B
	0x000002A4: 0x0C000C1B '....' - jal        strncmp
	0x000002A8: 0x24060004 '...$' - li         $a2, 4
	0x000002AC: 0x1440FF78 'x.@.' - bnez       $v0, loc_00000090
	0x000002B0: 0x32440001 '..D2' - andi       $a0, $s2, 0x1
	0x000002B4: 0x08000023 '#...' - j          loc_0000008C
	0x000002B8: 0x24120088 '...$' - li         $s2, 136

loc_000002BC:		; Refs: 0x00000028 
; Data ref 0x00230BF9
	0x000002BC: 0x0C000BF9 '....' - jal        SysMemForKernel_07C586A1
	0x000002C0: 0x00000000 '....' - nop        
	0x000002C4: 0x24040004 '...$' - li         $a0, 4
	0x000002C8: 0x1044FF59 'Y.D.' - beq        $v0, $a0, loc_00000030
	0x000002CC: 0x00000000 '....' - nop        
	0x000002D0: 0x0C000BF9 '....' - jal        SysMemForKernel_07C586A1
	0x000002D4: 0x00000000 '....' - nop        
	0x000002D8: 0x24050005 '...$' - li         $a1, 5
	0x000002DC: 0x1045FF54 'T.E.' - beq        $v0, $a1, loc_00000030
	0x000002E0: 0x00000000 '....' - nop        
	0x000002E4: 0x0C000BF9 '....' - jal        SysMemForKernel_07C586A1
	0x000002E8: 0x00000000 '....' - nop        
	0x000002EC: 0x24060007 '...$' - li         $a2, 7
	0x000002F0: 0x1046FF4F 'O.F.' - beq        $v0, $a2, loc_00000030
	0x000002F4: 0x00000000 '....' - nop        
	0x000002F8: 0x0C000BF9 '....' - jal        SysMemForKernel_07C586A1
	0x000002FC: 0x00000000 '....' - nop        
	0x00000300: 0x24070009 '...$' - li         $a3, 9
	0x00000304: 0x1047FF4A 'J.G.' - beq        $v0, $a3, loc_00000030
	0x00000308: 0x00000000 '....' - nop        
	0x0000030C: 0x0C000BF9 '....' - jal        SysMemForKernel_07C586A1
	0x00000310: 0x00000000 '....' - nop        
	0x00000314: 0x8E0600D4 '....' - lw         $a2, 212($s0)
; Data ref 0x00003628 "mediasync.c:%s:bootable=%d, mode=%d\n"
	0x00000318: 0x3C090000 '...<' - lui        $t1, 0x0
; Data ref 0x00003650 "_sceMediaSyncModuleStart"
	0x0000031C: 0x3C080000 '...<' - lui        $t0, 0x0
; Data ref 0x00003628 "mediasync.c:%s:bootable=%d, mode=%d\n"
	0x00000320: 0x25243628 '(6$%' - addiu      $a0, $t1, 13864
; Data ref 0x00003650 "_sceMediaSyncModuleStart"
	0x00000324: 0x25053650 'P6.%' - addiu      $a1, $t0, 13904
	0x00000328: 0x0C000BF1 '....' - jal        Kprintf
	0x0000032C: 0x00403821 '!8@.' - move       $a3, $v0
	0x00000330: 0x3C038002 '...<' - lui        $v1, 0x8002
	0x00000334: 0x08000042 'B...' - j          loc_00000108
	0x00000338: 0x34620148 'H.b4' - ori        $v0, $v1, 0x148

; ======================================================
; Subroutine sub_0000033C - Address 0x0000033C 
sub_0000033C:		; Refs: 0x000001C0 
	0x0000033C: 0x27BDFFF0 '...'' - addiu      $sp, $sp, -16
	0x00000340: 0xAFBF000C '....' - sw         $ra, 12($sp)
	0x00000344: 0xAFB20008 '....' - sw         $s2, 8($sp)
	0x00000348: 0xAFB10004 '....' - sw         $s1, 4($sp)
; Data ref 0x00420C51
	0x0000034C: 0x0C000C51 'Q...' - jal        sceUmd_A9B5B972
	0x00000350: 0xAFB00000 '....' - sw         $s0, 0($sp)
	0x00000354: 0x3C038001 '...<' - lui        $v1, 0x8001
	0x00000358: 0x10400031 '1.@.' - beqz       $v0, loc_00000420
	0x0000035C: 0x34660087 '..f4' - ori        $a2, $v1, 0x87
; Data ref 0x0000366C "disc0:"
	0x00000360: 0x3C020000 '...<' - lui        $v0, 0x0
	0x00000364: 0x24040001 '...$' - li         $a0, 1
	0x00000368: 0x0C000C55 'U...' - jal        sceUmdActivate
; Data ref 0x0000366C "disc0:"
	0x0000036C: 0x2445366C 'l6E$' - addiu      $a1, $v0, 13932
; Data ref 0x00003674 "mediasync.c:%s:sceUmdActivate failed 0x%08x\n"
	0x00000370: 0x3C030000 '...<' - lui        $v1, 0x0
	0x00000374: 0x00408021 '!.@.' - move       $s0, $v0
	0x00000378: 0x04400040 '@.@.' - bltz       $v0, loc_0000047C
; Data ref 0x00003674 "mediasync.c:%s:sceUmdActivate failed 0x%08x\n"
	0x0000037C: 0x24643674 't6d$' - addiu      $a0, $v1, 13940
	0x00000380: 0x0C000C4F 'O...' - jal        sceUmdWaitDriveStat
	0x00000384: 0x24040029 ')..$' - li         $a0, 41
	0x00000388: 0x0440003A ':.@.' - bltz       $v0, loc_00000474
	0x0000038C: 0x00408021 '!.@.' - move       $s0, $v0
	0x00000390: 0x0C000C53 'S...' - jal        sceUmd_B7BF4C31
	0x00000394: 0x00000000 '....' - nop        
	0x00000398: 0x30440020 ' .D0' - andi       $a0, $v0, 0x20
	0x0000039C: 0x1080002D '-...' - beqz       $a0, loc_00000454
	0x000003A0: 0x00403021 '!0@.' - move       $a2, $v0
	0x000003A4: 0x0C000C0F '....' - jal        SysMemForKernel_EF29061C
	0x000003A8: 0x00000000 '....' - nop        
	0x000003AC: 0x24520018 '..R$' - addiu      $s2, $v0, 24
	0x000003B0: 0x24510008 '..Q$' - addiu      $s1, $v0, 8
	0x000003B4: 0x02202021 '!  .' - move       $a0, $s1
	0x000003B8: 0x02402821 '!(@.' - move       $a1, $s2
	0x000003BC: 0x0C000C1B '....' - jal        strncmp
	0x000003C0: 0x2406000A '...$' - li         $a2, 10
	0x000003C4: 0x5040001D '..@P' - beqzl      $v0, loc_0000043C
; Data ref 0x000036A4 "mediasync.c:%s:UMD Media Check OK\n"
	0x000003C8: 0x3C110000 '...<' - lui        $s1, 0x0
; Data ref 0x000036C8 "DiscCheckMedia"
	0x000003CC: 0x3C180000 '...<' - lui        $t8, 0x0
; Data ref 0x000036C8 "DiscCheckMedia"
	0x000003D0: 0x271036C8 '.6.'' - addiu      $s0, $t8, 14024
; Data ref 0x000036D8 "mediasync.c:%s:SCE_MEDIASYNC_ERROR_INVALID_MEDIA\n"
	0x000003D4: 0x3C0F0000 '...<' - lui        $t7, 0x0
; Data ref 0x000036D8 "mediasync.c:%s:SCE_MEDIASYNC_ERROR_INVALID_MEDIA\n"
	0x000003D8: 0x25E436D8 '.6.%' - addiu      $a0, $t7, 14040
	0x000003DC: 0x0C000BF1 '....' - jal        Kprintf
	0x000003E0: 0x02002821 '!(..' - move       $a1, $s0
; Data ref 0x0000370C "mediasync.c:%s:[%16s] and [%16s]\n"
	0x000003E4: 0x3C0E0000 '...<' - lui        $t6, 0x0
; Data ref 0x0000370C "mediasync.c:%s:[%16s] and [%16s]\n"
	0x000003E8: 0x25C4370C '.7.%' - addiu      $a0, $t6, 14092
	0x000003EC: 0x02002821 '!(..' - move       $a1, $s0
	0x000003F0: 0x02203021 '!0 .' - move       $a2, $s1
	0x000003F4: 0x0C000BF1 '....' - jal        Kprintf
	0x000003F8: 0x02403821 '!8@.' - move       $a3, $s2
	0x000003FC: 0x3C0B8002 '...<' - lui        $t3, 0x8002
; Data ref 0x00003730 "mediasync.c:%s:DiscCheckMedia failed 0x%08x\n"
	0x00000400: 0x3C0D0000 '...<' - lui        $t5, 0x0
; Data ref 0x00003760 "WaitDisc"
	0x00000404: 0x3C0C0000 '...<' - lui        $t4, 0x0
	0x00000408: 0x3566014E 'N.f5' - ori        $a2, $t3, 0x14E
; Data ref 0x00003730 "mediasync.c:%s:DiscCheckMedia failed 0x%08x\n"
	0x0000040C: 0x25A43730 '07.%' - addiu      $a0, $t5, 14128
	0x00000410: 0x0C000BF1 '....' - jal        Kprintf
; Data ref 0x00003760 "WaitDisc"
	0x00000414: 0x25853760 '`7.%' - addiu      $a1, $t4, 14176
	0x00000418: 0x3C0A8002 '...<' - lui        $t2, 0x8002
	0x0000041C: 0x3546014E 'N.F5' - ori        $a2, $t2, 0x14E

loc_00000420:		; Refs: 0x00000358 0x0000044C 0x0000046C 0x0000048C 
	0x00000420: 0x8FBF000C '....' - lw         $ra, 12($sp)
	0x00000424: 0x8FB20008 '....' - lw         $s2, 8($sp)
	0x00000428: 0x8FB10004 '....' - lw         $s1, 4($sp)
	0x0000042C: 0x8FB00000 '....' - lw         $s0, 0($sp)
	0x00000430: 0x00C01021 '!...' - move       $v0, $a2
	0x00000434: 0x03E00008 '....' - jr         $ra
	0x00000438: 0x27BD0010 '...'' - addiu      $sp, $sp, 16

loc_0000043C:		; Refs: 0x000003C4 
; Data ref 0x000036C8 "DiscCheckMedia"
	0x0000043C: 0x3C100000 '...<' - lui        $s0, 0x0
; Data ref 0x000036A4 "mediasync.c:%s:UMD Media Check OK\n"
	0x00000440: 0x262436A4 '.6$&' - addiu      $a0, $s1, 13988
	0x00000444: 0x0C000BF1 '....' - jal        Kprintf
; Data ref 0x000036C8 "DiscCheckMedia"
	0x00000448: 0x260536C8 '.6.&' - addiu      $a1, $s0, 14024
	0x0000044C: 0x08000108 '....' - j          loc_00000420
	0x00000450: 0x00003021 '!0..' - move       $a2, $zr

loc_00000454:		; Refs: 0x0000039C 
; Data ref 0x0000376C "mediasync.c:%s:sceUmdGetDriveStatus:0x%08x\n"
	0x00000454: 0x3C090000 '...<' - lui        $t1, 0x0
; Data ref 0x00003760 "WaitDisc"
	0x00000458: 0x3C080000 '...<' - lui        $t0, 0x0
; Data ref 0x0000376C "mediasync.c:%s:sceUmdGetDriveStatus:0x%08x\n"
	0x0000045C: 0x2524376C 'l7$%' - addiu      $a0, $t1, 14188
; Data ref 0x01080BF1
	0x00000460: 0x0C000BF1 '....' - jal        Kprintf
; Data ref 0x00003760 "WaitDisc"
	0x00000464: 0x25053760 '`7.%' - addiu      $a1, $t0, 14176
	0x00000468: 0x3C078001 '...<' - lui        $a3, 0x8001
	0x0000046C: 0x08000108 '....' - j          loc_00000420
	0x00000470: 0x34E60087 '...4' - ori        $a2, $a3, 0x87

loc_00000474:		; Refs: 0x00000388 
; Data ref 0x00003798 "mediasync.c:%s:sceUmdWaitDriveStat failed 0x%08x\n"
	0x00000474: 0x3C050000 '...<' - lui        $a1, 0x0
; Data ref 0x00003798 "mediasync.c:%s:sceUmdWaitDriveStat failed 0x%08x\n"
	0x00000478: 0x24A43798 '.7.$' - addiu      $a0, $a1, 14232

loc_0000047C:		; Refs: 0x00000378 
; Data ref 0x00003760 "WaitDisc"
	0x0000047C: 0x3C060000 '...<' - lui        $a2, 0x0
; Data ref 0x00003760 "WaitDisc"
	0x00000480: 0x24C53760 '`7.$' - addiu      $a1, $a2, 14176
; Data ref 0x01080BF1
	0x00000484: 0x0C000BF1 '....' - jal        Kprintf
	0x00000488: 0x02003021 '!0..' - move       $a2, $s0
	0x0000048C: 0x08000108 '....' - j          loc_00000420
	0x00000490: 0x02003021 '!0..' - move       $a2, $s0

; ======================================================
; Subroutine sub_00000494 - Address 0x00000494 
sub_00000494:		; Refs: 0x000001A8 
	0x00000494: 0x3C050098 '...<' - lui        $a1, 0x98
; Data ref 0x000037CC "SceMediaSyncEvMs"
	0x00000498: 0x3C020000 '...<' - lui        $v0, 0x0
	0x0000049C: 0x27BDFFD0 '...'' - addiu      $sp, $sp, -48
	0x000004A0: 0x34A39680 '...4' - ori        $v1, $a1, 0x9680
; Data ref 0x000037CC "SceMediaSyncEvMs"
	0x000004A4: 0x244437CC '.7D$' - addiu      $a0, $v0, 14284
	0x000004A8: 0x00002821 '!(..' - move       $a1, $zr
	0x000004AC: 0x00003021 '!0..' - move       $a2, $zr
	0x000004B0: 0x00003821 '!8..' - move       $a3, $zr
	0x000004B4: 0xAFB00010 '....' - sw         $s0, 16($sp)
	0x000004B8: 0xAFBF0024 '$...' - sw         $ra, 36($sp)
	0x000004BC: 0xAFB40020 ' ...' - sw         $s4, 32($sp)
	0x000004C0: 0xAFB3001C '....' - sw         $s3, 28($sp)
	0x000004C4: 0xAFB20018 '....' - sw         $s2, 24($sp)
	0x000004C8: 0xAFB10014 '....' - sw         $s1, 20($sp)
; Data ref 0x01080C27
	0x000004CC: 0x0C000C27 ''...' - jal        sceKernelCreateEventFlag
	0x000004D0: 0xAFA30008 '....' - sw         $v1, 8($sp)
	0x000004D4: 0x04400093 '..@.' - bltz       $v0, loc_00000724
	0x000004D8: 0x00408021 '!.@.' - move       $s0, $v0
; Data ref 0x000037E0 "SceMediaSyncMs"
	0x000004DC: 0x3C090000 '...<' - lui        $t1, 0x0
; Data ref 0x00000EE0 ... 0x27BDFFF0 0x00A01821 0x3C020000 0x3C050000 
	0x000004E0: 0x3C080000 '...<' - lui        $t0, 0x0
; Data ref 0x000037E0 "SceMediaSyncMs"
	0x000004E4: 0x252437E0 '.7$%' - addiu      $a0, $t1, 14304
; Data ref 0x00000EE0 ... 0x27BDFFF0 0x00A01821 0x3C020000 0x3C050000 
	0x000004E8: 0x25050EE0 '...%' - addiu      $a1, $t0, 3808
; Data ref 0x00004A50 ... 0x00000000 0x00000000 0x00000000 0x00000000 
	0x000004EC: 0x3C140000 '...<' - lui        $s4, 0x0
	0x000004F0: 0x00003021 '!0..' - move       $a2, $zr
; Data ref 0x000037F0 "fatms0:"
	0x000004F4: 0x3C130000 '...<' - lui        $s3, 0x0
	0x000004F8: 0x0C000C33 '3...' - jal        sceKernelCreateCallback
; Data ref 0x00004A50 ... 0x00000000 0x00000000 0x00000000 0x00000000 
	0x000004FC: 0xAE824A50 'PJ..' - sw         $v0, 19024($s4)
	0x00000500: 0x3C070241 'A..<' - lui        $a3, 0x241
	0x00000504: 0x34E55821 '!X.4' - ori        $a1, $a3, 0x5821
; Data ref 0x000037F0 "fatms0:"
	0x00000508: 0x266437F0 '.7d&' - addiu      $a0, $s3, 14320
	0x0000050C: 0x03A03021 '!0..' - move       $a2, $sp
	0x00000510: 0x24070004 '...$' - li         $a3, 4
	0x00000514: 0x00004021 '!@..' - move       $t0, $zr
	0x00000518: 0x00004821 '!H..' - move       $t1, $zr
	0x0000051C: 0x00408821 '!.@.' - move       $s1, $v0
	0x00000520: 0x0C000BE3 '....' - jal        sceIoDevctl
	0x00000524: 0xAFA20000 '....' - sw         $v0, 0($sp)
	0x00000528: 0x00408021 '!.@.' - move       $s0, $v0
	0x0000052C: 0x04400075 'u.@.' - bltz       $v0, loc_00000704
; Data ref 0x00004A50 ... 0x00000000 0x00000000 0x00000000 0x00000000 
	0x00000530: 0x26924A50 'PJ.&' - addiu      $s2, $s4, 19024
; Data ref 0x00004A50 ... 0x00000000 0x00000000 0x00000000 0x00000000 
	0x00000534: 0x8E844A50 'PJ..' - lw         $a0, 19024($s4)
	0x00000538: 0x24050001 '...$' - li         $a1, 1
	0x0000053C: 0x24060011 '...$' - li         $a2, 17
	0x00000540: 0x27A70004 '...'' - addiu      $a3, $sp, 4
	0x00000544: 0x0C000C25 '%...' - jal        sceKernelWaitEventFlagCB
	0x00000548: 0x27A80008 '...'' - addiu      $t0, $sp, 8
	0x0000054C: 0x04400006 '..@.' - bltz       $v0, loc_00000568
	0x00000550: 0x00408021 '!.@.' - move       $s0, $v0
	0x00000554: 0x8E4D0004 '..M.' - lw         $t5, 4($s2)
	0x00000558: 0x3C0E8002 '...<' - lui        $t6, 0x8002
	0x0000055C: 0x35C4014E 'N..5' - ori        $a0, $t6, 0x14E
	0x00000560: 0x39AC0001 '...9' - xori       $t4, $t5, 0x1
	0x00000564: 0x008C800B '....' - movn       $s0, $a0, $t4

loc_00000568:		; Refs: 0x0000054C 
	0x00000568: 0x3C0F0241 'A..<' - lui        $t7, 0x241

loc_0000056C:		; Refs: 0x0000071C 
; Data ref 0x000037F0 "fatms0:"
	0x0000056C: 0x266437F0 '.7d&' - addiu      $a0, $s3, 14320
	0x00000570: 0x35E55822 '"X.5' - ori        $a1, $t7, 0x5822
	0x00000574: 0x27A6000C '...'' - addiu      $a2, $sp, 12
	0x00000578: 0x24070004 '...$' - li         $a3, 4
	0x0000057C: 0x00004021 '!@..' - move       $t0, $zr
	0x00000580: 0x00004821 '!H..' - move       $t1, $zr
	0x00000584: 0x0C000BE3 '....' - jal        sceIoDevctl
	0x00000588: 0xAFB1000C '....' - sw         $s1, 12($sp)
	0x0000058C: 0x04400056 'V.@.' - bltz       $v0, loc_000006E8
; Data ref 0x000037F8 "mediasync.c:%s:sceFatmsUnRegisterNotifyCallback failed 0x%08x\n"
	0x00000590: 0x3C130000 '...<' - lui        $s3, 0x0

loc_00000594:		; Refs: 0x000006FC 
	0x00000594: 0x0C000C35 '5...' - jal        sceKernelDeleteCallback
	0x00000598: 0x02202021 '!  .' - move       $a0, $s1
	0x0000059C: 0x0440004B 'K.@.' - bltz       $v0, loc_000006CC
; Data ref 0x00003838 "mediasync.c:%s:sceKernelDeleteCallback failed 0x%08x\n"
	0x000005A0: 0x3C180000 '...<' - lui        $t8, 0x0

loc_000005A4:		; Refs: 0x000006E0 0x0000073C 
	0x000005A4: 0x0C000C37 '7...' - jal        sceKernelDeleteEventFlag
; Data ref 0x00004A50 ... 0x00000000 0x00000000 0x00000000 0x00000000 
	0x000005A8: 0x8E844A50 'PJ..' - lw         $a0, 19024($s4)
	0x000005AC: 0x04400040 '@.@.' - bltz       $v0, loc_000006B0
; Data ref 0x00003870 "mediasync.c:%s:sceKernelDeleteEventFlag failed 0x%08x\n"
	0x000005B0: 0x3C190000 '...<' - lui        $t9, 0x0

loc_000005B4:		; Refs: 0x000006C4 
	0x000005B4: 0x06020019 '....' - bltzl      $s0, loc_0000061C
	0x000005B8: 0x02001021 '!...' - move       $v0, $s0
	0x000005BC: 0x0C000BD9 '....' - jal        sceKernelInitFileName
	0x000005C0: 0x00000000 '....' - nop        
	0x000005C4: 0x00408021 '!.@.' - move       $s0, $v0
	0x000005C8: 0x3C058002 '...<' - lui        $a1, 0x8002
	0x000005CC: 0x12000013 '....' - beqz       $s0, loc_0000061C
	0x000005D0: 0x34A2014E 'N..4' - ori        $v0, $a1, 0x14E
	0x000005D4: 0x0C000BD3 '....' - jal        sceKernelInitApitype
	0x000005D8: 0x00000000 '....' - nop        
	0x000005DC: 0x2443FEEE '..C$' - addiu      $v1, $v0, -274
	0x000005E0: 0x2C67005F '_.g,' - sltiu      $a3, $v1, 95
	0x000005E4: 0x10E00029 ')...' - beqz       $a3, loc_0000068C
	0x000005E8: 0x00403021 '!0@.' - move       $a2, $v0
; Data ref 0x00004330 ... 0x00000654 0x00000654 0x0000068C 0x0000068C 
	0x000005EC: 0x3C0A0000 '...<' - lui        $t2, 0x0
	0x000005F0: 0x00034880 '.H..' - sll        $t1, $v1, 2
; Data ref 0x00004330 ... 0x00000654 0x00000654 0x0000068C 0x0000068C 
	0x000005F4: 0x25424330 '0CB%' - addiu      $v0, $t2, 17200
	0x000005F8: 0x01224021 '!@".' - addu       $t0, $t1, $v0
	0x000005FC: 0x8D030000 '....' - lw         $v1, 0($t0)
	0x00000600: 0x00600008 '..`.' - jr         $v1
	0x00000604: 0x00000000 '....' - nop        
	0x00000608: 0x0C0001D1 '....' - jal        sub_00000744
	0x0000060C: 0x02002021 '! ..' - move       $a0, $s0
	0x00000610: 0x0440000A '..@.' - bltz       $v0, loc_0000063C
	0x00000614: 0x00408021 '!.@.' - move       $s0, $v0

loc_00000618:		; Refs: 0x00000654 0x00000664 0x000006A8 
	0x00000618: 0x02001021 '!...' - move       $v0, $s0

loc_0000061C:		; Refs: 0x000005B4 0x000005CC 0x00000684 
	0x0000061C: 0x8FBF0024 '$...' - lw         $ra, 36($sp)
	0x00000620: 0x8FB40020 ' ...' - lw         $s4, 32($sp)
	0x00000624: 0x8FB3001C '....' - lw         $s3, 28($sp)
	0x00000628: 0x8FB20018 '....' - lw         $s2, 24($sp)
	0x0000062C: 0x8FB10014 '....' - lw         $s1, 20($sp)
	0x00000630: 0x8FB00010 '....' - lw         $s0, 16($sp)
	0x00000634: 0x03E00008 '....' - jr         $ra
	0x00000638: 0x27BD0030 '0..'' - addiu      $sp, $sp, 48

loc_0000063C:		; Refs: 0x00000610 
; Data ref 0x000038A8 "mediasync.c:%s:Warning: MsCheckMediaFailed 0x%08x\n"
	0x0000063C: 0x3C0B0000 '...<' - lui        $t3, 0x0
; Data ref 0x000038DC "WaitMs"
	0x00000640: 0x3C100000 '...<' - lui        $s0, 0x0
	0x00000644: 0x00403021 '!0@.' - move       $a2, $v0
; Data ref 0x000038A8 "mediasync.c:%s:Warning: MsCheckMediaFailed 0x%08x\n"
	0x00000648: 0x256438A8 '.8d%' - addiu      $a0, $t3, 14504
	0x0000064C: 0x0C000BF1 '....' - jal        Kprintf
; Data ref 0x000038DC "WaitMs"
	0x00000650: 0x260538DC '.8.&' - addiu      $a1, $s0, 14556
	0x00000654: 0x08000186 '....' - j          loc_00000618
	0x00000658: 0x00008021 '!...' - move       $s0, $zr
; Data ref 0x018601D1
	0x0000065C: 0x0C0001D1 '....' - jal        sub_00000744
	0x00000660: 0x02002021 '! ..' - move       $a0, $s0
	0x00000664: 0x0441FFEC '..A.' - bgez       $v0, loc_00000618
	0x00000668: 0x00408021 '!.@.' - move       $s0, $v0
; Data ref 0x000038E4 "mediasync.c:%s:MsCheckMediaFailed 0x%08x\n"
	0x0000066C: 0x3C0C0000 '...<' - lui        $t4, 0x0
; Data ref 0x000038DC "WaitMs"
	0x00000670: 0x3C060000 '...<' - lui        $a2, 0x0
; Data ref 0x000038DC "WaitMs"
	0x00000674: 0x24C538DC '.8.$' - addiu      $a1, $a2, 14556
; Data ref 0x000038E4 "mediasync.c:%s:MsCheckMediaFailed 0x%08x\n"
	0x00000678: 0x258438E4 '.8.%' - addiu      $a0, $t4, 14564
	0x0000067C: 0x0C000BF1 '....' - jal        Kprintf
	0x00000680: 0x00403021 '!0@.' - move       $a2, $v0
	0x00000684: 0x08000187 '....' - j          loc_0000061C
	0x00000688: 0x02001021 '!...' - move       $v0, $s0

loc_0000068C:		; Refs: 0x000005E4 
; Data ref 0x00003910 "mediasync.c:%s:unsupported apitype=0x%08x[%d]\n"
	0x0000068C: 0x3C0E0000 '...<' - lui        $t6, 0x0
; Data ref 0x000038DC "WaitMs"
	0x00000690: 0x3C0D0000 '...<' - lui        $t5, 0x0
; Data ref 0x00003910 "mediasync.c:%s:unsupported apitype=0x%08x[%d]\n"
	0x00000694: 0x25C43910 '.9.%' - addiu      $a0, $t6, 14608
; Data ref 0x000038DC "WaitMs"
	0x00000698: 0x25A538DC '.8.%' - addiu      $a1, $t5, 14556
	0x0000069C: 0x00C03821 '!8..' - move       $a3, $a2
; Data ref 0x01870BF1
	0x000006A0: 0x0C000BF1 '....' - jal        Kprintf
	0x000006A4: 0x3C128002 '...<' - lui        $s2, 0x8002
	0x000006A8: 0x08000186 '....' - j          loc_00000618
	0x000006AC: 0x3650014E 'N.P6' - ori        $s0, $s2, 0x14E

loc_000006B0:		; Refs: 0x000005AC 
; Data ref 0x000038DC "WaitMs"
	0x000006B0: 0x3C140000 '...<' - lui        $s4, 0x0
; Data ref 0x00003870 "mediasync.c:%s:sceKernelDeleteEventFlag failed 0x%08x\n"
	0x000006B4: 0x27243870 'p8$'' - addiu      $a0, $t9, 14448
; Data ref 0x000038DC "WaitMs"
	0x000006B8: 0x268538DC '.8.&' - addiu      $a1, $s4, 14556
; Data ref 0x01860BF1
	0x000006BC: 0x0C000BF1 '....' - jal        Kprintf
	0x000006C0: 0x00403021 '!0@.' - move       $a2, $v0
	0x000006C4: 0x0800016D 'm...' - j          loc_000005B4
	0x000006C8: 0x00000000 '....' - nop        

loc_000006CC:		; Refs: 0x0000059C 
; Data ref 0x000038DC "WaitMs"
	0x000006CC: 0x3C110000 '...<' - lui        $s1, 0x0
; Data ref 0x00003838 "mediasync.c:%s:sceKernelDeleteCallback failed 0x%08x\n"
	0x000006D0: 0x27043838 '88.'' - addiu      $a0, $t8, 14392
; Data ref 0x000038DC "WaitMs"
	0x000006D4: 0x262538DC '.8%&' - addiu      $a1, $s1, 14556
; Data ref 0x016D0BF1
	0x000006D8: 0x0C000BF1 '....' - jal        Kprintf
	0x000006DC: 0x00403021 '!0@.' - move       $a2, $v0
	0x000006E0: 0x08000169 'i...' - j          loc_000005A4
	0x000006E4: 0x00000000 '....' - nop        

loc_000006E8:		; Refs: 0x0000058C 
; Data ref 0x000038DC "WaitMs"
	0x000006E8: 0x3C120000 '...<' - lui        $s2, 0x0
; Data ref 0x000037F8 "mediasync.c:%s:sceFatmsUnRegisterNotifyCallback failed 0x%08x\n"
	0x000006EC: 0x266437F8 '.7d&' - addiu      $a0, $s3, 14328
; Data ref 0x000038DC "WaitMs"
	0x000006F0: 0x264538DC '.8E&' - addiu      $a1, $s2, 14556
; Data ref 0x01690BF1
	0x000006F4: 0x0C000BF1 '....' - jal        Kprintf
	0x000006F8: 0x00403021 '!0@.' - move       $a2, $v0
	0x000006FC: 0x08000165 'e...' - j          loc_00000594
	0x00000700: 0x00000000 '....' - nop        

loc_00000704:		; Refs: 0x0000052C 
; Data ref 0x00003940 "mediasync.c:%s:sceFatmsRegisterNotifyCallback failed 0x%08x\n"
	0x00000704: 0x3C0B0000 '...<' - lui        $t3, 0x0
; Data ref 0x000038DC "WaitMs"
	0x00000708: 0x3C0A0000 '...<' - lui        $t2, 0x0
; Data ref 0x00003940 "mediasync.c:%s:sceFatmsRegisterNotifyCallback failed 0x%08x\n"
	0x0000070C: 0x25643940 '@9d%' - addiu      $a0, $t3, 14656
; Data ref 0x000038DC "WaitMs"
	0x00000710: 0x254538DC '.8E%' - addiu      $a1, $t2, 14556
; Data ref 0x01650BF1
	0x00000714: 0x0C000BF1 '....' - jal        Kprintf
	0x00000718: 0x00403021 '!0@.' - move       $a2, $v0
	0x0000071C: 0x0800015B '[...' - j          loc_0000056C
	0x00000720: 0x3C0F0241 'A..<' - lui        $t7, 0x241

loc_00000724:		; Refs: 0x000004D4 
; Data ref 0x00003980 "mediasync.c:%s:sceKernelCreateEventFlag failed 0x%08x\n"
	0x00000724: 0x3C060000 '...<' - lui        $a2, 0x0
; Data ref 0x000038DC "WaitMs"
	0x00000728: 0x3C030000 '...<' - lui        $v1, 0x0
; Data ref 0x00003980 "mediasync.c:%s:sceKernelCreateEventFlag failed 0x%08x\n"
	0x0000072C: 0x24C43980 '.9.$' - addiu      $a0, $a2, 14720
; Data ref 0x000038DC "WaitMs"
	0x00000730: 0x246538DC '.8e$' - addiu      $a1, $v1, 14556
; Data ref 0x015B0BF1
	0x00000734: 0x0C000BF1 '....' - jal        Kprintf
	0x00000738: 0x00403021 '!0@.' - move       $a2, $v0
	0x0000073C: 0x08000169 'i...' - j          loc_000005A4
; Data ref 0x00004A50 ... 0x00000000 0x00000000 0x00000000 0x00000000 
	0x00000740: 0x3C140000 '...<' - lui        $s4, 0x0

; ======================================================
; Subroutine sub_00000744 - Address 0x00000744 
sub_00000744:		; Refs: 0x00000608 0x0000065C 0x00000BD8 0x00000C2C 0x00000E80 
	0x00000744: 0x27BDFF90 '...'' - addiu      $sp, $sp, -112
	0x00000748: 0xAFBF0060 '`...' - sw         $ra, 96($sp)
; Data ref 0x000039B8 "SceMdiaSync:work2"
	0x0000074C: 0x3C0E0000 '...<' - lui        $t6, 0x0
; Data ref 0x000039B8 "SceMdiaSync:work2"
	0x00000750: 0x25C839B8 '.9.%' - addiu      $t0, $t6, 14776
	0x00000754: 0xAFB50054 'T...' - sw         $s5, 84($sp)
	0x00000758: 0xAFB20048 'H...' - sw         $s2, 72($sp)
	0x0000075C: 0xAFB00040 '@...' - sw         $s0, 64($sp)
; Data ref 0x000039CC "SceMdiaSync:work1"
	0x00000760: 0x3C100000 '...<' - lui        $s0, 0x0
; Data ref 0x000039CC "SceMdiaSync:work1"
	0x00000764: 0x260339CC '.9.&' - addiu      $v1, $s0, 14796
	0x00000768: 0xAFB7005C '\...' - sw         $s7, 92($sp)
	0x0000076C: 0xAFB60058 'X...' - sw         $s6, 88($sp)
	0x00000770: 0xAFB40050 'P...' - sw         $s4, 80($sp)
	0x00000774: 0xAFB3004C 'L...' - sw         $s3, 76($sp)
	0x00000778: 0xAFB10044 'D...' - sw         $s1, 68($sp)
; Data ref 0x000039CC "SceMdiaSync:work1"
	0x0000077C: 0x8E0F39CC '.9..' - lw         $t7, 14796($s0)
; Data ref 0x000039B8 "SceMdiaSync:work2"
	0x00000780: 0x8DC939B8 '.9..' - lw         $t1, 14776($t6)
	0x00000784: 0x8C660004 '..f.' - lw         $a2, 4($v1)
	0x00000788: 0x950E0010 '....' - lhu        $t6, 16($t0)
	0x0000078C: 0x8C670008 '..g.' - lw         $a3, 8($v1)
	0x00000790: 0x8D0A0008 '....' - lw         $t2, 8($t0)
	0x00000794: 0x8D0D000C '....' - lw         $t5, 12($t0)
	0x00000798: 0x946C0010 '..l.' - lhu        $t4, 16($v1)
	0x0000079C: 0x8C6B000C '..k.' - lw         $t3, 12($v1)
	0x000007A0: 0x8D020004 '....' - lw         $v0, 4($t0)
	0x000007A4: 0xAFAF0000 '....' - sw         $t7, 0($sp)
	0x000007A8: 0x00808021 '!...' - move       $s0, $a0
	0x000007AC: 0xAFA60004 '....' - sw         $a2, 4($sp)
	0x000007B0: 0xAFA70008 '....' - sw         $a3, 8($sp)
	0x000007B4: 0xAFAB000C '....' - sw         $t3, 12($sp)
	0x000007B8: 0xA7AC0010 '....' - sh         $t4, 16($sp)
	0x000007BC: 0xAFA90020 ' ...' - sw         $t1, 32($sp)
	0x000007C0: 0xAFAA0028 '(...' - sw         $t2, 40($sp)
	0x000007C4: 0xAFAD002C ',...' - sw         $t5, 44($sp)
	0x000007C8: 0xA7AE0030 '0...' - sh         $t6, 48($sp)
; Data ref 0x01690C0F
	0x000007CC: 0x0C000C0F '....' - jal        SysMemForKernel_EF29061C
	0x000007D0: 0xAFA20024 '$...' - sw         $v0, 36($sp)
	0x000007D4: 0x24040001 '...$' - li         $a0, 1
	0x000007D8: 0x03A02821 '!(..' - move       $a1, $sp
	0x000007DC: 0x24060001 '...$' - li         $a2, 1
	0x000007E0: 0x24070028 '(..$' - li         $a3, 40
	0x000007E4: 0x00004021 '!@..' - move       $t0, $zr
	0x000007E8: 0x0C000C03 '....' - jal        SysMemForKernel_7158CE7E
	0x000007EC: 0x0040A821 '!.@.' - move       $s5, $v0
	0x000007F0: 0x04400093 '..@.' - bltz       $v0, loc_00000A40
	0x000007F4: 0x00409021 '!.@.' - move       $s2, $v0
	0x000007F8: 0x02002021 '! ..' - move       $a0, $s0
	0x000007FC: 0x24050001 '...$' - li         $a1, 1
	0x00000800: 0x0C000BDD '....' - jal        sceIoOpen
	0x00000804: 0x240601FF '...$' - li         $a2, 511
	0x00000808: 0x04400084 '..@.' - bltz       $v0, loc_00000A1C
	0x0000080C: 0x0040A021 '!.@.' - move       $s4, $v0
	0x00000810: 0x0C000C11 '....' - jal        SysMemForKernel_F12A62F7
	0x00000814: 0x02402021 '! @.' - move       $a0, $s2
	0x00000818: 0x00402821 '!(@.' - move       $a1, $v0
	0x0000081C: 0x02802021 '! ..' - move       $a0, $s4
	0x00000820: 0x24060028 '(..$' - li         $a2, 40
	0x00000824: 0x0C000BE5 '....' - jal        sceIoRead
	0x00000828: 0x00408021 '!.@.' - move       $s0, $v0
	0x0000082C: 0x04400073 's.@.' - bltz       $v0, loc_000009FC
	0x00000830: 0x00409821 '!.@.' - move       $s3, $v0
	0x00000834: 0x82040000 '....' - lb         $a0, 0($s0)
	0x00000838: 0x14800004 '....' - bnez       $a0, loc_0000084C
	0x0000083C: 0x24170050 'P..$' - li         $s7, 80
	0x00000840: 0x82040001 '....' - lb         $a0, 1($s0)
	0x00000844: 0x50970023 '#..P' - beql       $a0, $s7, loc_000008D4
	0x00000848: 0x82190002 '....' - lb         $t9, 2($s0)

loc_0000084C:		; Refs: 0x00000838 
; Data ref 0x000039E0 "mediasync.c:%s:This is not PBP\n"
	0x0000084C: 0x3C020000 '...<' - lui        $v0, 0x0

loc_00000850:		; Refs: 0x000008D8 
; Data ref 0x00003A00 "MsCheckMedia"
	0x00000850: 0x3C0A0000 '...<' - lui        $t2, 0x0

loc_00000854:		; Refs: 0x000008E4 
; Data ref 0x000039E0 "mediasync.c:%s:This is not PBP\n"
	0x00000854: 0x244439E0 '.9D$' - addiu      $a0, $v0, 14816
; Data ref 0x00003A00 "MsCheckMedia"
	0x00000858: 0x25453A00 '.:E%' - addiu      $a1, $t2, 14848
	0x0000085C: 0x0C000BF1 '....' - jal        Kprintf
	0x00000860: 0x3C108002 '...<' - lui        $s0, 0x8002
	0x00000864: 0x3611014E 'N..6' - ori        $s1, $s0, 0x14E

loc_00000868:		; Refs: 0x0000096C 0x00000984 0x000009AC 0x00000A14 
	0x00000868: 0x1A400005 '..@.' - blez       $s2, loc_00000880
	0x0000086C: 0x00000000 '....' - nop        
	0x00000870: 0x0C000C0B '....' - jal        SysMemForKernel_C1A26C6F
	0x00000874: 0x02402021 '! @.' - move       $a0, $s2
	0x00000878: 0x0440000F '..@.' - bltz       $v0, loc_000008B8
; Data ref 0x00003A10 "mediasync.c:%s:sceKernelFreePartitionMemory failed 0x%08x\n"
	0x0000087C: 0x3C050000 '...<' - lui        $a1, 0x0

loc_00000880:		; Refs: 0x00000868 0x000008CC 0x000009D0 0x000009F4 
	0x00000880: 0x0C000BE7 '....' - jal        sceIoClose
	0x00000884: 0x02802021 '! ..' - move       $a0, $s4
	0x00000888: 0x02201021 '!. .' - move       $v0, $s1

loc_0000088C:		; Refs: 0x00000A38 0x00000A5C 
	0x0000088C: 0x8FBF0060 '`...' - lw         $ra, 96($sp)
	0x00000890: 0x8FB7005C '\...' - lw         $s7, 92($sp)
	0x00000894: 0x8FB60058 'X...' - lw         $s6, 88($sp)
	0x00000898: 0x8FB50054 'T...' - lw         $s5, 84($sp)
	0x0000089C: 0x8FB40050 'P...' - lw         $s4, 80($sp)
	0x000008A0: 0x8FB3004C 'L...' - lw         $s3, 76($sp)
	0x000008A4: 0x8FB20048 'H...' - lw         $s2, 72($sp)
	0x000008A8: 0x8FB10044 'D...' - lw         $s1, 68($sp)
	0x000008AC: 0x8FB00040 '@...' - lw         $s0, 64($sp)
	0x000008B0: 0x03E00008 '....' - jr         $ra
	0x000008B4: 0x27BD0070 'p..'' - addiu      $sp, $sp, 112

loc_000008B8:		; Refs: 0x00000878 
; Data ref 0x00003A00 "MsCheckMedia"
	0x000008B8: 0x3C0E0000 '...<' - lui        $t6, 0x0
; Data ref 0x00003A10 "mediasync.c:%s:sceKernelFreePartitionMemory failed 0x%08x\n"
	0x000008BC: 0x24A43A10 '.:.$' - addiu      $a0, $a1, 14864
	0x000008C0: 0x00403021 '!0@.' - move       $a2, $v0
	0x000008C4: 0x0C000BF1 '....' - jal        Kprintf
; Data ref 0x00003A00 "MsCheckMedia"
	0x000008C8: 0x25C53A00 '.:.%' - addiu      $a1, $t6, 14848
	0x000008CC: 0x08000220 ' ...' - j          loc_00000880
	0x000008D0: 0x00000000 '....' - nop        

loc_000008D4:		; Refs: 0x00000844 
	0x000008D4: 0x24180042 'B..$' - li         $t8, 66
	0x000008D8: 0x1738FFDD '..8.' - bne        $t9, $t8, loc_00000850
; Data ref 0x000039E0 "mediasync.c:%s:This is not PBP\n"
	0x000008DC: 0x3C020000 '...<' - lui        $v0, 0x0
	0x000008E0: 0x820D0003 '....' - lb         $t5, 3($s0)
	0x000008E4: 0x15A4FFDB '....' - bne        $t5, $a0, loc_00000854
; Data ref 0x00003A00 "MsCheckMedia"
	0x000008E8: 0x3C0A0000 '...<' - lui        $t2, 0x0
	0x000008EC: 0x8E090008 '....' - lw         $t1, 8($s0)
	0x000008F0: 0x8E0C000C '....' - lw         $t4, 12($s0)
	0x000008F4: 0x02402021 '! @.' - move       $a0, $s2
	0x000008F8: 0x0120B021 '!. .' - move       $s6, $t1
; Data ref 0x02200C0B
	0x000008FC: 0x0C000C0B '....' - jal        SysMemForKernel_C1A26C6F
	0x00000900: 0x01899823 '#...' - subu       $s3, $t4, $t1
	0x00000904: 0x0000B821 '!...' - move       $s7, $zr
	0x00000908: 0x04400033 '3.@.' - bltz       $v0, loc_000009D8
	0x0000090C: 0x00408821 '!.@.' - move       $s1, $v0
	0x00000910: 0x02C03021 '!0..' - move       $a2, $s6
	0x00000914: 0x02E03821 '!8..' - move       $a3, $s7
	0x00000918: 0x02802021 '! ..' - move       $a0, $s4
	0x0000091C: 0x0C000BDF '....' - jal        sceIoLseek
	0x00000920: 0x00004021 '!@..' - move       $t0, $zr
	0x00000924: 0x04600023 '#.`.' - bltz       $v1, loc_000009B4
	0x00000928: 0x0040B021 '!.@.' - move       $s6, $v0
	0x0000092C: 0x27B00020 ' ..'' - addiu      $s0, $sp, 32
	0x00000930: 0x24040001 '...$' - li         $a0, 1
	0x00000934: 0x02002821 '!(..' - move       $a1, $s0
	0x00000938: 0x24060001 '...$' - li         $a2, 1
	0x0000093C: 0x02603821 '!8`.' - move       $a3, $s3
	0x00000940: 0x0C000C03 '....' - jal        SysMemForKernel_7158CE7E
	0x00000944: 0x00004021 '!@..' - move       $t0, $zr
	0x00000948: 0x04400010 '..@.' - bltz       $v0, loc_0000098C
	0x0000094C: 0x00409021 '!.@.' - move       $s2, $v0
	0x00000950: 0x0C000C11 '....' - jal        SysMemForKernel_F12A62F7
	0x00000954: 0x00402021 '! @.' - move       $a0, $v0
	0x00000958: 0x00402821 '!(@.' - move       $a1, $v0
	0x0000095C: 0x02802021 '! ..' - move       $a0, $s4
	0x00000960: 0x02603021 '!0`.' - move       $a2, $s3
	0x00000964: 0x0C000BE5 '....' - jal        sceIoRead
	0x00000968: 0x00408021 '!.@.' - move       $s0, $v0
	0x0000096C: 0x0440FFBE '..@.' - bltz       $v0, loc_00000868
	0x00000970: 0x00408821 '!.@.' - move       $s1, $v0
	0x00000974: 0x02002021 '! ..' - move       $a0, $s0
	0x00000978: 0x00402821 '!(@.' - move       $a1, $v0
	0x0000097C: 0x0C0003D0 '....' - jal        sub_00000F40
	0x00000980: 0x02A03021 '!0..' - move       $a2, $s5
	0x00000984: 0x0800021A '....' - j          loc_00000868
	0x00000988: 0x00408821 '!.@.' - move       $s1, $v0

loc_0000098C:		; Refs: 0x00000948 
; Data ref 0x00003A4C "mediasync.c:%s:sceKernelAllocPartitionMemory failed %s:0x%08x:size 0x%x\n"
	0x0000098C: 0x3C030000 '...<' - lui        $v1, 0x0
; Data ref 0x00003A00 "MsCheckMedia"
	0x00000990: 0x3C080000 '...<' - lui        $t0, 0x0
; Data ref 0x00003A00 "MsCheckMedia"
	0x00000994: 0x25053A00 '.:.%' - addiu      $a1, $t0, 14848
; Data ref 0x00003A4C "mediasync.c:%s:sceKernelAllocPartitionMemory failed %s:0x%08x:size 0x%x\n"
	0x00000998: 0x24643A4C 'L:d$' - addiu      $a0, $v1, 14924
	0x0000099C: 0x02003021 '!0..' - move       $a2, $s0
	0x000009A0: 0x02604021 '!@`.' - move       $t0, $s3
; Data ref 0x021A0BF1
	0x000009A4: 0x0C000BF1 '....' - jal        Kprintf
	0x000009A8: 0x00403821 '!8@.' - move       $a3, $v0
	0x000009AC: 0x0800021A '....' - j          loc_00000868
	0x000009B0: 0x02408821 '!.@.' - move       $s1, $s2

loc_000009B4:		; Refs: 0x00000924 
; Data ref 0x00003A98 "mediasync.c:%s:sceIoLseek failed, 0x%x\n"
	0x000009B4: 0x3C0F0000 '...<' - lui        $t7, 0x0
; Data ref 0x00003A00 "MsCheckMedia"
	0x000009B8: 0x3C060000 '...<' - lui        $a2, 0x0
; Data ref 0x00003A00 "MsCheckMedia"
	0x000009BC: 0x24C53A00 '.:.$' - addiu      $a1, $a2, 14848
; Data ref 0x00003A98 "mediasync.c:%s:sceIoLseek failed, 0x%x\n"
	0x000009C0: 0x25E43A98 '.:.%' - addiu      $a0, $t7, 15000
	0x000009C4: 0x00403021 '!0@.' - move       $a2, $v0
; Data ref 0x021A0BF1
	0x000009C8: 0x0C000BF1 '....' - jal        Kprintf
	0x000009CC: 0x00603821 '!8`.' - move       $a3, $v1
	0x000009D0: 0x08000220 ' ...' - j          loc_00000880
	0x000009D4: 0x02C08821 '!...' - move       $s1, $s6

loc_000009D8:		; Refs: 0x00000908 
; Data ref 0x00003AC0 "mediasync.c:%s:sceKernelFreePartitionMemory failed %s:0x%08x\n"
	0x000009D8: 0x3C070000 '...<' - lui        $a3, 0x0
; Data ref 0x00003A00 "MsCheckMedia"
	0x000009DC: 0x3C0B0000 '...<' - lui        $t3, 0x0
; Data ref 0x00003AC0 "mediasync.c:%s:sceKernelFreePartitionMemory failed %s:0x%08x\n"
	0x000009E0: 0x24E43AC0 '.:.$' - addiu      $a0, $a3, 15040
; Data ref 0x00003A00 "MsCheckMedia"
	0x000009E4: 0x25653A00 '.:e%' - addiu      $a1, $t3, 14848
	0x000009E8: 0x03A03021 '!0..' - move       $a2, $sp
; Data ref 0x02200BF1
	0x000009EC: 0x0C000BF1 '....' - jal        Kprintf
	0x000009F0: 0x00403821 '!8@.' - move       $a3, $v0
	0x000009F4: 0x08000220 ' ...' - j          loc_00000880
	0x000009F8: 0x00000000 '....' - nop        

loc_000009FC:		; Refs: 0x0000082C 
; Data ref 0x00003B00 "mediasync.c:%s:read header failed 0x%08x\n"
	0x000009FC: 0x3C160000 '...<' - lui        $s6, 0x0
; Data ref 0x00003A00 "MsCheckMedia"
	0x00000A00: 0x3C150000 '...<' - lui        $s5, 0x0
; Data ref 0x00003B00 "mediasync.c:%s:read header failed 0x%08x\n"
	0x00000A04: 0x26C43B00 '.;.&' - addiu      $a0, $s6, 15104
; Data ref 0x00003A00 "MsCheckMedia"
	0x00000A08: 0x26A53A00 '.:.&' - addiu      $a1, $s5, 14848
; Data ref 0x02200BF1
	0x00000A0C: 0x0C000BF1 '....' - jal        Kprintf
	0x00000A10: 0x00403021 '!0@.' - move       $a2, $v0
	0x00000A14: 0x0800021A '....' - j          loc_00000868
	0x00000A18: 0x02608821 '!.`.' - move       $s1, $s3

loc_00000A1C:		; Refs: 0x00000808 
; Data ref 0x00003B2C "mediasync.c:%s:sceIoOpen failed [%s] 0x%08x\n"
	0x00000A1C: 0x3C130000 '...<' - lui        $s3, 0x0
; Data ref 0x00003A00 "MsCheckMedia"
	0x00000A20: 0x3C120000 '...<' - lui        $s2, 0x0
; Data ref 0x00003B2C "mediasync.c:%s:sceIoOpen failed [%s] 0x%08x\n"
	0x00000A24: 0x26643B2C ',;d&' - addiu      $a0, $s3, 15148
; Data ref 0x00003A00 "MsCheckMedia"
	0x00000A28: 0x26453A00 '.:E&' - addiu      $a1, $s2, 14848
	0x00000A2C: 0x02003021 '!0..' - move       $a2, $s0
; Data ref 0x021A0BF1
	0x00000A30: 0x0C000BF1 '....' - jal        Kprintf
	0x00000A34: 0x00403821 '!8@.' - move       $a3, $v0
	0x00000A38: 0x08000223 '#...' - j          loc_0000088C
	0x00000A3C: 0x02801021 '!...' - move       $v0, $s4

loc_00000A40:		; Refs: 0x000007F0 
; Data ref 0x00003B5C "mediasync.c:%s:sceKernelAllocPartitionMemory failed %s:0x%08x\n"
	0x00000A40: 0x3C050000 '...<' - lui        $a1, 0x0
; Data ref 0x00003A00 "MsCheckMedia"
	0x00000A44: 0x3C110000 '...<' - lui        $s1, 0x0
; Data ref 0x00003B5C "mediasync.c:%s:sceKernelAllocPartitionMemory failed %s:0x%08x\n"
	0x00000A48: 0x24A43B5C '\;.$' - addiu      $a0, $a1, 15196
	0x00000A4C: 0x03A03021 '!0..' - move       $a2, $sp
; Data ref 0x00003A00 "MsCheckMedia"
	0x00000A50: 0x26253A00 '.:%&' - addiu      $a1, $s1, 14848
; Data ref 0x02230BF1
	0x00000A54: 0x0C000BF1 '....' - jal        Kprintf
	0x00000A58: 0x00403821 '!8@.' - move       $a3, $v0
	0x00000A5C: 0x08000223 '#...' - j          loc_0000088C
	0x00000A60: 0x02401021 '!.@.' - move       $v0, $s2

; ======================================================
; Subroutine sub_00000A64 - Address 0x00000A64 
sub_00000A64:		; Refs: 0x00000190 
	0x00000A64: 0x3C050098 '...<' - lui        $a1, 0x98
; Data ref 0x00003B9C "SceMediaSyncEvEf"
	0x00000A68: 0x3C020000 '...<' - lui        $v0, 0x0
	0x00000A6C: 0x27BDFFD0 '...'' - addiu      $sp, $sp, -48
	0x00000A70: 0x34A39680 '...4' - ori        $v1, $a1, 0x9680
; Data ref 0x00003B9C "SceMediaSyncEvEf"
	0x00000A74: 0x24443B9C '.;D$' - addiu      $a0, $v0, 15260
	0x00000A78: 0x00002821 '!(..' - move       $a1, $zr
	0x00000A7C: 0x00003021 '!0..' - move       $a2, $zr
	0x00000A80: 0x00003821 '!8..' - move       $a3, $zr
	0x00000A84: 0xAFB00010 '....' - sw         $s0, 16($sp)
	0x00000A88: 0xAFBF0024 '$...' - sw         $ra, 36($sp)
	0x00000A8C: 0xAFB40020 ' ...' - sw         $s4, 32($sp)
	0x00000A90: 0xAFB3001C '....' - sw         $s3, 28($sp)
	0x00000A94: 0xAFB20018 '....' - sw         $s2, 24($sp)
	0x00000A98: 0xAFB10014 '....' - sw         $s1, 20($sp)
; Data ref 0x02230C27
	0x00000A9C: 0x0C000C27 ''...' - jal        sceKernelCreateEventFlag
	0x00000AA0: 0xAFA30008 '....' - sw         $v1, 8($sp)
	0x00000AA4: 0x04400093 '..@.' - bltz       $v0, loc_00000CF4
	0x00000AA8: 0x00408021 '!.@.' - move       $s0, $v0
; Data ref 0x00003BB0 "SceMediaSyncEf"
	0x00000AAC: 0x3C090000 '...<' - lui        $t1, 0x0
; Data ref 0x00000EE0 ... 0x27BDFFF0 0x00A01821 0x3C020000 0x3C050000 
	0x00000AB0: 0x3C080000 '...<' - lui        $t0, 0x0
; Data ref 0x00003BB0 "SceMediaSyncEf"
	0x00000AB4: 0x25243BB0 '.;$%' - addiu      $a0, $t1, 15280
; Data ref 0x00000EE0 ... 0x27BDFFF0 0x00A01821 0x3C020000 0x3C050000 
	0x00000AB8: 0x25050EE0 '...%' - addiu      $a1, $t0, 3808
; Data ref 0x00004A50 ... 0x00000000 0x00000000 0x00000000 0x00000000 
	0x00000ABC: 0x3C140000 '...<' - lui        $s4, 0x0
	0x00000AC0: 0x00003021 '!0..' - move       $a2, $zr
; Data ref 0x00003BC0 "fatef0:"
	0x00000AC4: 0x3C130000 '...<' - lui        $s3, 0x0
	0x00000AC8: 0x0C000C33 '3...' - jal        sceKernelCreateCallback
; Data ref 0x00004A50 ... 0x00000000 0x00000000 0x00000000 0x00000000 
	0x00000ACC: 0xAE824A50 'PJ..' - sw         $v0, 19024($s4)
	0x00000AD0: 0x3C070241 'A..<' - lui        $a3, 0x241
	0x00000AD4: 0x34E55821 '!X.4' - ori        $a1, $a3, 0x5821
; Data ref 0x00003BC0 "fatef0:"
	0x00000AD8: 0x26643BC0 '.;d&' - addiu      $a0, $s3, 15296
	0x00000ADC: 0x03A03021 '!0..' - move       $a2, $sp
	0x00000AE0: 0x24070004 '...$' - li         $a3, 4
	0x00000AE4: 0x00004021 '!@..' - move       $t0, $zr
	0x00000AE8: 0x00004821 '!H..' - move       $t1, $zr
	0x00000AEC: 0x00408821 '!.@.' - move       $s1, $v0
	0x00000AF0: 0x0C000BE3 '....' - jal        sceIoDevctl
	0x00000AF4: 0xAFA20000 '....' - sw         $v0, 0($sp)
	0x00000AF8: 0x00408021 '!.@.' - move       $s0, $v0
	0x00000AFC: 0x04400075 'u.@.' - bltz       $v0, loc_00000CD4
; Data ref 0x00004A50 ... 0x00000000 0x00000000 0x00000000 0x00000000 
	0x00000B00: 0x26924A50 'PJ.&' - addiu      $s2, $s4, 19024
; Data ref 0x00004A50 ... 0x00000000 0x00000000 0x00000000 0x00000000 
	0x00000B04: 0x8E844A50 'PJ..' - lw         $a0, 19024($s4)
	0x00000B08: 0x24050001 '...$' - li         $a1, 1
	0x00000B0C: 0x24060011 '...$' - li         $a2, 17
	0x00000B10: 0x27A70004 '...'' - addiu      $a3, $sp, 4
	0x00000B14: 0x0C000C25 '%...' - jal        sceKernelWaitEventFlagCB
	0x00000B18: 0x27A80008 '...'' - addiu      $t0, $sp, 8
	0x00000B1C: 0x04400006 '..@.' - bltz       $v0, loc_00000B38
	0x00000B20: 0x00408021 '!.@.' - move       $s0, $v0
	0x00000B24: 0x8E4D0004 '..M.' - lw         $t5, 4($s2)
	0x00000B28: 0x3C0E8002 '...<' - lui        $t6, 0x8002
	0x00000B2C: 0x35C4014E 'N..5' - ori        $a0, $t6, 0x14E
	0x00000B30: 0x39AC0001 '...9' - xori       $t4, $t5, 0x1
	0x00000B34: 0x008C800B '....' - movn       $s0, $a0, $t4

loc_00000B38:		; Refs: 0x00000B1C 
	0x00000B38: 0x3C0F0241 'A..<' - lui        $t7, 0x241

loc_00000B3C:		; Refs: 0x00000CEC 
; Data ref 0x00003BC0 "fatef0:"
	0x00000B3C: 0x26643BC0 '.;d&' - addiu      $a0, $s3, 15296
	0x00000B40: 0x35E55822 '"X.5' - ori        $a1, $t7, 0x5822
	0x00000B44: 0x27A6000C '...'' - addiu      $a2, $sp, 12
	0x00000B48: 0x24070004 '...$' - li         $a3, 4
	0x00000B4C: 0x00004021 '!@..' - move       $t0, $zr
	0x00000B50: 0x00004821 '!H..' - move       $t1, $zr
	0x00000B54: 0x0C000BE3 '....' - jal        sceIoDevctl
	0x00000B58: 0xAFB1000C '....' - sw         $s1, 12($sp)
	0x00000B5C: 0x04400056 'V.@.' - bltz       $v0, loc_00000CB8
; Data ref 0x00003BC8 "mediasync.c:%s:sceFatefUnRegisterNotifyCallback failed 0x%08x\n"
	0x00000B60: 0x3C130000 '...<' - lui        $s3, 0x0

loc_00000B64:		; Refs: 0x00000CCC 
	0x00000B64: 0x0C000C35 '5...' - jal        sceKernelDeleteCallback
	0x00000B68: 0x02202021 '!  .' - move       $a0, $s1
	0x00000B6C: 0x0440004B 'K.@.' - bltz       $v0, loc_00000C9C
; Data ref 0x00003838 "mediasync.c:%s:sceKernelDeleteCallback failed 0x%08x\n"
	0x00000B70: 0x3C180000 '...<' - lui        $t8, 0x0

loc_00000B74:		; Refs: 0x00000CB0 0x00000D0C 
	0x00000B74: 0x0C000C37 '7...' - jal        sceKernelDeleteEventFlag
; Data ref 0x00004A50 ... 0x00000000 0x00000000 0x00000000 0x00000000 
	0x00000B78: 0x8E844A50 'PJ..' - lw         $a0, 19024($s4)
	0x00000B7C: 0x04400040 '@.@.' - bltz       $v0, loc_00000C80
; Data ref 0x00003870 "mediasync.c:%s:sceKernelDeleteEventFlag failed 0x%08x\n"
	0x00000B80: 0x3C190000 '...<' - lui        $t9, 0x0

loc_00000B84:		; Refs: 0x00000C94 
	0x00000B84: 0x06020019 '....' - bltzl      $s0, loc_00000BEC
	0x00000B88: 0x02001021 '!...' - move       $v0, $s0
	0x00000B8C: 0x0C000BD9 '....' - jal        sceKernelInitFileName
	0x00000B90: 0x00000000 '....' - nop        
	0x00000B94: 0x00408021 '!.@.' - move       $s0, $v0
	0x00000B98: 0x3C058002 '...<' - lui        $a1, 0x8002
	0x00000B9C: 0x12000013 '....' - beqz       $s0, loc_00000BEC
	0x00000BA0: 0x34A2014E 'N..4' - ori        $v0, $a1, 0x14E
	0x00000BA4: 0x0C000BD3 '....' - jal        sceKernelInitApitype
	0x00000BA8: 0x00000000 '....' - nop        
	0x00000BAC: 0x2443FEEC '..C$' - addiu      $v1, $v0, -276
	0x00000BB0: 0x2C67005E '^.g,' - sltiu      $a3, $v1, 94
	0x00000BB4: 0x10E00029 ')...' - beqz       $a3, loc_00000C5C
	0x00000BB8: 0x00403021 '!0@.' - move       $a2, $v0
; Data ref 0x000044AC ... 0x00000C24 0x00000C24 0x00000C5C 0x00000C5C 
	0x00000BBC: 0x3C0A0000 '...<' - lui        $t2, 0x0
	0x00000BC0: 0x00034880 '.H..' - sll        $t1, $v1, 2
; Data ref 0x000044AC ... 0x00000C24 0x00000C24 0x00000C5C 0x00000C5C 
	0x00000BC4: 0x254244AC '.DB%' - addiu      $v0, $t2, 17580
	0x00000BC8: 0x01224021 '!@".' - addu       $t0, $t1, $v0
	0x00000BCC: 0x8D030000 '....' - lw         $v1, 0($t0)
	0x00000BD0: 0x00600008 '..`.' - jr         $v1
	0x00000BD4: 0x00000000 '....' - nop        
	0x00000BD8: 0x0C0001D1 '....' - jal        sub_00000744
	0x00000BDC: 0x02002021 '! ..' - move       $a0, $s0
	0x00000BE0: 0x0440000A '..@.' - bltz       $v0, loc_00000C0C
	0x00000BE4: 0x00408021 '!.@.' - move       $s0, $v0

loc_00000BE8:		; Refs: 0x00000C24 0x00000C34 0x00000C78 
	0x00000BE8: 0x02001021 '!...' - move       $v0, $s0

loc_00000BEC:		; Refs: 0x00000B84 0x00000B9C 0x00000C54 
	0x00000BEC: 0x8FBF0024 '$...' - lw         $ra, 36($sp)
	0x00000BF0: 0x8FB40020 ' ...' - lw         $s4, 32($sp)
	0x00000BF4: 0x8FB3001C '....' - lw         $s3, 28($sp)
	0x00000BF8: 0x8FB20018 '....' - lw         $s2, 24($sp)
	0x00000BFC: 0x8FB10014 '....' - lw         $s1, 20($sp)
	0x00000C00: 0x8FB00010 '....' - lw         $s0, 16($sp)
	0x00000C04: 0x03E00008 '....' - jr         $ra
	0x00000C08: 0x27BD0030 '0..'' - addiu      $sp, $sp, 48

loc_00000C0C:		; Refs: 0x00000BE0 
; Data ref 0x000038A8 "mediasync.c:%s:Warning: MsCheckMediaFailed 0x%08x\n"
	0x00000C0C: 0x3C0B0000 '...<' - lui        $t3, 0x0
; Data ref 0x00003C08 "WaitEflash"
	0x00000C10: 0x3C100000 '...<' - lui        $s0, 0x0
	0x00000C14: 0x00403021 '!0@.' - move       $a2, $v0
; Data ref 0x000038A8 "mediasync.c:%s:Warning: MsCheckMediaFailed 0x%08x\n"
	0x00000C18: 0x256438A8 '.8d%' - addiu      $a0, $t3, 14504
	0x00000C1C: 0x0C000BF1 '....' - jal        Kprintf
; Data ref 0x00003C08 "WaitEflash"
	0x00000C20: 0x26053C08 '.<.&' - addiu      $a1, $s0, 15368
	0x00000C24: 0x080002FA '....' - j          loc_00000BE8
	0x00000C28: 0x00008021 '!...' - move       $s0, $zr
; Data ref 0x02FA01D1
	0x00000C2C: 0x0C0001D1 '....' - jal        sub_00000744
	0x00000C30: 0x02002021 '! ..' - move       $a0, $s0
	0x00000C34: 0x0441FFEC '..A.' - bgez       $v0, loc_00000BE8
	0x00000C38: 0x00408021 '!.@.' - move       $s0, $v0
; Data ref 0x000038E4 "mediasync.c:%s:MsCheckMediaFailed 0x%08x\n"
	0x00000C3C: 0x3C0C0000 '...<' - lui        $t4, 0x0
; Data ref 0x00003C08 "WaitEflash"
	0x00000C40: 0x3C060000 '...<' - lui        $a2, 0x0
; Data ref 0x00003C08 "WaitEflash"
	0x00000C44: 0x24C53C08 '.<.$' - addiu      $a1, $a2, 15368
; Data ref 0x000038E4 "mediasync.c:%s:MsCheckMediaFailed 0x%08x\n"
	0x00000C48: 0x258438E4 '.8.%' - addiu      $a0, $t4, 14564
	0x00000C4C: 0x0C000BF1 '....' - jal        Kprintf
	0x00000C50: 0x00403021 '!0@.' - move       $a2, $v0
	0x00000C54: 0x080002FB '....' - j          loc_00000BEC
	0x00000C58: 0x02001021 '!...' - move       $v0, $s0

loc_00000C5C:		; Refs: 0x00000BB4 
; Data ref 0x00003910 "mediasync.c:%s:unsupported apitype=0x%08x[%d]\n"
	0x00000C5C: 0x3C0E0000 '...<' - lui        $t6, 0x0
; Data ref 0x00003C08 "WaitEflash"
	0x00000C60: 0x3C0D0000 '...<' - lui        $t5, 0x0
; Data ref 0x00003910 "mediasync.c:%s:unsupported apitype=0x%08x[%d]\n"
	0x00000C64: 0x25C43910 '.9.%' - addiu      $a0, $t6, 14608
; Data ref 0x00003C08 "WaitEflash"
	0x00000C68: 0x25A53C08 '.<.%' - addiu      $a1, $t5, 15368
	0x00000C6C: 0x00C03821 '!8..' - move       $a3, $a2
; Data ref 0x02FB0BF1
	0x00000C70: 0x0C000BF1 '....' - jal        Kprintf
	0x00000C74: 0x3C128002 '...<' - lui        $s2, 0x8002
	0x00000C78: 0x080002FA '....' - j          loc_00000BE8
	0x00000C7C: 0x3650014E 'N.P6' - ori        $s0, $s2, 0x14E

loc_00000C80:		; Refs: 0x00000B7C 
; Data ref 0x00003C08 "WaitEflash"
	0x00000C80: 0x3C140000 '...<' - lui        $s4, 0x0
; Data ref 0x00003870 "mediasync.c:%s:sceKernelDeleteEventFlag failed 0x%08x\n"
	0x00000C84: 0x27243870 'p8$'' - addiu      $a0, $t9, 14448
; Data ref 0x00003C08 "WaitEflash"
	0x00000C88: 0x26853C08 '.<.&' - addiu      $a1, $s4, 15368
; Data ref 0x02FA0BF1
	0x00000C8C: 0x0C000BF1 '....' - jal        Kprintf
	0x00000C90: 0x00403021 '!0@.' - move       $a2, $v0
	0x00000C94: 0x080002E1 '....' - j          loc_00000B84
	0x00000C98: 0x00000000 '....' - nop        

loc_00000C9C:		; Refs: 0x00000B6C 
; Data ref 0x00003C08 "WaitEflash"
	0x00000C9C: 0x3C110000 '...<' - lui        $s1, 0x0
; Data ref 0x00003838 "mediasync.c:%s:sceKernelDeleteCallback failed 0x%08x\n"
	0x00000CA0: 0x27043838 '88.'' - addiu      $a0, $t8, 14392
; Data ref 0x00003C08 "WaitEflash"
	0x00000CA4: 0x26253C08 '.<%&' - addiu      $a1, $s1, 15368
; Data ref 0x02E10BF1
	0x00000CA8: 0x0C000BF1 '....' - jal        Kprintf
	0x00000CAC: 0x00403021 '!0@.' - move       $a2, $v0
	0x00000CB0: 0x080002DD '....' - j          loc_00000B74
	0x00000CB4: 0x00000000 '....' - nop        

loc_00000CB8:		; Refs: 0x00000B5C 
; Data ref 0x00003C08 "WaitEflash"
	0x00000CB8: 0x3C120000 '...<' - lui        $s2, 0x0
; Data ref 0x00003BC8 "mediasync.c:%s:sceFatefUnRegisterNotifyCallback failed 0x%08x\n"
	0x00000CBC: 0x26643BC8 '.;d&' - addiu      $a0, $s3, 15304
; Data ref 0x00003C08 "WaitEflash"
	0x00000CC0: 0x26453C08 '.<E&' - addiu      $a1, $s2, 15368
; Data ref 0x02DD0BF1
	0x00000CC4: 0x0C000BF1 '....' - jal        Kprintf
	0x00000CC8: 0x00403021 '!0@.' - move       $a2, $v0
	0x00000CCC: 0x080002D9 '....' - j          loc_00000B64
	0x00000CD0: 0x00000000 '....' - nop        

loc_00000CD4:		; Refs: 0x00000AFC 
; Data ref 0x00003C14 "mediasync.c:%s:sceFatefRegisterNotifyCallback failed 0x%08x\n"
	0x00000CD4: 0x3C0B0000 '...<' - lui        $t3, 0x0
; Data ref 0x00003C08 "WaitEflash"
	0x00000CD8: 0x3C0A0000 '...<' - lui        $t2, 0x0
; Data ref 0x00003C14 "mediasync.c:%s:sceFatefRegisterNotifyCallback failed 0x%08x\n"
	0x00000CDC: 0x25643C14 '.<d%' - addiu      $a0, $t3, 15380
; Data ref 0x00003C08 "WaitEflash"
	0x00000CE0: 0x25453C08 '.<E%' - addiu      $a1, $t2, 15368
; Data ref 0x02D90BF1
	0x00000CE4: 0x0C000BF1 '....' - jal        Kprintf
	0x00000CE8: 0x00403021 '!0@.' - move       $a2, $v0
	0x00000CEC: 0x080002CF '....' - j          loc_00000B3C
	0x00000CF0: 0x3C0F0241 'A..<' - lui        $t7, 0x241

loc_00000CF4:		; Refs: 0x00000AA4 
; Data ref 0x00003980 "mediasync.c:%s:sceKernelCreateEventFlag failed 0x%08x\n"
	0x00000CF4: 0x3C060000 '...<' - lui        $a2, 0x0
; Data ref 0x00003C08 "WaitEflash"
	0x00000CF8: 0x3C030000 '...<' - lui        $v1, 0x0
; Data ref 0x00003980 "mediasync.c:%s:sceKernelCreateEventFlag failed 0x%08x\n"
	0x00000CFC: 0x24C43980 '.9.$' - addiu      $a0, $a2, 14720
; Data ref 0x00003C08 "WaitEflash"
	0x00000D00: 0x24653C08 '.<e$' - addiu      $a1, $v1, 15368
; Data ref 0x02CF0BF1
	0x00000D04: 0x0C000BF1 '....' - jal        Kprintf
	0x00000D08: 0x00403021 '!0@.' - move       $a2, $v0
	0x00000D0C: 0x080002DD '....' - j          loc_00000B74
; Data ref 0x00004A50 ... 0x00000000 0x00000000 0x00000000 0x00000000 
	0x00000D10: 0x3C140000 '...<' - lui        $s4, 0x0

; ======================================================
; Subroutine sub_00000D14 - Address 0x00000D14 
sub_00000D14:		; Refs: 0x00000180 
; Data ref 0x0000366C "disc0:"
	0x00000D14: 0x3C020000 '...<' - lui        $v0, 0x0
	0x00000D18: 0x27BDFFF0 '...'' - addiu      $sp, $sp, -16
; Data ref 0x0000366C "disc0:"
	0x00000D1C: 0x2445366C 'l6E$' - addiu      $a1, $v0, 13932
	0x00000D20: 0x24040001 '...$' - li         $a0, 1
	0x00000D24: 0xAFB00000 '....' - sw         $s0, 0($sp)
	0x00000D28: 0xAFBF000C '....' - sw         $ra, 12($sp)
	0x00000D2C: 0xAFB20008 '....' - sw         $s2, 8($sp)
; Data ref 0x02DD0C55
	0x00000D30: 0x0C000C55 'U...' - jal        sceUmdActivate
	0x00000D34: 0xAFB10004 '....' - sw         $s1, 4($sp)
	0x00000D38: 0x04400066 'f.@.' - bltz       $v0, loc_00000ED4
	0x00000D3C: 0x00408021 '!.@.' - move       $s0, $v0
	0x00000D40: 0x0C000BD7 '....' - jal        InitForKernel_A18A4A8B
	0x00000D44: 0x00000000 '....' - nop        
	0x00000D48: 0x3C038002 '...<' - lui        $v1, 0x8002
	0x00000D4C: 0x00408821 '!.@.' - move       $s1, $v0
	0x00000D50: 0x10400034 '4.@.' - beqz       $v0, loc_00000E24
	0x00000D54: 0x3470014E 'N.p4' - ori        $s0, $v1, 0x14E
	0x00000D58: 0x0C000C4F 'O...' - jal        sceUmdWaitDriveStat
	0x00000D5C: 0x24040029 ')..$' - li         $a0, 41
	0x00000D60: 0x04400059 'Y.@.' - bltz       $v0, loc_00000EC8
	0x00000D64: 0x00408021 '!.@.' - move       $s0, $v0
	0x00000D68: 0x0C000C53 'S...' - jal        sceUmd_B7BF4C31
	0x00000D6C: 0x00000000 '....' - nop        
	0x00000D70: 0x30440020 ' .D0' - andi       $a0, $v0, 0x20
	0x00000D74: 0x1080004C 'L...' - beqz       $a0, loc_00000EA8
; Data ref 0x00003C54 "mediasync.c:%s:Not READABLE : 0x%08x\n"
	0x00000D78: 0x3C080000 '...<' - lui        $t0, 0x0
	0x00000D7C: 0x0C000BD3 '....' - jal        sceKernelInitApitype
	0x00000D80: 0x00000000 '....' - nop        
	0x00000D84: 0x24090123 '#..$' - li         $t1, 291
	0x00000D88: 0x1049003D '=.I.' - beq        $v0, $t1, loc_00000E80
	0x00000D8C: 0x2C4A0124 '$.J,' - sltiu      $t2, $v0, 292
	0x00000D90: 0x11400033 '3.@.' - beqz       $t2, loc_00000E60
	0x00000D94: 0x240D0125 '%..$' - li         $t5, 293
	0x00000D98: 0x244CFEEE '..L$' - addiu      $t4, $v0, -274
	0x00000D9C: 0x2D8B0004 '...-' - sltiu      $t3, $t4, 4
	0x00000DA0: 0x1160002D '-.`.' - beqz       $t3, loc_00000E58
	0x00000DA4: 0x3C048002 '...<' - lui        $a0, 0x8002
	0x00000DA8: 0x0C000C0F '....' - jal        SysMemForKernel_EF29061C
	0x00000DAC: 0x00000000 '....' - nop        
	0x00000DB0: 0x24520018 '..R$' - addiu      $s2, $v0, 24
	0x00000DB4: 0x24510008 '..Q$' - addiu      $s1, $v0, 8
	0x00000DB8: 0x02202021 '!  .' - move       $a0, $s1
	0x00000DBC: 0x02402821 '!(@.' - move       $a1, $s2
	0x00000DC0: 0x0C000C1B '....' - jal        strncmp
	0x00000DC4: 0x2406000A '...$' - li         $a2, 10
	0x00000DC8: 0x1040001D '..@.' - beqz       $v0, loc_00000E40
; Data ref 0x000036A4 "mediasync.c:%s:UMD Media Check OK\n"
	0x00000DCC: 0x3C070000 '...<' - lui        $a3, 0x0
; Data ref 0x000036C8 "DiscCheckMedia"
	0x00000DD0: 0x3C060000 '...<' - lui        $a2, 0x0
; Data ref 0x000036C8 "DiscCheckMedia"
	0x00000DD4: 0x24D036C8 '.6.$' - addiu      $s0, $a2, 14024
; Data ref 0x000036D8 "mediasync.c:%s:SCE_MEDIASYNC_ERROR_INVALID_MEDIA\n"
	0x00000DD8: 0x3C050000 '...<' - lui        $a1, 0x0
; Data ref 0x000036D8 "mediasync.c:%s:SCE_MEDIASYNC_ERROR_INVALID_MEDIA\n"
	0x00000DDC: 0x24A436D8 '.6.$' - addiu      $a0, $a1, 14040
	0x00000DE0: 0x0C000BF1 '....' - jal        Kprintf
	0x00000DE4: 0x02002821 '!(..' - move       $a1, $s0
; Data ref 0x0000370C "mediasync.c:%s:[%16s] and [%16s]\n"
	0x00000DE8: 0x3C030000 '...<' - lui        $v1, 0x0
	0x00000DEC: 0x02002821 '!(..' - move       $a1, $s0
; Data ref 0x0000370C "mediasync.c:%s:[%16s] and [%16s]\n"
	0x00000DF0: 0x2464370C '.7d$' - addiu      $a0, $v1, 14092
	0x00000DF4: 0x02203021 '!0 .' - move       $a2, $s1
	0x00000DF8: 0x0C000BF1 '....' - jal        Kprintf
	0x00000DFC: 0x02403821 '!8@.' - move       $a3, $s2
	0x00000E00: 0x3C188002 '...<' - lui        $t8, 0x8002
; Data ref 0x00003730 "mediasync.c:%s:DiscCheckMedia failed 0x%08x\n"
	0x00000E04: 0x3C020000 '...<' - lui        $v0, 0x0
; Data ref 0x00003C7C "WaitDiscEmu"
	0x00000E08: 0x3C190000 '...<' - lui        $t9, 0x0
	0x00000E0C: 0x3706014E 'N..7' - ori        $a2, $t8, 0x14E
; Data ref 0x00003730 "mediasync.c:%s:DiscCheckMedia failed 0x%08x\n"
	0x00000E10: 0x24443730 '07D$' - addiu      $a0, $v0, 14128
; Data ref 0x00003C7C "WaitDiscEmu"
	0x00000E14: 0x27253C7C '|<%'' - addiu      $a1, $t9, 15484
	0x00000E18: 0x00C08021 '!...' - move       $s0, $a2

loc_00000E1C:		; Refs: 0x00000EA0 
	0x00000E1C: 0x0C000BF1 '....' - jal        Kprintf
	0x00000E20: 0x00000000 '....' - nop        

loc_00000E24:		; Refs: 0x00000D50 0x00000E50 0x00000E58 0x00000E78 0x00000EC0 
	0x00000E24: 0x02001021 '!...' - move       $v0, $s0
	0x00000E28: 0x8FBF000C '....' - lw         $ra, 12($sp)
	0x00000E2C: 0x8FB20008 '....' - lw         $s2, 8($sp)
	0x00000E30: 0x8FB10004 '....' - lw         $s1, 4($sp)
	0x00000E34: 0x8FB00000 '....' - lw         $s0, 0($sp)
	0x00000E38: 0x03E00008 '....' - jr         $ra
	0x00000E3C: 0x27BD0010 '...'' - addiu      $sp, $sp, 16

loc_00000E40:		; Refs: 0x00000DC8 
; Data ref 0x000036C8 "DiscCheckMedia"
	0x00000E40: 0x3C110000 '...<' - lui        $s1, 0x0
; Data ref 0x000036A4 "mediasync.c:%s:UMD Media Check OK\n"
	0x00000E44: 0x24E436A4 '.6.$' - addiu      $a0, $a3, 13988
	0x00000E48: 0x0C000BF1 '....' - jal        Kprintf
; Data ref 0x000036C8 "DiscCheckMedia"
	0x00000E4C: 0x262536C8 '.6%&' - addiu      $a1, $s1, 14024

loc_00000E50:		; Refs: 0x00000E90 
	0x00000E50: 0x08000389 '....' - j          loc_00000E24
	0x00000E54: 0x00008021 '!...' - move       $s0, $zr

loc_00000E58:		; Refs: 0x00000DA0 0x00000E68 
	0x00000E58: 0x08000389 '....' - j          loc_00000E24
	0x00000E5C: 0x3490014E 'N..4' - ori        $s0, $a0, 0x14E

loc_00000E60:		; Refs: 0x00000D90 
	0x00000E60: 0x104D0007 '..M.' - beq        $v0, $t5, loc_00000E80
	0x00000E64: 0x2C4E0125 '%.N,' - sltiu      $t6, $v0, 293
	0x00000E68: 0x15C0FFFB '....' - bnez       $t6, loc_00000E58
	0x00000E6C: 0x3C048002 '...<' - lui        $a0, 0x8002
	0x00000E70: 0x2450FE90 '..P$' - addiu      $s0, $v0, -368
	0x00000E74: 0x2E0F0002 '....' - sltiu      $t7, $s0, 2
	0x00000E78: 0x11E0FFEA '....' - beqz       $t7, loc_00000E24
	0x00000E7C: 0x3490014E 'N..4' - ori        $s0, $a0, 0x14E

loc_00000E80:		; Refs: 0x00000D88 0x00000E60 
; Data ref 0x038901D1
	0x00000E80: 0x0C0001D1 '....' - jal        sub_00000744
	0x00000E84: 0x02202021 '!  .' - move       $a0, $s1
; Data ref 0x00003C88 "mediasync.c:%s:MsCheckMedia failed 0x%08x\n"
	0x00000E88: 0x3C120000 '...<' - lui        $s2, 0x0
	0x00000E8C: 0x00408021 '!.@.' - move       $s0, $v0
	0x00000E90: 0x0441FFEF '..A.' - bgez       $v0, loc_00000E50
; Data ref 0x00003C88 "mediasync.c:%s:MsCheckMedia failed 0x%08x\n"
	0x00000E94: 0x26443C88 '.<D&' - addiu      $a0, $s2, 15496

loc_00000E98:		; Refs: 0x00000ECC 0x00000ED8 
; Data ref 0x00003C7C "WaitDiscEmu"
	0x00000E98: 0x3C060000 '...<' - lui        $a2, 0x0
; Data ref 0x00003C7C "WaitDiscEmu"
	0x00000E9C: 0x24C53C7C '|<.$' - addiu      $a1, $a2, 15484
	0x00000EA0: 0x08000387 '....' - j          loc_00000E1C
	0x00000EA4: 0x02003021 '!0..' - move       $a2, $s0

loc_00000EA8:		; Refs: 0x00000D74 
; Data ref 0x00003C7C "WaitDiscEmu"
	0x00000EA8: 0x3C070000 '...<' - lui        $a3, 0x0
	0x00000EAC: 0x00403021 '!0@.' - move       $a2, $v0
; Data ref 0x00003C54 "mediasync.c:%s:Not READABLE : 0x%08x\n"
	0x00000EB0: 0x25043C54 'T<.%' - addiu      $a0, $t0, 15444
; Data ref 0x00003C7C "WaitDiscEmu"
	0x00000EB4: 0x24E53C7C '|<.$' - addiu      $a1, $a3, 15484
; Data ref 0x03870BF1
	0x00000EB8: 0x0C000BF1 '....' - jal        Kprintf
	0x00000EBC: 0x3C118001 '...<' - lui        $s1, 0x8001
	0x00000EC0: 0x08000389 '....' - j          loc_00000E24
	0x00000EC4: 0x36300087 '..06' - ori        $s0, $s1, 0x87

loc_00000EC8:		; Refs: 0x00000D60 
; Data ref 0x00003CB4 "mediasync.c:%s:Cannot wait UMD event : 0x%08x\n"
	0x00000EC8: 0x3C050000 '...<' - lui        $a1, 0x0
	0x00000ECC: 0x080003A6 '....' - j          loc_00000E98
; Data ref 0x00003CB4 "mediasync.c:%s:Cannot wait UMD event : 0x%08x\n"
	0x00000ED0: 0x24A43CB4 '.<.$' - addiu      $a0, $a1, 15540

loc_00000ED4:		; Refs: 0x00000D38 
; Data ref 0x00003CE4 "mediasync.c:%s:Cannot activate UMD : 0x%08x\n"
	0x00000ED4: 0x3C020000 '...<' - lui        $v0, 0x0
	0x00000ED8: 0x080003A6 '....' - j          loc_00000E98
; Data ref 0x00003CE4 "mediasync.c:%s:Cannot activate UMD : 0x%08x\n"
	0x00000EDC: 0x24443CE4 '.<D$' - addiu      $a0, $v0, 15588
	0x00000EE0: 0x27BDFFF0 '...'' - addiu      $sp, $sp, -16
	0x00000EE4: 0x00A01821 '!...' - move       $v1, $a1
; Data ref 0x00003D14 "MsCallback"
	0x00000EE8: 0x3C020000 '...<' - lui        $v0, 0x0
; Data ref 0x00003D20 "mediasync.c:%s:EVENT, arg=0x%08x\n"
	0x00000EEC: 0x3C050000 '...<' - lui        $a1, 0x0
	0x00000EF0: 0xAFB00000 '....' - sw         $s0, 0($sp)
; Data ref 0x00004A50 ... 0x00000000 0x00000000 0x00000000 0x00000000 
	0x00000EF4: 0x3C100000 '...<' - lui        $s0, 0x0
; Data ref 0x00003D20 "mediasync.c:%s:EVENT, arg=0x%08x\n"
	0x00000EF8: 0x24A43D20 ' =.$' - addiu      $a0, $a1, 15648
; Data ref 0x00004A50 ... 0x00000000 0x00000000 0x00000000 0x00000000 
	0x00000EFC: 0x26074A50 'PJ.&' - addiu      $a3, $s0, 19024
; Data ref 0x00003D14 "MsCallback"
	0x00000F00: 0x24453D14 '.=E$' - addiu      $a1, $v0, 15636
	0x00000F04: 0x24020001 '...$' - li         $v0, 1
	0x00000F08: 0xAFBF0004 '....' - sw         $ra, 4($sp)
	0x00000F0C: 0x00603021 '!0`.' - move       $a2, $v1
	0x00000F10: 0x10620003 '..b.' - beq        $v1, $v0, loc_00000F20
	0x00000F14: 0xACE30004 '....' - sw         $v1, 4($a3)
; Data ref 0x03A60BF1
	0x00000F18: 0x0C000BF1 '....' - jal        Kprintf
	0x00000F1C: 0x00000000 '....' - nop        

loc_00000F20:		; Refs: 0x00000F10 
; Data ref 0x00004A50 ... 0x00000000 0x00000000 0x00000000 0x00000000 
	0x00000F20: 0x8E044A50 'PJ..' - lw         $a0, 19024($s0)
	0x00000F24: 0x0C000C23 '#...' - jal        sceKernelSetEventFlag
	0x00000F28: 0x24050001 '...$' - li         $a1, 1
	0x00000F2C: 0x8FBF0004 '....' - lw         $ra, 4($sp)
	0x00000F30: 0x8FB00000 '....' - lw         $s0, 0($sp)
	0x00000F34: 0x00001021 '!...' - move       $v0, $zr
	0x00000F38: 0x03E00008 '....' - jr         $ra
	0x00000F3C: 0x27BD0010 '...'' - addiu      $sp, $sp, 16

; ======================================================
; Subroutine sub_00000F40 - Address 0x00000F40 
sub_00000F40:		; Refs: 0x0000097C 
	0x00000F40: 0x27BDFFC0 '...'' - addiu      $sp, $sp, -64
	0x00000F44: 0xAFB40030 '0...' - sw         $s4, 48($sp)
; Data ref 0x00004A50 ... 0x00000000 0x00000000 0x00000000 0x00000000 
	0x00000F48: 0x3C140000 '...<' - lui        $s4, 0x0
; Data ref 0x00004A50 ... 0x00000000 0x00000000 0x00000000 0x00000000 
	0x00000F4C: 0x26874A50 'PJ.&' - addiu      $a3, $s4, 19024
	0x00000F50: 0x2403FFFF '...$' - li         $v1, -1
	0x00000F54: 0xACE30008 '....' - sw         $v1, 8($a3)
; Data ref 0x000012C4 ... 0x3C030000 0x00802821 0x8C644A58 0x27BDFFF0 
	0x00000F58: 0x3C020000 '...<' - lui        $v0, 0x0
	0x00000F5C: 0xAFB3002C ',...' - sw         $s3, 44($sp)
	0x00000F60: 0x00A09821 '!...' - move       $s3, $a1
; Data ref 0x00001180 ... 0x27BDFFF0 0xAFB20008 0x3C120000 0x3C020000 
	0x00000F64: 0x3C050000 '...<' - lui        $a1, 0x0
	0x00000F68: 0xAFB20028 '(...' - sw         $s2, 40($sp)
	0x00000F6C: 0x00809021 '!...' - move       $s2, $a0
; Data ref 0x00001180 ... 0x27BDFFF0 0xAFB20008 0x3C120000 0x3C020000 
	0x00000F70: 0x24A41180 '...$' - addiu      $a0, $a1, 4480
; Data ref 0x000012C4 ... 0x3C030000 0x00802821 0x8C644A58 0x27BDFFF0 
	0x00000F74: 0x244512C4 '..E$' - addiu      $a1, $v0, 4804
	0x00000F78: 0xAFBF0034 '4...' - sw         $ra, 52($sp)
	0x00000F7C: 0xAFB10024 '$...' - sw         $s1, 36($sp)
	0x00000F80: 0x00C08821 '!...' - move       $s1, $a2
	0x00000F84: 0x0C000B22 '"...' - jal        sub_00002C88
	0x00000F88: 0xAFB00020 ' ...' - sw         $s0, 32($sp)
	0x00000F8C: 0x02402021 '! @.' - move       $a0, $s2
	0x00000F90: 0x02602821 '!(`.' - move       $a1, $s3
	0x00000F94: 0x0C000876 'v...' - jal        sub_000021D8
	0x00000F98: 0x27A60010 '...'' - addiu      $a2, $sp, 16
	0x00000F9C: 0x1440006E 'n.@.' - bnez       $v0, loc_00001158
	0x00000FA0: 0x00408021 '!.@.' - move       $s0, $v0
	0x00000FA4: 0x8FA40010 '....' - lw         $a0, 16($sp)
; Data ref 0x00003D44 "DISC_ID"
	0x00000FA8: 0x3C080000 '...<' - lui        $t0, 0x0
; Data ref 0x00003D44 "DISC_ID"
	0x00000FAC: 0x25053D44 'D=.%' - addiu      $a1, $t0, 15684
	0x00000FB0: 0x0C000A52 'R...' - jal        sub_00002948
	0x00000FB4: 0x27A60014 '...'' - addiu      $a2, $sp, 20
; Data ref 0x00003D4C "mediasync.c:%s:sceSystemFileGetIndex failed (res=0x%x)\n"
	0x00000FB8: 0x3C090000 '...<' - lui        $t1, 0x0
	0x00000FBC: 0x00408021 '!.@.' - move       $s0, $v0
	0x00000FC0: 0x1440005F '_.@.' - bnez       $v0, loc_00001140
; Data ref 0x00003D4C "mediasync.c:%s:sceSystemFileGetIndex failed (res=0x%x)\n"
	0x00000FC4: 0x25243D4C 'L=$%' - addiu      $a0, $t1, 15692
	0x00000FC8: 0x8FA40010 '....' - lw         $a0, 16($sp)
	0x00000FCC: 0x8FA50014 '....' - lw         $a1, 20($sp)
	0x00000FD0: 0x03A03021 '!0..' - move       $a2, $sp
	0x00000FD4: 0x0C0007E2 '....' - jal        sub_00001F88
	0x00000FD8: 0x24070010 '...$' - li         $a3, 16
	0x00000FDC: 0x14400056 'V.@.' - bnez       $v0, loc_00001138
	0x00000FE0: 0x00408021 '!.@.' - move       $s0, $v0
	0x00000FE4: 0x922400B4 '..$.' - lbu        $a0, 180($s1)
	0x00000FE8: 0x50800008 '...P' - beqzl      $a0, loc_0000100C
	0x00000FEC: 0x26310044 'D.1&' - addiu      $s1, $s1, 68
	0x00000FF0: 0x262400B4 '..$&' - addiu      $a0, $s1, 180
	0x00000FF4: 0x03A02821 '!(..' - move       $a1, $sp
	0x00000FF8: 0x0C000C1B '....' - jal        strncmp
	0x00000FFC: 0x2406000A '...$' - li         $a2, 10
	0x00001000: 0x10400009 '..@.' - beqz       $v0, loc_00001028
	0x00001004: 0x8FA40010 '....' - lw         $a0, 16($sp)
	0x00001008: 0x26310044 'D.1&' - addiu      $s1, $s1, 68

loc_0000100C:		; Refs: 0x00000FE8 
	0x0000100C: 0x02202021 '!  .' - move       $a0, $s1
	0x00001010: 0x03A02821 '!(..' - move       $a1, $sp
	0x00001014: 0x0C000C1B '....' - jal        strncmp
	0x00001018: 0x2406000D '...$' - li         $a2, 13
	0x0000101C: 0x1440001A '..@.' - bnez       $v0, loc_00001088
; Data ref 0x00003D84 "ULJS00167"
	0x00001020: 0x3C0D0000 '...<' - lui        $t5, 0x0

loc_00001024:		; Refs: 0x000010B4 0x000010F0 
	0x00001024: 0x8FA40010 '....' - lw         $a0, 16($sp)

loc_00001028:		; Refs: 0x00001000 
	0x00001028: 0x0C0008A2 '....' - jal        sub_00002288
; Data ref 0x00004A50 ... 0x00000000 0x00000000 0x00000000 0x00000000 
	0x0000102C: 0x26914A50 'PJ.&' - addiu      $s1, $s4, 19024
	0x00001030: 0x0C000C0D '....' - jal        SysMemForKernel_DD6512D0
	0x00001034: 0x8E240008 '..$.' - lw         $a0, 8($s1)
	0x00001038: 0x0440000C '..@.' - bltz       $v0, loc_0000106C
; Data ref 0x00003D90 "mediasync.c:%s:sceKernelDeleteHeap failed 0x%08x\n"
	0x0000103C: 0x3C140000 '...<' - lui        $s4, 0x0
	0x00001040: 0x2405FFFF '...$' - li         $a1, -1

loc_00001044:		; Refs: 0x00001080 
	0x00001044: 0xAE250008 '..%.' - sw         $a1, 8($s1)
	0x00001048: 0x02001021 '!...' - move       $v0, $s0

loc_0000104C:		; Refs: 0x00001150 0x00001178 
	0x0000104C: 0x8FBF0034 '4...' - lw         $ra, 52($sp)
	0x00001050: 0x8FB40030 '0...' - lw         $s4, 48($sp)
	0x00001054: 0x8FB3002C ',...' - lw         $s3, 44($sp)
	0x00001058: 0x8FB20028 '(...' - lw         $s2, 40($sp)
	0x0000105C: 0x8FB10024 '$...' - lw         $s1, 36($sp)
	0x00001060: 0x8FB00020 ' ...' - lw         $s0, 32($sp)
	0x00001064: 0x03E00008 '....' - jr         $ra
	0x00001068: 0x27BD0040 '@..'' - addiu      $sp, $sp, 64

loc_0000106C:		; Refs: 0x00001038 
; Data ref 0x00003DC4 "MsSystemFile"
	0x0000106C: 0x3C120000 '...<' - lui        $s2, 0x0
; Data ref 0x00003D90 "mediasync.c:%s:sceKernelDeleteHeap failed 0x%08x\n"
	0x00001070: 0x26843D90 '.=.&' - addiu      $a0, $s4, 15760
; Data ref 0x00003DC4 "MsSystemFile"
	0x00001074: 0x26453DC4 '.=E&' - addiu      $a1, $s2, 15812
	0x00001078: 0x0C000BF1 '....' - jal        Kprintf
	0x0000107C: 0x02003021 '!0..' - move       $a2, $s0
	0x00001080: 0x08000411 '....' - j          loc_00001044
	0x00001084: 0x2405FFFF '...$' - li         $a1, -1

loc_00001088:		; Refs: 0x0000101C 
; Data ref 0x00003D84 "ULJS00167"
	0x00001088: 0x25A53D84 '.=.%' - addiu      $a1, $t5, 15748
	0x0000108C: 0x02202021 '!  .' - move       $a0, $s1
; Data ref 0x04110C1B
	0x00001090: 0x0C000C1B '....' - jal        strncmp
	0x00001094: 0x24060009 '...$' - li         $a2, 9
; Data ref 0x0000419C "NPJH90038"
	0x00001098: 0x3C0C0000 '...<' - lui        $t4, 0x0
	0x0000109C: 0x14400016 '..@.' - bnez       $v0, loc_000010F8
; Data ref 0x0000419C "NPJH90038"
	0x000010A0: 0x2592419C '.A.%' - addiu      $s2, $t4, 16796

loc_000010A4:		; Refs: 0x00001128 
	0x000010A4: 0x02402021 '! @.' - move       $a0, $s2

loc_000010A8:		; Refs: 0x0000110C 0x00001130 
	0x000010A8: 0x03A02821 '!(..' - move       $a1, $sp
	0x000010AC: 0x0C000C1B '....' - jal        strncmp
	0x000010B0: 0x2406000D '...$' - li         $a2, 13
	0x000010B4: 0x1040FFDB '..@.' - beqz       $v0, loc_00001024
	0x000010B8: 0x00008021 '!...' - move       $s0, $zr
; Data ref 0x00003DC4 "MsSystemFile"
	0x000010BC: 0x3C030000 '...<' - lui        $v1, 0x0
; Data ref 0x00003DC4 "MsSystemFile"
	0x000010C0: 0x24733DC4 '.=s$' - addiu      $s3, $v1, 15812
; Data ref 0x000036D8 "mediasync.c:%s:SCE_MEDIASYNC_ERROR_INVALID_MEDIA\n"
	0x000010C4: 0x3C190000 '...<' - lui        $t9, 0x0
; Data ref 0x000036D8 "mediasync.c:%s:SCE_MEDIASYNC_ERROR_INVALID_MEDIA\n"
	0x000010C8: 0x272436D8 '.6$'' - addiu      $a0, $t9, 14040
	0x000010CC: 0x0C000BF1 '....' - jal        Kprintf
	0x000010D0: 0x02602821 '!(`.' - move       $a1, $s3
; Data ref 0x0000370C "mediasync.c:%s:[%16s] and [%16s]\n"
	0x000010D4: 0x3C180000 '...<' - lui        $t8, 0x0
; Data ref 0x0000370C "mediasync.c:%s:[%16s] and [%16s]\n"
	0x000010D8: 0x2704370C '.7.'' - addiu      $a0, $t8, 14092
	0x000010DC: 0x02602821 '!(`.' - move       $a1, $s3
	0x000010E0: 0x02403021 '!0@.' - move       $a2, $s2
	0x000010E4: 0x03A03821 '!8..' - move       $a3, $sp
	0x000010E8: 0x0C000BF1 '....' - jal        Kprintf
	0x000010EC: 0x3C118002 '...<' - lui        $s1, 0x8002
	0x000010F0: 0x08000409 '....' - j          loc_00001024
	0x000010F4: 0x3630014E 'N.06' - ori        $s0, $s1, 0x14E

loc_000010F8:		; Refs: 0x0000109C 
; Data ref 0x00003DD4 "ULJS00168"
	0x000010F8: 0x3C0E0000 '...<' - lui        $t6, 0x0
; Data ref 0x00003DD4 "ULJS00168"
	0x000010FC: 0x25C53DD4 '.=.%' - addiu      $a1, $t6, 15828
	0x00001100: 0x02202021 '!  .' - move       $a0, $s1
; Data ref 0x04090C1B
	0x00001104: 0x0C000C1B '....' - jal        strncmp
	0x00001108: 0x24060009 '...$' - li         $a2, 9
	0x0000110C: 0x1040FFE6 '..@.' - beqz       $v0, loc_000010A8
	0x00001110: 0x02402021 '! @.' - move       $a0, $s2
; Data ref 0x00003DE0 "ULJS00169"
	0x00001114: 0x3C0F0000 '...<' - lui        $t7, 0x0
; Data ref 0x00003DE0 "ULJS00169"
	0x00001118: 0x25E53DE0 '.=.%' - addiu      $a1, $t7, 15840
	0x0000111C: 0x02202021 '!  .' - move       $a0, $s1
	0x00001120: 0x0C000C1B '....' - jal        strncmp
	0x00001124: 0x24060009 '...$' - li         $a2, 9
	0x00001128: 0x5440FFDE '..@T' - bnezl      $v0, loc_000010A4
	0x0000112C: 0x02209021 '!. .' - move       $s2, $s1
	0x00001130: 0x0800042A '*...' - j          loc_000010A8
	0x00001134: 0x02402021 '! @.' - move       $a0, $s2

loc_00001138:		; Refs: 0x00000FDC 
; Data ref 0x00003DEC "mediasync.c:%s:failed to get value of 'DISC_ID' (ret=0x%x)\n"
	0x00001138: 0x3C0A0000 '...<' - lui        $t2, 0x0
; Data ref 0x00003DEC "mediasync.c:%s:failed to get value of 'DISC_ID' (ret=0x%x)\n"
	0x0000113C: 0x25443DEC '.=D%' - addiu      $a0, $t2, 15852

loc_00001140:		; Refs: 0x00000FC0 
; Data ref 0x00003DC4 "MsSystemFile"
	0x00001140: 0x3C0B0000 '...<' - lui        $t3, 0x0
; Data ref 0x00003DC4 "MsSystemFile"
	0x00001144: 0x25653DC4 '.=e%' - addiu      $a1, $t3, 15812
; Data ref 0x042A0BF1
	0x00001148: 0x0C000BF1 '....' - jal        Kprintf
	0x0000114C: 0x02003021 '!0..' - move       $a2, $s0
	0x00001150: 0x08000413 '....' - j          loc_0000104C
	0x00001154: 0x02001021 '!...' - move       $v0, $s0

loc_00001158:		; Refs: 0x00000F9C 
; Data ref 0x00003E28 "mediasync.c:%s:sceSystemFileLoadAll2 failed 0x%08x, buf=0x%08x, size=0x%08x\n"
	0x00001158: 0x3C070000 '...<' - lui        $a3, 0x0
; Data ref 0x00003DC4 "MsSystemFile"
	0x0000115C: 0x3C060000 '...<' - lui        $a2, 0x0
; Data ref 0x00003E28 "mediasync.c:%s:sceSystemFileLoadAll2 failed 0x%08x, buf=0x%08x, size=0x%08x\n"
	0x00001160: 0x24E43E28 '(>.$' - addiu      $a0, $a3, 15912
; Data ref 0x00003DC4 "MsSystemFile"
	0x00001164: 0x24C53DC4 '.=.$' - addiu      $a1, $a2, 15812
	0x00001168: 0x02403821 '!8@.' - move       $a3, $s2
	0x0000116C: 0x02604021 '!@`.' - move       $t0, $s3
; Data ref 0x04130BF1
	0x00001170: 0x0C000BF1 '....' - jal        Kprintf
	0x00001174: 0x00403021 '!0@.' - move       $a2, $v0
	0x00001178: 0x08000413 '....' - j          loc_0000104C
	0x0000117C: 0x02001021 '!...' - move       $v0, $s0
	0x00001180: 0x27BDFFF0 '...'' - addiu      $sp, $sp, -16
	0x00001184: 0xAFB20008 '....' - sw         $s2, 8($sp)
; Data ref 0x00004A50 ... 0x00000000 0x00000000 0x00000000 0x00000000 
	0x00001188: 0x3C120000 '...<' - lui        $s2, 0x0
; Data ref 0x00003E78 "SceKernelMediaSyncHeap"
	0x0000118C: 0x3C020000 '...<' - lui        $v0, 0x0
	0x00001190: 0xAFB10004 '....' - sw         $s1, 4($sp)
; Data ref 0x00004A50 ... 0x00000000 0x00000000 0x00000000 0x00000000 
	0x00001194: 0x26514A50 'PJQ&' - addiu      $s1, $s2, 19024
; Data ref 0x00003E78 "SceKernelMediaSyncHeap"
	0x00001198: 0x24473E78 'x>G$' - addiu      $a3, $v0, 15992
	0x0000119C: 0x8E230008 '..#.' - lw         $v1, 8($s1)
	0x000011A0: 0xAFB00000 '....' - sw         $s0, 0($sp)
	0x000011A4: 0x24051000 '...$' - li         $a1, 4096
	0x000011A8: 0x00808021 '!...' - move       $s0, $a0
	0x000011AC: 0xAFBF000C '....' - sw         $ra, 12($sp)
	0x000011B0: 0x24060001 '...$' - li         $a2, 1
	0x000011B4: 0x0460000D '..`.' - bltz       $v1, loc_000011EC
	0x000011B8: 0x24040001 '...$' - li         $a0, 1

loc_000011BC:		; Refs: 0x00001208 
; Data ref 0x00004A50 ... 0x00000000 0x00000000 0x00000000 0x00000000 
	0x000011BC: 0x26464A50 'PJF&' - addiu      $a2, $s2, 19024
	0x000011C0: 0x8CC40008 '....' - lw         $a0, 8($a2)
; Data ref 0x04130BFF
	0x000011C4: 0x0C000BFF '....' - jal        SysMemForKernel_23D81675
	0x000011C8: 0x02002821 '!(..' - move       $a1, $s0
	0x000011CC: 0x00401821 '!.@.' - move       $v1, $v0

loc_000011D0:		; Refs: 0x00001220 
	0x000011D0: 0x8FBF000C '....' - lw         $ra, 12($sp)
	0x000011D4: 0x8FB20008 '....' - lw         $s2, 8($sp)
	0x000011D8: 0x8FB10004 '....' - lw         $s1, 4($sp)
	0x000011DC: 0x8FB00000 '....' - lw         $s0, 0($sp)
	0x000011E0: 0x00601021 '!.`.' - move       $v0, $v1
	0x000011E4: 0x03E00008 '....' - jr         $ra
	0x000011E8: 0x27BD0010 '...'' - addiu      $sp, $sp, 16

loc_000011EC:		; Refs: 0x000011B4 
	0x000011EC: 0x0C000C01 '....' - jal        SysMemForKernel_58148F07
	0x000011F0: 0x00000000 '....' - nop        
; Data ref 0x00003E90 "sf_malloc"
	0x000011F4: 0x3C040000 '...<' - lui        $a0, 0x0
; Data ref 0x00003E9C "mediasync.c:%s:sceKernelCreateHeap failed 0x%08x\n"
	0x000011F8: 0x3C030000 '...<' - lui        $v1, 0x0
; Data ref 0x00003E90 "sf_malloc"
	0x000011FC: 0x24853E90 '.>.$' - addiu      $a1, $a0, 16016
	0x00001200: 0xAE220008 '..".' - sw         $v0, 8($s1)
	0x00001204: 0x00403021 '!0@.' - move       $a2, $v0
	0x00001208: 0x0441FFEC '..A.' - bgez       $v0, loc_000011BC
; Data ref 0x00003E9C "mediasync.c:%s:sceKernelCreateHeap failed 0x%08x\n"
	0x0000120C: 0x24643E9C '.>d$' - addiu      $a0, $v1, 16028
	0x00001210: 0x0C000BF1 '....' - jal        Kprintf
	0x00001214: 0x00000000 '....' - nop        
	0x00001218: 0x2405FFFF '...$' - li         $a1, -1
	0x0000121C: 0x00001821 '!...' - move       $v1, $zr
	0x00001220: 0x08000474 't...' - j          loc_000011D0
	0x00001224: 0xAE250008 '..%.' - sw         $a1, 8($s1)
; Data ref 0x00004184 "sceLibSysfile"
	0x00001228: 0x3C030000 '...<' - lui        $v1, 0x0
	0x0000122C: 0x27BDFFE0 '...'' - addiu      $sp, $sp, -32
	0x00001230: 0x00801021 '!...' - move       $v0, $a0
	0x00001234: 0x24870120 ' ..$' - addiu      $a3, $a0, 288
	0x00001238: 0x24050002 '...$' - li         $a1, 2
; Data ref 0x00004184 "sceLibSysfile"
	0x0000123C: 0x24644184 '.Ad$' - addiu      $a0, $v1, 16772
	0x00001240: 0x00003021 '!0..' - move       $a2, $zr
	0x00001244: 0x00004021 '!@..' - move       $t0, $zr
	0x00001248: 0xAFB10014 '....' - sw         $s1, 20($sp)
	0x0000124C: 0xAFB00010 '....' - sw         $s0, 16($sp)
	0x00001250: 0xAFBF0018 '....' - sw         $ra, 24($sp)
; Data ref 0x04740C29
	0x00001254: 0x0C000C29 ')...' - jal        sceKernelCreateVpl
	0x00001258: 0x24500020 ' .P$' - addiu      $s0, $v0, 32
	0x0000125C: 0x00408821 '!.@.' - move       $s1, $v0
	0x00001260: 0x00402021 '! @.' - move       $a0, $v0
	0x00001264: 0x02002821 '!(..' - move       $a1, $s0
	0x00001268: 0x03A03021 '!0..' - move       $a2, $sp
	0x0000126C: 0x0620000E '.. .' - bltz       $s1, loc_000012A8
	0x00001270: 0x00001021 '!...' - move       $v0, $zr
	0x00001274: 0x0C000C2D '-...' - jal        sceKernelTryAllocateVpl
	0x00001278: 0x00000000 '....' - nop        
; Data ref 0x00004194 ... 0x46535000 0x00000000 0x484A504E 0x33303039 
	0x0000127C: 0x3C040000 '...<' - lui        $a0, 0x0
	0x00001280: 0x0440000E '..@.' - bltz       $v0, loc_000012BC
; Data ref 0x00004194 ... 0x46535000 0x00000000 0x484A504E 0x33303039 
	0x00001284: 0x24854194 '.A.$' - addiu      $a1, $a0, 16788
	0x00001288: 0x8FB00000 '....' - lw         $s0, 0($sp)
	0x0000128C: 0x0C000BC7 '....' - jal        sub_00002F1C
	0x00001290: 0x02002021 '! ..' - move       $a0, $s0
	0x00001294: 0x8FA60000 '....' - lw         $a2, 0($sp)
	0x00001298: 0xAE110004 '....' - sw         $s1, 4($s0)
	0x0000129C: 0x24C50020 ' ..$' - addiu      $a1, $a2, 32
	0x000012A0: 0xAFA50000 '....' - sw         $a1, 0($sp)

loc_000012A4:		; Refs: 0x000012BC 
	0x000012A4: 0x8FA20000 '....' - lw         $v0, 0($sp)

loc_000012A8:		; Refs: 0x0000126C 
	0x000012A8: 0x8FBF0018 '....' - lw         $ra, 24($sp)
	0x000012AC: 0x8FB10014 '....' - lw         $s1, 20($sp)
	0x000012B0: 0x8FB00010 '....' - lw         $s0, 16($sp)
	0x000012B4: 0x03E00008 '....' - jr         $ra
	0x000012B8: 0x27BD0020 ' ..'' - addiu      $sp, $sp, 32

loc_000012BC:		; Refs: 0x00001280 
	0x000012BC: 0x080004A9 '....' - j          loc_000012A4
	0x000012C0: 0xAFA00000 '....' - sw         $zr, 0($sp)
; Data ref 0x00004A58 ... 0x00000000 0x00000000 0x00000000 0x00000000 
	0x000012C4: 0x3C030000 '...<' - lui        $v1, 0x0
	0x000012C8: 0x00802821 '!(..' - move       $a1, $a0
; Data ref 0x00004A58 ... 0x00000000 0x00000000 0x00000000 0x00000000 
	0x000012CC: 0x8C644A58 'XJd.' - lw         $a0, 19032($v1)
	0x000012D0: 0x27BDFFF0 '...'' - addiu      $sp, $sp, -16
	0x000012D4: 0xAFBF0000 '....' - sw         $ra, 0($sp)
; Data ref 0x04A90C05
	0x000012D8: 0x0C000C05 '....' - jal        SysMemForKernel_87C2AB85
	0x000012DC: 0x00000000 '....' - nop        
	0x000012E0: 0x00001021 '!...' - move       $v0, $zr
	0x000012E4: 0x8FBF0000 '....' - lw         $ra, 0($sp)
	0x000012E8: 0x03E00008 '....' - jr         $ra
	0x000012EC: 0x27BD0010 '...'' - addiu      $sp, $sp, 16
	0x000012F0: 0x27BDFFF0 '...'' - addiu      $sp, $sp, -16
	0x000012F4: 0xAFB00000 '....' - sw         $s0, 0($sp)
; Data ref 0x00004194 ... 0x46535000 0x00000000 0x484A504E 0x33303039 
	0x000012F8: 0x3C020000 '...<' - lui        $v0, 0x0
	0x000012FC: 0x2490FFE0 '...$' - addiu      $s0, $a0, -32
; Data ref 0x00004194 ... 0x46535000 0x00000000 0x484A504E 0x33303039 
	0x00001300: 0x24454194 '.AE$' - addiu      $a1, $v0, 16788
	0x00001304: 0x02002021 '! ..' - move       $a0, $s0
	0x00001308: 0xAFBF0008 '....' - sw         $ra, 8($sp)
	0x0000130C: 0x0C000B44 'D...' - jal        sub_00002D10
	0x00001310: 0xAFB10004 '....' - sw         $s1, 4($sp)
	0x00001314: 0x02002821 '!(..' - move       $a1, $s0
	0x00001318: 0x1440000A '..@.' - bnez       $v0, loc_00001344
	0x0000131C: 0x2403FFFF '...$' - li         $v1, -1
	0x00001320: 0x8E100004 '....' - lw         $s0, 4($s0)
	0x00001324: 0x0C000C2F '/...' - jal        sceKernelFreeVpl
	0x00001328: 0x02002021 '! ..' - move       $a0, $s0
	0x0000132C: 0x00408821 '!.@.' - move       $s1, $v0
	0x00001330: 0x1A000003 '....' - blez       $s0, loc_00001340
	0x00001334: 0x02002021 '! ..' - move       $a0, $s0
	0x00001338: 0x0C000C2B '+...' - jal        sceKernelDeleteVpl
	0x0000133C: 0x00000000 '....' - nop        

loc_00001340:		; Refs: 0x00001330 
	0x00001340: 0x02201821 '!. .' - move       $v1, $s1

loc_00001344:		; Refs: 0x00001318 
	0x00001344: 0x8FBF0008 '....' - lw         $ra, 8($sp)
	0x00001348: 0x8FB10004 '....' - lw         $s1, 4($sp)
	0x0000134C: 0x8FB00000 '....' - lw         $s0, 0($sp)
	0x00001350: 0x00601021 '!.`.' - move       $v0, $v1
	0x00001354: 0x03E00008 '....' - jr         $ra
	0x00001358: 0x27BD0010 '...'' - addiu      $sp, $sp, 16

; ======================================================
; Subroutine sub_0000135C - Address 0x0000135C 
sub_0000135C:		; Refs: 0x000000C8 
	0x0000135C: 0x27BDFE60 '`..'' - addiu      $sp, $sp, -416
	0x00001360: 0x3C037361 'as.<' - lui        $v1, 0x7361
	0x00001364: 0x3C052F3A ':/.<' - lui        $a1, 0x2F3A
	0x00001368: 0x3C09002F '/..<' - lui        $t1, 0x2F
	0x0000136C: 0x34686C66 'flh4' - ori        $t0, $v1, 0x6C66
	0x00001370: 0x34A73068 'h0.4' - ori        $a3, $a1, 0x3068
	0x00001374: 0x3526646B 'kd&5' - ori        $a2, $t1, 0x646B
	0x00001378: 0x27A40170 'p..'' - addiu      $a0, $sp, 368
	0x0000137C: 0xAFB50194 '....' - sw         $s5, 404($sp)
	0x00001380: 0x0000A821 '!...' - move       $s5, $zr
	0x00001384: 0xAFB3018C '....' - sw         $s3, 396($sp)
	0x00001388: 0xAFBF0198 '....' - sw         $ra, 408($sp)
	0x0000138C: 0xAFB40190 '....' - sw         $s4, 400($sp)
	0x00001390: 0xAFB20188 '....' - sw         $s2, 392($sp)
	0x00001394: 0xAFB10184 '....' - sw         $s1, 388($sp)
	0x00001398: 0xAFB00180 '....' - sw         $s0, 384($sp)
	0x0000139C: 0xAFA80170 'p...' - sw         $t0, 368($sp)
	0x000013A0: 0xAFA70174 't...' - sw         $a3, 372($sp)
	0x000013A4: 0x0C000BE9 '....' - jal        sceIoDopen
	0x000013A8: 0xAFA60178 'x...' - sw         $a2, 376($sp)
	0x000013AC: 0x0440002C ',.@.' - bltz       $v0, loc_00001460
	0x000013B0: 0x00409821 '!.@.' - move       $s3, $v0
; Data ref 0x00004624 ... 0xFF785CF8 0x8C134305 0xD6702E16 0xB69B8745 
	0x000013B4: 0x3C140000 '...<' - lui        $s4, 0x0

loc_000013B8:		; Refs: 0x0000144C 
	0x000013B8: 0x02602021 '! `.' - move       $a0, $s3

loc_000013BC:		; Refs: 0x00001444 
	0x000013BC: 0x0C000BEB '....' - jal        sceIoDread
	0x000013C0: 0x03A02821 '!(..' - move       $a1, $sp
	0x000013C4: 0x18400023 '#.@.' - blez       $v0, loc_00001454
	0x000013C8: 0x83A40058 'X...' - lb         $a0, 88($sp)
	0x000013CC: 0x1080000C '....' - beqz       $a0, loc_00001400
	0x000013D0: 0x00009021 '!...' - move       $s2, $zr
	0x000013D4: 0x03A08021 '!...' - move       $s0, $sp
	0x000013D8: 0x03A08821 '!...' - move       $s1, $sp

loc_000013DC:		; Refs: 0x000013F8 
	0x000013DC: 0x26100050 'P..&' - addiu      $s0, $s0, 80
	0x000013E0: 0x82040008 '....' - lb         $a0, 8($s0)
	0x000013E4: 0x26310001 '..1&' - addiu      $s1, $s1, 1
	0x000013E8: 0x0C000C15 '....' - jal        tolower
	0x000013EC: 0x26520001 '..R&' - addiu      $s2, $s2, 1
	0x000013F0: 0xA2020008 '....' - sb         $v0, 8($s0)
	0x000013F4: 0x82260058 'X.&.' - lb         $a2, 88($s1)
	0x000013F8: 0x14C0FFF8 '....' - bnez       $a2, loc_000013DC
	0x000013FC: 0x02208021 '!. .' - move       $s0, $s1

loc_00001400:		; Refs: 0x000013CC 
	0x00001400: 0x27B10160 '`..'' - addiu      $s1, $sp, 352
	0x00001404: 0x02402821 '!(@.' - move       $a1, $s2
	0x00001408: 0x27A40058 'X..'' - addiu      $a0, $sp, 88
	0x0000140C: 0x0C000C3B ';...' - jal        sceKernelUtilsMd5Digest
	0x00001410: 0x02203021 '!0 .' - move       $a2, $s1
	0x00001414: 0x00009021 '!...' - move       $s2, $zr
; Data ref 0x00004624 ... 0xFF785CF8 0x8C134305 0xD6702E16 0xB69B8745 
	0x00001418: 0x26904624 '$F.&' - addiu      $s0, $s4, 17956
	0x0000141C: 0x02002821 '!(..' - move       $a1, $s0

loc_00001420:		; Refs: 0x0000143C 
	0x00001420: 0x02202021 '!  .' - move       $a0, $s1
	0x00001424: 0x24060010 '...$' - li         $a2, 16
	0x00001428: 0x0C000C1D '....' - jal        memcmp
	0x0000142C: 0x26520001 '..R&' - addiu      $s2, $s2, 1
	0x00001430: 0x26100010 '...&' - addiu      $s0, $s0, 16
	0x00001434: 0x10400005 '..@.' - beqz       $v0, loc_0000144C
	0x00001438: 0x2E430007 '..C.' - sltiu      $v1, $s2, 7
	0x0000143C: 0x1460FFF8 '..`.' - bnez       $v1, loc_00001420
	0x00001440: 0x02002821 '!(..' - move       $a1, $s0
	0x00001444: 0x080004EF '....' - j          loc_000013BC
	0x00001448: 0x02602021 '! `.' - move       $a0, $s3

loc_0000144C:		; Refs: 0x00001434 
	0x0000144C: 0x080004EE '....' - j          loc_000013B8
	0x00001450: 0x24150001 '...$' - li         $s5, 1

loc_00001454:		; Refs: 0x000013C4 
; Data ref 0x04EE0BED
	0x00001454: 0x0C000BED '....' - jal        sceIoDclose
	0x00001458: 0x02602021 '! `.' - move       $a0, $s3
	0x0000145C: 0x02A01021 '!...' - move       $v0, $s5

loc_00001460:		; Refs: 0x000013AC 
	0x00001460: 0x8FBF0198 '....' - lw         $ra, 408($sp)
	0x00001464: 0x8FB50194 '....' - lw         $s5, 404($sp)
	0x00001468: 0x8FB40190 '....' - lw         $s4, 400($sp)
	0x0000146C: 0x8FB3018C '....' - lw         $s3, 396($sp)
	0x00001470: 0x8FB20188 '....' - lw         $s2, 392($sp)
	0x00001474: 0x8FB10184 '....' - lw         $s1, 388($sp)
	0x00001478: 0x8FB00180 '....' - lw         $s0, 384($sp)
	0x0000147C: 0x03E00008 '....' - jr         $ra
	0x00001480: 0x27BD01A0 '...'' - addiu      $sp, $sp, 416

; ======================================================
; Subroutine sub_00001484 - Address 0x00001484 
sub_00001484:		; Refs: 0x000000EC 
	0x00001484: 0x27BDFFF0 '...'' - addiu      $sp, $sp, -16
	0x00001488: 0xAFBF0004 '....' - sw         $ra, 4($sp)
	0x0000148C: 0x0C00061C '....' - jal        sub_00001870
	0x00001490: 0xAFB00000 '....' - sw         $s0, 0($sp)
	0x00001494: 0x0C00055D ']...' - jal        sub_00001574
	0x00001498: 0x00000000 '....' - nop        
	0x0000149C: 0x0C000C0F '....' - jal        SysMemForKernel_EF29061C
	0x000014A0: 0x00000000 '....' - nop        
; Data ref 0x000040B0 "ULJS00019"
	0x000014A4: 0x3C030000 '...<' - lui        $v1, 0x0
	0x000014A8: 0x24440044 'D.D$' - addiu      $a0, $v0, 68
	0x000014AC: 0x10400005 '..@.' - beqz       $v0, loc_000014C4
; Data ref 0x000040B0 "ULJS00019"
	0x000014B0: 0x246540B0 '.@e$' - addiu      $a1, $v1, 16560
	0x000014B4: 0x0C000C21 '!...' - jal        strcmp
	0x000014B8: 0x00000000 '....' - nop        
	0x000014BC: 0x10400029 ').@.' - beqz       $v0, loc_00001564
	0x000014C0: 0x24040001 '...$' - li         $a0, 1

loc_000014C4:		; Refs: 0x000014AC 0x0000156C 
	0x000014C4: 0x0C000642 'B...' - jal        sub_00001908
	0x000014C8: 0x00000000 '....' - nop        
	0x000014CC: 0x0C000C0F '....' - jal        SysMemForKernel_EF29061C
	0x000014D0: 0x00000000 '....' - nop        
	0x000014D4: 0x24500044 'D.P$' - addiu      $s0, $v0, 68
; Data ref 0x000040BC "ULJS00167"
	0x000014D8: 0x3C030000 '...<' - lui        $v1, 0x0
; Data ref 0x000040BC "ULJS00167"
	0x000014DC: 0x246540BC '.@e$' - addiu      $a1, $v1, 16572
	0x000014E0: 0x02002021 '! ..' - move       $a0, $s0
	0x000014E4: 0x1040000C '..@.' - beqz       $v0, loc_00001518
	0x000014E8: 0x24060009 '...$' - li         $a2, 9
	0x000014EC: 0x0C000C1B '....' - jal        strncmp
	0x000014F0: 0x00000000 '....' - nop        
; Data ref 0x000040C8 "ULJS00168"
	0x000014F4: 0x3C040000 '...<' - lui        $a0, 0x0
; Data ref 0x000040C8 "ULJS00168"
	0x000014F8: 0x248540C8 '.@.$' - addiu      $a1, $a0, 16584
	0x000014FC: 0x24060009 '...$' - li         $a2, 9
	0x00001500: 0x1440000B '..@.' - bnez       $v0, loc_00001530
	0x00001504: 0x02002021 '! ..' - move       $a0, $s0

loc_00001508:		; Refs: 0x00001544 
	0x00001508: 0x3C088002 '...<' - lui        $t0, 0x8002
	0x0000150C: 0x35070323 '#..5' - ori        $a3, $t0, 0x323

loc_00001510:		; Refs: 0x0000155C 
	0x00001510: 0x3C050000 '...<' - lui        $a1, 0x0
	0x00001514: 0xACA70000 '....' - sw         $a3, 0($a1)

loc_00001518:		; Refs: 0x000014E4 0x00001554 
	0x00001518: 0x0C00065F '_...' - jal        sub_0000197C
	0x0000151C: 0x00000000 '....' - nop        
	0x00001520: 0x8FBF0004 '....' - lw         $ra, 4($sp)
	0x00001524: 0x8FB00000 '....' - lw         $s0, 0($sp)
	0x00001528: 0x03E00008 '....' - jr         $ra
	0x0000152C: 0x27BD0010 '...'' - addiu      $sp, $sp, 16

loc_00001530:		; Refs: 0x00001500 
	0x00001530: 0x0C000C1B '....' - jal        strncmp
	0x00001534: 0x00000000 '....' - nop        
; Data ref 0x000040D4 "ULJS00169"
	0x00001538: 0x3C060000 '...<' - lui        $a2, 0x0
; Data ref 0x000040D4 "ULJS00169"
	0x0000153C: 0x24C540D4 '.@.$' - addiu      $a1, $a2, 16596
	0x00001540: 0x02002021 '! ..' - move       $a0, $s0
	0x00001544: 0x1040FFF0 '..@.' - beqz       $v0, loc_00001508
	0x00001548: 0x24060009 '...$' - li         $a2, 9
	0x0000154C: 0x0C000C1B '....' - jal        strncmp
	0x00001550: 0x00000000 '....' - nop        
	0x00001554: 0x1440FFF0 '..@.' - bnez       $v0, loc_00001518
	0x00001558: 0x3C088002 '...<' - lui        $t0, 0x8002
	0x0000155C: 0x08000544 'D...' - j          loc_00001510
	0x00001560: 0x35070323 '#..5' - ori        $a3, $t0, 0x323

loc_00001564:		; Refs: 0x000014BC 
; Data ref 0x05440C3D
	0x00001564: 0x0C000C3D '=...' - jal        sceAudio_driver_FF298CE7
	0x00001568: 0x00000000 '....' - nop        
	0x0000156C: 0x08000531 '1...' - j          loc_000014C4
	0x00001570: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sub_00001574 - Address 0x00001574 
sub_00001574:		; Refs: 0x00001494 
; Data ref 0x000040E0 "sceUmdCache_driver"
	0x00001574: 0x3C020000 '...<' - lui        $v0, 0x0
	0x00001578: 0x27BDFFA0 '...'' - addiu      $sp, $sp, -96
; Data ref 0x000040E0 "sceUmdCache_driver"
	0x0000157C: 0x244440E0 '.@D$' - addiu      $a0, $v0, 16608
	0x00001580: 0xAFBF0054 'T...' - sw         $ra, 84($sp)
	0x00001584: 0xAFBE0050 'P...' - sw         $fp, 80($sp)
	0x00001588: 0xAFB7004C 'L...' - sw         $s7, 76($sp)
	0x0000158C: 0xAFB60048 'H...' - sw         $s6, 72($sp)
	0x00001590: 0xAFB50044 'D...' - sw         $s5, 68($sp)
	0x00001594: 0xAFB40040 '@...' - sw         $s4, 64($sp)
	0x00001598: 0xAFB3003C '<...' - sw         $s3, 60($sp)
	0x0000159C: 0xAFB20038 '8...' - sw         $s2, 56($sp)
	0x000015A0: 0xAFB10034 '4...' - sw         $s1, 52($sp)
; Data ref 0x05310BF7
	0x000015A4: 0x0C000BF7 '....' - jal        ModuleMgrForKernel_D86DD11B
	0x000015A8: 0xAFB00030 '0...' - sw         $s0, 48($sp)
	0x000015AC: 0x04400078 'x.@.' - bltz       $v0, loc_00001790
	0x000015B0: 0x00001821 '!...' - move       $v1, $zr
	0x000015B4: 0xAFA00000 '....' - sw         $zr, 0($sp)
	0x000015B8: 0xAFA00004 '....' - sw         $zr, 4($sp)
	0x000015BC: 0xA7A00008 '....' - sh         $zr, 8($sp)
	0x000015C0: 0xA3A0000A '....' - sb         $zr, 10($sp)
	0x000015C4: 0xAFA00010 '....' - sw         $zr, 16($sp)
	0x000015C8: 0xAFA00014 '....' - sw         $zr, 20($sp)
	0x000015CC: 0xA7A00018 '....' - sh         $zr, 24($sp)
	0x000015D0: 0x0C000C5D ']...' - jal        sceUmdCache_driver_A21D8213
	0x000015D4: 0xA3A0001A '....' - sb         $zr, 26($sp)
	0x000015D8: 0x88440003 '..D.' - lwl        $a0, 3($v0)
	0x000015DC: 0x98440000 '..D.' - lwr        $a0, 0($v0)
	0x000015E0: 0x88450007 '..E.' - lwl        $a1, 7($v0)
	0x000015E4: 0x98450004 '..E.' - lwr        $a1, 4($v0)
	0x000015E8: 0x80460008 '..F.' - lb         $a2, 8($v0)
	0x000015EC: 0x80430009 '..C.' - lb         $v1, 9($v0)
	0x000015F0: 0xABA40013 '....' - swl        $a0, 19($sp)
	0x000015F4: 0xBBA40010 '....' - swr        $a0, 16($sp)
	0x000015F8: 0xABA50017 '....' - swl        $a1, 23($sp)
	0x000015FC: 0xBBA50014 '....' - swr        $a1, 20($sp)
	0x00001600: 0xA3A60018 '....' - sb         $a2, 24($sp)
	0x00001604: 0x0C000BF5 '....' - jal        KDebugForKernel_ACF427DC
	0x00001608: 0xA3A30019 '....' - sb         $v1, 25($sp)
	0x0000160C: 0x1440005B '[.@.' - bnez       $v0, loc_0000177C
	0x00001610: 0x00009821 '!...' - move       $s3, $zr
; Data ref 0x00004698 ... 0x00840000 0x00076024 0x00940000 0x00074600 
	0x00001614: 0x3C040000 '...<' - lui        $a0, 0x0

loc_00001618:		; Refs: 0x00001774 
; Data ref 0x00004698 ... 0x00840000 0x00076024 0x00940000 0x00074600 
	0x00001618: 0x24834698 '.F.$' - addiu      $v1, $a0, 18072
	0x0000161C: 0xAFA30020 ' ...' - sw         $v1, 32($sp)
	0x00001620: 0x1260000B '..`.' - beqz       $s3, loc_00001650
	0x00001624: 0x24170022 '"..$' - li         $s7, 34
; Data ref 0x000047A8 ... 0x00240001 0x00036050 0x00C40002 0x00024001 
	0x00001628: 0x3C080000 '...<' - lui        $t0, 0x0
; Data ref 0x000047A8 ... 0x00240001 0x00036050 0x00C40002 0x00024001 
	0x0000162C: 0x250747A8 '.G.%' - addiu      $a3, $t0, 18344
	0x00001630: 0x24060001 '...$' - li         $a2, 1
	0x00001634: 0xAFA70020 ' ...' - sw         $a3, 32($sp)
	0x00001638: 0x12660005 '..f.' - beq        $s3, $a2, loc_00001650
	0x0000163C: 0x2417002A '*..$' - li         $s7, 42
; Data ref 0x000048F8 ... 0x0020002B 0x00010459 0x0024002C 0x00058150 
	0x00001640: 0x3C0A0000 '...<' - lui        $t2, 0x0
; Data ref 0x000048F8 ... 0x0020002B 0x00010459 0x0024002C 0x00058150 
	0x00001644: 0x254948F8 '.HI%' - addiu      $t1, $t2, 18680
	0x00001648: 0xAFA90020 ' ...' - sw         $t1, 32($sp)
	0x0000164C: 0x24170008 '...$' - li         $s7, 8

loc_00001650:		; Refs: 0x00001620 0x00001638 
	0x00001650: 0x12E00046 'F...' - beqz       $s7, loc_0000176C
	0x00001654: 0x00009021 '!...' - move       $s2, $zr
	0x00001658: 0x8FB00020 ' ...' - lw         $s0, 32($sp)
	0x0000165C: 0x27BE0010 '...'' - addiu      $fp, $sp, 16
	0x00001660: 0x2415FF80 '...$' - li         $s5, -128
	0x00001664: 0x24140001 '...$' - li         $s4, 1
	0x00001668: 0x02008821 '!...' - move       $s1, $s0

loc_0000166C:		; Refs: 0x00001764 
	0x0000166C: 0xAFA00000 '....' - sw         $zr, 0($sp)
	0x00001670: 0xAFA00004 '....' - sw         $zr, 4($sp)
	0x00001674: 0xA7A00008 '....' - sh         $zr, 8($sp)
	0x00001678: 0x960C0002 '....' - lhu        $t4, 2($s0)
	0x0000167C: 0x318B0003 '...1' - andi       $t3, $t4, 0x3
	0x00001680: 0x15600079 'y.`.' - bnez       $t3, loc_00001868
	0x00001684: 0x0012B0C0 '....' - sll        $s6, $s2, 3
	0x00001688: 0x240D0055 'U..$' - li         $t5, 85
	0x0000168C: 0xA3AD0000 '....' - sb         $t5, 0($sp)

loc_00001690:		; Refs: 0x00001868 
	0x00001690: 0x960E0002 '....' - lhu        $t6, 2($s0)
	0x00001694: 0x7DC30880 '...}' - ext        $v1, $t6, 2, 2
	0x00001698: 0x10600071 'q.`.' - beqz       $v1, loc_00001860
	0x0000169C: 0x24020043 'C..$' - li         $v0, 67
	0x000016A0: 0x5074006F 'o.tP' - beql       $v1, $s4, loc_00001860
	0x000016A4: 0x2402004C 'L..$' - li         $v0, 76
	0x000016A8: 0xA3B50001 '....' - sb         $s5, 1($sp)

loc_000016AC:		; Refs: 0x00001860 
	0x000016AC: 0x960F0002 '....' - lhu        $t7, 2($s0)
	0x000016B0: 0x7DE31100 '...}' - ext        $v1, $t7, 4, 3
	0x000016B4: 0x10600066 'f.`.' - beqz       $v1, loc_00001850
	0x000016B8: 0x24020041 'A..$' - li         $v0, 65
	0x000016BC: 0x10740064 'd.t.' - beq        $v1, $s4, loc_00001850
	0x000016C0: 0x24020045 'E..$' - li         $v0, 69
	0x000016C4: 0x24180002 '...$' - li         $t8, 2
	0x000016C8: 0x10780061 'a.x.' - beq        $v1, $t8, loc_00001850
	0x000016CC: 0x2402004A 'J..$' - li         $v0, 74
	0x000016D0: 0x24190003 '...$' - li         $t9, 3
	0x000016D4: 0x10790060 '`.y.' - beq        $v1, $t9, loc_00001858
	0x000016D8: 0x24050004 '...$' - li         $a1, 4
	0x000016DC: 0x1065005C '\.e.' - beq        $v1, $a1, loc_00001850
	0x000016E0: 0x24020055 'U..$' - li         $v0, 85
	0x000016E4: 0xA3B50002 '....' - sb         $s5, 2($sp)

loc_000016E8:		; Refs: 0x00001850 
	0x000016E8: 0x96040002 '....' - lhu        $a0, 2($s0)
	0x000016EC: 0x7C8309C0 '...|' - ext        $v1, $a0, 7, 2
	0x000016F0: 0x10600055 'U.`.' - beqz       $v1, loc_00001848
	0x000016F4: 0x2402004D 'M..$' - li         $v0, 77
	0x000016F8: 0x50740053 'S.tP' - beql       $v1, $s4, loc_00001848
	0x000016FC: 0x24020053 'S..$' - li         $v0, 83
	0x00001700: 0xA3B50003 '....' - sb         $s5, 3($sp)

loc_00001704:		; Refs: 0x00001848 
	0x00001704: 0x2403002D '-..$' - li         $v1, 45
	0x00001708: 0xA3A30004 '....' - sb         $v1, 4($sp)
	0x0000170C: 0x00002821 '!(..' - move       $a1, $zr
	0x00001710: 0x02203021 '!0 .' - move       $a2, $s1

loc_00001714:		; Refs: 0x00001734 
	0x00001714: 0x8CCC0004 '....' - lw         $t4, 4($a2)
	0x00001718: 0x00056880 '.h..' - sll        $t5, $a1, 2
	0x0000171C: 0x00BD4021 '!@..' - addu       $t0, $a1, $sp
	0x00001720: 0x01AC5806 '.X..' - srlv       $t3, $t4, $t5
	0x00001724: 0x316A000F '..j1' - andi       $t2, $t3, 0xF
	0x00001728: 0x24A50001 '...$' - addiu      $a1, $a1, 1
	0x0000172C: 0x25490030 '0.I%' - addiu      $t1, $t2, 48
	0x00001730: 0x28A70005 '...(' - slti       $a3, $a1, 5
	0x00001734: 0x14E0FFF7 '....' - bnez       $a3, loc_00001714
	0x00001738: 0xA1090005 '....' - sb         $t1, 5($t0)
	0x0000173C: 0x12600037 '7.`.' - beqz       $s3, loc_0000181C
	0x00001740: 0x03C02021 '! ..' - move       $a0, $fp
	0x00001744: 0x12740029 ').t.' - beq        $s3, $s4, loc_000017EC
	0x00001748: 0x03A02821 '!(..' - move       $a1, $sp
	0x0000174C: 0x0C000C1D '....' - jal        memcmp
	0x00001750: 0x2406000A '...$' - li         $a2, 10
	0x00001754: 0x1040001B '..@.' - beqz       $v0, loc_000017C4
	0x00001758: 0x26520001 '..R&' - addiu      $s2, $s2, 1

loc_0000175C:		; Refs: 0x000017F4 0x00001828 
	0x0000175C: 0x0257B02A '*.W.' - slt        $s6, $s2, $s7
	0x00001760: 0x26310008 '..1&' - addiu      $s1, $s1, 8
	0x00001764: 0x16C0FFC1 '....' - bnez       $s6, loc_0000166C
	0x00001768: 0x26100008 '...&' - addiu      $s0, $s0, 8

loc_0000176C:		; Refs: 0x00001650 
	0x0000176C: 0x26730001 '..s&' - addiu      $s3, $s3, 1
	0x00001770: 0x2A700003 '..p*' - slti       $s0, $s3, 3
	0x00001774: 0x1600FFA8 '....' - bnez       $s0, loc_00001618
; Data ref 0x00004698 ... 0x00840000 0x00076024 0x00940000 0x00074600 
	0x00001778: 0x3C040000 '...<' - lui        $a0, 0x0

loc_0000177C:		; Refs: 0x0000160C 
	0x0000177C: 0x0C000C5F '_...' - jal        sceUmdCache_driver_DB97E432
	0x00001780: 0x24040001 '...$' - li         $a0, 1
	0x00001784: 0x0C000C57 'W...' - jal        sceUmd9660_driver_3CC9CE54
	0x00001788: 0x00000000 '....' - nop        
	0x0000178C: 0x24030001 '...$' - li         $v1, 1

loc_00001790:		; Refs: 0x000015AC 0x000017E4 0x00001814 0x00001840 
	0x00001790: 0x8FBF0054 'T...' - lw         $ra, 84($sp)
	0x00001794: 0x8FBE0050 'P...' - lw         $fp, 80($sp)
	0x00001798: 0x8FB7004C 'L...' - lw         $s7, 76($sp)
	0x0000179C: 0x8FB60048 'H...' - lw         $s6, 72($sp)
	0x000017A0: 0x8FB50044 'D...' - lw         $s5, 68($sp)
	0x000017A4: 0x8FB40040 '@...' - lw         $s4, 64($sp)
	0x000017A8: 0x8FB3003C '<...' - lw         $s3, 60($sp)
	0x000017AC: 0x8FB20038 '8...' - lw         $s2, 56($sp)
	0x000017B0: 0x8FB10034 '4...' - lw         $s1, 52($sp)
	0x000017B4: 0x8FB00030 '0...' - lw         $s0, 48($sp)
	0x000017B8: 0x00601021 '!.`.' - move       $v0, $v1
	0x000017BC: 0x03E00008 '....' - jr         $ra
	0x000017C0: 0x27BD0060 '`..'' - addiu      $sp, $sp, 96

loc_000017C4:		; Refs: 0x00001754 
	0x000017C4: 0x0C000C5F '_...' - jal        sceUmdCache_driver_DB97E432
	0x000017C8: 0x24040002 '...$' - li         $a0, 2
	0x000017CC: 0x0C000C57 'W...' - jal        sceUmd9660_driver_3CC9CE54
	0x000017D0: 0x00000000 '....' - nop        
	0x000017D4: 0x8FA20020 ' ...' - lw         $v0, 32($sp)
	0x000017D8: 0x02C22821 '!(..' - addu       $a1, $s6, $v0
	0x000017DC: 0x0C000C5B '[...' - jal        sceUmdCache_driver_576E0F06
	0x000017E0: 0x94A40000 '....' - lhu        $a0, 0($a1)
	0x000017E4: 0x080005E4 '....' - j          loc_00001790
	0x000017E8: 0x24030002 '...$' - li         $v1, 2

loc_000017EC:		; Refs: 0x00001744 
; Data ref 0x05E40C1D
	0x000017EC: 0x0C000C1D '....' - jal        memcmp
	0x000017F0: 0x2406000A '...$' - li         $a2, 10
	0x000017F4: 0x1440FFD9 '..@.' - bnez       $v0, loc_0000175C
	0x000017F8: 0x26520001 '..R&' - addiu      $s2, $s2, 1
	0x000017FC: 0x0C000C5F '_...' - jal        sceUmdCache_driver_DB97E432
	0x00001800: 0x24040003 '...$' - li         $a0, 3
	0x00001804: 0x0C000C57 'W...' - jal        sceUmd9660_driver_3CC9CE54
	0x00001808: 0x00000000 '....' - nop        
	0x0000180C: 0x0C000C5B '[...' - jal        sceUmdCache_driver_576E0F06
	0x00001810: 0x96040000 '....' - lhu        $a0, 0($s0)
	0x00001814: 0x080005E4 '....' - j          loc_00001790
	0x00001818: 0x24030003 '...$' - li         $v1, 3

loc_0000181C:		; Refs: 0x0000173C 
	0x0000181C: 0x03A02821 '!(..' - move       $a1, $sp
; Data ref 0x05E40C1D
	0x00001820: 0x0C000C1D '....' - jal        memcmp
	0x00001824: 0x2406000A '...$' - li         $a2, 10
	0x00001828: 0x1440FFCC '..@.' - bnez       $v0, loc_0000175C
	0x0000182C: 0x26520001 '..R&' - addiu      $s2, $s2, 1
	0x00001830: 0x0C000C5F '_...' - jal        sceUmdCache_driver_DB97E432
	0x00001834: 0x00002021 '! ..' - move       $a0, $zr
	0x00001838: 0x0C000C59 'Y...' - jal        sceUmd9660_driver_FE3A8B67
	0x0000183C: 0x00000000 '....' - nop        
	0x00001840: 0x080005E4 '....' - j          loc_00001790
	0x00001844: 0x00001821 '!...' - move       $v1, $zr

loc_00001848:		; Refs: 0x000016F0 0x000016F8 
	0x00001848: 0x080005C1 '....' - j          loc_00001704
	0x0000184C: 0xA3A20003 '....' - sb         $v0, 3($sp)

loc_00001850:		; Refs: 0x000016B4 0x000016BC 0x000016C8 0x000016DC 0x00001858 
	0x00001850: 0x080005BA '....' - j          loc_000016E8
	0x00001854: 0xA3A20002 '....' - sb         $v0, 2($sp)

loc_00001858:		; Refs: 0x000016D4 
	0x00001858: 0x08000614 '....' - j          loc_00001850
	0x0000185C: 0x2402004B 'K..$' - li         $v0, 75

loc_00001860:		; Refs: 0x00001698 0x000016A0 
	0x00001860: 0x080005AB '....' - j          loc_000016AC
	0x00001864: 0xA3A20001 '....' - sb         $v0, 1($sp)

loc_00001868:		; Refs: 0x00001680 
	0x00001868: 0x080005A4 '....' - j          loc_00001690
	0x0000186C: 0xA3B50000 '....' - sb         $s5, 0($sp)

; ======================================================
; Subroutine sub_00001870 - Address 0x00001870 
sub_00001870:		; Refs: 0x0000148C 
	0x00001870: 0x27BDFFF0 '...'' - addiu      $sp, $sp, -16
	0x00001874: 0xAFBF0000 '....' - sw         $ra, 0($sp)
; Data ref 0x05A40C0F
	0x00001878: 0x0C000C0F '....' - jal        SysMemForKernel_EF29061C
	0x0000187C: 0x00000000 '....' - nop        
	0x00001880: 0x10400006 '..@.' - beqz       $v0, loc_0000189C
	0x00001884: 0x24440044 'D.D$' - addiu      $a0, $v0, 68
; Data ref 0x000040F4 "ULJM05109"
	0x00001888: 0x3C020000 '...<' - lui        $v0, 0x0
	0x0000188C: 0x0C000C21 '!...' - jal        strcmp
; Data ref 0x000040F4 "ULJM05109"
	0x00001890: 0x244540F4 '.@E$' - addiu      $a1, $v0, 16628
	0x00001894: 0x10400018 '..@.' - beqz       $v0, loc_000018F8
	0x00001898: 0x00000000 '....' - nop        

loc_0000189C:		; Refs: 0x00001880 0x00001900 
	0x0000189C: 0x0C000BD1 '....' - jal        sceKernelApplicationType
	0x000018A0: 0x00000000 '....' - nop        
	0x000018A4: 0x24040110 '...$' - li         $a0, 272
	0x000018A8: 0x10440009 '..D.' - beq        $v0, $a0, loc_000018D0
	0x000018AC: 0x2C450111 '..E,' - sltiu      $a1, $v0, 273
	0x000018B0: 0x10A0000B '....' - beqz       $a1, loc_000018E0
	0x000018B4: 0x24070300 '...$' - li         $a3, 768
	0x000018B8: 0x24060100 '...$' - li         $a2, 256
	0x000018BC: 0x10460004 '..F.' - beq        $v0, $a2, loc_000018D0
	0x000018C0: 0x00000000 '....' - nop        
	0x000018C4: 0x8FBF0000 '....' - lw         $ra, 0($sp)

loc_000018C8:		; Refs: 0x000018D8 0x000018E0 
	0x000018C8: 0x03E00008 '....' - jr         $ra
	0x000018CC: 0x27BD0010 '...'' - addiu      $sp, $sp, 16

loc_000018D0:		; Refs: 0x000018A8 0x000018BC 0x000018F0 
	0x000018D0: 0x0C000BDB '....' - jal        InterruptManagerForKernel_E526B767
	0x000018D4: 0x00002021 '! ..' - move       $a0, $zr
	0x000018D8: 0x08000632 '2...' - j          loc_000018C8
	0x000018DC: 0x8FBF0000 '....' - lw         $ra, 0($sp)

loc_000018E0:		; Refs: 0x000018B0 
	0x000018E0: 0x5447FFF9 '..GT' - bnel       $v0, $a3, loc_000018C8
	0x000018E4: 0x8FBF0000 '....' - lw         $ra, 0($sp)
; Data ref 0x06320C31
	0x000018E8: 0x0C000C31 '1...' - jal        ThreadManForKernel_D366D35A
	0x000018EC: 0x24040001 '...$' - li         $a0, 1
	0x000018F0: 0x08000634 '4...' - j          loc_000018D0
	0x000018F4: 0x00000000 '....' - nop        

loc_000018F8:		; Refs: 0x00001894 
; Data ref 0x06340BDB
	0x000018F8: 0x0C000BDB '....' - jal        InterruptManagerForKernel_E526B767
	0x000018FC: 0x24040001 '...$' - li         $a0, 1
	0x00001900: 0x08000627 ''...' - j          loc_0000189C
	0x00001904: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sub_00001908 - Address 0x00001908 
sub_00001908:		; Refs: 0x000014C4 
	0x00001908: 0x27BDFFF0 '...'' - addiu      $sp, $sp, -16
	0x0000190C: 0xAFBF000C '....' - sw         $ra, 12($sp)
	0x00001910: 0xAFB20008 '....' - sw         $s2, 8($sp)
	0x00001914: 0xAFB10004 '....' - sw         $s1, 4($sp)
; Data ref 0x06270C0F
	0x00001918: 0x0C000C0F '....' - jal        SysMemForKernel_EF29061C
	0x0000191C: 0xAFB00000 '....' - sw         $s0, 0($sp)
	0x00001920: 0x1040000C '..@.' - beqz       $v0, loc_00001954
; Data ref 0x00004938 ... 0x00003ED0 0x00000001 0x00003EDC 0x00000001 
	0x00001924: 0x3C030000 '...<' - lui        $v1, 0x0
	0x00001928: 0x24520044 'D.R$' - addiu      $s2, $v0, 68
; Data ref 0x00004938 ... 0x00003ED0 0x00000001 0x00003EDC 0x00000001 
	0x0000192C: 0x24714938 '8Iq$' - addiu      $s1, $v1, 18744
	0x00001930: 0x00008021 '!...' - move       $s0, $zr

loc_00001934:		; Refs: 0x0000194C 
	0x00001934: 0x8E250000 '..%.' - lw         $a1, 0($s1)
	0x00001938: 0x02402021 '! @.' - move       $a0, $s2
	0x0000193C: 0x0C000C21 '!...' - jal        strcmp
	0x00001940: 0x26100001 '...&' - addiu      $s0, $s0, 1
	0x00001944: 0x10400009 '..@.' - beqz       $v0, loc_0000196C
	0x00001948: 0x2A03000C '...*' - slti       $v1, $s0, 12
	0x0000194C: 0x1460FFF9 '..`.' - bnez       $v1, loc_00001934
	0x00001950: 0x26310008 '..1&' - addiu      $s1, $s1, 8

loc_00001954:		; Refs: 0x00001920 
	0x00001954: 0x8FBF000C '....' - lw         $ra, 12($sp)

loc_00001958:		; Refs: 0x00001974 
	0x00001958: 0x8FB20008 '....' - lw         $s2, 8($sp)
	0x0000195C: 0x8FB10004 '....' - lw         $s1, 4($sp)
	0x00001960: 0x8FB00000 '....' - lw         $s0, 0($sp)
	0x00001964: 0x03E00008 '....' - jr         $ra
	0x00001968: 0x27BD0010 '...'' - addiu      $sp, $sp, 16

loc_0000196C:		; Refs: 0x00001944 
	0x0000196C: 0x0C000C3F '?...' - jal        sceCtrl_driver_5886194C
	0x00001970: 0x8E240004 '..$.' - lw         $a0, 4($s1)
	0x00001974: 0x08000656 'V...' - j          loc_00001958
	0x00001978: 0x8FBF000C '....' - lw         $ra, 12($sp)

; ======================================================
; Subroutine sub_0000197C - Address 0x0000197C 
sub_0000197C:		; Refs: 0x00001518 
	0x0000197C: 0x27BDFFC0 '...'' - addiu      $sp, $sp, -64
	0x00001980: 0xAFBF0034 '4...' - sw         $ra, 52($sp)
	0x00001984: 0xAFBE0030 '0...' - sw         $fp, 48($sp)
	0x00001988: 0xAFB7002C ',...' - sw         $s7, 44($sp)
	0x0000198C: 0xAFB60028 '(...' - sw         $s6, 40($sp)
	0x00001990: 0xAFB50024 '$...' - sw         $s5, 36($sp)
	0x00001994: 0xAFB40020 ' ...' - sw         $s4, 32($sp)
	0x00001998: 0xAFB3001C '....' - sw         $s3, 28($sp)
	0x0000199C: 0xAFB20018 '....' - sw         $s2, 24($sp)
	0x000019A0: 0xAFB10014 '....' - sw         $s1, 20($sp)
; Data ref 0x06560C0F
	0x000019A4: 0x0C000C0F '....' - jal        SysMemForKernel_EF29061C
	0x000019A8: 0xAFB00010 '....' - sw         $s0, 16($sp)
	0x000019AC: 0x10400019 '..@.' - beqz       $v0, loc_00001A14
; Data ref 0x00004998 ... 0x00003F60 0x00003F6C 0x00003F78 0x00003F84 
	0x000019B0: 0x3C030000 '...<' - lui        $v1, 0x0
	0x000019B4: 0x24520044 'D.R$' - addiu      $s2, $v0, 68
; Data ref 0x00004998 ... 0x00003F60 0x00003F6C 0x00003F78 0x00003F84 
	0x000019B8: 0x24714998 '.Iq$' - addiu      $s1, $v1, 18840
	0x000019BC: 0x24150001 '...$' - li         $s5, 1
; Data ref 0x00004100 "fatms0:"
	0x000019C0: 0x3C130000 '...<' - lui        $s3, 0x0
	0x000019C4: 0x24140050 'P..$' - li         $s4, 80
	0x000019C8: 0x24170200 '...$' - li         $s7, 512
	0x000019CC: 0x24160030 '0..$' - li         $s6, 48
; Data ref 0x00004108 "fatef0:"
	0x000019D0: 0x3C1E0000 '...<' - lui        $fp, 0x0
	0x000019D4: 0x2410001B '...$' - li         $s0, 27
	0x000019D8: 0x8E250000 '..%.' - lw         $a1, 0($s1)

loc_000019DC:		; Refs: 0x00001A0C 
	0x000019DC: 0x02402021 '! @.' - move       $a0, $s2
	0x000019E0: 0x0C000C21 '!...' - jal        strcmp
	0x000019E4: 0x2610FFFF '...&' - addiu      $s0, $s0, -1
	0x000019E8: 0x3C030242 'B..<' - lui        $v1, 0x242
; Data ref 0x00004100 "fatms0:"
	0x000019EC: 0x26644100 '.Ad&' - addiu      $a0, $s3, 16640
	0x000019F0: 0x3465585A 'ZXe4' - ori        $a1, $v1, 0x585A
	0x000019F4: 0x03A03021 '!0..' - move       $a2, $sp
	0x000019F8: 0x24070004 '...$' - li         $a3, 4
	0x000019FC: 0x00004021 '!@..' - move       $t0, $zr
	0x00001A00: 0x00004821 '!H..' - move       $t1, $zr
	0x00001A04: 0x1040000F '..@.' - beqz       $v0, loc_00001A44
	0x00001A08: 0x26310004 '..1&' - addiu      $s1, $s1, 4

loc_00001A0C:		; Refs: 0x00001A64 0x00001A8C 0x00001AB0 
	0x00001A0C: 0x0603FFF3 '....' - bgezl      $s0, loc_000019DC
	0x00001A10: 0x8E250000 '..%.' - lw         $a1, 0($s1)

loc_00001A14:		; Refs: 0x000019AC 
	0x00001A14: 0x8FBF0034 '4...' - lw         $ra, 52($sp)
	0x00001A18: 0x8FBE0030 '0...' - lw         $fp, 48($sp)
	0x00001A1C: 0x8FB7002C ',...' - lw         $s7, 44($sp)
	0x00001A20: 0x8FB60028 '(...' - lw         $s6, 40($sp)
	0x00001A24: 0x8FB50024 '$...' - lw         $s5, 36($sp)
	0x00001A28: 0x8FB40020 ' ...' - lw         $s4, 32($sp)
	0x00001A2C: 0x8FB3001C '....' - lw         $s3, 28($sp)
	0x00001A30: 0x8FB20018 '....' - lw         $s2, 24($sp)
	0x00001A34: 0x8FB10014 '....' - lw         $s1, 20($sp)
	0x00001A38: 0x8FB00010 '....' - lw         $s0, 16($sp)
	0x00001A3C: 0x03E00008 '....' - jr         $ra
	0x00001A40: 0x27BD0040 '@..'' - addiu      $sp, $sp, 64

loc_00001A44:		; Refs: 0x00001A04 
	0x00001A44: 0x0C000BE3 '....' - jal        sceIoDevctl
	0x00001A48: 0xAFB50000 '....' - sw         $s5, 0($sp)
	0x00001A4C: 0x0C000BD5 '....' - jal        InitForKernel_9D33A110
	0x00001A50: 0x00000000 '....' - nop        
	0x00001A54: 0x10540018 '..T.' - beq        $v0, $s4, loc_00001AB8
	0x00001A58: 0x00000000 '....' - nop        

loc_00001A5C:		; Refs: 0x00001AC0 
	0x00001A5C: 0x0C000BD5 '....' - jal        InitForKernel_9D33A110
	0x00001A60: 0x00000000 '....' - nop        
	0x00001A64: 0x1456FFE9 '..V.' - bne        $v0, $s6, loc_00001A0C
	0x00001A68: 0x00000000 '....' - nop        
	0x00001A6C: 0x0C000BD3 '....' - jal        sceKernelInitApitype
	0x00001A70: 0x00000000 '....' - nop        
	0x00001A74: 0x24040132 '2..$' - li         $a0, 306
	0x00001A78: 0x10440006 '..D.' - beq        $v0, $a0, loc_00001A94
	0x00001A7C: 0x3C020242 'B..<' - lui        $v0, 0x242
	0x00001A80: 0x0C000BD3 '....' - jal        sceKernelInitApitype
	0x00001A84: 0x00000000 '....' - nop        
	0x00001A88: 0x24050133 '3..$' - li         $a1, 307
	0x00001A8C: 0x1445FFDF '..E.' - bne        $v0, $a1, loc_00001A0C
	0x00001A90: 0x3C020242 'B..<' - lui        $v0, 0x242

loc_00001A94:		; Refs: 0x00001A78 
; Data ref 0x00004108 "fatef0:"
	0x00001A94: 0x27C44108 '.A.'' - addiu      $a0, $fp, 16648

loc_00001A98:		; Refs: 0x00001AC8 
	0x00001A98: 0x3445585A 'ZXE4' - ori        $a1, $v0, 0x585A
	0x00001A9C: 0x03A03021 '!0..' - move       $a2, $sp
	0x00001AA0: 0x24070004 '...$' - li         $a3, 4
	0x00001AA4: 0x00004021 '!@..' - move       $t0, $zr
	0x00001AA8: 0x0C000BE3 '....' - jal        sceIoDevctl
	0x00001AAC: 0x00004821 '!H..' - move       $t1, $zr
	0x00001AB0: 0x08000683 '....' - j          loc_00001A0C
	0x00001AB4: 0x00000000 '....' - nop        

loc_00001AB8:		; Refs: 0x00001A54 
; Data ref 0x06830BD1
	0x00001AB8: 0x0C000BD1 '....' - jal        sceKernelApplicationType
	0x00001ABC: 0x00000000 '....' - nop        
	0x00001AC0: 0x1457FFE6 '..W.' - bne        $v0, $s7, loc_00001A5C
	0x00001AC4: 0x3C020242 'B..<' - lui        $v0, 0x242
	0x00001AC8: 0x080006A6 '....' - j          loc_00001A98
; Data ref 0x00004108 "fatef0:"
	0x00001ACC: 0x27C44108 '.A.'' - addiu      $a0, $fp, 16648

; ======================================================
; Subroutine sub_00001AD0 - Address 0x00001AD0 
sub_00001AD0:		; Refs: 0x000000F4 
	0x00001AD0: 0x27BDFEA0 '...'' - addiu      $sp, $sp, -352
	0x00001AD4: 0x03A02021 '! ..' - move       $a0, $sp
	0x00001AD8: 0x00002821 '!(..' - move       $a1, $zr
	0x00001ADC: 0x24060110 '...$' - li         $a2, 272
	0x00001AE0: 0xAFBF0158 'X...' - sw         $ra, 344($sp)
	0x00001AE4: 0xAFB10154 'T...' - sw         $s1, 340($sp)
; Data ref 0x06A60C13
	0x00001AE8: 0x0C000C13 '....' - jal        memset
	0x00001AEC: 0xAFB00150 'P...' - sw         $s0, 336($sp)
	0x00001AF0: 0x24020001 '...$' - li         $v0, 1
	0x00001AF4: 0x24050002 '...$' - li         $a1, 2
	0x00001AF8: 0x27A60130 '0..'' - addiu      $a2, $sp, 304
	0x00001AFC: 0x03A02021 '! ..' - move       $a0, $sp
	0x00001B00: 0xAFA2010C '....' - sw         $v0, 268($sp)
	0x00001B04: 0xAFA20000 '....' - sw         $v0, 0($sp)
	0x00001B08: 0x0C000C49 'I...' - jal        sceReg_driver_DBA46704
	0x00001B0C: 0xAFA20108 '....' - sw         $v0, 264($sp)
; Data ref 0x00004110 "/CONFIG/NP"
	0x00001B10: 0x3C030000 '...<' - lui        $v1, 0x0
; Data ref 0x00004110 "/CONFIG/NP"
	0x00001B14: 0x24654110 '.Ae$' - addiu      $a1, $v1, 16656
	0x00001B18: 0x24060002 '...$' - li         $a2, 2
	0x00001B1C: 0x04400014 '..@.' - bltz       $v0, loc_00001B70
	0x00001B20: 0x27A70134 '4..'' - addiu      $a3, $sp, 308
	0x00001B24: 0x0C000C45 'E...' - jal        sceReg_driver_4F471457
	0x00001B28: 0x8FA40130 '0...' - lw         $a0, 304($sp)
; Data ref 0x0000411C ... 0x00766E65 0x61766E69 0x0064696C 0x4E4F432F 
	0x00001B2C: 0x3C030000 '...<' - lui        $v1, 0x0
; Data ref 0x0000411C ... 0x00766E65 0x61766E69 0x0064696C 0x4E4F432F 
	0x00001B30: 0x2465411C '.Ae$' - addiu      $a1, $v1, 16668
	0x00001B34: 0x27A80140 '@..'' - addiu      $t0, $sp, 320
	0x00001B38: 0x27A60138 '8..'' - addiu      $a2, $sp, 312
	0x00001B3C: 0x0440000A '..@.' - bltz       $v0, loc_00001B68
	0x00001B40: 0x27A7013C '<..'' - addiu      $a3, $sp, 316
	0x00001B44: 0x0C000C47 'G...' - jal        sceReg_driver_9980519F
	0x00001B48: 0x8FA40134 '4...' - lw         $a0, 308($sp)
	0x00001B4C: 0x04400004 '..@.' - bltz       $v0, loc_00001B60
	0x00001B50: 0x8FA5013C '<...' - lw         $a1, 316($sp)
	0x00001B54: 0x24040003 '...$' - li         $a0, 3
	0x00001B58: 0x10A4000A '....' - beq        $a1, $a0, loc_00001B84
	0x00001B5C: 0x8FA90140 '@...' - lw         $t1, 320($sp)

loc_00001B60:		; Refs: 0x00001B4C 0x00001B90 0x00001BB8 0x00001BE4 
	0x00001B60: 0x0C000C4D 'M...' - jal        sceReg_driver_FC742751
	0x00001B64: 0x8FA40134 '4...' - lw         $a0, 308($sp)

loc_00001B68:		; Refs: 0x00001B3C 
	0x00001B68: 0x0C000C43 'C...' - jal        sceReg_driver_49D77D65
	0x00001B6C: 0x8FA40130 '0...' - lw         $a0, 304($sp)

loc_00001B70:		; Refs: 0x00001B1C 
	0x00001B70: 0x8FBF0158 'X...' - lw         $ra, 344($sp)
	0x00001B74: 0x8FB10154 'T...' - lw         $s1, 340($sp)
	0x00001B78: 0x8FB00150 'P...' - lw         $s0, 336($sp)
	0x00001B7C: 0x03E00008 '....' - jr         $ra
	0x00001B80: 0x27BD0160 '`..'' - addiu      $sp, $sp, 352

loc_00001B84:		; Refs: 0x00001B58 
	0x00001B84: 0x27B00110 '...'' - addiu      $s0, $sp, 272
	0x00001B88: 0x24070009 '...$' - li         $a3, 9
	0x00001B8C: 0x2D28000A '..(-' - sltiu      $t0, $t1, 10
	0x00001B90: 0x1100FFF3 '....' - beqz       $t0, loc_00001B60
	0x00001B94: 0x02003021 '!0..' - move       $a2, $s0
	0x00001B98: 0x8FA50138 '8...' - lw         $a1, 312($sp)
	0x00001B9C: 0x8FA40134 '4...' - lw         $a0, 308($sp)
	0x00001BA0: 0x27B10120 ' ..'' - addiu      $s1, $sp, 288
	0x00001BA4: 0xAFA00110 '....' - sw         $zr, 272($sp)
	0x00001BA8: 0xAFA00114 '....' - sw         $zr, 276($sp)
	0x00001BAC: 0x0C000C4B 'K...' - jal        sceReg_driver_F4A3E396
	0x00001BB0: 0xA3A00118 '....' - sb         $zr, 280($sp)
	0x00001BB4: 0x02002821 '!(..' - move       $a1, $s0
	0x00001BB8: 0x0440FFE9 '..@.' - bltz       $v0, loc_00001B60
	0x00001BBC: 0x02202021 '!  .' - move       $a0, $s1
	0x00001BC0: 0x8FA70140 '@...' - lw         $a3, 320($sp)
	0x00001BC4: 0xAFA00120 ' ...' - sw         $zr, 288($sp)
	0x00001BC8: 0x03A73021 '!0..' - addu       $a2, $sp, $a3
	0x00001BCC: 0xA0C0010F '....' - sb         $zr, 271($a2)
	0x00001BD0: 0xAFA00124 '$...' - sw         $zr, 292($sp)
	0x00001BD4: 0x0C0006FB '....' - jal        sub_00001BEC
	0x00001BD8: 0xA3A00128 '(...' - sb         $zr, 296($sp)
	0x00001BDC: 0x0C000C09 '....' - jal        SysMemForKernel_A03CB480
	0x00001BE0: 0x02202021 '!  .' - move       $a0, $s1
	0x00001BE4: 0x080006D8 '....' - j          loc_00001B60
	0x00001BE8: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sub_00001BEC - Address 0x00001BEC 
sub_00001BEC:		; Refs: 0x00001BD4 
	0x00001BEC: 0x27BDFF50 'P..'' - addiu      $sp, $sp, -176
	0x00001BF0: 0xAFB200A8 '....' - sw         $s2, 168($sp)
	0x00001BF4: 0x00809021 '!...' - move       $s2, $a0
	0x00001BF8: 0xAFB100A4 '....' - sw         $s1, 164($sp)
	0x00001BFC: 0xAFBF00AC '....' - sw         $ra, 172($sp)
	0x00001C00: 0xAFB000A0 '....' - sw         $s0, 160($sp)
	0x00001C04: 0x80A30000 '....' - lb         $v1, 0($a1)
	0x00001C08: 0x14600011 '..`.' - bnez       $v1, loc_00001C50
	0x00001C0C: 0x00A08821 '!...' - move       $s1, $a1
; Data ref 0x00004120 "invalid"
	0x00001C10: 0x3C050000 '...<' - lui        $a1, 0x0

loc_00001C14:		; Refs: 0x00001D2C 
; Data ref 0x00004120 "invalid"
	0x00001C14: 0x24AA4120 ' A.$' - addiu      $t2, $a1, 16672
	0x00001C18: 0x89430003 '..C.' - lwl        $v1, 3($t2)
	0x00001C1C: 0x99430000 '..C.' - lwr        $v1, 0($t2)
	0x00001C20: 0x89440007 '..D.' - lwl        $a0, 7($t2)
	0x00001C24: 0x99440004 '..D.' - lwr        $a0, 4($t2)
	0x00001C28: 0xAA430003 '..C.' - swl        $v1, 3($s2)
	0x00001C2C: 0xBA430000 '..C.' - swr        $v1, 0($s2)
	0x00001C30: 0xAA440007 '..D.' - swl        $a0, 7($s2)
	0x00001C34: 0xBA440004 '..D.' - swr        $a0, 4($s2)
	0x00001C38: 0x8FBF00AC '....' - lw         $ra, 172($sp)

loc_00001C3C:		; Refs: 0x00001D18 
	0x00001C3C: 0x8FB200A8 '....' - lw         $s2, 168($sp)
	0x00001C40: 0x8FB100A4 '....' - lw         $s1, 164($sp)
	0x00001C44: 0x8FB000A0 '....' - lw         $s0, 160($sp)
	0x00001C48: 0x03E00008 '....' - jr         $ra
	0x00001C4C: 0x27BD00B0 '...'' - addiu      $sp, $sp, 176

loc_00001C50:		; Refs: 0x00001C08 
	0x00001C50: 0x27A40090 '...'' - addiu      $a0, $sp, 144
; Data ref 0x06D80C41
	0x00001C54: 0x0C000C41 'A...' - jal        sceOpenPSID_driver_E8316C16
	0x00001C58: 0x00002821 '!(..' - move       $a1, $zr
	0x00001C5C: 0x04400005 '..@.' - bltz       $v0, loc_00001C74
	0x00001C60: 0x27B00010 '...'' - addiu      $s0, $sp, 16
	0x00001C64: 0x93A40090 '....' - lbu        $a0, 144($sp)
	0x00001C68: 0x30830002 '...0' - andi       $v1, $a0, 0x2
	0x00001C6C: 0x54600027 ''.`T' - bnezl      $v1, loc_00001D0C
	0x00001C70: 0x02402021 '! @.' - move       $a0, $s2

loc_00001C74:		; Refs: 0x00001C5C 
	0x00001C74: 0x02002021 '! ..' - move       $a0, $s0
	0x00001C78: 0x00002821 '!(..' - move       $a1, $zr
	0x00001C7C: 0x0C000C07 '....' - jal        SysMemForKernel_8AE776AF
	0x00001C80: 0x24060080 '...$' - li         $a2, 128

loc_00001C84:		; Refs: 0x00001CA4 
	0x00001C84: 0x0C000C19 '....' - jal        strlen
	0x00001C88: 0x02002021 '! ..' - move       $a0, $s0
	0x00001C8C: 0x2C460064 'd.F,' - sltiu      $a2, $v0, 100
	0x00001C90: 0x02002021 '! ..' - move       $a0, $s0
	0x00001C94: 0x10C00005 '....' - beqz       $a2, loc_00001CAC
	0x00001C98: 0x02202821 '!( .' - move       $a1, $s1
	0x00001C9C: 0x0C000C17 '....' - jal        strcat
	0x00001CA0: 0x00000000 '....' - nop        
	0x00001CA4: 0x08000721 '!...' - j          loc_00001C84
	0x00001CA8: 0x00000000 '....' - nop        

loc_00001CAC:		; Refs: 0x00001C94 
	0x00001CAC: 0x24050080 '...$' - li         $a1, 128
; Data ref 0x07210C3B
	0x00001CB0: 0x0C000C3B ';...' - jal        sceKernelUtilsMd5Digest
	0x00001CB4: 0x03A03021 '!0..' - move       $a2, $sp
	0x00001CB8: 0x0C000BEF '....' - jal        KDebugForKernel_47570AC5
	0x00001CBC: 0x00000000 '....' - nop        
; Data ref 0x00004A08 ... 0x22368FFD 0x4D9BAD8E 0x5B6DAD92 0xCDF56413 
	0x00001CC0: 0x3C090000 '...<' - lui        $t1, 0x0
	0x00001CC4: 0x10400016 '..@.' - beqz       $v0, loc_00001D20
; Data ref 0x00004A08 ... 0x22368FFD 0x4D9BAD8E 0x5B6DAD92 0xCDF56413 
	0x00001CC8: 0x25254A08 '.J%%' - addiu      $a1, $t1, 18952
; Data ref 0x00004A08 ... 0x22368FFD 0x4D9BAD8E 0x5B6DAD92 0xCDF56413 
	0x00001CCC: 0x3C020000 '...<' - lui        $v0, 0x0
; Data ref 0x00004A08 ... 0x22368FFD 0x4D9BAD8E 0x5B6DAD92 0xCDF56413 
	0x00001CD0: 0x24454A08 '.JE$' - addiu      $a1, $v0, 18952
	0x00001CD4: 0x03A02021 '! ..' - move       $a0, $sp
	0x00001CD8: 0x0C000C1D '....' - jal        memcmp
	0x00001CDC: 0x24060010 '...$' - li         $a2, 16
	0x00001CE0: 0x1040000A '..@.' - beqz       $v0, loc_00001D0C
	0x00001CE4: 0x02402021 '! @.' - move       $a0, $s2
; Data ref 0x00004A18 ... 0x8EE5EF17 0x9330556F 0xC956E857 0x65B31C35 
	0x00001CE8: 0x3C070000 '...<' - lui        $a3, 0x0
; Data ref 0x00004A18 ... 0x8EE5EF17 0x9330556F 0xC956E857 0x65B31C35 
	0x00001CEC: 0x24E54A18 '.J.$' - addiu      $a1, $a3, 18968
	0x00001CF0: 0x03A02021 '! ..' - move       $a0, $sp
	0x00001CF4: 0x0C000C1D '....' - jal        memcmp
	0x00001CF8: 0x24060010 '...$' - li         $a2, 16
; Data ref 0x00004A28 ... 0xF91DDD4A 0xE8CA515A 0x37778DB8 0xCC18FF71 
	0x00001CFC: 0x3C080000 '...<' - lui        $t0, 0x0
	0x00001D00: 0x14400007 '..@.' - bnez       $v0, loc_00001D20
; Data ref 0x00004A28 ... 0xF91DDD4A 0xE8CA515A 0x37778DB8 0xCC18FF71 
	0x00001D04: 0x25054A28 '(J.%' - addiu      $a1, $t0, 18984
	0x00001D08: 0x02402021 '! @.' - move       $a0, $s2

loc_00001D0C:		; Refs: 0x00001C6C 0x00001CE0 0x00001D34 
	0x00001D0C: 0x02202821 '!( .' - move       $a1, $s1
	0x00001D10: 0x0C000C1F '....' - jal        strncpy
	0x00001D14: 0x24060008 '...$' - li         $a2, 8
	0x00001D18: 0x0800070F '....' - j          loc_00001C3C
	0x00001D1C: 0x8FBF00AC '....' - lw         $ra, 172($sp)

loc_00001D20:		; Refs: 0x00001CC4 0x00001D00 
	0x00001D20: 0x03A02021 '! ..' - move       $a0, $sp
; Data ref 0x070F0C1D
	0x00001D24: 0x0C000C1D '....' - jal        memcmp
	0x00001D28: 0x24060010 '...$' - li         $a2, 16
	0x00001D2C: 0x1440FFB9 '..@.' - bnez       $v0, loc_00001C14
; Data ref 0x00004120 "invalid"
	0x00001D30: 0x3C050000 '...<' - lui        $a1, 0x0
	0x00001D34: 0x08000743 'C...' - j          loc_00001D0C
	0x00001D38: 0x02402021 '! @.' - move       $a0, $s2

; ======================================================
; Subroutine sub_00001D3C - Address 0x00001D3C 
sub_00001D3C:		; Refs: 0x000000FC 
	0x00001D3C: 0x27BDFEB0 '...'' - addiu      $sp, $sp, -336
	0x00001D40: 0x03A02021 '! ..' - move       $a0, $sp
	0x00001D44: 0x00002821 '!(..' - move       $a1, $zr
	0x00001D48: 0x24060110 '...$' - li         $a2, 272
	0x00001D4C: 0xAFBF0144 'D...' - sw         $ra, 324($sp)
; Data ref 0x07430C13
	0x00001D50: 0x0C000C13 '....' - jal        memset
	0x00001D54: 0xAFB00140 '@...' - sw         $s0, 320($sp)
	0x00001D58: 0x24020001 '...$' - li         $v0, 1
	0x00001D5C: 0x27A60120 ' ..'' - addiu      $a2, $sp, 288
	0x00001D60: 0x24050002 '...$' - li         $a1, 2
	0x00001D64: 0x03A02021 '! ..' - move       $a0, $sp
	0x00001D68: 0xAFA2010C '....' - sw         $v0, 268($sp)
	0x00001D6C: 0xAFA20000 '....' - sw         $v0, 0($sp)
	0x00001D70: 0x0C000C49 'I...' - jal        sceReg_driver_DBA46704
	0x00001D74: 0xAFA20108 '....' - sw         $v0, 264($sp)
; Data ref 0x00004128 "/CONFIG/NETWORK/ADHOC"
	0x00001D78: 0x3C030000 '...<' - lui        $v1, 0x0
; Data ref 0x00004128 "/CONFIG/NETWORK/ADHOC"
	0x00001D7C: 0x24654128 '(Ae$' - addiu      $a1, $v1, 16680
	0x00001D80: 0x24060002 '...$' - li         $a2, 2
	0x00001D84: 0x04400015 '..@.' - bltz       $v0, loc_00001DDC
	0x00001D88: 0x27A70124 '$..'' - addiu      $a3, $sp, 292
	0x00001D8C: 0x0C000C45 'E...' - jal        sceReg_driver_4F471457
	0x00001D90: 0x8FA40120 ' ...' - lw         $a0, 288($sp)
; Data ref 0x00004140 "ssid_prefix"
	0x00001D94: 0x3C030000 '...<' - lui        $v1, 0x0
; Data ref 0x00004140 "ssid_prefix"
	0x00001D98: 0x24654140 '@Ae$' - addiu      $a1, $v1, 16704
	0x00001D9C: 0x27A80130 '0..'' - addiu      $t0, $sp, 304
	0x00001DA0: 0x27A60128 '(..'' - addiu      $a2, $sp, 296
	0x00001DA4: 0x0440001F '..@.' - bltz       $v0, loc_00001E24
	0x00001DA8: 0x27A7012C ',..'' - addiu      $a3, $sp, 300
	0x00001DAC: 0x0C000C47 'G...' - jal        sceReg_driver_9980519F
	0x00001DB0: 0x8FA40124 '$...' - lw         $a0, 292($sp)
	0x00001DB4: 0x04400004 '..@.' - bltz       $v0, loc_00001DC8
	0x00001DB8: 0x8FA3012C ',...' - lw         $v1, 300($sp)
	0x00001DBC: 0x24040003 '...$' - li         $a0, 3
	0x00001DC0: 0x1064000A '..d.' - beq        $v1, $a0, loc_00001DEC
	0x00001DC4: 0x8FA50130 '0...' - lw         $a1, 304($sp)

loc_00001DC8:		; Refs: 0x00001DB4 0x00001DF4 0x00001E0C 0x00001E1C 
	0x00001DC8: 0x0C000C4D 'M...' - jal        sceReg_driver_FC742751
	0x00001DCC: 0x8FA40124 '$...' - lw         $a0, 292($sp)
	0x00001DD0: 0x8FA40120 ' ...' - lw         $a0, 288($sp)

loc_00001DD4:		; Refs: 0x00001E24 
	0x00001DD4: 0x0C000C43 'C...' - jal        sceReg_driver_49D77D65
	0x00001DD8: 0x00000000 '....' - nop        

loc_00001DDC:		; Refs: 0x00001D84 
	0x00001DDC: 0x8FBF0144 'D...' - lw         $ra, 324($sp)
	0x00001DE0: 0x8FB00140 '@...' - lw         $s0, 320($sp)
	0x00001DE4: 0x03E00008 '....' - jr         $ra
	0x00001DE8: 0x27BD0150 'P..'' - addiu      $sp, $sp, 336

loc_00001DEC:		; Refs: 0x00001DC0 
	0x00001DEC: 0x27B00110 '...'' - addiu      $s0, $sp, 272
	0x00001DF0: 0x24070004 '...$' - li         $a3, 4
	0x00001DF4: 0x10A3FFF4 '....' - beq        $a1, $v1, loc_00001DC8
	0x00001DF8: 0x02003021 '!0..' - move       $a2, $s0
	0x00001DFC: 0x8FA40124 '$...' - lw         $a0, 292($sp)
	0x00001E00: 0x0C000C4B 'K...' - jal        sceReg_driver_F4A3E396
	0x00001E04: 0x8FA50128 '(...' - lw         $a1, 296($sp)
	0x00001E08: 0x02002821 '!(..' - move       $a1, $s0
	0x00001E0C: 0x0440FFEE '..@.' - bltz       $v0, loc_00001DC8
	0x00001E10: 0x00002021 '! ..' - move       $a0, $zr
	0x00001E14: 0x0C000C61 'a...' - jal        sceWlanDrv_driver_EDD207B1
	0x00001E18: 0xA3A00113 '....' - sb         $zr, 275($sp)
	0x00001E1C: 0x08000772 'r...' - j          loc_00001DC8
	0x00001E20: 0x00000000 '....' - nop        

loc_00001E24:		; Refs: 0x00001DA4 
	0x00001E24: 0x08000775 'u...' - j          loc_00001DD4
	0x00001E28: 0x8FA40124 '$...' - lw         $a0, 292($sp)

; ======================================================
; Subroutine sub_00001E2C - Address 0x00001E2C 
sub_00001E2C:		; Refs: 0x00001FD4 
	0x00001E2C: 0x27BDFFF0 '...'' - addiu      $sp, $sp, -16
	0x00001E30: 0x3C028012 '...<' - lui        $v0, 0x8012
	0x00001E34: 0xAFB20008 '....' - sw         $s2, 8($sp)
	0x00001E38: 0x34470003 '..G4' - ori        $a3, $v0, 0x3
	0x00001E3C: 0x00C09021 '!...' - move       $s2, $a2
	0x00001E40: 0xAFB10004 '....' - sw         $s1, 4($sp)
	0x00001E44: 0x00808821 '!...' - move       $s1, $a0
	0x00001E48: 0xAFB00000 '....' - sw         $s0, 0($sp)
	0x00001E4C: 0x00A08021 '!...' - move       $s0, $a1
	0x00001E50: 0x10800033 '3...' - beqz       $a0, loc_00001F20
	0x00001E54: 0xAFBF000C '....' - sw         $ra, 12($sp)
	0x00001E58: 0x3C028012 '...<' - lui        $v0, 0x8012
	0x00001E5C: 0x10A00030 '0...' - beqz       $a1, loc_00001F20
	0x00001E60: 0x34470003 '..G4' - ori        $a3, $v0, 0x3
	0x00001E64: 0x8A240003 '..$.' - lwl        $a0, 3($s1)
	0x00001E68: 0x9A240000 '..$.' - lwr        $a0, 0($s1)
	0x00001E6C: 0x30830002 '...0' - andi       $v1, $a0, 0x2
	0x00001E70: 0x5460003D '=.`T' - bnezl      $v1, loc_00001F68
	0x00001E74: 0x02402021 '! @.' - move       $a0, $s2
	0x00001E78: 0x88A30003 '....' - lwl        $v1, 3($a1)
	0x00001E7C: 0x98A30000 '....' - lwr        $v1, 0($a1)
	0x00001E80: 0x3C058012 '...<' - lui        $a1, 0x8012
	0x00001E84: 0x10600026 '&.`.' - beqz       $v1, loc_00001F20
	0x00001E88: 0x34A70003 '...4' - ori        $a3, $a1, 0x3
	0x00001E8C: 0x8A05000B '....' - lwl        $a1, 11($s0)
	0x00001E90: 0x9A050008 '....' - lwr        $a1, 8($s0)
	0x00001E94: 0x14A0002D '-...' - bnez       $a1, loc_00001F4C
	0x00001E98: 0x3C068012 '...<' - lui        $a2, 0x8012
	0x00001E9C: 0x8A24000F '..$.' - lwl        $a0, 15($s1)
	0x00001EA0: 0x9A24000C '..$.' - lwr        $a0, 12($s1)
	0x00001EA4: 0x0480001E '....' - bltz       $a0, loc_00001F20
	0x00001EA8: 0x34C70003 '...4' - ori        $a3, $a2, 0x3
	0x00001EAC: 0x8A2B0007 '..+.' - lwl        $t3, 7($s1)
	0x00001EB0: 0x9A2B0004 '..+.' - lwr        $t3, 4($s1)
	0x00001EB4: 0x886C000F '..l.' - lwl        $t4, 15($v1)
	0x00001EB8: 0x986C000C '..l.' - lwr        $t4, 12($v1)
	0x00001EBC: 0x00004021 '!@..' - move       $t0, $zr
	0x00001EC0: 0x8969000F '..i.' - lwl        $t1, 15($t3)
	0x00001EC4: 0x9969000C '..i.' - lwr        $t1, 12($t3)
	0x00001EC8: 0x00003821 '!8..' - move       $a3, $zr
; Data ref 0x07750BDF
	0x00001ECC: 0x0C000BDF '....' - jal        sceIoLseek
	0x00001ED0: 0x012C3021 '!0,.' - addu       $a2, $t1, $t4
	0x00001ED4: 0x2404FFFF '...$' - li         $a0, -1
	0x00001ED8: 0x10440018 '..D.' - beq        $v0, $a0, loc_00001F3C
	0x00001EDC: 0x3C0D8012 '...<' - lui        $t5, 0x8012

loc_00001EE0:		; Refs: 0x00001F3C 
	0x00001EE0: 0x8A180003 '....' - lwl        $t8, 3($s0)
	0x00001EE4: 0x9A180000 '....' - lwr        $t8, 0($s0)
	0x00001EE8: 0x8A24000F '..$.' - lwl        $a0, 15($s1)
	0x00001EEC: 0x9A24000C '..$.' - lwr        $a0, 12($s1)
	0x00001EF0: 0x8B060007 '....' - lwl        $a2, 7($t8)
	0x00001EF4: 0x9B060004 '....' - lwr        $a2, 4($t8)
	0x00001EF8: 0x0C000BE5 '....' - jal        sceIoRead
	0x00001EFC: 0x02402821 '!(@.' - move       $a1, $s2
	0x00001F00: 0x8A0F0003 '....' - lwl        $t7, 3($s0)
	0x00001F04: 0x9A0F0000 '....' - lwr        $t7, 0($s0)
	0x00001F08: 0x3C108012 '...<' - lui        $s0, 0x8012
	0x00001F0C: 0x89EE0007 '....' - lwl        $t6, 7($t7)
	0x00001F10: 0x99EE0004 '....' - lwr        $t6, 4($t7)
	0x00001F14: 0x144E0002 '..N.' - bne        $v0, $t6, loc_00001F20
	0x00001F18: 0x36070002 '...6' - ori        $a3, $s0, 0x2
	0x00001F1C: 0x00003821 '!8..' - move       $a3, $zr

loc_00001F20:		; Refs: 0x00001E50 0x00001E5C 0x00001E84 0x00001EA4 0x00001F14 0x00001F60 
	0x00001F20: 0x8FBF000C '....' - lw         $ra, 12($sp)

loc_00001F24:		; Refs: 0x00001F44 
	0x00001F24: 0x8FB20008 '....' - lw         $s2, 8($sp)
	0x00001F28: 0x8FB10004 '....' - lw         $s1, 4($sp)
	0x00001F2C: 0x8FB00000 '....' - lw         $s0, 0($sp)
	0x00001F30: 0x00E01021 '!...' - move       $v0, $a3
	0x00001F34: 0x03E00008 '....' - jr         $ra
	0x00001F38: 0x27BD0010 '...'' - addiu      $sp, $sp, 16

loc_00001F3C:		; Refs: 0x00001ED8 
	0x00001F3C: 0x1464FFE8 '..d.' - bne        $v1, $a0, loc_00001EE0
	0x00001F40: 0x35A70002 '...5' - ori        $a3, $t5, 0x2
	0x00001F44: 0x080007C9 '....' - j          loc_00001F24
	0x00001F48: 0x8FBF000C '....' - lw         $ra, 12($sp)

loc_00001F4C:		; Refs: 0x00001E94 
	0x00001F4C: 0x88660007 '..f.' - lwl        $a2, 7($v1)
	0x00001F50: 0x98660004 '..f.' - lwr        $a2, 4($v1)
	0x00001F54: 0x02402021 '! @.' - move       $a0, $s2

loc_00001F58:		; Refs: 0x00001F80 
; Data ref 0x07C90BFD
	0x00001F58: 0x0C000BFD '....' - jal        SysMemForKernel_181065AB
	0x00001F5C: 0x00000000 '....' - nop        
	0x00001F60: 0x080007C8 '....' - j          loc_00001F20
	0x00001F64: 0x00003821 '!8..' - move       $a3, $zr

loc_00001F68:		; Refs: 0x00001E70 
	0x00001F68: 0x8A270013 '..'.' - lwl        $a3, 19($s1)
	0x00001F6C: 0x9A270010 '..'.' - lwr        $a3, 16($s1)
	0x00001F70: 0x88A8000F '....' - lwl        $t0, 15($a1)
	0x00001F74: 0x98A8000C '....' - lwr        $t0, 12($a1)
	0x00001F78: 0x88A60007 '....' - lwl        $a2, 7($a1)
	0x00001F7C: 0x98A60004 '....' - lwr        $a2, 4($a1)
	0x00001F80: 0x080007D6 '....' - j          loc_00001F58
	0x00001F84: 0x00E82821 '!(..' - addu       $a1, $a3, $t0

; ======================================================
; Subroutine sub_00001F88 - Address 0x00001F88 
sub_00001F88:		; Refs: 0x00000FD4 
	0x00001F88: 0x27BDFFF0 '...'' - addiu      $sp, $sp, -16
	0x00001F8C: 0x3C028012 '...<' - lui        $v0, 0x8012
	0x00001F90: 0xAFBF0000 '....' - sw         $ra, 0($sp)
	0x00001F94: 0x10800012 '....' - beqz       $a0, loc_00001FE0
	0x00001F98: 0x34430003 '..C4' - ori        $v1, $v0, 0x3
	0x00001F9C: 0x3C028012 '...<' - lui        $v0, 0x8012
	0x00001FA0: 0x10A0000F '....' - beqz       $a1, loc_00001FE0
	0x00001FA4: 0x34430003 '..C4' - ori        $v1, $v0, 0x3
	0x00001FA8: 0x88890003 '....' - lwl        $t1, 3($a0)
	0x00001FAC: 0x98890000 '....' - lwr        $t1, 0($a0)
	0x00001FB0: 0x3C0A8012 '...<' - lui        $t2, 0x8012
	0x00001FB4: 0x31280002 '..(1' - andi       $t0, $t1, 0x2
	0x00001FB8: 0x1100000D '....' - beqz       $t0, loc_00001FF0
	0x00001FBC: 0x35430008 '..C5' - ori        $v1, $t2, 0x8
	0x00001FC0: 0x88A20007 '....' - lwl        $v0, 7($a1)
	0x00001FC4: 0x98A20004 '....' - lwr        $v0, 4($a1)

loc_00001FC8:		; Refs: 0x00002014 
	0x00001FC8: 0x00E2702B '+p..' - sltu       $t6, $a3, $v0
	0x00001FCC: 0x15C00005 '....' - bnez       $t6, loc_00001FE4
	0x00001FD0: 0x8FBF0000 '....' - lw         $ra, 0($sp)
; Data ref 0x07D6078B
	0x00001FD4: 0x0C00078B '....' - jal        sub_00001E2C
	0x00001FD8: 0x00000000 '....' - nop        
	0x00001FDC: 0x00401821 '!.@.' - move       $v1, $v0

loc_00001FE0:		; Refs: 0x00001F94 0x00001FA0 0x00001FFC 
	0x00001FE0: 0x8FBF0000 '....' - lw         $ra, 0($sp)

loc_00001FE4:		; Refs: 0x00001FCC 
	0x00001FE4: 0x00601021 '!.`.' - move       $v0, $v1
	0x00001FE8: 0x03E00008 '....' - jr         $ra
	0x00001FEC: 0x27BD0010 '...'' - addiu      $sp, $sp, 16

loc_00001FF0:		; Refs: 0x00001FB8 
	0x00001FF0: 0x88A20003 '....' - lwl        $v0, 3($a1)
	0x00001FF4: 0x98A20000 '....' - lwr        $v0, 0($a1)
	0x00001FF8: 0x3C0B8012 '...<' - lui        $t3, 0x8012
	0x00001FFC: 0x1040FFF8 '..@.' - beqz       $v0, loc_00001FE0
	0x00002000: 0x35630003 '..c5' - ori        $v1, $t3, 0x3
	0x00002004: 0x884D0007 '..M.' - lwl        $t5, 7($v0)
	0x00002008: 0x984D0004 '..M.' - lwr        $t5, 4($v0)
	0x0000200C: 0x3C0C8012 '...<' - lui        $t4, 0x8012
	0x00002010: 0x35830008 '...5' - ori        $v1, $t4, 0x8
	0x00002014: 0x080007F2 '....' - j          loc_00001FC8
	0x00002018: 0x01A01021 '!...' - move       $v0, $t5

; ======================================================
; Subroutine sub_0000201C - Address 0x0000201C 
sub_0000201C:		; Refs: 0x00002278 
	0x0000201C: 0x27BDFFE0 '...'' - addiu      $sp, $sp, -32
	0x00002020: 0xAFB3000C '....' - sw         $s3, 12($sp)
	0x00002024: 0x3C038012 '...<' - lui        $v1, 0x8012
	0x00002028: 0x00A09821 '!...' - move       $s3, $a1
	0x0000202C: 0xAFB10004 '....' - sw         $s1, 4($sp)
	0x00002030: 0x00002821 '!(..' - move       $a1, $zr
	0x00002034: 0x00808821 '!...' - move       $s1, $a0
	0x00002038: 0xAFBF0010 '....' - sw         $ra, 16($sp)
	0x0000203C: 0x24040001 '...$' - li         $a0, 1
	0x00002040: 0x34620003 '..b4' - ori        $v0, $v1, 0x3
	0x00002044: 0xAFB20008 '....' - sw         $s2, 8($sp)
	0x00002048: 0x1260002D '-.`.' - beqz       $s3, loc_00002100
	0x0000204C: 0xAFB00000 '....' - sw         $s0, 0($sp)
	0x00002050: 0xAE600000 '..`.' - sw         $zr, 0($s3)
	0x00002054: 0x3C068012 '...<' - lui        $a2, 0x8012
; Data ref 0x07F208CB
	0x00002058: 0x0C0008CB '....' - jal        sub_0000232C
	0x0000205C: 0x34D20004 '...4' - ori        $s2, $a2, 0x4
	0x00002060: 0x00408021 '!.@.' - move       $s0, $v0
	0x00002064: 0x3C038012 '...<' - lui        $v1, 0x8012
; Data ref 0x0000414C ... 0x46535000 0x00000000 0x3D72797C 0x30606360 
	0x00002068: 0x3C020000 '...<' - lui        $v0, 0x0
; Data ref 0x0000414C ... 0x46535000 0x00000000 0x3D72797C 0x30606360 
	0x0000206C: 0x2445414C 'LAE$' - addiu      $a1, $v0, 16716
	0x00002070: 0x7E283A00 '.:(~' - ext        $t0, $s1, 8, 8
	0x00002074: 0x7E293C00 '.<)~' - ext        $t1, $s1, 16, 8
	0x00002078: 0x00115602 '.V..' - srl        $t2, $s1, 24
	0x0000207C: 0x24060004 '...$' - li         $a2, 4
	0x00002080: 0x1200001F '....' - beqz       $s0, loc_00002100
	0x00002084: 0x34620001 '..b4' - ori        $v0, $v1, 0x1
	0x00002088: 0x8A0D0003 '....' - lwl        $t5, 3($s0)
	0x0000208C: 0x9A0D0000 '....' - lwr        $t5, 0($s0)
	0x00002090: 0xA2080005 '....' - sb         $t0, 5($s0)
	0x00002094: 0x35AB0004 '...5' - ori        $t3, $t5, 0x4
	0x00002098: 0x7D643A00 '.:d}' - ext        $a0, $t3, 8, 8
	0x0000209C: 0x000B6602 '.f..' - srl        $t4, $t3, 24
	0x000020A0: 0x7D673C00 '.<g}' - ext        $a3, $t3, 16, 8
	0x000020A4: 0xA2040001 '....' - sb         $a0, 1($s0)
	0x000020A8: 0xA2070002 '....' - sb         $a3, 2($s0)
	0x000020AC: 0xA20C0003 '....' - sb         $t4, 3($s0)
	0x000020B0: 0xA2090006 '....' - sb         $t1, 6($s0)
	0x000020B4: 0xA20A0007 '....' - sb         $t2, 7($s0)
	0x000020B8: 0xA20B0000 '....' - sb         $t3, 0($s0)
	0x000020BC: 0xA2110004 '....' - sb         $s1, 4($s0)
	0x000020C0: 0x8A040007 '....' - lwl        $a0, 7($s0)
	0x000020C4: 0x0C000B2F '/...' - jal        sub_00002CBC
	0x000020C8: 0x9A040004 '....' - lwr        $a0, 4($s0)
	0x000020CC: 0x14400009 '..@.' - bnez       $v0, loc_000020F4
	0x000020D0: 0x02002021 '! ..' - move       $a0, $s0
	0x000020D4: 0x8A050007 '....' - lwl        $a1, 7($s0)
	0x000020D8: 0x9A050004 '....' - lwr        $a1, 4($s0)
	0x000020DC: 0x3C0A8012 '...<' - lui        $t2, 0x8012
	0x000020E0: 0x24080101 '...$' - li         $t0, 257
	0x000020E4: 0x88A90007 '....' - lwl        $t1, 7($a1)
	0x000020E8: 0x98A90004 '....' - lwr        $t1, 4($a1)
	0x000020EC: 0x1128000B '..(.' - beq        $t1, $t0, loc_0000211C
	0x000020F0: 0x35520007 '..R5' - ori        $s2, $t2, 0x7

loc_000020F4:		; Refs: 0x000020CC 0x0000214C 
	0x000020F4: 0x0C0008A2 '....' - jal        sub_00002288
	0x000020F8: 0x00000000 '....' - nop        
	0x000020FC: 0x02401021 '!.@.' - move       $v0, $s2

loc_00002100:		; Refs: 0x00002048 0x00002080 0x000021D0 
	0x00002100: 0x8FBF0010 '....' - lw         $ra, 16($sp)
	0x00002104: 0x8FB3000C '....' - lw         $s3, 12($sp)
	0x00002108: 0x8FB20008 '....' - lw         $s2, 8($sp)
	0x0000210C: 0x8FB10004 '....' - lw         $s1, 4($sp)
	0x00002110: 0x8FB00000 '....' - lw         $s0, 0($sp)
	0x00002114: 0x03E00008 '....' - jr         $ra
	0x00002118: 0x27BD0020 ' ..'' - addiu      $sp, $sp, 32

loc_0000211C:		; Refs: 0x000020EC 
	0x0000211C: 0x8A060007 '....' - lwl        $a2, 7($s0)
	0x00002120: 0x9A060004 '....' - lwr        $a2, 4($s0)
	0x00002124: 0x88AE000F '....' - lwl        $t6, 15($a1)
	0x00002128: 0x98AE000C '....' - lwr        $t6, 12($a1)
	0x0000212C: 0x26270014 '..'&' - addiu      $a3, $s1, 20
	0x00002130: 0x88CF000B '....' - lwl        $t7, 11($a2)
	0x00002134: 0x98CF0008 '....' - lwr        $t7, 8($a2)
	0x00002138: 0x3C118012 '...<' - lui        $s1, 0x8012
	0x0000213C: 0x36320003 '..26' - ori        $s2, $s1, 0x3
	0x00002140: 0x01CF282B '+(..' - sltu       $a1, $t6, $t7
	0x00002144: 0x00075602 '.V..' - srl        $t2, $a3, 24
	0x00002148: 0x7CE83A00 '.:.|' - ext        $t0, $a3, 8, 8
	0x0000214C: 0x14A0FFE9 '....' - bnez       $a1, loc_000020F4
	0x00002150: 0x7CE93C00 '.<.|' - ext        $t1, $a3, 16, 8
	0x00002154: 0x88D10013 '....' - lwl        $s1, 19($a2)
	0x00002158: 0x98D10010 '....' - lwr        $s1, 16($a2)
	0x0000215C: 0x00111900 '....' - sll        $v1, $s1, 4
	0x00002160: 0x00676821 '!hg.' - addu       $t5, $v1, $a3
	0x00002164: 0x000D7602 '.v..' - srl        $t6, $t5, 24
	0x00002168: 0x7DA53A00 '.:.}' - ext        $a1, $t5, 8, 8
	0x0000216C: 0x7DAF3C00 '.<.}' - ext        $t7, $t5, 16, 8
	0x00002170: 0xA205000D '....' - sb         $a1, 13($s0)
	0x00002174: 0xA20F000E '....' - sb         $t7, 14($s0)
	0x00002178: 0xA20E000F '....' - sb         $t6, 15($s0)
	0x0000217C: 0xA20D000C '....' - sb         $t5, 12($s0)
	0x00002180: 0x8A0C0007 '....' - lwl        $t4, 7($s0)
	0x00002184: 0x9A0C0004 '....' - lwr        $t4, 4($s0)
	0x00002188: 0x8A0B0007 '....' - lwl        $t3, 7($s0)
	0x0000218C: 0x9A0B0004 '....' - lwr        $t3, 4($s0)
	0x00002190: 0x8982000F '....' - lwl        $v0, 15($t4)
	0x00002194: 0x9982000C '....' - lwr        $v0, 12($t4)
	0x00002198: 0xA2080009 '....' - sb         $t0, 9($s0)
	0x0000219C: 0x01629021 '!.b.' - addu       $s2, $t3, $v0
	0x000021A0: 0x0012C602 '....' - srl        $t8, $s2, 24
	0x000021A4: 0x7E463A00 '.:F~' - ext        $a2, $s2, 8, 8
	0x000021A8: 0x7E593C00 '.<Y~' - ext        $t9, $s2, 16, 8
	0x000021AC: 0xA2060011 '....' - sb         $a2, 17($s0)
	0x000021B0: 0xA2190012 '....' - sb         $t9, 18($s0)
	0x000021B4: 0xA2180013 '....' - sb         $t8, 19($s0)
	0x000021B8: 0xA209000A '....' - sb         $t1, 10($s0)
	0x000021BC: 0xA20A000B '....' - sb         $t2, 11($s0)
	0x000021C0: 0xA2120010 '....' - sb         $s2, 16($s0)
	0x000021C4: 0x0C000937 '7...' - jal        sub_000024DC
	0x000021C8: 0xA2070008 '....' - sb         $a3, 8($s0)
	0x000021CC: 0x00001021 '!...' - move       $v0, $zr
	0x000021D0: 0x08000840 '@...' - j          loc_00002100
	0x000021D4: 0xAE700000 '..p.' - sw         $s0, 0($s3)

; ======================================================
; Subroutine sub_000021D8 - Address 0x000021D8 
sub_000021D8:		; Refs: 0x00000F94 
	0x000021D8: 0x27BDFFF0 '...'' - addiu      $sp, $sp, -16
	0x000021DC: 0x3C028012 '...<' - lui        $v0, 0x8012
	0x000021E0: 0xAFB20008 '....' - sw         $s2, 8($sp)
	0x000021E4: 0x34430003 '..C4' - ori        $v1, $v0, 0x3
	0x000021E8: 0x00C09021 '!...' - move       $s2, $a2
	0x000021EC: 0xAFB10004 '....' - sw         $s1, 4($sp)
	0x000021F0: 0x00A08821 '!...' - move       $s1, $a1
	0x000021F4: 0xAFB00000 '....' - sw         $s0, 0($sp)
	0x000021F8: 0x00808021 '!...' - move       $s0, $a0
	0x000021FC: 0x10800008 '....' - beqz       $a0, loc_00002220
	0x00002200: 0xAFBF000C '....' - sw         $ra, 12($sp)
; Data ref 0x08400965
	0x00002204: 0x0C000965 'e...' - jal        sub_00002594
	0x00002208: 0x00000000 '....' - nop        
	0x0000220C: 0x00401821 '!.@.' - move       $v1, $v0
	0x00002210: 0x02002021 '! ..' - move       $a0, $s0
	0x00002214: 0x02003021 '!0..' - move       $a2, $s0
	0x00002218: 0x10400008 '..@.' - beqz       $v0, loc_0000223C
	0x0000221C: 0x02203821 '!8 .' - move       $a3, $s1

loc_00002220:		; Refs: 0x000021FC 0x00002258 0x00002270 0x00002280 
	0x00002220: 0x8FBF000C '....' - lw         $ra, 12($sp)
	0x00002224: 0x8FB20008 '....' - lw         $s2, 8($sp)
	0x00002228: 0x8FB10004 '....' - lw         $s1, 4($sp)
	0x0000222C: 0x8FB00000 '....' - lw         $s0, 0($sp)
	0x00002230: 0x00601021 '!.`.' - move       $v0, $v1
	0x00002234: 0x03E00008 '....' - jr         $ra
	0x00002238: 0x27BD0010 '...'' - addiu      $sp, $sp, 16

loc_0000223C:		; Refs: 0x00002218 
	0x0000223C: 0x8A05000B '....' - lwl        $a1, 11($s0)
	0x00002240: 0x0C000997 '....' - jal        sub_0000265C
	0x00002244: 0x9A050008 '....' - lwr        $a1, 8($s0)
	0x00002248: 0x00401821 '!.@.' - move       $v1, $v0
	0x0000224C: 0x02203821 '!8 .' - move       $a3, $s1
	0x00002250: 0x02002021 '! ..' - move       $a0, $s0
	0x00002254: 0x24050014 '...$' - li         $a1, 20
	0x00002258: 0x1440FFF1 '..@.' - bnez       $v0, loc_00002220
	0x0000225C: 0x02003021 '!0..' - move       $a2, $s0
	0x00002260: 0x0C0009BF '....' - jal        sub_000026FC
	0x00002264: 0x00000000 '....' - nop        
	0x00002268: 0x00401821 '!.@.' - move       $v1, $v0
	0x0000226C: 0x02002021 '! ..' - move       $a0, $s0
	0x00002270: 0x1440FFEB '..@.' - bnez       $v0, loc_00002220
	0x00002274: 0x02402821 '!(@.' - move       $a1, $s2
	0x00002278: 0x0C000807 '....' - jal        sub_0000201C
	0x0000227C: 0x00000000 '....' - nop        
	0x00002280: 0x08000888 '....' - j          loc_00002220
	0x00002284: 0x00401821 '!.@.' - move       $v1, $v0

; ======================================================
; Subroutine sub_00002288 - Address 0x00002288 
sub_00002288:		; Refs: 0x00001028 0x000020F4 
	0x00002288: 0x27BDFFF0 '...'' - addiu      $sp, $sp, -16
	0x0000228C: 0x3C038012 '...<' - lui        $v1, 0x8012
	0x00002290: 0xAFB00000 '....' - sw         $s0, 0($sp)
	0x00002294: 0x34620003 '..b4' - ori        $v0, $v1, 0x3
	0x00002298: 0x00808021 '!...' - move       $s0, $a0
	0x0000229C: 0xAFBF0008 '....' - sw         $ra, 8($sp)
	0x000022A0: 0x1080000F '....' - beqz       $a0, loc_000022E0
	0x000022A4: 0xAFB10004 '....' - sw         $s1, 4($sp)
	0x000022A8: 0x8A040003 '....' - lwl        $a0, 3($s0)
	0x000022AC: 0x9A040000 '....' - lwr        $a0, 0($s0)
	0x000022B0: 0x30830002 '...0' - andi       $v1, $a0, 0x2
	0x000022B4: 0x14600006 '..`.' - bnez       $v1, loc_000022D0
; Data ref 0x00004A44 ... 0x000012F0 0x00000000 0x00000000 0x00000000 
	0x000022B8: 0x3C020000 '...<' - lui        $v0, 0x0
	0x000022BC: 0x8A050013 '....' - lwl        $a1, 19($s0)
	0x000022C0: 0x9A050010 '....' - lwr        $a1, 16($s0)
	0x000022C4: 0x14A0000B '....' - bnez       $a1, loc_000022F4
	0x000022C8: 0x00008821 '!...' - move       $s1, $zr
; Data ref 0x00004A44 ... 0x000012F0 0x00000000 0x00000000 0x00000000 
	0x000022CC: 0x3C020000 '...<' - lui        $v0, 0x0

loc_000022D0:		; Refs: 0x000022B4 
; Data ref 0x00004A44 ... 0x000012F0 0x00000000 0x00000000 0x00000000 
	0x000022D0: 0x8C514A44 'DJQ.' - lw         $s1, 19012($v0)

loc_000022D4:		; Refs: 0x00002324 
	0x000022D4: 0x0220F809 '.. .' - jalr       $s1
	0x000022D8: 0x02002021 '! ..' - move       $a0, $s0
	0x000022DC: 0x00001021 '!...' - move       $v0, $zr

loc_000022E0:		; Refs: 0x000022A0 
	0x000022E0: 0x8FBF0008 '....' - lw         $ra, 8($sp)
	0x000022E4: 0x8FB10004 '....' - lw         $s1, 4($sp)
	0x000022E8: 0x8FB00000 '....' - lw         $s0, 0($sp)
	0x000022EC: 0x03E00008 '....' - jr         $ra
	0x000022F0: 0x27BD0010 '...'' - addiu      $sp, $sp, 16

loc_000022F4:		; Refs: 0x000022C4 0x0000231C 
	0x000022F4: 0x8A080037 '7...' - lwl        $t0, 55($s0)
	0x000022F8: 0x9A080034 '4...' - lwr        $t0, 52($s0)
	0x000022FC: 0x00114900 '.I..' - sll        $t1, $s1, 4
	0x00002300: 0x02002021 '! ..' - move       $a0, $s0
; Data ref 0x08880AA7
	0x00002304: 0x0C000AA7 '....' - jal        sub_00002A9C
	0x00002308: 0x01092821 '!(..' - addu       $a1, $t0, $t1
	0x0000230C: 0x8A070013 '....' - lwl        $a3, 19($s0)
	0x00002310: 0x9A070010 '....' - lwr        $a3, 16($s0)
	0x00002314: 0x26310001 '..1&' - addiu      $s1, $s1, 1
	0x00002318: 0x0227302B '+0'.' - sltu       $a2, $s1, $a3
	0x0000231C: 0x14C0FFF5 '....' - bnez       $a2, loc_000022F4
; Data ref 0x00004A44 ... 0x000012F0 0x00000000 0x00000000 0x00000000 
	0x00002320: 0x3C020000 '...<' - lui        $v0, 0x0
	0x00002324: 0x080008B5 '....' - j          loc_000022D4
; Data ref 0x00004A44 ... 0x000012F0 0x00000000 0x00000000 0x00000000 
	0x00002328: 0x8C514A44 'DJQ.' - lw         $s1, 19012($v0)

; ======================================================
; Subroutine sub_0000232C - Address 0x0000232C 
sub_0000232C:		; Refs: 0x00002058 
	0x0000232C: 0x27BDFFE0 '...'' - addiu      $sp, $sp, -32
	0x00002330: 0xAFB3000C '....' - sw         $s3, 12($sp)
	0x00002334: 0x00809821 '!...' - move       $s3, $a0
	0x00002338: 0xAFB20008 '....' - sw         $s2, 8($sp)
	0x0000233C: 0x24120014 '...$' - li         $s2, 20
	0x00002340: 0xAFB10004 '....' - sw         $s1, 4($sp)
	0x00002344: 0x00A08821 '!...' - move       $s1, $a1
	0x00002348: 0xAFBF0018 '....' - sw         $ra, 24($sp)
	0x0000234C: 0xAFB50014 '....' - sw         $s5, 20($sp)
	0x00002350: 0xAFB40010 '....' - sw         $s4, 16($sp)
	0x00002354: 0x14800009 '....' - bnez       $a0, loc_0000237C
	0x00002358: 0xAFB00000 '....' - sw         $s0, 0($sp)
	0x0000235C: 0x3C040004 '...<' - lui        $a0, 0x4
	0x00002360: 0x0085182B '+...' - sltu       $v1, $a0, $a1
	0x00002364: 0x1460001C '..`.' - bnez       $v1, loc_000023D8
	0x00002368: 0x00002021 '! ..' - move       $a0, $zr
	0x0000236C: 0x0005A100 '....' - sll        $s4, $a1, 4
	0x00002370: 0x00051980 '....' - sll        $v1, $a1, 6
	0x00002374: 0x2472003C '<.r$' - addiu      $s2, $v1, 60
	0x00002378: 0x0280A821 '!...' - move       $s5, $s4

loc_0000237C:		; Refs: 0x00002354 
; Data ref 0x00004A40 ... 0x00001228 0x000012F0 0x00000000 0x00000000 
	0x0000237C: 0x3C020000 '...<' - lui        $v0, 0x0
; Data ref 0x00004A40 ... 0x00001228 0x000012F0 0x00000000 0x00000000 
	0x00002380: 0x8C504A40 '@JP.' - lw         $s0, 19008($v0)
	0x00002384: 0x0200F809 '....' - jalr       $s0
	0x00002388: 0x02402021 '! @.' - move       $a0, $s2
	0x0000238C: 0x10400011 '..@.' - beqz       $v0, loc_000023D4
	0x00002390: 0x00408021 '!.@.' - move       $s0, $v0
	0x00002394: 0x02403021 '!0@.' - move       $a2, $s2
	0x00002398: 0x00402021 '! @.' - move       $a0, $v0
; Data ref 0x08B50C07
	0x0000239C: 0x0C000C07 '....' - jal        SysMemForKernel_8AE776AF
	0x000023A0: 0x00002821 '!(..' - move       $a1, $zr
	0x000023A4: 0x12600016 '..`.' - beqz       $s3, loc_00002400
	0x000023A8: 0x26130020 ' ..&' - addiu      $s3, $s0, 32
	0x000023AC: 0x8A090003 '....' - lwl        $t1, 3($s0)
	0x000023B0: 0x9A090000 '....' - lwr        $t1, 0($s0)
	0x000023B4: 0x35260002 '..&5' - ori        $a2, $t1, 0x2
	0x000023B8: 0x00063E02 '.>..' - srl        $a3, $a2, 24
	0x000023BC: 0x7CC83A00 '.:.|' - ext        $t0, $a2, 8, 8
	0x000023C0: 0x7CC53C00 '.<.|' - ext        $a1, $a2, 16, 8
	0x000023C4: 0xA2080001 '....' - sb         $t0, 1($s0)
	0x000023C8: 0xA2050002 '....' - sb         $a1, 2($s0)
	0x000023CC: 0xA2070003 '....' - sb         $a3, 3($s0)
	0x000023D0: 0xA2060000 '....' - sb         $a2, 0($s0)

loc_000023D4:		; Refs: 0x0000238C 0x000024D4 
	0x000023D4: 0x02002021 '! ..' - move       $a0, $s0

loc_000023D8:		; Refs: 0x00002364 
	0x000023D8: 0x8FBF0018 '....' - lw         $ra, 24($sp)
	0x000023DC: 0x8FB50014 '....' - lw         $s5, 20($sp)
	0x000023E0: 0x8FB40010 '....' - lw         $s4, 16($sp)
	0x000023E4: 0x8FB3000C '....' - lw         $s3, 12($sp)
	0x000023E8: 0x8FB20008 '....' - lw         $s2, 8($sp)
	0x000023EC: 0x8FB10004 '....' - lw         $s1, 4($sp)
	0x000023F0: 0x8FB00000 '....' - lw         $s0, 0($sp)
	0x000023F4: 0x00801021 '!...' - move       $v0, $a0
	0x000023F8: 0x03E00008 '....' - jr         $ra
	0x000023FC: 0x27BD0020 ' ..'' - addiu      $sp, $sp, 32

loc_00002400:		; Refs: 0x000023A4 
	0x00002400: 0x2612003C '<..&' - addiu      $s2, $s0, 60
	0x00002404: 0x7E6D3A00 '.:m~' - ext        $t5, $s3, 8, 8
	0x00002408: 0x00133602 '.6..' - srl        $a2, $s3, 24
	0x0000240C: 0x0012C602 '....' - srl        $t8, $s2, 24
	0x00002410: 0x7E693C00 '.<i~' - ext        $t1, $s3, 16, 8
	0x00002414: 0x7E273A00 '.:'~' - ext        $a3, $s1, 8, 8
	0x00002418: 0x7E283C00 '.<(~' - ext        $t0, $s1, 16, 8
	0x0000241C: 0x00111602 '....' - srl        $v0, $s1, 24
	0x00002420: 0x7E433A00 '.:C~' - ext        $v1, $s2, 8, 8
	0x00002424: 0x7E593C00 '.<Y~' - ext        $t9, $s2, 16, 8
	0x00002428: 0xA20D0005 '....' - sb         $t5, 5($s0)
	0x0000242C: 0x00006821 '!h..' - move       $t5, $zr
	0x00002430: 0xA2090006 '....' - sb         $t1, 6($s0)
	0x00002434: 0xA2060007 '....' - sb         $a2, 7($s0)
	0x00002438: 0xA2070011 '....' - sb         $a3, 17($s0)
	0x0000243C: 0xA2080012 '....' - sb         $t0, 18($s0)
	0x00002440: 0xA2020013 '....' - sb         $v0, 19($s0)
	0x00002444: 0xA2030035 '5...' - sb         $v1, 53($s0)
	0x00002448: 0xA2190036 '6...' - sb         $t9, 54($s0)
	0x0000244C: 0xA2180037 '7...' - sb         $t8, 55($s0)
	0x00002450: 0xA2130004 '....' - sb         $s3, 4($s0)
	0x00002454: 0xA2110010 '....' - sb         $s1, 16($s0)
	0x00002458: 0xA2120034 '4...' - sb         $s2, 52($s0)
	0x0000245C: 0x8A0F0037 '7...' - lwl        $t7, 55($s0)
	0x00002460: 0x9A0F0034 '4...' - lwr        $t7, 52($s0)
	0x00002464: 0x01F55021 '!P..' - addu       $t2, $t7, $s5
	0x00002468: 0x000A5E02 '.^..' - srl        $t3, $t2, 24
	0x0000246C: 0x7D4E3A00 '.:N}' - ext        $t6, $t2, 8, 8
	0x00002470: 0x7D4C3C00 '.<L}' - ext        $t4, $t2, 16, 8
	0x00002474: 0xA20E0009 '....' - sb         $t6, 9($s0)
	0x00002478: 0xA20C000A '....' - sb         $t4, 10($s0)
	0x0000247C: 0xA20B000B '....' - sb         $t3, 11($s0)
	0x00002480: 0x1220000B '.. .' - beqz       $s1, loc_000024B0
	0x00002484: 0xA20A0008 '....' - sb         $t2, 8($s0)
	0x00002488: 0x2405FFFF '...$' - li         $a1, -1

loc_0000248C:		; Refs: 0x000024A8 
	0x0000248C: 0x8A0B000B '....' - lwl        $t3, 11($s0)
	0x00002490: 0x9A0B0008 '....' - lwr        $t3, 8($s0)
	0x00002494: 0x000D5100 '.Q..' - sll        $t2, $t5, 4
	0x00002498: 0x25AD0001 '...%' - addiu      $t5, $t5, 1
	0x0000249C: 0x014B2021 '! K.' - addu       $a0, $t2, $t3
	0x000024A0: 0x01B1A82B '+...' - sltu       $s5, $t5, $s1
	0x000024A4: 0xA0850001 '....' - sb         $a1, 1($a0)
	0x000024A8: 0x16A0FFF8 '....' - bnez       $s5, loc_0000248C
	0x000024AC: 0xA0850000 '....' - sb         $a1, 0($a0)

loc_000024B0:		; Refs: 0x00002480 
	0x000024B0: 0x8A0E000B '....' - lwl        $t6, 11($s0)
	0x000024B4: 0x9A0E0008 '....' - lwr        $t6, 8($s0)
	0x000024B8: 0x01D48821 '!...' - addu       $s1, $t6, $s4
	0x000024BC: 0x0011A602 '....' - srl        $s4, $s1, 24
	0x000024C0: 0x7E2C3A00 '.:,~' - ext        $t4, $s1, 8, 8
	0x000024C4: 0x7E253C00 '.<%~' - ext        $a1, $s1, 16, 8
	0x000024C8: 0xA20C0039 '9...' - sb         $t4, 57($s0)
	0x000024CC: 0xA205003A ':...' - sb         $a1, 58($s0)
	0x000024D0: 0xA214003B ';...' - sb         $s4, 59($s0)
	0x000024D4: 0x080008F5 '....' - j          loc_000023D4
	0x000024D8: 0xA2110038 '8...' - sb         $s1, 56($s0)

; ======================================================
; Subroutine sub_000024DC - Address 0x000024DC 
sub_000024DC:		; Refs: 0x000021C4 
	0x000024DC: 0x88850007 '....' - lwl        $a1, 7($a0)
	0x000024E0: 0x98850004 '....' - lwr        $a1, 4($a0)
	0x000024E4: 0x00804021 '!@..' - move       $t0, $a0
	0x000024E8: 0x88A30013 '....' - lwl        $v1, 19($a1)
	0x000024EC: 0x98A30010 '....' - lwr        $v1, 16($a1)
	0x000024F0: 0x10600026 '&.`.' - beqz       $v1, loc_0000258C
	0x000024F4: 0x00004821 '!H..' - move       $t1, $zr

loc_000024F8:		; Refs: 0x00002584 
	0x000024F8: 0x890A000B '....' - lwl        $t2, 11($t0)
	0x000024FC: 0x990A0008 '....' - lwr        $t2, 8($t0)
	0x00002500: 0x89030003 '....' - lwl        $v1, 3($t0)
	0x00002504: 0x99030000 '....' - lwr        $v1, 0($t0)
	0x00002508: 0x00092100 '.!..' - sll        $a0, $t1, 4
	0x0000250C: 0x01443821 '!8D.' - addu       $a3, $t2, $a0
	0x00002510: 0x306D0002 '..m0' - andi       $t5, $v1, 0x2
	0x00002514: 0x00093140 '@1..' - sll        $a2, $t1, 5
	0x00002518: 0x00075602 '.V..' - srl        $t2, $a3, 24
	0x0000251C: 0x25290001 '..)%' - addiu      $t1, $t1, 1
	0x00002520: 0x7CEB3A00 '.:.|' - ext        $t3, $a3, 8, 8
	0x00002524: 0x15A00014 '....' - bnez       $t5, loc_00002578
	0x00002528: 0x7CEC3C00 '.<.|' - ext        $t4, $a3, 16, 8
	0x0000252C: 0x8902003B ';...' - lwl        $v0, 59($t0)
	0x00002530: 0x99020038 '8...' - lwr        $v0, 56($t0)
	0x00002534: 0x89190037 '7...' - lwl        $t9, 55($t0)
	0x00002538: 0x99190034 '4...' - lwr        $t9, 52($t0)
	0x0000253C: 0x00467821 '!xF.' - addu       $t7, $v0, $a2
	0x00002540: 0x03247021 '!p$.' - addu       $t6, $t9, $a0
	0x00002544: 0x7DE53A00 '.:.}' - ext        $a1, $t7, 8, 8
	0x00002548: 0x000FC602 '....' - srl        $t8, $t7, 24
	0x0000254C: 0x7DE63C00 '.<.}' - ext        $a2, $t7, 16, 8
	0x00002550: 0xA1CA0003 '....' - sb         $t2, 3($t6)
	0x00002554: 0xA1C50005 '....' - sb         $a1, 5($t6)
	0x00002558: 0xA1C60006 '....' - sb         $a2, 6($t6)
	0x0000255C: 0xA1D80007 '....' - sb         $t8, 7($t6)
	0x00002560: 0xA1CB0001 '....' - sb         $t3, 1($t6)
	0x00002564: 0xA1CC0002 '....' - sb         $t4, 2($t6)
	0x00002568: 0xA1CF0004 '....' - sb         $t7, 4($t6)
	0x0000256C: 0xA1C70000 '....' - sb         $a3, 0($t6)
	0x00002570: 0x89050007 '....' - lwl        $a1, 7($t0)
	0x00002574: 0x99050004 '....' - lwr        $a1, 4($t0)

loc_00002578:		; Refs: 0x00002524 
	0x00002578: 0x88A70013 '....' - lwl        $a3, 19($a1)
	0x0000257C: 0x98A70010 '....' - lwr        $a3, 16($a1)
	0x00002580: 0x0127202B '+ '.' - sltu       $a0, $t1, $a3
	0x00002584: 0x1480FFDC '....' - bnez       $a0, loc_000024F8
	0x00002588: 0x00000000 '....' - nop        

loc_0000258C:		; Refs: 0x000024F0 
	0x0000258C: 0x03E00008 '....' - jr         $ra
	0x00002590: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sub_00002594 - Address 0x00002594 
sub_00002594:		; Refs: 0x00002204 
	0x00002594: 0x27BDFFF0 '...'' - addiu      $sp, $sp, -16
; Data ref 0x0000414C ... 0x46535000 0x00000000 0x3D72797C 0x30606360 
	0x00002598: 0x3C020000 '...<' - lui        $v0, 0x0
	0x0000259C: 0xAFB10004 '....' - sw         $s1, 4($sp)
	0x000025A0: 0x24060004 '...$' - li         $a2, 4
	0x000025A4: 0x00A08821 '!...' - move       $s1, $a1
; Data ref 0x0000414C ... 0x46535000 0x00000000 0x3D72797C 0x30606360 
	0x000025A8: 0x2445414C 'LAE$' - addiu      $a1, $v0, 16716
	0x000025AC: 0xAFB00000 '....' - sw         $s0, 0($sp)
	0x000025B0: 0xAFBF0008 '....' - sw         $ra, 8($sp)
; Data ref 0x08F50B2F
	0x000025B4: 0x0C000B2F '/...' - jal        sub_00002CBC
	0x000025B8: 0x00808021 '!...' - move       $s0, $a0
	0x000025BC: 0x3C038012 '...<' - lui        $v1, 0x8012
	0x000025C0: 0x14400007 '..@.' - bnez       $v0, loc_000025E0
	0x000025C4: 0x34660004 '..f4' - ori        $a2, $v1, 0x4
	0x000025C8: 0x8A050007 '....' - lwl        $a1, 7($s0)
	0x000025CC: 0x9A050004 '....' - lwr        $a1, 4($s0)
	0x000025D0: 0x3C038012 '...<' - lui        $v1, 0x8012
	0x000025D4: 0x24040101 '...$' - li         $a0, 257
	0x000025D8: 0x10A40007 '....' - beq        $a1, $a0, loc_000025F8
	0x000025DC: 0x34660007 '..f4' - ori        $a2, $v1, 0x7

loc_000025E0:		; Refs: 0x000025C0 0x00002608 0x00002620 0x00002644 0x00002654 
	0x000025E0: 0x8FBF0008 '....' - lw         $ra, 8($sp)
	0x000025E4: 0x8FB10004 '....' - lw         $s1, 4($sp)
	0x000025E8: 0x8FB00000 '....' - lw         $s0, 0($sp)
	0x000025EC: 0x00C01021 '!...' - move       $v0, $a2
	0x000025F0: 0x03E00008 '....' - jr         $ra
	0x000025F4: 0x27BD0010 '...'' - addiu      $sp, $sp, 16

loc_000025F8:		; Refs: 0x000025D8 
	0x000025F8: 0x8A04000B '....' - lwl        $a0, 11($s0)
	0x000025FC: 0x9A040008 '....' - lwr        $a0, 8($s0)
	0x00002600: 0x3C088012 '...<' - lui        $t0, 0x8012
	0x00002604: 0x0224382B '+8$.' - sltu       $a3, $s1, $a0
	0x00002608: 0x14E0FFF5 '....' - bnez       $a3, loc_000025E0
	0x0000260C: 0x35060003 '...5' - ori        $a2, $t0, 0x3
	0x00002610: 0x8A03000F '....' - lwl        $v1, 15($s0)
	0x00002614: 0x9A03000C '....' - lwr        $v1, 12($s0)
	0x00002618: 0x3C0A8012 '...<' - lui        $t2, 0x8012
	0x0000261C: 0x0223482B '+H#.' - sltu       $t1, $s1, $v1
	0x00002620: 0x1520FFEF '.. .' - bnez       $t1, loc_000025E0
	0x00002624: 0x35460003 '..F5' - ori        $a2, $t2, 0x3
	0x00002628: 0x8A0E0013 '....' - lwl        $t6, 19($s0)
	0x0000262C: 0x9A0E0010 '....' - lwr        $t6, 16($s0)
	0x00002630: 0x3C0C8012 '...<' - lui        $t4, 0x8012
	0x00002634: 0x0064182B '+.d.' - sltu       $v1, $v1, $a0
	0x00002638: 0x000E6900 '.i..' - sll        $t5, $t6, 4
	0x0000263C: 0x25A60014 '...%' - addiu      $a2, $t5, 20
	0x00002640: 0x0226582B '+X&.' - sltu       $t3, $s1, $a2
	0x00002644: 0x1560FFE6 '..`.' - bnez       $t3, loc_000025E0
	0x00002648: 0x35860003 '...5' - ori        $a2, $t4, 0x3
	0x0000264C: 0x3C0F8012 '...<' - lui        $t7, 0x8012
	0x00002650: 0x35E60003 '...5' - ori        $a2, $t7, 0x3
	0x00002654: 0x08000978 'x...' - j          loc_000025E0
	0x00002658: 0x0003300A '.0..' - movz       $a2, $zr, $v1

; ======================================================
; Subroutine sub_0000265C - Address 0x0000265C 
sub_0000265C:		; Refs: 0x00002240 
	0x0000265C: 0x00854021 '!@..' - addu       $t0, $a0, $a1
	0x00002660: 0x00004821 '!H..' - move       $t1, $zr
	0x00002664: 0x240A0023 '#..$' - li         $t2, 35
	0x00002668: 0x81050000 '....' - lb         $a1, 0($t0)

loc_0000266C:		; Refs: 0x000026A8 
	0x0000266C: 0x28A20021 '!..(' - slti       $v0, $a1, 33
	0x00002670: 0x38AD007F '...8' - xori       $t5, $a1, 0x7F
	0x00002674: 0x384B0001 '..K8' - xori       $t3, $v0, 0x1
	0x00002678: 0x000D602B '+`..' - sltu       $t4, $zr, $t5
	0x0000267C: 0x14A00018 '....' - bnez       $a1, loc_000026E0
	0x00002680: 0x016C1824 '$.l.' - and        $v1, $t3, $t4
	0x00002684: 0x25290001 '..)%' - addiu      $t1, $t1, 1
	0x00002688: 0x25080001 '...%' - addiu      $t0, $t0, 1

loc_0000268C:		; Refs: 0x000026E8 
	0x0000268C: 0x88CB0013 '....' - lwl        $t3, 19($a2)
	0x00002690: 0x98CB0010 '....' - lwr        $t3, 16($a2)
	0x00002694: 0x01047023 '#p..' - subu       $t6, $t0, $a0
	0x00002698: 0x3C0F8012 '...<' - lui        $t7, 0x8012
	0x0000269C: 0x35E50003 '...5' - ori        $a1, $t7, 0x3
	0x000026A0: 0x11690005 '..i.' - beq        $t3, $t1, loc_000026B8
	0x000026A4: 0x01C7102B '+...' - sltu       $v0, $t6, $a3
	0x000026A8: 0x5440FFF0 '..@T' - bnezl      $v0, loc_0000266C
	0x000026AC: 0x81050000 '....' - lb         $a1, 0($t0)

loc_000026B0:		; Refs: 0x000026D0 0x000026D8 0x000026F4 
	0x000026B0: 0x03E00008 '....' - jr         $ra
	0x000026B4: 0x00A01021 '!...' - move       $v0, $a1

loc_000026B8:		; Refs: 0x000026A0 
	0x000026B8: 0x88C5000F '....' - lwl        $a1, 15($a2)
	0x000026BC: 0x98C5000C '....' - lwr        $a1, 12($a2)
	0x000026C0: 0x25070003 '...%' - addiu      $a3, $t0, 3
	0x000026C4: 0x7C070804 '...|' - ins        $a3, $zr, 0, 2
	0x000026C8: 0x3C088012 '...<' - lui        $t0, 0x8012
	0x000026CC: 0x00853021 '!0..' - addu       $a2, $a0, $a1
	0x000026D0: 0x14E6FFF7 '....' - bne        $a3, $a2, loc_000026B0
	0x000026D4: 0x35050003 '...5' - ori        $a1, $t0, 0x3
	0x000026D8: 0x080009AC '....' - j          loc_000026B0
	0x000026DC: 0x00002821 '!(..' - move       $a1, $zr

loc_000026E0:		; Refs: 0x0000267C 
	0x000026E0: 0x10600004 '..`.' - beqz       $v1, loc_000026F4
	0x000026E4: 0x3C038012 '...<' - lui        $v1, 0x8012
	0x000026E8: 0x14AAFFE8 '....' - bne        $a1, $t2, loc_0000268C
	0x000026EC: 0x25080001 '...%' - addiu      $t0, $t0, 1
	0x000026F0: 0x3C038012 '...<' - lui        $v1, 0x8012

loc_000026F4:		; Refs: 0x000026E0 
	0x000026F4: 0x080009AC '....' - j          loc_000026B0
	0x000026F8: 0x34650003 '..e4' - ori        $a1, $v1, 0x3

; ======================================================
; Subroutine sub_000026FC - Address 0x000026FC 
sub_000026FC:		; Refs: 0x00002260 
	0x000026FC: 0x27BDFFD0 '...'' - addiu      $sp, $sp, -48
	0x00002700: 0xAFBE0020 ' ...' - sw         $fp, 32($sp)
	0x00002704: 0x00852021 '! ..' - addu       $a0, $a0, $a1
	0x00002708: 0xAFB7001C '....' - sw         $s7, 28($sp)
	0x0000270C: 0xAFB60018 '....' - sw         $s6, 24($sp)
	0x00002710: 0xAFB50014 '....' - sw         $s5, 20($sp)
	0x00002714: 0xAFB40010 '....' - sw         $s4, 16($sp)
	0x00002718: 0xAFB3000C '....' - sw         $s3, 12($sp)
	0x0000271C: 0xAFB20008 '....' - sw         $s2, 8($sp)
	0x00002720: 0xAFB10004 '....' - sw         $s1, 4($sp)
	0x00002724: 0xAFB00000 '....' - sw         $s0, 0($sp)
	0x00002728: 0x88CB0013 '....' - lwl        $t3, 19($a2)
	0x0000272C: 0x98CB0010 '....' - lwr        $t3, 16($a2)
	0x00002730: 0x11600047 'G.`.' - beqz       $t3, loc_00002850
	0x00002734: 0x00005021 '!P..' - move       $t2, $zr
	0x00002738: 0x88C3000F '....' - lwl        $v1, 15($a2)
	0x0000273C: 0x98C3000C '....' - lwr        $v1, 12($a2)
	0x00002740: 0x88CC000B '....' - lwl        $t4, 11($a2)
	0x00002744: 0x98CC0008 '....' - lwr        $t4, 8($a2)
	0x00002748: 0x240E0104 '...$' - li         $t6, 260
	0x0000274C: 0x00603021 '!0`.' - move       $a2, $v1
	0x00002750: 0x24180008 '...$' - li         $t8, 8
	0x00002754: 0x24140004 '...$' - li         $s4, 4
	0x00002758: 0x24110020 ' ..$' - li         $s1, 32
	0x0000275C: 0x241E0010 '...$' - li         $fp, 16
	0x00002760: 0x24170100 '...$' - li         $s7, 256
	0x00002764: 0x240F0204 '...$' - li         $t7, 516
	0x00002768: 0x24100110 '...$' - li         $s0, 272
	0x0000276C: 0x24150108 '...$' - li         $s5, 264
	0x00002770: 0x24160120 ' ..$' - li         $s6, 288
	0x00002774: 0x24190404 '...$' - li         $t9, 1028
	0x00002778: 0x24120304 '...$' - li         $s2, 772
	0x0000277C: 0x24130504 '...$' - li         $s3, 1284
	0x00002780: 0x256DFFFF '..m%' - addiu      $t5, $t3, -1

loc_00002784:		; Refs: 0x00002848 
	0x00002784: 0x90890001 '....' - lbu        $t1, 1($a0)
	0x00002788: 0x90830000 '....' - lbu        $v1, 0($a0)
	0x0000278C: 0x3C088012 '...<' - lui        $t0, 0x8012
	0x00002790: 0x00092A00 '.*..' - sll        $a1, $t1, 8
	0x00002794: 0x00A34825 '%H..' - or         $t1, $a1, $v1
	0x00002798: 0x01892821 '!(..' - addu       $a1, $t4, $t1
	0x0000279C: 0x00A6182B '+...' - sltu       $v1, $a1, $a2
	0x000027A0: 0x1060002C ',.`.' - beqz       $v1, loc_00002854
	0x000027A4: 0x35050003 '...5' - ori        $a1, $t0, 0x3
	0x000027A8: 0x90830003 '....' - lbu        $v1, 3($a0)
	0x000027AC: 0x90880002 '....' - lbu        $t0, 2($a0)
	0x000027B0: 0x00032A00 '.*..' - sll        $a1, $v1, 8
	0x000027B4: 0x00A81825 '%...' - or         $v1, $a1, $t0
	0x000027B8: 0x106E000B '..n.' - beq        $v1, $t6, loc_000027E8
	0x000027BC: 0x28690105 '..i(' - slti       $t1, $v1, 261
	0x000027C0: 0x11200045 'E. .' - beqz       $t1, loc_000028D8
	0x000027C4: 0x00000000 '....' - nop        
	0x000027C8: 0x10780007 '..x.' - beq        $v1, $t8, loc_000027E8
	0x000027CC: 0x28680009 '..h(' - slti       $t0, $v1, 9
	0x000027D0: 0x11000035 '5...' - beqz       $t0, loc_000028A8
	0x000027D4: 0x00000000 '....' - nop        
	0x000027D8: 0x10600003 '..`.' - beqz       $v1, loc_000027E8
	0x000027DC: 0x00000000 '....' - nop        
	0x000027E0: 0x1474002F '/.t.' - bne        $v1, $s4, loc_000028A0
	0x000027E4: 0x3C028012 '...<' - lui        $v0, 0x8012

loc_000027E8:		; Refs: 0x000027B8 0x000027C8 0x000027D8 0x000028A8 0x000028B8 0x000028C8 0x000028D8 0x000028E8 0x000028F8 0x00002908 0x00002918 0x00002928 0x00002938 
	0x000027E8: 0x8883000B '....' - lwl        $v1, 11($a0)
	0x000027EC: 0x98830008 '....' - lwr        $v1, 8($a0)
	0x000027F0: 0x88880007 '....' - lwl        $t0, 7($a0)
	0x000027F4: 0x98880004 '....' - lwr        $t0, 4($a0)
	0x000027F8: 0x3C028012 '...<' - lui        $v0, 0x8012
	0x000027FC: 0x0068482B '+Hh.' - sltu       $t1, $v1, $t0
	0x00002800: 0x15200014 '.. .' - bnez       $t1, loc_00002854
	0x00002804: 0x34450003 '..E4' - ori        $a1, $v0, 0x3
	0x00002808: 0x8889000F '....' - lwl        $t1, 15($a0)
	0x0000280C: 0x9889000C '....' - lwr        $t1, 12($a0)
	0x00002810: 0x3C088012 '...<' - lui        $t0, 0x8012
	0x00002814: 0x35050003 '...5' - ori        $a1, $t0, 0x3
	0x00002818: 0x01234021 '!@#.' - addu       $t0, $t1, $v1
	0x0000281C: 0x00E8102B '+...' - sltu       $v0, $a3, $t0
	0x00002820: 0x5440000D '..@T' - bnezl      $v0, loc_00002858
	0x00002824: 0x8FBE0020 ' ...' - lw         $fp, 32($sp)
	0x00002828: 0x114D0016 '..M.' - beq        $t2, $t5, loc_00002884
	0x0000282C: 0x3C028012 '...<' - lui        $v0, 0x8012
	0x00002830: 0x8889001F '....' - lwl        $t1, 31($a0)
	0x00002834: 0x9889001C '....' - lwr        $t1, 28($a0)
	0x00002838: 0x15090006 '....' - bne        $t0, $t1, loc_00002854
	0x0000283C: 0x34450003 '..E4' - ori        $a1, $v0, 0x3

loc_00002840:		; Refs: 0x00002890 
	0x00002840: 0x254A0001 '..J%' - addiu      $t2, $t2, 1
	0x00002844: 0x014B402B '+@K.' - sltu       $t0, $t2, $t3
	0x00002848: 0x1500FFCE '....' - bnez       $t0, loc_00002784
	0x0000284C: 0x24840010 '...$' - addiu      $a0, $a0, 16

loc_00002850:		; Refs: 0x00002730 
	0x00002850: 0x00002821 '!(..' - move       $a1, $zr

loc_00002854:		; Refs: 0x000027A0 0x00002800 0x00002838 0x000028A0 0x000028C0 0x000028D0 0x00002900 0x00002910 0x00002930 0x00002940 
	0x00002854: 0x8FBE0020 ' ...' - lw         $fp, 32($sp)

loc_00002858:		; Refs: 0x00002820 0x00002898 
	0x00002858: 0x8FB7001C '....' - lw         $s7, 28($sp)
	0x0000285C: 0x8FB60018 '....' - lw         $s6, 24($sp)
	0x00002860: 0x8FB50014 '....' - lw         $s5, 20($sp)
	0x00002864: 0x8FB40010 '....' - lw         $s4, 16($sp)
	0x00002868: 0x8FB3000C '....' - lw         $s3, 12($sp)
	0x0000286C: 0x8FB20008 '....' - lw         $s2, 8($sp)
	0x00002870: 0x8FB10004 '....' - lw         $s1, 4($sp)
	0x00002874: 0x8FB00000 '....' - lw         $s0, 0($sp)
	0x00002878: 0x00A01021 '!...' - move       $v0, $a1
	0x0000287C: 0x03E00008 '....' - jr         $ra
	0x00002880: 0x27BD0030 '0..'' - addiu      $sp, $sp, 48

loc_00002884:		; Refs: 0x00002828 
	0x00002884: 0x00C92821 '!(..' - addu       $a1, $a2, $t1
	0x00002888: 0x00A34821 '!H..' - addu       $t1, $a1, $v1
	0x0000288C: 0x3C038012 '...<' - lui        $v1, 0x8012
	0x00002890: 0x1127FFEB '..'.' - beq        $t1, $a3, loc_00002840
	0x00002894: 0x34650003 '..e4' - ori        $a1, $v1, 0x3
	0x00002898: 0x08000A16 '....' - j          loc_00002858
	0x0000289C: 0x8FBE0020 ' ...' - lw         $fp, 32($sp)

loc_000028A0:		; Refs: 0x000027E0 
	0x000028A0: 0x08000A15 '....' - j          loc_00002854
	0x000028A4: 0x34450003 '..E4' - ori        $a1, $v0, 0x3

loc_000028A8:		; Refs: 0x000027D0 
	0x000028A8: 0x1071FFCF '..q.' - beq        $v1, $s1, loc_000027E8
	0x000028AC: 0x28650021 '!.e(' - slti       $a1, $v1, 33
	0x000028B0: 0x10A00005 '....' - beqz       $a1, loc_000028C8
	0x000028B4: 0x00000000 '....' - nop        
	0x000028B8: 0x107EFFCB '..~.' - beq        $v1, $fp, loc_000027E8
	0x000028BC: 0x3C028012 '...<' - lui        $v0, 0x8012
	0x000028C0: 0x08000A15 '....' - j          loc_00002854
	0x000028C4: 0x34450003 '..E4' - ori        $a1, $v0, 0x3

loc_000028C8:		; Refs: 0x000028B0 
	0x000028C8: 0x1077FFC7 '..w.' - beq        $v1, $s7, loc_000027E8
	0x000028CC: 0x3C028012 '...<' - lui        $v0, 0x8012
	0x000028D0: 0x08000A15 '....' - j          loc_00002854
	0x000028D4: 0x34450003 '..E4' - ori        $a1, $v0, 0x3

loc_000028D8:		; Refs: 0x000027C0 
	0x000028D8: 0x106FFFC3 '..o.' - beq        $v1, $t7, loc_000027E8
	0x000028DC: 0x28690205 '..i(' - slti       $t1, $v1, 517
	0x000028E0: 0x1120000D '.. .' - beqz       $t1, loc_00002918
	0x000028E4: 0x00000000 '....' - nop        
	0x000028E8: 0x1070FFBF '..p.' - beq        $v1, $s0, loc_000027E8
	0x000028EC: 0x28680111 '..h(' - slti       $t0, $v1, 273
	0x000028F0: 0x11000005 '....' - beqz       $t0, loc_00002908
	0x000028F4: 0x00000000 '....' - nop        
	0x000028F8: 0x1075FFBB '..u.' - beq        $v1, $s5, loc_000027E8
	0x000028FC: 0x3C028012 '...<' - lui        $v0, 0x8012
	0x00002900: 0x08000A15 '....' - j          loc_00002854
	0x00002904: 0x34450003 '..E4' - ori        $a1, $v0, 0x3

loc_00002908:		; Refs: 0x000028F0 
	0x00002908: 0x1076FFB7 '..v.' - beq        $v1, $s6, loc_000027E8
	0x0000290C: 0x3C028012 '...<' - lui        $v0, 0x8012
	0x00002910: 0x08000A15 '....' - j          loc_00002854
	0x00002914: 0x34450003 '..E4' - ori        $a1, $v0, 0x3

loc_00002918:		; Refs: 0x000028E0 
	0x00002918: 0x1079FFB3 '..y.' - beq        $v1, $t9, loc_000027E8
	0x0000291C: 0x28650405 '..e(' - slti       $a1, $v1, 1029
	0x00002920: 0x10A00005 '....' - beqz       $a1, loc_00002938
	0x00002924: 0x00000000 '....' - nop        
	0x00002928: 0x1072FFAF '..r.' - beq        $v1, $s2, loc_000027E8
	0x0000292C: 0x3C028012 '...<' - lui        $v0, 0x8012
	0x00002930: 0x08000A15 '....' - j          loc_00002854
	0x00002934: 0x34450003 '..E4' - ori        $a1, $v0, 0x3

loc_00002938:		; Refs: 0x00002920 
	0x00002938: 0x1073FFAB '..s.' - beq        $v1, $s3, loc_000027E8
	0x0000293C: 0x3C028012 '...<' - lui        $v0, 0x8012
	0x00002940: 0x08000A15 '....' - j          loc_00002854
	0x00002944: 0x34450003 '..E4' - ori        $a1, $v0, 0x3

; ======================================================
; Subroutine sub_00002948 - Address 0x00002948 
sub_00002948:		; Refs: 0x00000FB0 
	0x00002948: 0x27BDFFE0 '...'' - addiu      $sp, $sp, -32
	0x0000294C: 0x3C038012 '...<' - lui        $v1, 0x8012
	0x00002950: 0xAFB20018 '....' - sw         $s2, 24($sp)
	0x00002954: 0x34620003 '..b4' - ori        $v0, $v1, 0x3
	0x00002958: 0x00A09021 '!...' - move       $s2, $a1
	0x0000295C: 0xAFB10014 '....' - sw         $s1, 20($sp)
	0x00002960: 0x00C08821 '!...' - move       $s1, $a2
	0x00002964: 0xAFB00010 '....' - sw         $s0, 16($sp)
	0x00002968: 0x00808021 '!...' - move       $s0, $a0
	0x0000296C: 0xAFBF001C '....' - sw         $ra, 28($sp)
	0x00002970: 0x10800039 '9...' - beqz       $a0, loc_00002A58
	0x00002974: 0xACC00000 '....' - sw         $zr, 0($a2)
	0x00002978: 0x88860003 '....' - lwl        $a2, 3($a0)
	0x0000297C: 0x98860000 '....' - lwr        $a2, 0($a0)
; Data ref 0x00002C54 ... 0x27BDFFF0 0xAFBF0000 0x90A30001 0x90A80000 
	0x00002980: 0x3C020000 '...<' - lui        $v0, 0x0
; Data ref 0x00002C54 ... 0x27BDFFF0 0xAFBF0000 0x90A30001 0x90A80000 
	0x00002984: 0x24482C54 'T,H$' - addiu      $t0, $v0, 11348
	0x00002988: 0x30C50002 '...0' - andi       $a1, $a2, 0x2
	0x0000298C: 0x03A02021 '! ..' - move       $a0, $sp
	0x00002990: 0x24070010 '...$' - li         $a3, 16
	0x00002994: 0x14A00036 '6...' - bnez       $a1, loc_00002A70
	0x00002998: 0x30C30004 '...0' - andi       $v1, $a2, 0x4
; Data ref 0x00002BEC ... 0x27BDFFF0 0xAFBF0000 0x88830007 0x98830004 
	0x0000299C: 0x3C040000 '...<' - lui        $a0, 0x0
; Data ref 0x00002BEC ... 0x27BDFFF0 0xAFBF0000 0x88830007 0x98830004 
	0x000029A0: 0x24872BEC '.+.$' - addiu      $a3, $a0, 11244
	0x000029A4: 0x1460001B '..`.' - bnez       $v1, loc_00002A14
	0x000029A8: 0x24060010 '...$' - li         $a2, 16
	0x000029AC: 0x8A040037 '7...' - lwl        $a0, 55($s0)
	0x000029B0: 0x9A040034 '4...' - lwr        $a0, 52($s0)
	0x000029B4: 0x8A050017 '....' - lwl        $a1, 23($s0)
; Data ref 0x0A150B7A
	0x000029B8: 0x0C000B7A 'z...' - jal        sub_00002DE8
	0x000029BC: 0x9A050014 '....' - lwr        $a1, 20($s0)
	0x000029C0: 0x8A190003 '....' - lwl        $t9, 3($s0)
	0x000029C4: 0x9A190000 '....' - lwr        $t9, 0($s0)
	0x000029C8: 0x372D0004 '..-7' - ori        $t5, $t9, 0x4
	0x000029CC: 0x000D7602 '.v..' - srl        $t6, $t5, 24
	0x000029D0: 0x7DB83A00 '.:.}' - ext        $t8, $t5, 8, 8
	0x000029D4: 0x7DAF3C00 '.<.}' - ext        $t7, $t5, 16, 8
	0x000029D8: 0xA2180001 '....' - sb         $t8, 1($s0)
	0x000029DC: 0xA20F0002 '....' - sb         $t7, 2($s0)
	0x000029E0: 0xA20E0003 '....' - sb         $t6, 3($s0)
	0x000029E4: 0xA20D0000 '....' - sb         $t5, 0($s0)
	0x000029E8: 0x8A0C0007 '....' - lwl        $t4, 7($s0)
	0x000029EC: 0x9A0C0004 '....' - lwr        $t4, 4($s0)
	0x000029F0: 0x89870013 '....' - lwl        $a3, 19($t4)
	0x000029F4: 0x99870010 '....' - lwr        $a3, 16($t4)
	0x000029F8: 0x00074602 '.F..' - srl        $t0, $a3, 24
	0x000029FC: 0x7CEB3A00 '.:.|' - ext        $t3, $a3, 8, 8
	0x00002A00: 0x7CEA3C00 '.<.|' - ext        $t2, $a3, 16, 8
	0x00002A04: 0xA20B0015 '....' - sb         $t3, 21($s0)
	0x00002A08: 0xA20A0016 '....' - sb         $t2, 22($s0)
	0x00002A0C: 0xA2080017 '....' - sb         $t0, 23($s0)
	0x00002A10: 0xA2070014 '....' - sb         $a3, 20($s0)

loc_00002A14:		; Refs: 0x000029A4 
	0x00002A14: 0x8A090007 '....' - lwl        $t1, 7($s0)
	0x00002A18: 0x9A090004 '....' - lwr        $t1, 4($s0)
	0x00002A1C: 0x8A050037 '7...' - lwl        $a1, 55($s0)
	0x00002A20: 0x9A050034 '4...' - lwr        $a1, 52($s0)
; Data ref 0x00002C30 ... 0x27BDFFF0 0xAFBF0000 0x88A30007 0x98A30004 
	0x00002A24: 0x3C100000 '...<' - lui        $s0, 0x0
	0x00002A28: 0x89260013 '..&.' - lwl        $a2, 19($t1)
	0x00002A2C: 0x99260010 '..&.' - lwr        $a2, 16($t1)
	0x00002A30: 0x02402021 '! @.' - move       $a0, $s2
	0x00002A34: 0x24070010 '...$' - li         $a3, 16
; Data ref 0x00002C30 ... 0x27BDFFF0 0xAFBF0000 0x88A30007 0x98A30004 
	0x00002A38: 0x26082C30 '0,.&' - addiu      $t0, $s0, 11312

loc_00002A3C:		; Refs: 0x00002A94 
	0x00002A3C: 0x0C000B4F 'O...' - jal        sub_00002D3C
	0x00002A40: 0x3C128012 '...<' - lui        $s2, 0x8012
	0x00002A44: 0x00401821 '!.@.' - move       $v1, $v0
	0x00002A48: 0x10600003 '..`.' - beqz       $v1, loc_00002A58
	0x00002A4C: 0x36420005 '..B6' - ori        $v0, $s2, 0x5
	0x00002A50: 0xAE230000 '..#.' - sw         $v1, 0($s1)
	0x00002A54: 0x00001021 '!...' - move       $v0, $zr

loc_00002A58:		; Refs: 0x00002970 0x00002A48 
	0x00002A58: 0x8FBF001C '....' - lw         $ra, 28($sp)
	0x00002A5C: 0x8FB20018 '....' - lw         $s2, 24($sp)
	0x00002A60: 0x8FB10014 '....' - lw         $s1, 20($sp)
	0x00002A64: 0x8FB00010 '....' - lw         $s0, 16($sp)
	0x00002A68: 0x03E00008 '....' - jr         $ra
	0x00002A6C: 0x27BD0020 ' ..'' - addiu      $sp, $sp, 32

loc_00002A70:		; Refs: 0x00002994 
	0x00002A70: 0x8A03000F '....' - lwl        $v1, 15($s0)
	0x00002A74: 0x9A03000C '....' - lwr        $v1, 12($s0)
	0x00002A78: 0x8A090007 '....' - lwl        $t1, 7($s0)
	0x00002A7C: 0x9A090004 '....' - lwr        $t1, 4($s0)
	0x00002A80: 0xAFB20000 '....' - sw         $s2, 0($sp)
	0x00002A84: 0xAFA30004 '....' - sw         $v1, 4($sp)
	0x00002A88: 0x8A05000B '....' - lwl        $a1, 11($s0)
	0x00002A8C: 0x9A050008 '....' - lwr        $a1, 8($s0)
	0x00002A90: 0x89260013 '..&.' - lwl        $a2, 19($t1)
	0x00002A94: 0x08000A8F '....' - j          loc_00002A3C
	0x00002A98: 0x99260010 '..&.' - lwr        $a2, 16($t1)

; ======================================================
; Subroutine sub_00002A9C - Address 0x00002A9C 
sub_00002A9C:		; Refs: 0x00002304 
	0x00002A9C: 0x2C860001 '...,' - sltiu      $a2, $a0, 1
	0x00002AA0: 0x2CA20001 '...,' - sltiu      $v0, $a1, 1
	0x00002AA4: 0x00803821 '!8..' - move       $a3, $a0
	0x00002AA8: 0x00C22025 '% ..' - or         $a0, $a2, $v0
	0x00002AAC: 0x1480004D 'M...' - bnez       $a0, loc_00002BE4
	0x00002AB0: 0x00000000 '....' - nop        
	0x00002AB4: 0x88E40003 '....' - lwl        $a0, 3($a3)
	0x00002AB8: 0x98E40000 '....' - lwr        $a0, 0($a3)
	0x00002ABC: 0x30830002 '...0' - andi       $v1, $a0, 0x2
	0x00002AC0: 0x14600048 'H.`.' - bnez       $v1, loc_00002BE4
	0x00002AC4: 0x00000000 '....' - nop        
	0x00002AC8: 0x88A30003 '....' - lwl        $v1, 3($a1)
	0x00002ACC: 0x98A30000 '....' - lwr        $v1, 0($a1)
	0x00002AD0: 0x50600018 '..`P' - beqzl      $v1, loc_00002B34
	0x00002AD4: 0xA0A0000F '....' - sb         $zr, 15($a1)
	0x00002AD8: 0x88A9000F '....' - lwl        $t1, 15($a1)
	0x00002ADC: 0x98A9000C '....' - lwr        $t1, 12($a1)
	0x00002AE0: 0x31280002 '..(1' - andi       $t0, $t1, 0x2
	0x00002AE4: 0x51000013 '...Q' - beqzl      $t0, loc_00002B34
	0x00002AE8: 0xA0A0000F '....' - sb         $zr, 15($a1)
	0x00002AEC: 0x88A20007 '....' - lwl        $v0, 7($a1)
	0x00002AF0: 0x98A20004 '....' - lwr        $v0, 4($a1)
	0x00002AF4: 0x10400008 '..@.' - beqz       $v0, loc_00002B18
	0x00002AF8: 0x240AFFFF '...$' - li         $t2, -1
	0x00002AFC: 0xA0400000 '..@.' - sb         $zr, 0($v0)
	0x00002B00: 0xA0A00004 '....' - sb         $zr, 4($a1)
	0x00002B04: 0xA0A00005 '....' - sb         $zr, 5($a1)
	0x00002B08: 0xA0A00006 '....' - sb         $zr, 6($a1)
	0x00002B0C: 0xA0A00007 '....' - sb         $zr, 7($a1)
	0x00002B10: 0x88A30003 '....' - lwl        $v1, 3($a1)
	0x00002B14: 0x98A30000 '....' - lwr        $v1, 0($a1)

loc_00002B18:		; Refs: 0x00002AF4 
	0x00002B18: 0xA06A0001 '..j.' - sb         $t2, 1($v1)
	0x00002B1C: 0xA06A0000 '..j.' - sb         $t2, 0($v1)
	0x00002B20: 0xA0A00000 '....' - sb         $zr, 0($a1)
	0x00002B24: 0xA0A00001 '....' - sb         $zr, 1($a1)
	0x00002B28: 0xA0A00002 '....' - sb         $zr, 2($a1)
	0x00002B2C: 0xA0A00003 '....' - sb         $zr, 3($a1)
	0x00002B30: 0xA0A0000F '....' - sb         $zr, 15($a1)

loc_00002B34:		; Refs: 0x00002AD0 0x00002AE4 
	0x00002B34: 0xA0A00008 '....' - sb         $zr, 8($a1)
	0x00002B38: 0xA0A00009 '....' - sb         $zr, 9($a1)
	0x00002B3C: 0xA0A0000A '....' - sb         $zr, 10($a1)
	0x00002B40: 0xA0A0000B '....' - sb         $zr, 11($a1)
	0x00002B44: 0xA0A0000C '....' - sb         $zr, 12($a1)
	0x00002B48: 0xA0A0000D '....' - sb         $zr, 13($a1)
	0x00002B4C: 0xA0A0000E '....' - sb         $zr, 14($a1)
	0x00002B50: 0x88F90007 '....' - lwl        $t9, 7($a3)
	0x00002B54: 0x98F90004 '....' - lwr        $t9, 4($a3)
	0x00002B58: 0x88EB0007 '....' - lwl        $t3, 7($a3)
	0x00002B5C: 0x98EB0004 '....' - lwr        $t3, 4($a3)
	0x00002B60: 0x8B380013 '..8.' - lwl        $t8, 19($t9)
	0x00002B64: 0x9B380010 '..8.' - lwr        $t8, 16($t9)
	0x00002B68: 0x270CFFFF '...'' - addiu      $t4, $t8, -1
	0x00002B6C: 0x000C7E02 '.~..' - srl        $t7, $t4, 24
	0x00002B70: 0x7D8E3A00 '.:.}' - ext        $t6, $t4, 8, 8
	0x00002B74: 0x7D8D3C00 '.<.}' - ext        $t5, $t4, 16, 8
	0x00002B78: 0xA16F0013 '..o.' - sb         $t7, 19($t3)
	0x00002B7C: 0xA16E0011 '..n.' - sb         $t6, 17($t3)
	0x00002B80: 0xA16D0012 '..m.' - sb         $t5, 18($t3)
	0x00002B84: 0xA16C0010 '..l.' - sb         $t4, 16($t3)
	0x00002B88: 0x88E30003 '....' - lwl        $v1, 3($a3)
	0x00002B8C: 0x98E30000 '....' - lwr        $v1, 0($a3)
	0x00002B90: 0x30650004 '..e0' - andi       $a1, $v1, 0x4
	0x00002B94: 0x10A0000C '....' - beqz       $a1, loc_00002BC8
	0x00002B98: 0x34690040 '@.i4' - ori        $t1, $v1, 0x40
	0x00002B9C: 0x38660004 '..f8' - xori       $a2, $v1, 0x4
	0x00002BA0: 0x00061602 '....' - srl        $v0, $a2, 24
	0x00002BA4: 0x7CC43A00 '.:.|' - ext        $a0, $a2, 8, 8
	0x00002BA8: 0x7CC83C00 '.<.|' - ext        $t0, $a2, 16, 8
	0x00002BAC: 0xA0E40001 '....' - sb         $a0, 1($a3)
	0x00002BB0: 0xA0E80002 '....' - sb         $t0, 2($a3)
	0x00002BB4: 0xA0E20003 '....' - sb         $v0, 3($a3)
	0x00002BB8: 0xA0E60000 '....' - sb         $a2, 0($a3)
	0x00002BBC: 0x88E30003 '....' - lwl        $v1, 3($a3)
	0x00002BC0: 0x98E30000 '....' - lwr        $v1, 0($a3)
	0x00002BC4: 0x34690040 '@.i4' - ori        $t1, $v1, 0x40

loc_00002BC8:		; Refs: 0x00002B94 
	0x00002BC8: 0x00095E02 '.^..' - srl        $t3, $t1, 24
	0x00002BCC: 0x7D253A00 '.:%}' - ext        $a1, $t1, 8, 8
	0x00002BD0: 0x7D2A3C00 '.<*}' - ext        $t2, $t1, 16, 8
	0x00002BD4: 0xA0EB0003 '....' - sb         $t3, 3($a3)
	0x00002BD8: 0xA0E50001 '....' - sb         $a1, 1($a3)
	0x00002BDC: 0xA0EA0002 '....' - sb         $t2, 2($a3)
	0x00002BE0: 0xA0E90000 '....' - sb         $t1, 0($a3)

loc_00002BE4:		; Refs: 0x00002AAC 0x00002AC0 
	0x00002BE4: 0x03E00008 '....' - jr         $ra
	0x00002BE8: 0x00000000 '....' - nop        
	0x00002BEC: 0x27BDFFF0 '...'' - addiu      $sp, $sp, -16
	0x00002BF0: 0xAFBF0000 '....' - sw         $ra, 0($sp)
	0x00002BF4: 0x88830007 '....' - lwl        $v1, 7($a0)
	0x00002BF8: 0x98830004 '....' - lwr        $v1, 4($a0)
	0x00002BFC: 0x00602021 '! `.' - move       $a0, $v1
	0x00002C00: 0x88A30007 '....' - lwl        $v1, 7($a1)
	0x00002C04: 0x98A30004 '....' - lwr        $v1, 4($a1)
	0x00002C08: 0x10800006 '....' - beqz       $a0, loc_00002C24
	0x00002C0C: 0x24020001 '...$' - li         $v0, 1
	0x00002C10: 0x00602821 '!(`.' - move       $a1, $v1
	0x00002C14: 0x10600003 '..`.' - beqz       $v1, loc_00002C24
	0x00002C18: 0x2402FFFF '...$' - li         $v0, -1
; Data ref 0x0A8F0B44
	0x00002C1C: 0x0C000B44 'D...' - jal        sub_00002D10
	0x00002C20: 0x00000000 '....' - nop        

loc_00002C24:		; Refs: 0x00002C08 0x00002C14 
	0x00002C24: 0x8FBF0000 '....' - lw         $ra, 0($sp)
	0x00002C28: 0x03E00008 '....' - jr         $ra
	0x00002C2C: 0x27BD0010 '...'' - addiu      $sp, $sp, 16
	0x00002C30: 0x27BDFFF0 '...'' - addiu      $sp, $sp, -16
	0x00002C34: 0xAFBF0000 '....' - sw         $ra, 0($sp)
	0x00002C38: 0x88A30007 '....' - lwl        $v1, 7($a1)
	0x00002C3C: 0x98A30004 '....' - lwr        $v1, 4($a1)
	0x00002C40: 0x0C000B44 'D...' - jal        sub_00002D10
	0x00002C44: 0x00602821 '!(`.' - move       $a1, $v1
	0x00002C48: 0x8FBF0000 '....' - lw         $ra, 0($sp)
	0x00002C4C: 0x03E00008 '....' - jr         $ra
	0x00002C50: 0x27BD0010 '...'' - addiu      $sp, $sp, 16
	0x00002C54: 0x27BDFFF0 '...'' - addiu      $sp, $sp, -16
	0x00002C58: 0xAFBF0000 '....' - sw         $ra, 0($sp)
	0x00002C5C: 0x90A30001 '....' - lbu        $v1, 1($a1)
	0x00002C60: 0x90A80000 '....' - lbu        $t0, 0($a1)
	0x00002C64: 0x8C860004 '....' - lw         $a2, 4($a0)
	0x00002C68: 0x00032A00 '.*..' - sll        $a1, $v1, 8
	0x00002C6C: 0x8C840000 '....' - lw         $a0, 0($a0)
	0x00002C70: 0x00A83825 '%8..' - or         $a3, $a1, $t0
	0x00002C74: 0x0C000B44 'D...' - jal        sub_00002D10
	0x00002C78: 0x00C72821 '!(..' - addu       $a1, $a2, $a3
	0x00002C7C: 0x8FBF0000 '....' - lw         $ra, 0($sp)
	0x00002C80: 0x03E00008 '....' - jr         $ra
	0x00002C84: 0x27BD0010 '...'' - addiu      $sp, $sp, 16

; ======================================================
; Subroutine sub_00002C88 - Address 0x00002C88 
sub_00002C88:		; Refs: 0x00000F84 
	0x00002C88: 0x2C860001 '...,' - sltiu      $a2, $a0, 1
	0x00002C8C: 0x2CA30001 '...,' - sltiu      $v1, $a1, 1
	0x00002C90: 0x00C33825 '%8..' - or         $a3, $a2, $v1
	0x00002C94: 0x3C028012 '...<' - lui        $v0, 0x8012
	0x00002C98: 0x14E00006 '....' - bnez       $a3, loc_00002CB4
	0x00002C9C: 0x34460003 '..F4' - ori        $a2, $v0, 0x3
; Data ref 0x00004A40 ... 0x00001228 0x000012F0 0x00000000 0x00000000 
	0x00002CA0: 0x3C070000 '...<' - lui        $a3, 0x0
; Data ref 0x00004A40 ... 0x00001228 0x000012F0 0x00000000 0x00000000 
	0x00002CA4: 0xACE44A40 '@J..' - sw         $a0, 19008($a3)
; Data ref 0x00004A44 ... 0x000012F0 0x00000000 0x00000000 0x00000000 
	0x00002CA8: 0x3C040000 '...<' - lui        $a0, 0x0
	0x00002CAC: 0x00003021 '!0..' - move       $a2, $zr
; Data ref 0x00004A44 ... 0x000012F0 0x00000000 0x00000000 0x00000000 
	0x00002CB0: 0xAC854A44 'DJ..' - sw         $a1, 19012($a0)

loc_00002CB4:		; Refs: 0x00002C98 
	0x00002CB4: 0x03E00008 '....' - jr         $ra
	0x00002CB8: 0x00C01021 '!...' - move       $v0, $a2

; ======================================================
; Subroutine sub_00002CBC - Address 0x00002CBC 
sub_00002CBC:		; Refs: 0x000020C4 0x000025B4 
	0x00002CBC: 0x24C6FFFF '...$' - addiu      $a2, $a2, -1
	0x00002CC0: 0x2403FFFF '...$' - li         $v1, -1
	0x00002CC4: 0x50C30010 '...P' - beql       $a2, $v1, loc_00002D08
	0x00002CC8: 0x00001821 '!...' - move       $v1, $zr
	0x00002CCC: 0x2408FFFF '...$' - li         $t0, -1
	0x00002CD0: 0x80820000 '....' - lb         $v0, 0($a0)

loc_00002CD4:		; Refs: 0x00002CFC 
	0x00002CD4: 0x80AA0000 '....' - lb         $t2, 0($a1)
	0x00002CD8: 0x24C6FFFF '...$' - addiu      $a2, $a2, -1
	0x00002CDC: 0x24840001 '...$' - addiu      $a0, $a0, 1
	0x00002CE0: 0x0142482A '*HB.' - slt        $t1, $t2, $v0
	0x00002CE4: 0x004A382A '*8J.' - slt        $a3, $v0, $t2
	0x00002CE8: 0x24A50001 '...$' - addiu      $a1, $a1, 1
	0x00002CEC: 0x15200006 '.. .' - bnez       $t1, loc_00002D08
	0x00002CF0: 0x24030001 '...$' - li         $v1, 1
	0x00002CF4: 0x14E00004 '....' - bnez       $a3, loc_00002D08
	0x00002CF8: 0x2403FFFF '...$' - li         $v1, -1
	0x00002CFC: 0x54C8FFF5 '...T' - bnel       $a2, $t0, loc_00002CD4
	0x00002D00: 0x80820000 '....' - lb         $v0, 0($a0)
	0x00002D04: 0x00001821 '!...' - move       $v1, $zr

loc_00002D08:		; Refs: 0x00002CC4 0x00002CEC 0x00002CF4 
	0x00002D08: 0x03E00008 '....' - jr         $ra
	0x00002D0C: 0x00601021 '!.`.' - move       $v0, $v1

; ======================================================
; Subroutine sub_00002D10 - Address 0x00002D10 
sub_00002D10:		; Refs: 0x0000130C 0x00002C1C 0x00002C40 0x00002C74 0x00002D2C 
	0x00002D10: 0x90A60000 '....' - lbu        $a2, 0($a1)
	0x00002D14: 0x90830000 '....' - lbu        $v1, 0($a0)
	0x00002D18: 0x24A50001 '...$' - addiu      $a1, $a1, 1
	0x00002D1C: 0x24840001 '...$' - addiu      $a0, $a0, 1
	0x00002D20: 0x00663823 '#8f.' - subu       $a3, $v1, $a2
	0x00002D24: 0x14E00003 '....' - bnez       $a3, loc_00002D34
	0x00002D28: 0x00E03021 '!0..' - move       $a2, $a3
	0x00002D2C: 0x1460FFF8 '..`.' - bnez       $v1, sub_00002D10
	0x00002D30: 0x00003021 '!0..' - move       $a2, $zr

loc_00002D34:		; Refs: 0x00002D24 
	0x00002D34: 0x03E00008 '....' - jr         $ra
	0x00002D38: 0x00C01021 '!...' - move       $v0, $a2

; ======================================================
; Subroutine sub_00002D3C - Address 0x00002D3C 
sub_00002D3C:		; Refs: 0x00002A3C 
	0x00002D3C: 0x27BDFFE0 '...'' - addiu      $sp, $sp, -32
	0x00002D40: 0xAFB60018 '....' - sw         $s6, 24($sp)
	0x00002D44: 0x0080B021 '!...' - move       $s6, $a0
	0x00002D48: 0xAFB50014 '....' - sw         $s5, 20($sp)
	0x00002D4C: 0x0100A821 '!...' - move       $s5, $t0
	0x00002D50: 0xAFB40010 '....' - sw         $s4, 16($sp)
	0x00002D54: 0x00E0A021 '!...' - move       $s4, $a3
	0x00002D58: 0xAFB3000C '....' - sw         $s3, 12($sp)
	0x00002D5C: 0x00A09821 '!...' - move       $s3, $a1
	0x00002D60: 0xAFB20008 '....' - sw         $s2, 8($sp)
	0x00002D64: 0x00C09021 '!...' - move       $s2, $a2
	0x00002D68: 0xAFBF001C '....' - sw         $ra, 28($sp)
	0x00002D6C: 0xAFB10004 '....' - sw         $s1, 4($sp)
	0x00002D70: 0x10C00011 '....' - beqz       $a2, loc_00002DB8
	0x00002D74: 0xAFB00000 '....' - sw         $s0, 0($sp)

loc_00002D78:		; Refs: 0x00002DB0 
	0x00002D78: 0x00128843 'C...' - sra        $s1, $s2, 1
	0x00002D7C: 0x02340018 '..4.' - mult       $s1, $s4
	0x00002D80: 0x02C02021 '! ..' - move       $a0, $s6
	0x00002D84: 0x2652FFFF '..R&' - addiu      $s2, $s2, -1
	0x00002D88: 0x00001812 '....' - mflo       $v1
	0x00002D8C: 0x02638021 '!.c.' - addu       $s0, $s3, $v1
	0x00002D90: 0x02A0F809 '....' - jalr       $s5
	0x00002D94: 0x02002821 '!(..' - move       $a1, $s0
	0x00002D98: 0x10400008 '..@.' - beqz       $v0, loc_00002DBC
	0x00002D9C: 0x02001821 '!...' - move       $v1, $s0
	0x00002DA0: 0x18400003 '..@.' - blez       $v0, loc_00002DB0
	0x00002DA4: 0x00000000 '....' - nop        
	0x00002DA8: 0x02149821 '!...' - addu       $s3, $s0, $s4
	0x00002DAC: 0x00128843 'C...' - sra        $s1, $s2, 1

loc_00002DB0:		; Refs: 0x00002DA0 
	0x00002DB0: 0x1620FFF1 '.. .' - bnez       $s1, loc_00002D78
	0x00002DB4: 0x02209021 '!. .' - move       $s2, $s1

loc_00002DB8:		; Refs: 0x00002D70 
	0x00002DB8: 0x00001821 '!...' - move       $v1, $zr

loc_00002DBC:		; Refs: 0x00002D98 
	0x00002DBC: 0x8FBF001C '....' - lw         $ra, 28($sp)
	0x00002DC0: 0x8FB60018 '....' - lw         $s6, 24($sp)
	0x00002DC4: 0x8FB50014 '....' - lw         $s5, 20($sp)
	0x00002DC8: 0x8FB40010 '....' - lw         $s4, 16($sp)
	0x00002DCC: 0x8FB3000C '....' - lw         $s3, 12($sp)
	0x00002DD0: 0x8FB20008 '....' - lw         $s2, 8($sp)
	0x00002DD4: 0x8FB10004 '....' - lw         $s1, 4($sp)
	0x00002DD8: 0x8FB00000 '....' - lw         $s0, 0($sp)
	0x00002DDC: 0x00601021 '!.`.' - move       $v0, $v1
	0x00002DE0: 0x03E00008 '....' - jr         $ra
	0x00002DE4: 0x27BD0020 ' ..'' - addiu      $sp, $sp, 32

; ======================================================
; Subroutine sub_00002DE8 - Address 0x00002DE8 
sub_00002DE8:		; Refs: 0x000029B8 
	0x00002DE8: 0x27BDFFC0 '...'' - addiu      $sp, $sp, -64
	0x00002DEC: 0xAFB40020 ' ...' - sw         $s4, 32($sp)
	0x00002DF0: 0x0005A042 'B...' - srl        $s4, $a1, 1
	0x00002DF4: 0xAFBE0030 '0...' - sw         $fp, 48($sp)
	0x00002DF8: 0x00E0F021 '!...' - move       $fp, $a3
	0x00002DFC: 0xAFB50024 '$...' - sw         $s5, 36($sp)
	0x00002E00: 0x0080A821 '!...' - move       $s5, $a0
	0x00002E04: 0xAFB00010 '....' - sw         $s0, 16($sp)
	0x00002E08: 0x00C08021 '!...' - move       $s0, $a2
	0x00002E0C: 0xAFBF0034 '4...' - sw         $ra, 52($sp)
	0x00002E10: 0xAFB7002C ',...' - sw         $s7, 44($sp)
	0x00002E14: 0xAFB60028 '(...' - sw         $s6, 40($sp)
	0x00002E18: 0xAFB3001C '....' - sw         $s3, 28($sp)
	0x00002E1C: 0xAFB20018 '....' - sw         $s2, 24($sp)
	0x00002E20: 0xAFB10014 '....' - sw         $s1, 20($sp)
	0x00002E24: 0x12800031 '1...' - beqz       $s4, loc_00002EEC
	0x00002E28: 0xAFA50000 '....' - sw         $a1, 0($sp)
	0x00002E2C: 0x8FA20000 '....' - lw         $v0, 0($sp)

loc_00002E30:		; Refs: 0x00002EE4 
	0x00002E30: 0x0282202B '+ ..' - sltu       $a0, $s4, $v0
	0x00002E34: 0x1080002A '*...' - beqz       $a0, loc_00002EE0
	0x00002E38: 0x0280B021 '!...' - move       $s6, $s4
	0x00002E3C: 0x02D49023 '#...' - subu       $s2, $s6, $s4

loc_00002E40:		; Refs: 0x00002ED8 
	0x00002E40: 0x06400023 '#.@.' - bltz       $s2, loc_00002ED0
	0x00002E44: 0x8FB10000 '....' - lw         $s1, 0($sp)
	0x00002E48: 0x0010B82B '+...' - sltu       $s7, $zr, $s0
	0x00002E4C: 0x02542821 '!(T.' - addu       $a1, $s2, $s4

loc_00002E50:		; Refs: 0x00002EC4 
	0x00002E50: 0x00B00018 '....' - mult       $a1, $s0
	0x00002E54: 0x00001812 '....' - mflo       $v1
	0x00002E58: 0x02500018 '..P.' - mult       $s2, $s0
	0x00002E5C: 0x02A39821 '!...' - addu       $s3, $s5, $v1
	0x00002E60: 0x02602821 '!(`.' - move       $a1, $s3
	0x00002E64: 0x00002012 '. ..' - mflo       $a0
	0x00002E68: 0x02A48821 '!...' - addu       $s1, $s5, $a0
	0x00002E6C: 0x03C0F809 '....' - jalr       $fp
	0x00002E70: 0x02202021 '!  .' - move       $a0, $s1
	0x00002E74: 0x58400016 '..@X' - blezl      $v0, loc_00002ED0
	0x00002E78: 0x8FB10000 '....' - lw         $s1, 0($sp)
	0x00002E7C: 0x12E00010 '....' - beqz       $s7, loc_00002EC0
	0x00002E80: 0x00003821 '!8..' - move       $a3, $zr

loc_00002E84:		; Refs: 0x00002EB8 
	0x00002E84: 0x02273021 '!0'.' - addu       $a2, $s1, $a3
	0x00002E88: 0x02676021 '!`g.' - addu       $t4, $s3, $a3
	0x00002E8C: 0x90CF0000 '....' - lbu        $t7, 0($a2)
	0x00002E90: 0x91980000 '....' - lbu        $t8, 0($t4)
	0x00002E94: 0x24E70001 '...$' - addiu      $a3, $a3, 1
	0x00002E98: 0x00F0402B '+@..' - sltu       $t0, $a3, $s0
	0x00002E9C: 0x01F87026 '&p..' - xor        $t6, $t7, $t8
	0x00002EA0: 0xA0CE0000 '....' - sb         $t6, 0($a2)
	0x00002EA4: 0x918D0000 '....' - lbu        $t5, 0($t4)
	0x00002EA8: 0x01AE5826 '&X..' - xor        $t3, $t5, $t6
	0x00002EAC: 0xA18B0000 '....' - sb         $t3, 0($t4)
	0x00002EB0: 0x90CA0000 '....' - lbu        $t2, 0($a2)
	0x00002EB4: 0x014B4826 '&HK.' - xor        $t1, $t2, $t3
	0x00002EB8: 0x1500FFF2 '....' - bnez       $t0, loc_00002E84
	0x00002EBC: 0xA0C90000 '....' - sb         $t1, 0($a2)

loc_00002EC0:		; Refs: 0x00002E7C 
	0x00002EC0: 0x02549023 '#.T.' - subu       $s2, $s2, $s4
	0x00002EC4: 0x0641FFE2 '..A.' - bgez       $s2, loc_00002E50
	0x00002EC8: 0x02542821 '!(T.' - addu       $a1, $s2, $s4
	0x00002ECC: 0x8FB10000 '....' - lw         $s1, 0($sp)

loc_00002ED0:		; Refs: 0x00002E40 0x00002E74 
	0x00002ED0: 0x26D60001 '...&' - addiu      $s6, $s6, 1
	0x00002ED4: 0x02D1382B '+8..' - sltu       $a3, $s6, $s1
	0x00002ED8: 0x54E0FFD9 '...T' - bnezl      $a3, loc_00002E40
	0x00002EDC: 0x02D49023 '#...' - subu       $s2, $s6, $s4

loc_00002EE0:		; Refs: 0x00002E34 
	0x00002EE0: 0x0014A042 'B...' - srl        $s4, $s4, 1
	0x00002EE4: 0x1680FFD2 '....' - bnez       $s4, loc_00002E30
	0x00002EE8: 0x8FA20000 '....' - lw         $v0, 0($sp)

loc_00002EEC:		; Refs: 0x00002E24 
	0x00002EEC: 0x8FBF0034 '4...' - lw         $ra, 52($sp)
	0x00002EF0: 0x8FBE0030 '0...' - lw         $fp, 48($sp)
	0x00002EF4: 0x8FB7002C ',...' - lw         $s7, 44($sp)
	0x00002EF8: 0x8FB60028 '(...' - lw         $s6, 40($sp)
	0x00002EFC: 0x8FB50024 '$...' - lw         $s5, 36($sp)
	0x00002F00: 0x8FB40020 ' ...' - lw         $s4, 32($sp)
	0x00002F04: 0x8FB3001C '....' - lw         $s3, 28($sp)
	0x00002F08: 0x8FB20018 '....' - lw         $s2, 24($sp)
	0x00002F0C: 0x8FB10014 '....' - lw         $s1, 20($sp)
	0x00002F10: 0x8FB00010 '....' - lw         $s0, 16($sp)
	0x00002F14: 0x03E00008 '....' - jr         $ra
	0x00002F18: 0x27BD0040 '@..'' - addiu      $sp, $sp, 64

; ======================================================
; Subroutine sub_00002F1C - Address 0x00002F1C 
sub_00002F1C:		; Refs: 0x0000128C 
	0x00002F1C: 0x00801821 '!...' - move       $v1, $a0

loc_00002F20:		; Refs: 0x00002F2C 
	0x00002F20: 0x90A60000 '....' - lbu        $a2, 0($a1)
	0x00002F24: 0x24A50001 '...$' - addiu      $a1, $a1, 1
	0x00002F28: 0xA0660000 '..f.' - sb         $a2, 0($v1)
	0x00002F2C: 0x14C0FFFC '....' - bnez       $a2, loc_00002F20
	0x00002F30: 0x24630001 '..c$' - addiu      $v1, $v1, 1
	0x00002F34: 0x03E00008 '....' - jr         $ra
	0x00002F38: 0x00801021 '!...' - move       $v0, $a0

; ======================================================
; Subroutine sceKernelGetChunk - Address 0x00002F3C 
; Imported from InitForKernel
sceKernelGetChunk:		; Refs: 0x0000014C 
	0x00002F3C: 0x03E00008 '....' - jr         $ra
	0x00002F40: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceKernelApplicationType - Address 0x00002F44 
; Imported from InitForKernel
sceKernelApplicationType:		; Refs: 0x0000189C 0x00001AB8 
	0x00002F44: 0x03E00008 '....' - jr         $ra
	0x00002F48: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceKernelInitApitype - Address 0x00002F4C 
; Imported from InitForKernel
sceKernelInitApitype:		; Refs: 0x00000030 0x000005D4 0x00000BA4 0x00000D7C 0x00001A6C 0x00001A80 
	0x00002F4C: 0x03E00008 '....' - jr         $ra
	0x00002F50: 0x00000000 '....' - nop        

; ======================================================
; Subroutine InitForKernel_9D33A110 - Address 0x00002F54 
; Imported from InitForKernel
InitForKernel_9D33A110:		; Refs: 0x00001A4C 0x00001A5C 
	0x00002F54: 0x03E00008 '....' - jr         $ra
	0x00002F58: 0x00000000 '....' - nop        

; ======================================================
; Subroutine InitForKernel_A18A4A8B - Address 0x00002F5C 
; Imported from InitForKernel
InitForKernel_A18A4A8B:		; Refs: 0x00000D40 
	0x00002F5C: 0x03E00008 '....' - jr         $ra
	0x00002F60: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceKernelInitFileName - Address 0x00002F64 
; Imported from InitForKernel
sceKernelInitFileName:		; Refs: 0x0000003C 0x000005BC 0x00000B8C 
	0x00002F64: 0x03E00008 '....' - jr         $ra
	0x00002F68: 0x00000000 '....' - nop        

; ======================================================
; Subroutine InterruptManagerForKernel_E526B767 - Address 0x00002F6C 
; Imported from InterruptManagerForKernel
InterruptManagerForKernel_E526B767:		; Refs: 0x000018D0 0x000018F8 
	0x00002F6C: 0x03E00008 '....' - jr         $ra
	0x00002F70: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceIoOpen - Address 0x00002F74 
; Imported from IoFileMgrForKernel
sceIoOpen:		; Refs: 0x00000800 
	0x00002F74: 0x03E00008 '....' - jr         $ra
	0x00002F78: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceIoLseek - Address 0x00002F7C 
; Imported from IoFileMgrForKernel
sceIoLseek:		; Refs: 0x0000091C 0x00001ECC 
	0x00002F7C: 0x03E00008 '....' - jr         $ra
	0x00002F80: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceIoWrite - Address 0x00002F84 
; Imported from IoFileMgrForKernel
sceIoWrite:
	0x00002F84: 0x03E00008 '....' - jr         $ra
	0x00002F88: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceIoDevctl - Address 0x00002F8C 
; Imported from IoFileMgrForKernel
sceIoDevctl:		; Refs: 0x00000520 0x00000584 0x00000AF0 0x00000B54 0x00001A44 0x00001AA8 
	0x00002F8C: 0x03E00008 '....' - jr         $ra
	0x00002F90: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceIoRead - Address 0x00002F94 
; Imported from IoFileMgrForKernel
sceIoRead:		; Refs: 0x00000824 0x00000964 0x00001EF8 
	0x00002F94: 0x03E00008 '....' - jr         $ra
	0x00002F98: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceIoClose - Address 0x00002F9C 
; Imported from IoFileMgrForKernel
sceIoClose:		; Refs: 0x00000880 
	0x00002F9C: 0x03E00008 '....' - jr         $ra
	0x00002FA0: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceIoDopen - Address 0x00002FA4 
; Imported from IoFileMgrForKernel
sceIoDopen:		; Refs: 0x000013A4 
	0x00002FA4: 0x03E00008 '....' - jr         $ra
	0x00002FA8: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceIoDread - Address 0x00002FAC 
; Imported from IoFileMgrForKernel
sceIoDread:		; Refs: 0x000013BC 
	0x00002FAC: 0x03E00008 '....' - jr         $ra
	0x00002FB0: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceIoDclose - Address 0x00002FB4 
; Imported from IoFileMgrForKernel
sceIoDclose:		; Refs: 0x00001454 
	0x00002FB4: 0x03E00008 '....' - jr         $ra
	0x00002FB8: 0x00000000 '....' - nop        

; ======================================================
; Subroutine KDebugForKernel_47570AC5 - Address 0x00002FBC 
; Imported from KDebugForKernel
KDebugForKernel_47570AC5:		; Refs: 0x00001CB8 
	0x00002FBC: 0x03E00008 '....' - jr         $ra
	0x00002FC0: 0x00000000 '....' - nop        

; ======================================================
; Subroutine Kprintf - Address 0x00002FC4 
; Imported from KDebugForKernel
Kprintf:		; Refs: 0x00000328 0x000003DC 0x000003F4 0x00000410 0x00000444 0x00000460 0x00000484 0x0000064C 0x0000067C 0x000006A0 0x000006BC 0x000006D8 0x000006F4 0x00000714 0x00000734 0x0000085C 0x000008C4 0x000009A4 0x000009C8 0x000009EC 0x00000A0C 0x00000A30 0x00000A54 0x00000C1C 0x00000C4C 0x00000C70 0x00000C8C 0x00000CA8 0x00000CC4 0x00000CE4 0x00000D04 0x00000DE0 0x00000DF8 0x00000E1C 0x00000E48 0x00000EB8 0x00000F18 0x00001078 0x000010CC 0x000010E8 0x00001148 0x00001170 0x00001210 
	0x00002FC4: 0x03E00008 '....' - jr         $ra
	0x00002FC8: 0x00000000 '....' - nop        

; ======================================================
; Subroutine KDebugForKernel_86010FCB - Address 0x00002FCC 
; Imported from KDebugForKernel
KDebugForKernel_86010FCB:		; Refs: 0x000000D4 
	0x00002FCC: 0x03E00008 '....' - jr         $ra
	0x00002FD0: 0x00000000 '....' - nop        

; ======================================================
; Subroutine KDebugForKernel_ACF427DC - Address 0x00002FD4 
; Imported from KDebugForKernel
KDebugForKernel_ACF427DC:		; Refs: 0x00001604 
	0x00002FD4: 0x03E00008 '....' - jr         $ra
	0x00002FD8: 0x00000000 '....' - nop        

; ======================================================
; Subroutine ModuleMgrForKernel_D86DD11B - Address 0x00002FDC 
; Imported from ModuleMgrForKernel
ModuleMgrForKernel_D86DD11B:		; Refs: 0x000015A4 
	0x00002FDC: 0x03E00008 '....' - jr         $ra
	0x00002FE0: 0x00000000 '....' - nop        

; ======================================================
; Subroutine SysMemForKernel_07C586A1 - Address 0x00002FE4 
; Imported from SysMemForKernel
SysMemForKernel_07C586A1:		; Refs: 0x000002BC 0x000002D0 0x000002E4 0x000002F8 0x0000030C 
	0x00002FE4: 0x03E00008 '....' - jr         $ra
	0x00002FE8: 0x00000000 '....' - nop        

; ======================================================
; Subroutine SysMemForKernel_13EE28DA - Address 0x00002FEC 
; Imported from SysMemForKernel
SysMemForKernel_13EE28DA:		; Refs: 0x00000120 
	0x00002FEC: 0x03E00008 '....' - jr         $ra
	0x00002FF0: 0x00000000 '....' - nop        

; ======================================================
; Subroutine SysMemForKernel_181065AB - Address 0x00002FF4 
; Imported from SysMemForKernel
SysMemForKernel_181065AB:		; Refs: 0x00001F58 
	0x00002FF4: 0x03E00008 '....' - jr         $ra
	0x00002FF8: 0x00000000 '....' - nop        

; ======================================================
; Subroutine SysMemForKernel_23D81675 - Address 0x00002FFC 
; Imported from SysMemForKernel
SysMemForKernel_23D81675:		; Refs: 0x000011C4 
	0x00002FFC: 0x03E00008 '....' - jr         $ra
	0x00003000: 0x00000000 '....' - nop        

; ======================================================
; Subroutine SysMemForKernel_58148F07 - Address 0x00003004 
; Imported from SysMemForKernel
SysMemForKernel_58148F07:		; Refs: 0x000011EC 
	0x00003004: 0x03E00008 '....' - jr         $ra
	0x00003008: 0x00000000 '....' - nop        

; ======================================================
; Subroutine SysMemForKernel_7158CE7E - Address 0x0000300C 
; Imported from SysMemForKernel
SysMemForKernel_7158CE7E:		; Refs: 0x000007E8 0x00000940 
	0x0000300C: 0x03E00008 '....' - jr         $ra
	0x00003010: 0x00000000 '....' - nop        

; ======================================================
; Subroutine SysMemForKernel_87C2AB85 - Address 0x00003014 
; Imported from SysMemForKernel
SysMemForKernel_87C2AB85:		; Refs: 0x000012D8 
	0x00003014: 0x03E00008 '....' - jr         $ra
	0x00003018: 0x00000000 '....' - nop        

; ======================================================
; Subroutine SysMemForKernel_8AE776AF - Address 0x0000301C 
; Imported from SysMemForKernel
SysMemForKernel_8AE776AF:		; Refs: 0x00001C7C 0x0000239C 
	0x0000301C: 0x03E00008 '....' - jr         $ra
	0x00003020: 0x00000000 '....' - nop        

; ======================================================
; Subroutine SysMemForKernel_A03CB480 - Address 0x00003024 
; Imported from SysMemForKernel
SysMemForKernel_A03CB480:		; Refs: 0x00001BDC 
	0x00003024: 0x03E00008 '....' - jr         $ra
	0x00003028: 0x00000000 '....' - nop        

; ======================================================
; Subroutine SysMemForKernel_C1A26C6F - Address 0x0000302C 
; Imported from SysMemForKernel
SysMemForKernel_C1A26C6F:		; Refs: 0x00000870 0x000008FC 
	0x0000302C: 0x03E00008 '....' - jr         $ra
	0x00003030: 0x00000000 '....' - nop        

; ======================================================
; Subroutine SysMemForKernel_DD6512D0 - Address 0x00003034 
; Imported from SysMemForKernel
SysMemForKernel_DD6512D0:		; Refs: 0x00001030 
	0x00003034: 0x03E00008 '....' - jr         $ra
	0x00003038: 0x00000000 '....' - nop        

; ======================================================
; Subroutine SysMemForKernel_EF29061C - Address 0x0000303C 
; Imported from SysMemForKernel
SysMemForKernel_EF29061C:		; Refs: 0x00000014 0x000003A4 0x000007CC 0x00000DA8 0x0000149C 0x000014CC 0x00001878 0x00001918 0x000019A4 
	0x0000303C: 0x03E00008 '....' - jr         $ra
	0x00003040: 0x00000000 '....' - nop        

; ======================================================
; Subroutine SysMemForKernel_F12A62F7 - Address 0x00003044 
; Imported from SysMemForKernel
SysMemForKernel_F12A62F7:		; Refs: 0x00000170 0x00000810 0x00000950 
	0x00003044: 0x03E00008 '....' - jr         $ra
	0x00003048: 0x00000000 '....' - nop        

; ======================================================
; Subroutine memset - Address 0x0000304C 
; Imported from SysclibForKernel
memset:		; Refs: 0x00001AE8 0x00001D50 
	0x0000304C: 0x03E00008 '....' - jr         $ra
	0x00003050: 0x00000000 '....' - nop        

; ======================================================
; Subroutine tolower - Address 0x00003054 
; Imported from SysclibForKernel
tolower:		; Refs: 0x000013E8 
	0x00003054: 0x03E00008 '....' - jr         $ra
	0x00003058: 0x00000000 '....' - nop        

; ======================================================
; Subroutine strcat - Address 0x0000305C 
; Imported from SysclibForKernel
strcat:		; Refs: 0x00001C9C 
	0x0000305C: 0x03E00008 '....' - jr         $ra
	0x00003060: 0x00000000 '....' - nop        

; ======================================================
; Subroutine strlen - Address 0x00003064 
; Imported from SysclibForKernel
strlen:		; Refs: 0x00001C84 
	0x00003064: 0x03E00008 '....' - jr         $ra
	0x00003068: 0x00000000 '....' - nop        

; ======================================================
; Subroutine strncmp - Address 0x0000306C 
; Imported from SysclibForKernel
strncmp:		; Refs: 0x0000007C 0x00000208 0x00000224 0x0000024C 0x0000027C 0x000002A4 0x000003BC 0x00000DC0 0x00000FF8 0x00001014 0x00001090 0x000010AC 0x00001104 0x00001120 0x000014EC 0x00001530 0x0000154C 
	0x0000306C: 0x03E00008 '....' - jr         $ra
	0x00003070: 0x00000000 '....' - nop        

; ======================================================
; Subroutine memcmp - Address 0x00003074 
; Imported from SysclibForKernel
memcmp:		; Refs: 0x00001428 0x0000174C 0x000017EC 0x00001820 0x00001CD8 0x00001CF4 0x00001D24 
	0x00003074: 0x03E00008 '....' - jr         $ra
	0x00003078: 0x00000000 '....' - nop        

; ======================================================
; Subroutine strncpy - Address 0x0000307C 
; Imported from SysclibForKernel
strncpy:		; Refs: 0x00001D10 
	0x0000307C: 0x03E00008 '....' - jr         $ra
	0x00003080: 0x00000000 '....' - nop        

; ======================================================
; Subroutine strcmp - Address 0x00003084 
; Imported from SysclibForKernel
strcmp:		; Refs: 0x000014B4 0x0000188C 0x0000193C 0x000019E0 
	0x00003084: 0x03E00008 '....' - jr         $ra
	0x00003088: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceKernelSetEventFlag - Address 0x0000308C 
; Imported from ThreadManForKernel
sceKernelSetEventFlag:		; Refs: 0x00000F24 
	0x0000308C: 0x03E00008 '....' - jr         $ra
	0x00003090: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceKernelWaitEventFlagCB - Address 0x00003094 
; Imported from ThreadManForKernel
sceKernelWaitEventFlagCB:		; Refs: 0x00000544 0x00000B14 
	0x00003094: 0x03E00008 '....' - jr         $ra
	0x00003098: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceKernelCreateEventFlag - Address 0x0000309C 
; Imported from ThreadManForKernel
sceKernelCreateEventFlag:		; Refs: 0x000004CC 0x00000A9C 
	0x0000309C: 0x03E00008 '....' - jr         $ra
	0x000030A0: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceKernelCreateVpl - Address 0x000030A4 
; Imported from ThreadManForKernel
sceKernelCreateVpl:		; Refs: 0x00001254 
	0x000030A4: 0x03E00008 '....' - jr         $ra
	0x000030A8: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceKernelDeleteVpl - Address 0x000030AC 
; Imported from ThreadManForKernel
sceKernelDeleteVpl:		; Refs: 0x00001338 
	0x000030AC: 0x03E00008 '....' - jr         $ra
	0x000030B0: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceKernelTryAllocateVpl - Address 0x000030B4 
; Imported from ThreadManForKernel
sceKernelTryAllocateVpl:		; Refs: 0x00001274 
	0x000030B4: 0x03E00008 '....' - jr         $ra
	0x000030B8: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceKernelFreeVpl - Address 0x000030BC 
; Imported from ThreadManForKernel
sceKernelFreeVpl:		; Refs: 0x00001324 
	0x000030BC: 0x03E00008 '....' - jr         $ra
	0x000030C0: 0x00000000 '....' - nop        

; ======================================================
; Subroutine ThreadManForKernel_D366D35A - Address 0x000030C4 
; Imported from ThreadManForKernel
ThreadManForKernel_D366D35A:		; Refs: 0x000018E8 
	0x000030C4: 0x03E00008 '....' - jr         $ra
	0x000030C8: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceKernelCreateCallback - Address 0x000030CC 
; Imported from ThreadManForKernel
sceKernelCreateCallback:		; Refs: 0x000004F8 0x00000AC8 
	0x000030CC: 0x03E00008 '....' - jr         $ra
	0x000030D0: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceKernelDeleteCallback - Address 0x000030D4 
; Imported from ThreadManForKernel
sceKernelDeleteCallback:		; Refs: 0x00000594 0x00000B64 
	0x000030D4: 0x03E00008 '....' - jr         $ra
	0x000030D8: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceKernelDeleteEventFlag - Address 0x000030DC 
; Imported from ThreadManForKernel
sceKernelDeleteEventFlag:		; Refs: 0x000005A4 0x00000B74 
	0x000030DC: 0x03E00008 '....' - jr         $ra
	0x000030E0: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceKernelGetGPI - Address 0x000030E4 
; Imported from UtilsForKernel
sceKernelGetGPI:		; Refs: 0x00000130 
	0x000030E4: 0x03E00008 '....' - jr         $ra
	0x000030E8: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceKernelUtilsMd5Digest - Address 0x000030EC 
; Imported from UtilsForKernel
sceKernelUtilsMd5Digest:		; Refs: 0x0000140C 0x00001CB0 
	0x000030EC: 0x03E00008 '....' - jr         $ra
	0x000030F0: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceAudio_driver_FF298CE7 - Address 0x000030F4 
; Imported from sceAudio_driver
sceAudio_driver_FF298CE7:		; Refs: 0x00001564 
	0x000030F4: 0x03E00008 '....' - jr         $ra
	0x000030F8: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceCtrl_driver_5886194C - Address 0x000030FC 
; Imported from sceCtrl_driver
sceCtrl_driver_5886194C:		; Refs: 0x0000196C 
	0x000030FC: 0x03E00008 '....' - jr         $ra
	0x00003100: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceOpenPSID_driver_E8316C16 - Address 0x00003104 
; Imported from sceOpenPSID_driver
sceOpenPSID_driver_E8316C16:		; Refs: 0x00001C54 
	0x00003104: 0x03E00008 '....' - jr         $ra
	0x00003108: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceReg_driver_49D77D65 - Address 0x0000310C 
; Imported from sceReg_driver
sceReg_driver_49D77D65:		; Refs: 0x00001B68 0x00001DD4 
	0x0000310C: 0x03E00008 '....' - jr         $ra
	0x00003110: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceReg_driver_4F471457 - Address 0x00003114 
; Imported from sceReg_driver
sceReg_driver_4F471457:		; Refs: 0x00001B24 0x00001D8C 
	0x00003114: 0x03E00008 '....' - jr         $ra
	0x00003118: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceReg_driver_9980519F - Address 0x0000311C 
; Imported from sceReg_driver
sceReg_driver_9980519F:		; Refs: 0x00001B44 0x00001DAC 
	0x0000311C: 0x03E00008 '....' - jr         $ra
	0x00003120: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceReg_driver_DBA46704 - Address 0x00003124 
; Imported from sceReg_driver
sceReg_driver_DBA46704:		; Refs: 0x00001B08 0x00001D70 
	0x00003124: 0x03E00008 '....' - jr         $ra
	0x00003128: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceReg_driver_F4A3E396 - Address 0x0000312C 
; Imported from sceReg_driver
sceReg_driver_F4A3E396:		; Refs: 0x00001BAC 0x00001E00 
	0x0000312C: 0x03E00008 '....' - jr         $ra
	0x00003130: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceReg_driver_FC742751 - Address 0x00003134 
; Imported from sceReg_driver
sceReg_driver_FC742751:		; Refs: 0x00001B60 0x00001DC8 
	0x00003134: 0x03E00008 '....' - jr         $ra
	0x00003138: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceUmdWaitDriveStat - Address 0x0000313C 
; Imported from sceUmd
sceUmdWaitDriveStat:		; Refs: 0x00000380 0x00000D58 
	0x0000313C: 0x03E00008 '....' - jr         $ra
	0x00003140: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceUmd_A9B5B972 - Address 0x00003144 
; Imported from sceUmd
sceUmd_A9B5B972:		; Refs: 0x0000034C 
	0x00003144: 0x03E00008 '....' - jr         $ra
	0x00003148: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceUmd_B7BF4C31 - Address 0x0000314C 
; Imported from sceUmd
sceUmd_B7BF4C31:		; Refs: 0x00000390 0x00000D68 
	0x0000314C: 0x03E00008 '....' - jr         $ra
	0x00003150: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceUmdActivate - Address 0x00003154 
; Imported from sceUmd
sceUmdActivate:		; Refs: 0x00000368 0x00000D30 
	0x00003154: 0x03E00008 '....' - jr         $ra
	0x00003158: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceUmd9660_driver_3CC9CE54 - Address 0x0000315C 
; Imported from sceUmd9660_driver
sceUmd9660_driver_3CC9CE54:		; Refs: 0x00001784 0x000017CC 0x00001804 
	0x0000315C: 0x03E00008 '....' - jr         $ra
	0x00003160: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceUmd9660_driver_FE3A8B67 - Address 0x00003164 
; Imported from sceUmd9660_driver
sceUmd9660_driver_FE3A8B67:		; Refs: 0x00001838 
	0x00003164: 0x03E00008 '....' - jr         $ra
	0x00003168: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceUmdCache_driver_576E0F06 - Address 0x0000316C 
; Imported from sceUmdCache_driver
sceUmdCache_driver_576E0F06:		; Refs: 0x000017DC 0x0000180C 
	0x0000316C: 0x03E00008 '....' - jr         $ra
	0x00003170: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceUmdCache_driver_A21D8213 - Address 0x00003174 
; Imported from sceUmdCache_driver
sceUmdCache_driver_A21D8213:		; Refs: 0x000015D0 
	0x00003174: 0x03E00008 '....' - jr         $ra
	0x00003178: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceUmdCache_driver_DB97E432 - Address 0x0000317C 
; Imported from sceUmdCache_driver
sceUmdCache_driver_DB97E432:		; Refs: 0x0000177C 0x000017C4 0x000017FC 0x00001830 
	0x0000317C: 0x03E00008 '....' - jr         $ra
	0x00003180: 0x00000000 '....' - nop        

; ======================================================
; Subroutine sceWlanDrv_driver_EDD207B1 - Address 0x00003184 
; Imported from sceWlanDrv_driver
sceWlanDrv_driver_EDD207B1:		; Refs: 0x00001E14 
	0x00003184: 0x03E00008 '....' - jr         $ra
	0x00003188: 0x00000000 '....' - nop        
	0x0000318C: 0x00000000 '....' - nop        

; ==== Section .rodata - Address 0x00003190 Size 0x000018A8 Flags 0x0002
           - 00 01 02 03 | 04 05 06 07 | 08 09 0A 0B | 0C 0D 0E 0F - 0123456789ABCDEF
-------------------------------------------------------------------------------------
0x00003190 - 00 00 00 00 | 00 00 00 00 | 00 00 00 80 | 04 02 01 00 - ................
0x000031A0 - BC 34 00 00 | 00 00 00 00 | 00 00 00 00 | 48 33 00 00 - .4..........H3..
0x000031B0 - 11 00 01 00 | 05 00 04 00 | 14 35 00 00 | BC 2F 00 00 - .........5.../..
0x000031C0 - 5C 33 00 00 | 11 00 01 00 | 05 00 0D 00 | 28 35 00 00 - \3..........(5..
0x000031D0 - E4 2F 00 00 | 70 33 00 00 | 11 00 01 00 | 05 00 08 00 - ./..p3..........
0x000031E0 - 5C 35 00 00 | 4C 30 00 00 | 88 33 00 00 | 11 00 01 00 - \5..L0...3......
0x000031F0 - 06 01 09 00 | F0 34 00 00 | 74 2F 00 00 | FC 35 00 00 - .....4..t/...5..
0x00003200 - A0 33 00 00 | 11 00 01 00 | 06 00 0B 00 | 7C 35 00 00 - .3..........|5..
0x00003210 - 8C 30 00 00 | 00 00 00 00 | B8 33 00 00 | 11 00 09 00 - .0.......3......
0x00003220 - 05 00 02 00 | A8 35 00 00 | E4 30 00 00 | CC 33 00 00 - .....5...0...3..
0x00003230 - 11 00 09 00 | 06 00 01 00 | EC 34 00 00 | 6C 2F 00 00 - .........4..l/..
0x00003240 - 00 00 00 00 | EC 33 00 00 | 11 00 09 00 | 05 00 06 00 - .....3..........
0x00003250 - D4 34 00 00 | 3C 2F 00 00 | 00 34 00 00 | 11 00 09 00 - .4..</...4......
0x00003260 - 05 00 01 00 | 24 35 00 00 | DC 2F 00 00 | 18 34 00 00 - ....$5.../...4..
0x00003270 - 11 00 09 00 | 05 00 04 00 | D4 35 00 00 | 3C 31 00 00 - .........5..<1..
0x00003280 - 24 34 00 00 | 11 00 09 00 | 05 00 03 00 | EC 35 00 00 - $4...........5..
0x00003290 - 6C 31 00 00 | 3C 34 00 00 | 11 00 09 00 | 05 00 02 00 - l1..<4..........
0x000032A0 - E4 35 00 00 | 5C 31 00 00 | 54 34 00 00 | 11 00 01 00 - .5..\1..T4......
0x000032B0 - 05 00 01 00 | F8 35 00 00 | 84 31 00 00 | 6C 34 00 00 - .....5...1..l4..
0x000032C0 - 11 00 01 00 | 05 00 01 00 | B4 35 00 00 | FC 30 00 00 - .........5...0..
0x000032D0 - 80 34 00 00 | 11 00 01 00 | 05 00 01 00 | B0 35 00 00 - .4...........5..
0x000032E0 - F4 30 00 00 | 94 34 00 00 | 11 00 01 00 | 05 00 01 00 - .0...4..........
0x000032F0 - B8 35 00 00 | 04 31 00 00 | AC 34 00 00 | 11 00 01 00 - .5...1...4......
0x00003300 - 05 00 06 00 | BC 35 00 00 | 0C 31 00 00 | 00 00 00 00 - .....5...1......
0x00003310 - 07 10 09 01 | 73 63 65 4D | 65 64 69 61 | 53 79 6E 63 - ....sceMediaSync
0x00003320 - 00 00 00 00 | 00 00 00 00 | 00 00 00 00 | 00 00 00 00 - ................
0x00003330 - 40 CA 00 00 | 94 31 00 00 | A4 31 00 00 | AC 31 00 00 - @....1...1...1..
0x00003340 - 0C 33 00 00 | 00 00 00 00 | 4B 44 65 62 | 75 67 46 6F - .3......KDebugFo
0x00003350 - 72 4B 65 72 | 6E 65 6C 00 | 00 00 00 00 | 53 79 73 4D - rKernel.....SysM
0x00003360 - 65 6D 46 6F | 72 4B 65 72 | 6E 65 6C 00 | 00 00 00 00 - emForKernel.....
0x00003370 - 53 79 73 63 | 6C 69 62 46 | 6F 72 4B 65 | 72 6E 65 6C - SysclibForKernel
0x00003380 - 00 00 00 00 | 00 00 00 00 | 49 6F 46 69 | 6C 65 4D 67 - ........IoFileMg
0x00003390 - 72 46 6F 72 | 4B 65 72 6E | 65 6C 00 00 | 00 00 00 00 - rForKernel......
0x000033A0 - 54 68 72 65 | 61 64 4D 61 | 6E 46 6F 72 | 4B 65 72 6E - ThreadManForKern
0x000033B0 - 65 6C 00 00 | 00 00 00 00 | 55 74 69 6C | 73 46 6F 72 - el......UtilsFor
0x000033C0 - 4B 65 72 6E | 65 6C 00 00 | 00 00 00 00 | 49 6E 74 65 - Kernel......Inte
0x000033D0 - 72 72 75 70 | 74 4D 61 6E | 61 67 65 72 | 46 6F 72 4B - rruptManagerForK
0x000033E0 - 65 72 6E 65 | 6C 00 00 00 | 00 00 00 00 | 49 6E 69 74 - ernel.......Init
0x000033F0 - 46 6F 72 4B | 65 72 6E 65 | 6C 00 00 00 | 00 00 00 00 - ForKernel.......
0x00003400 - 4D 6F 64 75 | 6C 65 4D 67 | 72 46 6F 72 | 4B 65 72 6E - ModuleMgrForKern
0x00003410 - 65 6C 00 00 | 00 00 00 00 | 73 63 65 55 | 6D 64 00 00 - el......sceUmd..
0x00003420 - 00 00 00 00 | 73 63 65 55 | 6D 64 43 61 | 63 68 65 5F - ....sceUmdCache_
0x00003430 - 64 72 69 76 | 65 72 00 00 | 00 00 00 00 | 73 63 65 55 - driver......sceU
0x00003440 - 6D 64 39 36 | 36 30 5F 64 | 72 69 76 65 | 72 00 00 00 - md9660_driver...
0x00003450 - 00 00 00 00 | 73 63 65 57 | 6C 61 6E 44 | 72 76 5F 64 - ....sceWlanDrv_d
0x00003460 - 72 69 76 65 | 72 00 00 00 | 00 00 00 00 | 73 63 65 43 - river.......sceC
0x00003470 - 74 72 6C 5F | 64 72 69 76 | 65 72 00 00 | 00 00 00 00 - trl_driver......
0x00003480 - 73 63 65 41 | 75 64 69 6F | 5F 64 72 69 | 76 65 72 00 - sceAudio_driver.
0x00003490 - 00 00 00 00 | 73 63 65 4F | 70 65 6E 50 | 53 49 44 5F - ....sceOpenPSID_
0x000034A0 - 64 72 69 76 | 65 72 00 00 | 00 00 00 00 | 73 63 65 52 - driver......sceR
0x000034B0 - 65 67 5F 64 | 72 69 76 65 | 72 00 00 00 | DB AC 32 D6 - eg_driver.....2.
0x000034C0 - A7 73 1D F0 | 06 75 B9 11 | 00 00 00 00 | 10 33 00 00 - .s...u.......3..
0x000034D0 - 94 46 00 00 | E9 9F 6E 2C | BC B5 33 72 | AD 33 23 7A - .F....n,..3r.3#z
0x000034E0 - 10 A1 33 9D | 8B 4A 8A A1 | 93 1B E7 A6 | 67 B7 26 E5 - ..3..J......g.&.
0x000034F0 - BC 50 9F 10 | B8 27 EB 27 | AC 03 EC 42 | 11 FB F5 54 - .P...'.'...B...T
0x00003500 - 83 8D 63 6A | C3 4B 0C 81 | 9C DF 9D B2 | 4C 00 EB E3 - ..cj.K......L...
0x00003510 - 69 24 09 EB | C5 0A 57 47 | BC 70 F3 84 | CB 0F 01 86 - i$....WG.p......
0x00003520 - DC 27 F4 AC | 1B D1 6D D8 | A1 86 C5 07 | DA 28 EE 13 - .'....m......(..
0x00003530 - AB 65 10 18 | 75 16 D8 23 | 07 8F 14 58 | 7E CE 58 71 - .e..u..#...X~.Xq
0x00003540 - 85 AB C2 87 | AF 76 E7 8A | 80 B4 3C A0 | 6F 6C A2 C1 - .....v....<.ol..
0x00003550 - D0 12 65 DD | 1C 06 29 EF | F7 62 2A F1 | 61 BB F3 10 - ..e...)..b*.a...
0x00003560 - F6 BB C5 3E | 4A D9 6F 47 | 6C 19 DF 52 | 14 52 B3 7A - ...>J.oGl..R.R.z
0x00003570 - F7 D1 D0 81 | 97 76 9A B4 | 32 89 AB C0 | 32 5A B1 1F - .....v..2...2Z..
0x00003580 - 6A 54 8C 32 | 00 0A C2 55 | B5 39 C0 56 | 8C D4 B3 89 - jT.2...U.9.V....
0x00003590 - 08 D7 36 AF | FF E9 36 B7 | 5A D3 66 D3 | 8F AF 1C E8 - ..6...6.Z.f.....
0x000035A0 - 44 58 BA ED | 70 4C 9E EF | 42 5C FB 37 | 58 6A 18 C8 - DX..pL..B\.7Xj..
0x000035B0 - E7 8C 29 FF | 4C 19 86 58 | 16 6C 31 E8 | 65 7D D7 49 - ..).L..X.l1.e}.I
0x000035C0 - 57 14 47 4F | 9F 51 80 99 | 04 67 A4 DB | 96 E3 A3 F4 - W.GO.Q...g......
0x000035D0 - 51 27 74 FC | CE 8F F0 8E | 72 B9 B5 A9 | 31 4C BF B7 - Q't.....r...1L..
0x000035E0 - 47 3D 18 C6 | 54 CE C9 3C | 67 8B 3A FE | 06 0F 6E 57 - G=..T..<g.:...nW
0x000035F0 - 13 82 1D A2 | 32 E4 97 DB | B1 07 D2 ED | 04 36 00 00 - ....2........6..
0x00003600 - C0 5B D7 E4 | 44 05 00 14 | 45 05 00 18 | 00 00 00 00 - .[..D...E.......
0x00003610 - 64 69 73 63 | 00 00 00 00 | 6D 73 00 00 | 66 6C 61 73 - disc....ms..flas
0x00003620 - 68 33 00 00 | 65 66 00 00 | 6D 65 64 69 | 61 73 79 6E - h3..ef..mediasyn
0x00003630 - 63 2E 63 3A | 25 73 3A 62 | 6F 6F 74 61 | 62 6C 65 3D - c.c:%s:bootable=
0x00003640 - 25 64 2C 20 | 6D 6F 64 65 | 3D 25 64 0A | 00 00 00 00 - %d, mode=%d.....
0x00003650 - 5F 73 63 65 | 4D 65 64 69 | 61 53 79 6E | 63 4D 6F 64 - _sceMediaSyncMod
0x00003660 - 75 6C 65 53 | 74 61 72 74 | 00 00 00 00 | 64 69 73 63 - uleStart....disc
0x00003670 - 30 3A 00 00 | 6D 65 64 69 | 61 73 79 6E | 63 2E 63 3A - 0:..mediasync.c:
0x00003680 - 25 73 3A 73 | 63 65 55 6D | 64 41 63 74 | 69 76 61 74 - %s:sceUmdActivat
0x00003690 - 65 20 66 61 | 69 6C 65 64 | 20 30 78 25 | 30 38 78 0A - e failed 0x%08x.
0x000036A0 - 00 00 00 00 | 6D 65 64 69 | 61 73 79 6E | 63 2E 63 3A - ....mediasync.c:
0x000036B0 - 25 73 3A 55 | 4D 44 20 4D | 65 64 69 61 | 20 43 68 65 - %s:UMD Media Che
0x000036C0 - 63 6B 20 4F | 4B 0A 00 00 | 44 69 73 63 | 43 68 65 63 - ck OK...DiscChec
0x000036D0 - 6B 4D 65 64 | 69 61 00 00 | 6D 65 64 69 | 61 73 79 6E - kMedia..mediasyn
0x000036E0 - 63 2E 63 3A | 25 73 3A 53 | 43 45 5F 4D | 45 44 49 41 - c.c:%s:SCE_MEDIA
0x000036F0 - 53 59 4E 43 | 5F 45 52 52 | 4F 52 5F 49 | 4E 56 41 4C - SYNC_ERROR_INVAL
0x00003700 - 49 44 5F 4D | 45 44 49 41 | 0A 00 00 00 | 6D 65 64 69 - ID_MEDIA....medi
0x00003710 - 61 73 79 6E | 63 2E 63 3A | 25 73 3A 5B | 25 31 36 73 - async.c:%s:[%16s
0x00003720 - 5D 20 61 6E | 64 20 5B 25 | 31 36 73 5D | 0A 00 00 00 - ] and [%16s]....
0x00003730 - 6D 65 64 69 | 61 73 79 6E | 63 2E 63 3A | 25 73 3A 44 - mediasync.c:%s:D
0x00003740 - 69 73 63 43 | 68 65 63 6B | 4D 65 64 69 | 61 20 66 61 - iscCheckMedia fa
0x00003750 - 69 6C 65 64 | 20 30 78 25 | 30 38 78 0A | 00 00 00 00 - iled 0x%08x.....
0x00003760 - 57 61 69 74 | 44 69 73 63 | 00 00 00 00 | 6D 65 64 69 - WaitDisc....medi
0x00003770 - 61 73 79 6E | 63 2E 63 3A | 25 73 3A 73 | 63 65 55 6D - async.c:%s:sceUm
0x00003780 - 64 47 65 74 | 44 72 69 76 | 65 53 74 61 | 74 75 73 3A - dGetDriveStatus:
0x00003790 - 30 78 25 30 | 38 78 0A 00 | 6D 65 64 69 | 61 73 79 6E - 0x%08x..mediasyn
0x000037A0 - 63 2E 63 3A | 25 73 3A 73 | 63 65 55 6D | 64 57 61 69 - c.c:%s:sceUmdWai
0x000037B0 - 74 44 72 69 | 76 65 53 74 | 61 74 20 66 | 61 69 6C 65 - tDriveStat faile
0x000037C0 - 64 20 30 78 | 25 30 38 78 | 0A 00 00 00 | 53 63 65 4D - d 0x%08x....SceM
0x000037D0 - 65 64 69 61 | 53 79 6E 63 | 45 76 4D 73 | 00 00 00 00 - ediaSyncEvMs....
0x000037E0 - 53 63 65 4D | 65 64 69 61 | 53 79 6E 63 | 4D 73 00 00 - SceMediaSyncMs..
0x000037F0 - 66 61 74 6D | 73 30 3A 00 | 6D 65 64 69 | 61 73 79 6E - fatms0:.mediasyn
0x00003800 - 63 2E 63 3A | 25 73 3A 73 | 63 65 46 61 | 74 6D 73 55 - c.c:%s:sceFatmsU
0x00003810 - 6E 52 65 67 | 69 73 74 65 | 72 4E 6F 74 | 69 66 79 43 - nRegisterNotifyC
0x00003820 - 61 6C 6C 62 | 61 63 6B 20 | 66 61 69 6C | 65 64 20 30 - allback failed 0
0x00003830 - 78 25 30 38 | 78 0A 00 00 | 6D 65 64 69 | 61 73 79 6E - x%08x...mediasyn
0x00003840 - 63 2E 63 3A | 25 73 3A 73 | 63 65 4B 65 | 72 6E 65 6C - c.c:%s:sceKernel
0x00003850 - 44 65 6C 65 | 74 65 43 61 | 6C 6C 62 61 | 63 6B 20 66 - DeleteCallback f
0x00003860 - 61 69 6C 65 | 64 20 30 78 | 25 30 38 78 | 0A 00 00 00 - ailed 0x%08x....
0x00003870 - 6D 65 64 69 | 61 73 79 6E | 63 2E 63 3A | 25 73 3A 73 - mediasync.c:%s:s
0x00003880 - 63 65 4B 65 | 72 6E 65 6C | 44 65 6C 65 | 74 65 45 76 - ceKernelDeleteEv
0x00003890 - 65 6E 74 46 | 6C 61 67 20 | 66 61 69 6C | 65 64 20 30 - entFlag failed 0
0x000038A0 - 78 25 30 38 | 78 0A 00 00 | 6D 65 64 69 | 61 73 79 6E - x%08x...mediasyn
0x000038B0 - 63 2E 63 3A | 25 73 3A 57 | 61 72 6E 69 | 6E 67 3A 20 - c.c:%s:Warning: 
0x000038C0 - 4D 73 43 68 | 65 63 6B 4D | 65 64 69 61 | 46 61 69 6C - MsCheckMediaFail
0x000038D0 - 65 64 20 30 | 78 25 30 38 | 78 0A 00 00 | 57 61 69 74 - ed 0x%08x...Wait
0x000038E0 - 4D 73 00 00 | 6D 65 64 69 | 61 73 79 6E | 63 2E 63 3A - Ms..mediasync.c:
0x000038F0 - 25 73 3A 4D | 73 43 68 65 | 63 6B 4D 65 | 64 69 61 46 - %s:MsCheckMediaF
0x00003900 - 61 69 6C 65 | 64 20 30 78 | 25 30 38 78 | 0A 00 00 00 - ailed 0x%08x....
0x00003910 - 6D 65 64 69 | 61 73 79 6E | 63 2E 63 3A | 25 73 3A 75 - mediasync.c:%s:u
0x00003920 - 6E 73 75 70 | 70 6F 72 74 | 65 64 20 61 | 70 69 74 79 - nsupported apity
0x00003930 - 70 65 3D 30 | 78 25 30 38 | 78 5B 25 64 | 5D 0A 00 00 - pe=0x%08x[%d]...
0x00003940 - 6D 65 64 69 | 61 73 79 6E | 63 2E 63 3A | 25 73 3A 73 - mediasync.c:%s:s
0x00003950 - 63 65 46 61 | 74 6D 73 52 | 65 67 69 73 | 74 65 72 4E - ceFatmsRegisterN
0x00003960 - 6F 74 69 66 | 79 43 61 6C | 6C 62 61 63 | 6B 20 66 61 - otifyCallback fa
0x00003970 - 69 6C 65 64 | 20 30 78 25 | 30 38 78 0A | 00 00 00 00 - iled 0x%08x.....
0x00003980 - 6D 65 64 69 | 61 73 79 6E | 63 2E 63 3A | 25 73 3A 73 - mediasync.c:%s:s
0x00003990 - 63 65 4B 65 | 72 6E 65 6C | 43 72 65 61 | 74 65 45 76 - ceKernelCreateEv
0x000039A0 - 65 6E 74 46 | 6C 61 67 20 | 66 61 69 6C | 65 64 20 30 - entFlag failed 0
0x000039B0 - 78 25 30 38 | 78 0A 00 00 | 53 63 65 4D | 64 69 61 53 - x%08x...SceMdiaS
0x000039C0 - 79 6E 63 3A | 77 6F 72 6B | 32 00 00 00 | 53 63 65 4D - ync:work2...SceM
0x000039D0 - 64 69 61 53 | 79 6E 63 3A | 77 6F 72 6B | 31 00 00 00 - diaSync:work1...
0x000039E0 - 6D 65 64 69 | 61 73 79 6E | 63 2E 63 3A | 25 73 3A 54 - mediasync.c:%s:T
0x000039F0 - 68 69 73 20 | 69 73 20 6E | 6F 74 20 50 | 42 50 0A 00 - his is not PBP..
0x00003A00 - 4D 73 43 68 | 65 63 6B 4D | 65 64 69 61 | 00 00 00 00 - MsCheckMedia....
0x00003A10 - 6D 65 64 69 | 61 73 79 6E | 63 2E 63 3A | 25 73 3A 73 - mediasync.c:%s:s
0x00003A20 - 63 65 4B 65 | 72 6E 65 6C | 46 72 65 65 | 50 61 72 74 - ceKernelFreePart
0x00003A30 - 69 74 69 6F | 6E 4D 65 6D | 6F 72 79 20 | 66 61 69 6C - itionMemory fail
0x00003A40 - 65 64 20 30 | 78 25 30 38 | 78 0A 00 00 | 6D 65 64 69 - ed 0x%08x...medi
0x00003A50 - 61 73 79 6E | 63 2E 63 3A | 25 73 3A 73 | 63 65 4B 65 - async.c:%s:sceKe
0x00003A60 - 72 6E 65 6C | 41 6C 6C 6F | 63 50 61 72 | 74 69 74 69 - rnelAllocPartiti
0x00003A70 - 6F 6E 4D 65 | 6D 6F 72 79 | 20 66 61 69 | 6C 65 64 20 - onMemory failed 
0x00003A80 - 25 73 3A 30 | 78 25 30 38 | 78 3A 73 69 | 7A 65 20 30 - %s:0x%08x:size 0
0x00003A90 - 78 25 78 0A | 00 00 00 00 | 6D 65 64 69 | 61 73 79 6E - x%x.....mediasyn
0x00003AA0 - 63 2E 63 3A | 25 73 3A 73 | 63 65 49 6F | 4C 73 65 65 - c.c:%s:sceIoLsee
0x00003AB0 - 6B 20 66 61 | 69 6C 65 64 | 2C 20 30 78 | 25 78 0A 00 - k failed, 0x%x..
0x00003AC0 - 6D 65 64 69 | 61 73 79 6E | 63 2E 63 3A | 25 73 3A 73 - mediasync.c:%s:s
0x00003AD0 - 63 65 4B 65 | 72 6E 65 6C | 46 72 65 65 | 50 61 72 74 - ceKernelFreePart
0x00003AE0 - 69 74 69 6F | 6E 4D 65 6D | 6F 72 79 20 | 66 61 69 6C - itionMemory fail
0x00003AF0 - 65 64 20 25 | 73 3A 30 78 | 25 30 38 78 | 0A 00 00 00 - ed %s:0x%08x....
0x00003B00 - 6D 65 64 69 | 61 73 79 6E | 63 2E 63 3A | 25 73 3A 72 - mediasync.c:%s:r
0x00003B10 - 65 61 64 20 | 68 65 61 64 | 65 72 20 66 | 61 69 6C 65 - ead header faile
0x00003B20 - 64 20 30 78 | 25 30 38 78 | 0A 00 00 00 | 6D 65 64 69 - d 0x%08x....medi
0x00003B30 - 61 73 79 6E | 63 2E 63 3A | 25 73 3A 73 | 63 65 49 6F - async.c:%s:sceIo
0x00003B40 - 4F 70 65 6E | 20 66 61 69 | 6C 65 64 20 | 5B 25 73 5D - Open failed [%s]
0x00003B50 - 20 30 78 25 | 30 38 78 0A | 00 00 00 00 | 6D 65 64 69 -  0x%08x.....medi
0x00003B60 - 61 73 79 6E | 63 2E 63 3A | 25 73 3A 73 | 63 65 4B 65 - async.c:%s:sceKe
0x00003B70 - 72 6E 65 6C | 41 6C 6C 6F | 63 50 61 72 | 74 69 74 69 - rnelAllocPartiti
0x00003B80 - 6F 6E 4D 65 | 6D 6F 72 79 | 20 66 61 69 | 6C 65 64 20 - onMemory failed 
0x00003B90 - 25 73 3A 30 | 78 25 30 38 | 78 0A 00 00 | 53 63 65 4D - %s:0x%08x...SceM
0x00003BA0 - 65 64 69 61 | 53 79 6E 63 | 45 76 45 66 | 00 00 00 00 - ediaSyncEvEf....
0x00003BB0 - 53 63 65 4D | 65 64 69 61 | 53 79 6E 63 | 45 66 00 00 - SceMediaSyncEf..
0x00003BC0 - 66 61 74 65 | 66 30 3A 00 | 6D 65 64 69 | 61 73 79 6E - fatef0:.mediasyn
0x00003BD0 - 63 2E 63 3A | 25 73 3A 73 | 63 65 46 61 | 74 65 66 55 - c.c:%s:sceFatefU
0x00003BE0 - 6E 52 65 67 | 69 73 74 65 | 72 4E 6F 74 | 69 66 79 43 - nRegisterNotifyC
0x00003BF0 - 61 6C 6C 62 | 61 63 6B 20 | 66 61 69 6C | 65 64 20 30 - allback failed 0
0x00003C00 - 78 25 30 38 | 78 0A 00 00 | 57 61 69 74 | 45 66 6C 61 - x%08x...WaitEfla
0x00003C10 - 73 68 00 00 | 6D 65 64 69 | 61 73 79 6E | 63 2E 63 3A - sh..mediasync.c:
0x00003C20 - 25 73 3A 73 | 63 65 46 61 | 74 65 66 52 | 65 67 69 73 - %s:sceFatefRegis
0x00003C30 - 74 65 72 4E | 6F 74 69 66 | 79 43 61 6C | 6C 62 61 63 - terNotifyCallbac
0x00003C40 - 6B 20 66 61 | 69 6C 65 64 | 20 30 78 25 | 30 38 78 0A - k failed 0x%08x.
0x00003C50 - 00 00 00 00 | 6D 65 64 69 | 61 73 79 6E | 63 2E 63 3A - ....mediasync.c:
0x00003C60 - 25 73 3A 4E | 6F 74 20 52 | 45 41 44 41 | 42 4C 45 20 - %s:Not READABLE 
0x00003C70 - 3A 20 30 78 | 25 30 38 78 | 0A 00 00 00 | 57 61 69 74 - : 0x%08x....Wait
0x00003C80 - 44 69 73 63 | 45 6D 75 00 | 6D 65 64 69 | 61 73 79 6E - DiscEmu.mediasyn
0x00003C90 - 63 2E 63 3A | 25 73 3A 4D | 73 43 68 65 | 63 6B 4D 65 - c.c:%s:MsCheckMe
0x00003CA0 - 64 69 61 20 | 66 61 69 6C | 65 64 20 30 | 78 25 30 38 - dia failed 0x%08
0x00003CB0 - 78 0A 00 00 | 6D 65 64 69 | 61 73 79 6E | 63 2E 63 3A - x...mediasync.c:
0x00003CC0 - 25 73 3A 43 | 61 6E 6E 6F | 74 20 77 61 | 69 74 20 55 - %s:Cannot wait U
0x00003CD0 - 4D 44 20 65 | 76 65 6E 74 | 20 3A 20 30 | 78 25 30 38 - MD event : 0x%08
0x00003CE0 - 78 0A 00 00 | 6D 65 64 69 | 61 73 79 6E | 63 2E 63 3A - x...mediasync.c:
0x00003CF0 - 25 73 3A 43 | 61 6E 6E 6F | 74 20 61 63 | 74 69 76 61 - %s:Cannot activa
0x00003D00 - 74 65 20 55 | 4D 44 20 3A | 20 30 78 25 | 30 38 78 0A - te UMD : 0x%08x.
0x00003D10 - 00 00 00 00 | 4D 73 43 61 | 6C 6C 62 61 | 63 6B 00 00 - ....MsCallback..
0x00003D20 - 6D 65 64 69 | 61 73 79 6E | 63 2E 63 3A | 25 73 3A 45 - mediasync.c:%s:E
0x00003D30 - 56 45 4E 54 | 2C 20 61 72 | 67 3D 30 78 | 25 30 38 78 - VENT, arg=0x%08x
0x00003D40 - 0A 00 00 00 | 44 49 53 43 | 5F 49 44 00 | 6D 65 64 69 - ....DISC_ID.medi
0x00003D50 - 61 73 79 6E | 63 2E 63 3A | 25 73 3A 73 | 63 65 53 79 - async.c:%s:sceSy
0x00003D60 - 73 74 65 6D | 46 69 6C 65 | 47 65 74 49 | 6E 64 65 78 - stemFileGetIndex
0x00003D70 - 20 66 61 69 | 6C 65 64 20 | 28 72 65 73 | 3D 30 78 25 -  failed (res=0x%
0x00003D80 - 78 29 0A 00 | 55 4C 4A 53 | 30 30 31 36 | 37 00 00 00 - x)..ULJS00167...
0x00003D90 - 6D 65 64 69 | 61 73 79 6E | 63 2E 63 3A | 25 73 3A 73 - mediasync.c:%s:s
0x00003DA0 - 63 65 4B 65 | 72 6E 65 6C | 44 65 6C 65 | 74 65 48 65 - ceKernelDeleteHe
0x00003DB0 - 61 70 20 66 | 61 69 6C 65 | 64 20 30 78 | 25 30 38 78 - ap failed 0x%08x
0x00003DC0 - 0A 00 00 00 | 4D 73 53 79 | 73 74 65 6D | 46 69 6C 65 - ....MsSystemFile
0x00003DD0 - 00 00 00 00 | 55 4C 4A 53 | 30 30 31 36 | 38 00 00 00 - ....ULJS00168...
0x00003DE0 - 55 4C 4A 53 | 30 30 31 36 | 39 00 00 00 | 6D 65 64 69 - ULJS00169...medi
0x00003DF0 - 61 73 79 6E | 63 2E 63 3A | 25 73 3A 66 | 61 69 6C 65 - async.c:%s:faile
0x00003E00 - 64 20 74 6F | 20 67 65 74 | 20 76 61 6C | 75 65 20 6F - d to get value o
0x00003E10 - 66 20 27 44 | 49 53 43 5F | 49 44 27 20 | 28 72 65 74 - f 'DISC_ID' (ret
0x00003E20 - 3D 30 78 25 | 78 29 0A 00 | 6D 65 64 69 | 61 73 79 6E - =0x%x)..mediasyn
0x00003E30 - 63 2E 63 3A | 25 73 3A 73 | 63 65 53 79 | 73 74 65 6D - c.c:%s:sceSystem
0x00003E40 - 46 69 6C 65 | 4C 6F 61 64 | 41 6C 6C 32 | 20 66 61 69 - FileLoadAll2 fai
0x00003E50 - 6C 65 64 20 | 30 78 25 30 | 38 78 2C 20 | 62 75 66 3D - led 0x%08x, buf=
0x00003E60 - 30 78 25 30 | 38 78 2C 20 | 73 69 7A 65 | 3D 30 78 25 - 0x%08x, size=0x%
0x00003E70 - 30 38 78 0A | 00 00 00 00 | 53 63 65 4B | 65 72 6E 65 - 08x.....SceKerne
0x00003E80 - 6C 4D 65 64 | 69 61 53 79 | 6E 63 48 65 | 61 70 00 00 - lMediaSyncHeap..
0x00003E90 - 73 66 5F 6D | 61 6C 6C 6F | 63 00 00 00 | 6D 65 64 69 - sf_malloc...medi
0x00003EA0 - 61 73 79 6E | 63 2E 63 3A | 25 73 3A 73 | 63 65 4B 65 - async.c:%s:sceKe
0x00003EB0 - 72 6E 65 6C | 43 72 65 61 | 74 65 48 65 | 61 70 20 66 - rnelCreateHeap f
0x00003EC0 - 61 69 6C 65 | 64 20 30 78 | 25 30 38 78 | 0A 00 00 00 - ailed 0x%08x....
0x00003ED0 - 55 4C 4A 53 | 30 30 30 32 | 32 00 00 00 | 55 4C 4A 4D - ULJS00022...ULJM
0x00003EE0 - 30 35 30 37 | 31 00 00 00 | 55 4C 4A 4D | 30 35 33 35 - 05071...ULJM0535
0x00003EF0 - 32 00 00 00 | 55 4C 4A 53 | 30 30 31 35 | 34 00 00 00 - 2...ULJS00154...
0x00003F00 - 55 4C 4A 4D | 30 35 30 35 | 38 00 00 00 | 55 4C 4A 4D - ULJM05058...ULJM
0x00003F10 - 30 35 31 38 | 39 00 00 00 | 55 4C 4A 4D | 30 35 32 30 - 05189...ULJM0520
0x00003F20 - 31 00 00 00 | 55 4C 4A 53 | 30 30 31 37 | 30 00 00 00 - 1...ULJS00170...
0x00003F30 - 55 4C 4A 4D | 30 35 31 36 | 34 00 00 00 | 55 4C 4A 4D - ULJM05164...ULJM
0x00003F40 - 30 35 31 35 | 36 00 00 00 | 55 4C 4A 4D | 30 35 35 30 - 05156...ULJM0550
0x00003F50 - 30 00 00 00 | 55 4C 4A 4D | 30 35 31 33 | 35 00 00 00 - 0...ULJM05135...
0x00003F60 - 55 43 4B 53 | 34 35 31 32 | 34 00 00 00 | 55 43 4A 53 - UCKS45124...UCJS
0x00003F70 - 31 30 31 30 | 34 00 00 00 | 55 43 55 53 | 39 38 37 34 - 10104...UCUS9874
0x00003F80 - 33 00 00 00 | 55 43 45 53 | 30 31 32 35 | 30 00 00 00 - 3...UCES01250...
0x00003F90 - 55 43 41 53 | 34 30 32 36 | 36 00 00 00 | 55 4C 4A 53 - UCAS40266...ULJS
0x00003FA0 - 30 30 32 32 | 34 00 00 00 | 4E 50 4A 48 | 35 30 31 38 - 00224...NPJH5018
0x00003FB0 - 34 00 00 00 | 55 4C 55 53 | 31 30 34 36 | 36 00 00 00 - 4...ULUS10466...
0x00003FC0 - 55 4C 45 53 | 30 31 33 37 | 36 00 00 00 | 55 4C 41 53 - ULES01376...ULAS
0x00003FD0 - 34 32 32 31 | 34 00 00 00 | 55 4C 4B 53 | 34 36 32 33 - 42214...ULKS4623
0x00003FE0 - 35 00 00 00 | 55 4C 4A 53 | 30 30 32 30 | 32 00 00 00 - 5...ULJS00202...
0x00003FF0 - 55 4C 55 53 | 31 30 34 35 | 37 00 00 00 | 55 4C 45 53 - ULUS10457...ULES
0x00004000 - 30 31 32 39 | 38 00 00 00 | 55 43 4B 53 | 34 35 31 32 - 01298...UCKS4512
0x00004010 - 36 00 00 00 | 55 4C 4A 53 | 30 30 32 31 | 37 00 00 00 - 6...ULJS00217...
0x00004020 - 55 4C 4A 4D | 30 35 35 31 | 39 00 00 00 | 55 4C 4A 4D - ULJM05519...ULJM
0x00004030 - 30 35 34 38 | 39 00 00 00 | 4E 50 4A 48 | 35 30 30 34 - 05489...NPJH5004
0x00004040 - 30 00 00 00 | 55 43 4B 53 | 34 35 31 34 | 30 00 00 00 - 0...UCKS45140...
0x00004050 - 55 4C 55 53 | 31 30 35 31 | 32 00 00 00 | 55 4C 4A 53 - ULUS10512...ULJS
0x00004060 - 30 30 32 33 | 38 00 00 00 | 55 4C 4A 53 | 30 30 32 33 - 00238...ULJS0023
0x00004070 - 36 00 00 00 | 55 4C 55 53 | 31 30 35 31 | 38 00 00 00 - 6...ULUS10518...
0x00004080 - 55 4C 41 53 | 34 32 32 33 | 31 00 00 00 | 55 4C 45 53 - ULAS42231...ULES
0x00004090 - 30 31 34 30 | 37 00 00 00 | 55 4C 4A 4D | 30 35 35 34 - 01407...ULJM0554
0x000040A0 - 32 00 00 00 | 55 4C 4A 4D | 30 35 35 34 | 34 00 00 00 - 2...ULJM05544...
0x000040B0 - 55 4C 4A 53 | 30 30 30 31 | 39 00 00 00 | 55 4C 4A 53 - ULJS00019...ULJS
0x000040C0 - 30 30 31 36 | 37 00 00 00 | 55 4C 4A 53 | 30 30 31 36 - 00167...ULJS0016
0x000040D0 - 38 00 00 00 | 55 4C 4A 53 | 30 30 31 36 | 39 00 00 00 - 8...ULJS00169...
0x000040E0 - 73 63 65 55 | 6D 64 43 61 | 63 68 65 5F | 64 72 69 76 - sceUmdCache_driv
0x000040F0 - 65 72 00 00 | 55 4C 4A 4D | 30 35 31 30 | 39 00 00 00 - er..ULJM05109...
0x00004100 - 66 61 74 6D | 73 30 3A 00 | 66 61 74 65 | 66 30 3A 00 - fatms0:.fatef0:.
0x00004110 - 2F 43 4F 4E | 46 49 47 2F | 4E 50 00 00 | 65 6E 76 00 - /CONFIG/NP..env.
0x00004120 - 69 6E 76 61 | 6C 69 64 00 | 2F 43 4F 4E | 46 49 47 2F - invalid./CONFIG/
0x00004130 - 4E 45 54 57 | 4F 52 4B 2F | 41 44 48 4F | 43 00 00 00 - NETWORK/ADHOC...
0x00004140 - 73 73 69 64 | 5F 70 72 65 | 66 69 78 00 | 00 50 53 46 - ssid_prefix..PSF
0x00004150 - 00 00 00 00 | 7C 79 72 3D | 60 63 60 30 | 7E 7B 74 85 - ....|yr=`c`0~{t.
0x00004160 - 8B 85 78 7B | 7E 77 32 32 | 42 42 42 42 | 00 50 53 46 - ..x{~w22BBBB.PSF
0x00004170 - 00 00 00 00 | 34 00 00 00 | 33 32 00 00 | 31 36 00 00 - ....4...32..16..
0x00004180 - 38 00 00 00 | 73 63 65 4C | 69 62 53 79 | 73 66 69 6C - 8...sceLibSysfil
0x00004190 - 65 00 00 00 | 00 50 53 46 | 00 00 00 00 | 4E 50 4A 48 - e....PSF....NPJH
0x000041A0 - 39 30 30 33 | 38 00 00 00 | 6C 00 00 00 | 6C 00 00 00 - 90038...l...l...
0x000041B0 - 6C 02 00 00 | 6C 02 00 00 | 94 02 00 00 | 94 02 00 00 - l...l...........
0x000041C0 - D8 01 00 00 | E0 01 00 00 | E8 01 00 00 | F0 01 00 00 - ................
0x000041D0 - 8C 00 00 00 | 8C 00 00 00 | 8C 00 00 00 | 8C 00 00 00 - ................
0x000041E0 - 8C 00 00 00 | 8C 00 00 00 | 6C 00 00 00 | 6C 00 00 00 - ........l...l...
0x000041F0 - 6C 00 00 00 | 6C 02 00 00 | 8C 00 00 00 | 94 02 00 00 - l...l...........
0x00004200 - 8C 00 00 00 | 8C 00 00 00 | 8C 00 00 00 | 8C 00 00 00 - ................
0x00004210 - 8C 00 00 00 | 8C 00 00 00 | 8C 00 00 00 | 8C 00 00 00 - ................
0x00004220 - 8C 00 00 00 | 8C 00 00 00 | 8C 00 00 00 | 8C 00 00 00 - ................
0x00004230 - 8C 00 00 00 | 8C 00 00 00 | 8C 00 00 00 | 8C 00 00 00 - ................
0x00004240 - 8C 00 00 00 | 8C 00 00 00 | 8C 00 00 00 | 8C 00 00 00 - ................
0x00004250 - 8C 00 00 00 | 8C 00 00 00 | 8C 00 00 00 | 8C 00 00 00 - ................
0x00004260 - 8C 00 00 00 | 8C 00 00 00 | F8 01 00 00 | F8 01 00 00 - ................
0x00004270 - 8C 00 00 00 | F8 01 00 00 | F8 01 00 00 | F8 01 00 00 - ................
0x00004280 - 8C 00 00 00 | 8C 00 00 00 | 8C 00 00 00 | 8C 00 00 00 - ................
0x00004290 - 8C 00 00 00 | 8C 00 00 00 | 8C 00 00 00 | 8C 00 00 00 - ................
0x000042A0 - 8C 00 00 00 | 8C 00 00 00 | 8C 00 00 00 | 3C 02 00 00 - ............<...
0x000042B0 - 3C 02 00 00 | 8C 00 00 00 | 3C 02 00 00 | 3C 02 00 00 - <.......<...<...
0x000042C0 - 3C 02 00 00 | 8C 00 00 00 | 8C 00 00 00 | 8C 00 00 00 - <...............
0x000042D0 - 8C 00 00 00 | 8C 00 00 00 | 8C 00 00 00 | 8C 00 00 00 - ................
0x000042E0 - 8C 00 00 00 | 8C 00 00 00 | D8 01 00 00 | 8C 00 00 00 - ................
0x000042F0 - 8C 00 00 00 | 8C 00 00 00 | 8C 00 00 00 | 8C 00 00 00 - ................
0x00004300 - 8C 00 00 00 | 8C 00 00 00 | 8C 00 00 00 | 8C 00 00 00 - ................
0x00004310 - 8C 00 00 00 | 8C 00 00 00 | 8C 00 00 00 | 8C 00 00 00 - ................
0x00004320 - 8C 00 00 00 | 8C 00 00 00 | 6C 02 00 00 | 94 02 00 00 - ........l.......
0x00004330 - 54 06 00 00 | 54 06 00 00 | 8C 06 00 00 | 8C 06 00 00 - T...T...........
0x00004340 - 54 06 00 00 | 54 06 00 00 | 8C 06 00 00 | 8C 06 00 00 - T...T...........
0x00004350 - 8C 06 00 00 | 8C 06 00 00 | 8C 06 00 00 | 8C 06 00 00 - ................
0x00004360 - 8C 06 00 00 | 8C 06 00 00 | 8C 06 00 00 | 8C 06 00 00 - ................
0x00004370 - 8C 06 00 00 | 54 06 00 00 | 8C 06 00 00 | 8C 06 00 00 - ....T...........
0x00004380 - 8C 06 00 00 | 8C 06 00 00 | 8C 06 00 00 | 8C 06 00 00 - ................
0x00004390 - 8C 06 00 00 | 8C 06 00 00 | 8C 06 00 00 | 8C 06 00 00 - ................
0x000043A0 - 8C 06 00 00 | 8C 06 00 00 | 8C 06 00 00 | 8C 06 00 00 - ................
0x000043B0 - 8C 06 00 00 | 8C 06 00 00 | 8C 06 00 00 | 8C 06 00 00 - ................
0x000043C0 - 8C 06 00 00 | 8C 06 00 00 | 8C 06 00 00 | 8C 06 00 00 - ................
0x000043D0 - 8C 06 00 00 | 8C 06 00 00 | 8C 06 00 00 | 8C 06 00 00 - ................
0x000043E0 - 8C 06 00 00 | 8C 06 00 00 | 08 06 00 00 | 5C 06 00 00 - ............\...
0x000043F0 - 8C 06 00 00 | 5C 06 00 00 | 5C 06 00 00 | 5C 06 00 00 - ....\...\...\...
0x00004400 - 8C 06 00 00 | 8C 06 00 00 | 8C 06 00 00 | 8C 06 00 00 - ................
0x00004410 - 8C 06 00 00 | 8C 06 00 00 | 8C 06 00 00 | 8C 06 00 00 - ................
0x00004420 - 8C 06 00 00 | 8C 06 00 00 | 8C 06 00 00 | 8C 06 00 00 - ................
0x00004430 - 8C 06 00 00 | 8C 06 00 00 | 8C 06 00 00 | 8C 06 00 00 - ................
0x00004440 - 8C 06 00 00 | 8C 06 00 00 | 8C 06 00 00 | 8C 06 00 00 - ................
0x00004450 - 8C 06 00 00 | 8C 06 00 00 | 8C 06 00 00 | 8C 06 00 00 - ................
0x00004460 - 8C 06 00 00 | 8C 06 00 00 | 5C 06 00 00 | 8C 06 00 00 - ........\.......
0x00004470 - 8C 06 00 00 | 8C 06 00 00 | 8C 06 00 00 | 8C 06 00 00 - ................
0x00004480 - 8C 06 00 00 | 8C 06 00 00 | 8C 06 00 00 | 8C 06 00 00 - ................
0x00004490 - 8C 06 00 00 | 8C 06 00 00 | 8C 06 00 00 | 8C 06 00 00 - ................
0x000044A0 - 8C 06 00 00 | 8C 06 00 00 | 54 06 00 00 | 24 0C 00 00 - ........T...$...
0x000044B0 - 24 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 | 24 0C 00 00 - $...\...\...$...
0x000044C0 - 24 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 - $...\...\...\...
0x000044D0 - 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 - \...\...\...\...
0x000044E0 - 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 - \...\...\...\...
0x000044F0 - 24 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 - $...\...\...\...
0x00004500 - 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 - \...\...\...\...
0x00004510 - 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 - \...\...\...\...
0x00004520 - 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 - \...\...\...\...
0x00004530 - 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 - \...\...\...\...
0x00004540 - 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 - \...\...\...\...
0x00004550 - 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 - \...\...\...\...
0x00004560 - 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 - \...\...\...\...
0x00004570 - 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 - \...\...\...\...
0x00004580 - 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 - \...\...\...\...
0x00004590 - 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 - \...\...\...\...
0x000045A0 - D8 0B 00 00 | 2C 0C 00 00 | 5C 0C 00 00 | 2C 0C 00 00 - ....,...\...,...
0x000045B0 - 2C 0C 00 00 | 2C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 - ,...,...\...\...
0x000045C0 - 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 - \...\...\...\...
0x000045D0 - 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 - \...\...\...\...
0x000045E0 - 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 - \...\...\...\...
0x000045F0 - 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 - \...\...\...\...
0x00004600 - 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 - \...\...\...\...
0x00004610 - 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 | 5C 0C 00 00 - \...\...\...\...
0x00004620 - 24 0C 00 00 | F8 5C 78 FF | 05 43 13 8C | 16 2E 70 D6 - $....\x..C....p.
0x00004630 - 45 87 9B B6 | 00 39 77 BF | 22 0C C6 05 | 59 97 A4 AD - E....9w."...Y...
0x00004640 - E4 93 5D D8 | 85 57 0A 3B | F6 61 64 E1 | EC 72 C6 FE - ..]..W.;.ad..r..
0x00004650 - B1 A3 2B 79 | DB 76 34 A4 | F2 EE 54 6E | B7 C6 8D 6C - ..+y.v4...Tn...l
0x00004660 - 13 19 C9 8B | 88 F9 BA E7 | E6 34 36 01 | A8 1D CB 95 - .........46.....
0x00004670 - F3 E3 26 16 | F7 35 82 B4 | 2E 98 C7 0F | 37 D7 F2 BB - ..&..5......7...
0x00004680 - 50 3F B0 28 | E7 E0 13 77 | 4D 9A 5E D5 | 73 F1 28 DD - P?.(...wM.^.s.(.
0x00004690 - C2 34 19 38 | 10 00 06 06 | 00 00 84 00 | 24 60 07 00 - .4.8........$`..
0x000046A0 - 00 00 94 00 | 00 46 07 00 | 00 00 C4 00 | 01 12 07 00 - .....F..........
0x000046B0 - 00 00 94 00 | 00 41 07 00 | 00 00 C4 00 | 01 20 04 00 - .....A....... ..
0x000046C0 - 00 00 C4 00 | 01 12 06 00 | 00 00 A4 00 | 00 11 03 00 - ................
0x000046D0 - 00 00 94 00 | 00 81 09 00 | 00 00 94 00 | 00 91 00 00 - ................
0x000046E0 - 00 00 94 00 | 00 91 01 00 | 00 00 B4 00 | 64 30 07 00 - ............d0..
0x000046F0 - 00 00 C4 00 | 01 40 05 00 | 00 00 84 00 | 24 90 09 00 - .....@......$...
0x00004700 - 00 00 C4 00 | 01 81 04 00 | 00 00 B4 00 | 64 31 09 00 - ............d1..
0x00004710 - 00 00 C4 00 | 01 03 07 00 | 00 00 94 00 | 00 09 05 00 - ................
0x00004720 - 00 00 94 00 | 00 09 06 00 | 00 00 C4 00 | 01 70 00 00 - .............p..
0x00004730 - 00 00 94 00 | 00 82 01 00 | 00 00 A4 00 | 00 60 00 00 - .............`..
0x00004740 - 00 00 A4 00 | 00 11 04 00 | 00 00 24 00 | 50 81 08 00 - ..........$.P...
0x00004750 - 00 00 24 00 | 50 70 00 00 | 00 00 80 00 | 04 41 07 00 - ..$.Pp.......A..
0x00004760 - 00 00 C0 00 | 89 66 07 00 | 00 00 C4 00 | 01 51 08 00 - .....f.......Q..
0x00004770 - 00 00 C4 00 | 01 20 00 00 | 00 00 A0 00 | 01 10 08 00 - ..... ..........
0x00004780 - 00 00 94 00 | 00 31 05 00 | 00 00 80 00 | 04 20 05 00 - .....1....... ..
0x00004790 - 00 00 A4 00 | 00 50 09 00 | 00 00 C4 00 | 01 32 05 00 - .....P.......2..
0x000047A0 - 00 00 94 00 | 00 47 00 00 | 01 00 24 00 | 50 60 03 00 - .....G....$.P`..
0x000047B0 - 02 00 C4 00 | 01 40 02 00 | 03 00 A0 00 | 01 60 01 00 - .....@.......`..
0x000047C0 - 04 00 A4 00 | 00 40 02 00 | 05 00 24 00 | 50 80 07 00 - .....@....$.P...
0x000047D0 - 06 00 B4 00 | 64 10 05 00 | 07 00 A0 00 | 01 30 06 00 - ....d........0..
0x000047E0 - 08 00 90 00 | 00 51 02 00 | 09 00 C4 00 | 01 90 05 00 - .....Q..........
0x000047F0 - 0A 00 24 00 | 50 01 08 00 | 0B 00 C4 00 | 01 13 08 00 - ..$.P...........
0x00004800 - 0C 00 24 00 | 50 70 05 00 | 0D 00 C4 00 | 01 61 07 00 - ..$.Pp.......a..
0x00004810 - 0E 00 94 00 | 00 45 02 00 | 0F 00 84 00 | 24 21 01 00 - .....E......$!..
0x00004820 - 10 00 C4 00 | 01 92 06 00 | 11 00 24 00 | 50 40 06 00 - ..........$.P@..
0x00004830 - 12 00 24 00 | 50 31 02 00 | 13 00 B4 00 | 64 21 00 00 - ..$.P1......d!..
0x00004840 - 14 00 C4 00 | 01 61 09 00 | 15 00 94 00 | 00 74 06 00 - .....a.......t..
0x00004850 - 16 00 24 00 | 50 10 08 00 | 17 00 A0 00 | 01 10 00 00 - ..$.P...........
0x00004860 - 18 00 80 00 | 04 10 08 00 | 19 00 20 00 | 59 04 02 00 - .......... .Y...
0x00004870 - 1A 00 C0 00 | 89 26 03 00 | 1B 00 B0 00 | 54 10 01 00 - .....&......T...
0x00004880 - 1C 00 A0 00 | 01 20 07 00 | 1D 00 A4 00 | 00 70 02 00 - ..... .......p..
0x00004890 - 1E 00 A4 00 | 00 70 03 00 | 1F 00 B4 00 | 64 90 01 00 - .....p......d...
0x000048A0 - 20 00 A4 00 | 00 70 06 00 | 21 00 A4 00 | 00 80 02 00 -  ....p..!.......
0x000048B0 - 22 00 24 00 | 50 20 06 00 | 23 00 A0 00 | 01 40 03 00 - ".$.P ..#....@..
0x000048C0 - 24 00 24 00 | 50 90 01 00 | 25 00 24 00 | 50 82 07 00 - $.$.P...%.$.P...
0x000048D0 - 26 00 94 00 | 00 48 01 00 | 27 00 C4 00 | 01 72 07 00 - &....H..'....r..
0x000048E0 - 28 00 84 00 | 24 11 08 00 | 29 00 B4 00 | 64 51 05 00 - (...$...)...dQ..
0x000048F0 - 2A 00 A0 00 | 01 00 01 00 | 2B 00 20 00 | 59 04 01 00 - *.......+. .Y...
0x00004900 - 2C 00 24 00 | 50 81 05 00 | 2D 00 94 00 | 00 64 08 00 - ,.$.P...-....d..
0x00004910 - 2E 00 C4 00 | 01 41 07 00 | 2F 00 C4 00 | 01 22 01 00 - .....A../...."..
0x00004920 - 30 00 94 00 | 00 96 09 00 | 31 00 B4 00 | 64 70 06 00 - 0.......1...dp..
0x00004930 - 32 00 84 00 | 24 90 06 00 | D0 3E 00 00 | 01 00 00 00 - 2...$....>......
0x00004940 - DC 3E 00 00 | 01 00 00 00 | E8 3E 00 00 | 03 00 00 00 - .>.......>......
0x00004950 - F4 3E 00 00 | 01 00 00 00 | 00 3F 00 00 | 01 00 00 00 - .>.......?......
0x00004960 - 0C 3F 00 00 | 01 00 00 00 | 18 3F 00 00 | 01 00 00 00 - .?.......?......
0x00004970 - 24 3F 00 00 | 01 00 00 00 | 30 3F 00 00 | 01 00 00 00 - $?......0?......
0x00004980 - 3C 3F 00 00 | 01 00 00 00 | 48 3F 00 00 | 01 00 00 00 - <?......H?......
0x00004990 - 54 3F 00 00 | 01 00 00 00 | 60 3F 00 00 | 6C 3F 00 00 - T?......`?..l?..
0x000049A0 - 78 3F 00 00 | 84 3F 00 00 | 90 3F 00 00 | 9C 3F 00 00 - x?...?...?...?..
0x000049B0 - A8 3F 00 00 | B4 3F 00 00 | C0 3F 00 00 | CC 3F 00 00 - .?...?...?...?..
0x000049C0 - D8 3F 00 00 | E4 3F 00 00 | F0 3F 00 00 | FC 3F 00 00 - .?...?...?...?..
0x000049D0 - 08 40 00 00 | 14 40 00 00 | 20 40 00 00 | 2C 40 00 00 - .@...@.. @..,@..
0x000049E0 - 38 40 00 00 | 44 40 00 00 | 50 40 00 00 | 5C 40 00 00 - 8@..D@..P@..\@..
0x000049F0 - 68 40 00 00 | 74 40 00 00 | 80 40 00 00 | 8C 40 00 00 - h@..t@...@...@..
0x00004A00 - 98 40 00 00 | A4 40 00 00 | FD 8F 36 22 | 8E AD 9B 4D - .@...@....6"...M
0x00004A10 - 92 AD 6D 5B | 13 64 F5 CD | 17 EF E5 8E | 6F 55 30 93 - ..m[.d......oU0.
0x00004A20 - 57 E8 56 C9 | 35 1C B3 65 | 4A DD 1D F9 | 5A 51 CA E8 - W.V.5..eJ...ZQ..
0x00004A30 - B8 8D 77 37 | 71 FF 18 CC | -- -- -- -- | -- -- -- -- - ..w7q...........

; Strings
0x00003314: "sceMediaSync"
0x00003348: "KDebugForKernel"
0x0000335C: "SysMemForKernel"
0x00003370: "SysclibForKernel"
0x00003388: "IoFileMgrForKernel"
0x000033A0: "ThreadManForKernel"
0x000033B8: "UtilsForKernel"
0x000033CC: "InterruptManagerForKernel"
0x000033EC: "InitForKernel"
0x00003400: "ModuleMgrForKernel"
0x00003418: "sceUmd"
0x00003424: "sceUmdCache_driver"
0x0000343C: "sceUmd9660_driver"
0x00003454: "sceWlanDrv_driver"
0x0000346C: "sceCtrl_driver"
0x00003480: "sceAudio_driver"
0x00003494: "sceOpenPSID_driver"
0x000034AC: "sceReg_driver"
0x00003610: "disc"
0x0000361C: "flash3"
0x00003628: "mediasync.c:%s:bootable=%d, mode=%d\n"
0x00003650: "_sceMediaSyncModuleStart"
0x0000366C: "disc0:"
0x00003674: "mediasync.c:%s:sceUmdActivate failed 0x%08x\n"
0x000036A4: "mediasync.c:%s:UMD Media Check OK\n"
0x000036C8: "DiscCheckMedia"
0x000036D8: "mediasync.c:%s:SCE_MEDIASYNC_ERROR_INVALID_MEDIA\n"
0x0000370C: "mediasync.c:%s:[%16s] and [%16s]\n"
0x00003730: "mediasync.c:%s:DiscCheckMedia failed 0x%08x\n"
0x00003760: "WaitDisc"
0x0000376C: "mediasync.c:%s:sceUmdGetDriveStatus:0x%08x\n"
0x00003798: "mediasync.c:%s:sceUmdWaitDriveStat failed 0x%08x\n"
0x000037CC: "SceMediaSyncEvMs"
0x000037E0: "SceMediaSyncMs"
0x000037F0: "fatms0:"
0x000037F8: "mediasync.c:%s:sceFatmsUnRegisterNotifyCallback failed 0x%08x\n"
0x00003838: "mediasync.c:%s:sceKernelDeleteCallback failed 0x%08x\n"
0x00003870: "mediasync.c:%s:sceKernelDeleteEventFlag failed 0x%08x\n"
0x000038A8: "mediasync.c:%s:Warning: MsCheckMediaFailed 0x%08x\n"
0x000038DC: "WaitMs"
0x000038E4: "mediasync.c:%s:MsCheckMediaFailed 0x%08x\n"
0x00003910: "mediasync.c:%s:unsupported apitype=0x%08x[%d]\n"
0x00003940: "mediasync.c:%s:sceFatmsRegisterNotifyCallback failed 0x%08x\n"
0x00003980: "mediasync.c:%s:sceKernelCreateEventFlag failed 0x%08x\n"
0x000039B8: "SceMdiaSync:work2"
0x000039CC: "SceMdiaSync:work1"
0x000039E0: "mediasync.c:%s:This is not PBP\n"
0x00003A00: "MsCheckMedia"
0x00003A10: "mediasync.c:%s:sceKernelFreePartitionMemory failed 0x%08x\n"
0x00003A4C: "mediasync.c:%s:sceKernelAllocPartitionMemory failed %s:0x%08x:size 0x%x\n"
0x00003A98: "mediasync.c:%s:sceIoLseek failed, 0x%x\n"
0x00003AC0: "mediasync.c:%s:sceKernelFreePartitionMemory failed %s:0x%08x\n"
0x00003B00: "mediasync.c:%s:read header failed 0x%08x\n"
0x00003B2C: "mediasync.c:%s:sceIoOpen failed [%s] 0x%08x\n"
0x00003B5C: "mediasync.c:%s:sceKernelAllocPartitionMemory failed %s:0x%08x\n"
0x00003B9C: "SceMediaSyncEvEf"
0x00003BB0: "SceMediaSyncEf"
0x00003BC0: "fatef0:"
0x00003BC8: "mediasync.c:%s:sceFatefUnRegisterNotifyCallback failed 0x%08x\n"
0x00003C08: "WaitEflash"
0x00003C14: "mediasync.c:%s:sceFatefRegisterNotifyCallback failed 0x%08x\n"
0x00003C54: "mediasync.c:%s:Not READABLE : 0x%08x\n"
0x00003C7C: "WaitDiscEmu"
0x00003C88: "mediasync.c:%s:MsCheckMedia failed 0x%08x\n"
0x00003CB4: "mediasync.c:%s:Cannot wait UMD event : 0x%08x\n"
0x00003CE4: "mediasync.c:%s:Cannot activate UMD : 0x%08x\n"
0x00003D14: "MsCallback"
0x00003D20: "mediasync.c:%s:EVENT, arg=0x%08x\n"
0x00003D44: "DISC_ID"
0x00003D4C: "mediasync.c:%s:sceSystemFileGetIndex failed (res=0x%x)\n"
0x00003D84: "ULJS00167"
0x00003D90: "mediasync.c:%s:sceKernelDeleteHeap failed 0x%08x\n"
0x00003DC4: "MsSystemFile"
0x00003DD4: "ULJS00168"
0x00003DE0: "ULJS00169"
0x00003DEC: "mediasync.c:%s:failed to get value of 'DISC_ID' (ret=0x%x)\n"
0x00003E28: "mediasync.c:%s:sceSystemFileLoadAll2 failed 0x%08x, buf=0x%08x, size=0x%08x\n"
0x00003E78: "SceKernelMediaSyncHeap"
0x00003E90: "sf_malloc"
0x00003E9C: "mediasync.c:%s:sceKernelCreateHeap failed 0x%08x\n"
0x00003ED0: "ULJS00022"
0x00003EDC: "ULJM05071"
0x00003EE8: "ULJM05352"
0x00003EF4: "ULJS00154"
0x00003F00: "ULJM05058"
0x00003F0C: "ULJM05189"
0x00003F18: "ULJM05201"
0x00003F24: "ULJS00170"
0x00003F30: "ULJM05164"
0x00003F3C: "ULJM05156"
0x00003F48: "ULJM05500"
0x00003F54: "ULJM05135"
0x00003F60: "UCKS45124"
0x00003F6C: "UCJS10104"
0x00003F78: "UCUS98743"
0x00003F84: "UCES01250"
0x00003F90: "UCAS40266"
0x00003F9C: "ULJS00224"
0x00003FA8: "NPJH50184"
0x00003FB4: "ULUS10466"
0x00003FC0: "ULES01376"
0x00003FCC: "ULAS42214"
0x00003FD8: "ULKS46235"
0x00003FE4: "ULJS00202"
0x00003FF0: "ULUS10457"
0x00003FFC: "ULES01298"
0x00004008: "UCKS45126"
0x00004014: "ULJS00217"
0x00004020: "ULJM05519"
0x0000402C: "ULJM05489"
0x00004038: "NPJH50040"
0x00004044: "UCKS45140"
0x00004050: "ULUS10512"
0x0000405C: "ULJS00238"
0x00004068: "ULJS00236"
0x00004074: "ULUS10518"
0x00004080: "ULAS42231"
0x0000408C: "ULES01407"
0x00004098: "ULJM05542"
0x000040A4: "ULJM05544"
0x000040B0: "ULJS00019"
0x000040BC: "ULJS00167"
0x000040C8: "ULJS00168"
0x000040D4: "ULJS00169"
0x000040E0: "sceUmdCache_driver"
0x000040F4: "ULJM05109"
0x00004100: "fatms0:"
0x00004108: "fatef0:"
0x00004110: "/CONFIG/NP"
0x00004120: "invalid"
0x00004128: "/CONFIG/NETWORK/ADHOC"
0x00004140: "ssid_prefix"
0x00004162: "x{~w22BBBB"
0x00004184: "sceLibSysfile"
0x0000419C: "NPJH90038"

; ==== Section .data - Address 0x00004A40 Size 0x00000008 Flags 0x0003
           - 00 01 02 03 | 04 05 06 07 | 08 09 0A 0B | 0C 0D 0E 0F - 0123456789ABCDEF
-------------------------------------------------------------------------------------
0x00004A40 - 28 12 00 00 | F0 12 00 00 | -- -- -- -- | -- -- -- -- - (...............
