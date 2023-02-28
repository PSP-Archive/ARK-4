#include <pspsdk.h>
#include <pspsysmem_kernel.h>
#include <psputilsforkernel.h>
#include <pspinit.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <systemctrl_private.h>
#include <globals.h> 
#include "functions.h"
#include "macros.h"
#include "exitgame.h"
#include "adrenaline_compat.h"

void PatchLoadExec(u32 text_addr, u32 text_size) {
	u32 jump = 0;
	int i;
	for (i = 0; i < text_size; i += 4) {
		u32 addr = text_addr + i;

		// Remove apitype check in FW's above 2.60
		if (_lw(addr) == 0x24070200) {
			memset((void *)addr, 0, 0x20);
			continue;
		}

		// Ignore kermit calls
		if (_lw(addr) == 0x17C001D3) {
			_sw(0, addr);
			jump = addr + 8;
			continue;
		}

		// Fix type check
		if (_lw(addr) == 0x34650002) {
			_sw(0x24050002, addr); //ori $a1, $v1, 0x2 -> li $a1, 2
			_sw(0x12E500B7, addr + 4); //bnez $s7, loc_XXXXXXXX -> beq $s7, $a1, loc_XXXXXXXX
			_sw(0xAC570018, addr + 8); //sw $a1, 24($v0) -> sw $s7, 24($v0)
			continue;
		}

		if (_lw(addr) == 0x24100200) {
			// Some registers are reserved. Use other registers to avoid malfunction
			_sw(0x24050200, addr); //li $s0, 0x200 -> li $a1, 0x200
			_sw(0x12650003, addr + 4); //beq $s3, $s0, loc_XXXXXXXX - > beq $s3, $a1, loc_XXXXXXXX
			_sw(0x241E0210, addr + 8); //li $s5, 0x210 -> li $fp, 0x210
			_sw(0x567EFFDE, addr + 0xC); //bne $s3, $s5, loc_XXXXXXXX -> bne $s3, $fp, loc_XXXXXXXX

			// Allow LoadExecVSH type 1. Ignore peripheralCommon KERMIT_CMD_ERROR_EXIT
			MAKE_JUMP(addr + 0x14, jump);
			_sw(0x24170001, addr + 0x18); //li $s7, 1

			continue;
		}
	}
}