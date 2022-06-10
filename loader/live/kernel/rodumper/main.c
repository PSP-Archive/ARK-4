/*
 *  Copyright (C) 2018 qwikrazor87/AcidSnake
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
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

PSP_MODULE_INFO("dumper", 0, 1, 0);

u32 buf[256];
int count = 0;
void sceNetMCopyback(u32 *, u32, u32, u32);
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

int ispos(u32 addr)
{
	u32 a0[7], tmp;
	a0[0] = addr - 12;
	a0[3] = 0;
	a0[4] = 0x20000;
	a0[6] = -1;
	sceNetMCopyback(a0, 1, 0, 1);
	tmp = a0[6];
	a0[6] = -1;
	sceNetMCopyback(a0, 0x7FFFFFFF, 0, 1);

	return (tmp != -1) || (a0[6] != -1);
}

int readpos(u32 addr, u32 value)
{
	u32 a0[7];
	a0[0] = addr - 12;
	a0[3] = 0;
	a0[4] = 0x20000;
	a0[6] = -1;
	sceNetMCopyback(a0, value, 0, 1);

	return (a0[6] != -1);
}

int readneg(u32 addr, u32 value)
{
	u32 a0[7];
	a0[0] = addr - 12;
	a0[3] = 0x80000001;
	a0[4] = 0x20000;
	a0[6] = 0x80000000;
	sceNetMCopyback(a0, value + 0x80000001, 0, 1);

	return (a0[6] != 0x80000000);
}

int main(int argc, char *argv[]) {
	pspDebugScreenInit();

	printf("Kernel RAM dumper\n");
	printf("by qwikrazor87 and AcidSnake\n\n");
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

	u32 kram = 0x88000000, ret = 0, track = 0;
	SceUID fd = sceIoOpen("ms0:/kram.bin", PSP_O_CREAT | PSP_O_TRUNC | PSP_O_WRONLY, 0777);

	while (kram < 0x88400000) {
		u32 poslow = 1, poshi = 0x7FFFFFFF, neglow = 0x80000000, neghi = 0xFFFFFFFE;

		if (ispos(kram)) {
			while (1) {
				u32 posarg = (poshi + poslow) / 2;

				if (readpos(kram, posarg)) {
					poslow = posarg;
				} else
					poshi = posarg;

				if ((poshi - poslow) == 1)
					break;
			}

			ret = poslow;
		} else {
			while (1) {
				u32 negarg = ((neghi + neglow) / 2) | 0x80000000;

				if (readneg(kram, negarg)) {
					neglow = negarg;
				} else
					neghi = negarg;

				if ((neghi - neglow) == 1)
					break;
			}

			// note: 0, -1 and -2 cannot be determined due to how the kernel checks work,
			// they return -3, so 0 is substituted.
			if (neglow == 0xFFFFFFFD)
				neglow = 0;

			ret = neglow;
		}

		buf[count++] = ret;
		track += 4;

		if ((count * sizeof(u32)) >= sizeof(buf)) {
			sceIoWrite(fd, buf, sizeof(buf));
			int y = pspDebugScreenGetY();
			printf("%d/%d bytes dumped.\n", track, 0x00400000);
			pspDebugScreenSetXY(0, y);
			count = 0;
		}

		kram += 4;
	}

	sceIoClose(fd);

	int y = pspDebugScreenGetY();
	pspDebugScreenSetXY(0, y + 1);

	printf("Dumped to ms0:/kram.bin\n");
	sceKernelDelayThread(2 * 1000 * 1000);
	errorExit(5000, "Exiting...\n");

	return 0;
}
