/*
  Adrenaline
  Copyright (C) 2016-2018, TheFloW

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __MAIN_H__
#define __MAIN_H__

#include <psp2/compat.h>
#include <psp2/ctrl.h>
#include <psp2/io/stat.h>
#include <taihen.h>

#define ALIGN(x, align) (((x) + ((align) - 1)) & ~((align) - 1))

#define MAX_PATH_LENGTH 1024
#define MAX_NAME_LENGTH 256

#define PSP_SCREEN_WIDTH 480
#define PSP_SCREEN_HEIGHT 272
#define PSP_SCREEN_LINE 512

enum MemoryStickLocations {
  MEMORY_STICK_LOCATION_UX0,
  MEMORY_STICK_LOCATION_UR0,
  MEMORY_STICK_LOCATION_IMC0,
  MEMORY_STICK_LOCATION_XMC0,
  MEMORY_STICK_LOCATION_UMA0,
};

typedef struct {
	uint32_t cmd; //0x0
	SceUID sema_id; //0x4
	uint64_t *response; //0x8
	uint32_t padding; //0xC
	uint64_t args[14]; // 0x10
} SceKermitRequest; //0x80


enum KermitArgumentModes {
  KERMIT_INPUT_MODE = 0x1,
  KERMIT_OUTPUT_MODE = 0x2,
};

enum KermitModes {
	KERMIT_MODE_NONE,
	KERMIT_MODE_UNK_1,
	KERMIT_MODE_UNK_2,
	KERMIT_MODE_MSFS,
	KERMIT_MODE_FLASHFS,
	KERMIT_MODE_AUDIOOUT,
	KERMIT_MODE_ME,
	KERMIT_MODE_LOWIO,
	KERMIT_MODE_POCS_USBPSPCM,
	KERMIT_MODE_PERIPHERAL,
	KERMIT_MODE_WLAN,
	KERMIT_MODE_AUDIOIN,
	KERMIT_MODE_USB,
	KERMIT_MODE_UTILITY,
	KERMIT_MODE_EXTRA_1,
	KERMIT_MODE_EXTRA_2,
};

typedef struct {
	unsigned int max_clusters;
	unsigned int free_clusters;
	unsigned int max_sectors;
	unsigned int sector_size;
	unsigned int sector_count;
} ScePspemuIoDevInfo;

typedef struct {
	SceSize size;
	char shortFileName[13];
	char __padding__[3];
	char longFileName[1024];
} SceFatMsDirent;

extern int (* ScePspemuDivide)(uint64_t x, uint64_t y);
extern int (* ScePspemuErrorExit)(int error);
extern int (* ScePspemuConvertAddress)(uint32_t addr, int mode, uint32_t cache_size);
extern int (* ScePspemuWritebackCache)(void *addr, int size);
extern int (* ScePspemuKermitWaitAndGetRequest)(int mode, SceKermitRequest **request);
extern int (* ScePspemuKermitSendResponse)(int mode, SceKermitRequest *request, uint64_t response);
extern int (* ScePspemuConvertStatTimeToUtc)(SceIoStat *stat);
extern int (* ScePspemuConvertStatTimeToLocaltime)(SceIoStat *stat);
extern int (* ScePspemuSettingsHandler)(int a1, int a2, int a3, int a4);
extern int (* ScePspemuSetDisplayConfig)();
extern int (* ScePspemuPausePops)(int pause);
extern int (* ScePspemuInitPops)();
extern int (* ScePspemuInitPocs)();


#endif
