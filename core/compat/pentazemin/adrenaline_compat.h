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

#ifndef __ADRENALINE_COMPAT_H__
#define __ADRENALINE_COMPAT_H__

#define ADRENALINE_VERSION_MAJOR 7
#define ADRENALINE_VERSION_MINOR 0
#define ADRENALINE_VERSION ((ADRENALINE_VERSION_MAJOR << 16) | ADRENALINE_VERSION_MINOR)

#define xstr(s) #s
#define str(s) xstr(s)
#define ADRENALINE_VERSION_MAJOR_STR str(ADRENALINE_VERSION_MAJOR)
#define ADRENALINE_VERSION_MINOR_STR str(ADRENALINE_VERSION_MINOR)

#define ADRENALINE_TITLEID "PSPEMUCFW"

#define SCE_PSPEMU_FLASH0_PACKAGE_SIZE 0x920000
#define SCE_PSPEMU_EXTRA_MEMORY 0x8B000000
#define SCE_PSPEMU_SCRATCHPAD 0x8BD00000
#define SCE_PSPEMU_VRAM 0x8BE00000

#define SCE_PSPEMU_FRAMEBUFFER 0x74000000
#define SCE_PSPEMU_FRAMEBUFFER_SIZE 0x88000

#define PSP_RAM_SIZE (64 * 1024 * 1024)

#define PSP_SCREEN_WIDTH 480
#define PSP_SCREEN_HEIGHT 272
#define PSP_SCREEN_LINE 512

typedef struct {
	SceSize size;
	char shortFileName[13];
	char __padding__[3];
	char longFileName[1024];
} SceFatMsDirent;

typedef struct {
	unsigned int max_clusters;
	unsigned int free_clusters;
	unsigned int max_sectors;
	unsigned int sector_size;
	unsigned int sector_count;
} ScePspemuIoDevInfo;

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

enum KermitVirtualInterrupts {
	KERMIT_VIRTUAL_INTR_NONE,
	KERMIT_VIRTUAL_INTR_AUDIO_CH1,
	KERMIT_VIRTUAL_INTR_AUDIO_CH2,
	KERMIT_VIRTUAL_INTR_AUDIO_CH3,
	KERMIT_VIRTUAL_INTR_ME_DMA_CH1,
	KERMIT_VIRTUAL_INTR_ME_DMA_CH2,
	KERMIT_VIRTUAL_INTR_ME_DMA_CH3,
	KERMIT_VIRTUAL_INTR_WLAN_CH1,
	KERMIT_VIRTUAL_INTR_WLAN_CH2,
	KERMIT_VIRTUAL_INTR_IMPOSE_CH1,
	KERMIT_VIRTUAL_INTR_POWER_CH1,
	KERMIT_VIRTUAL_INTR_UNKNOWN_CH1,	// <- used after settings
	KERMIT_VIRTUAL_INTR_USBGPS_CH1,
	KERMIT_VIRTUAL_INTR_USBPSPCM_CH1,
};

enum KermitArgumentModes {
  KERMIT_INPUT_MODE = 0x1,
  KERMIT_OUTPUT_MODE = 0x2,
};

typedef struct {
	uint32_t cmd; //0x0
	SceUID sema_id; //0x4
	uint64_t *response; //0x8
	uint32_t padding; //0xC
	uint64_t args[14]; // 0x10
} SceKermitRequest; //0x80

// 0xBFC00800
typedef struct {
	uint32_t cmd; //0x00
	SceKermitRequest *request; //0x04
} SceKermitCommand; //0x8

// 0xBFC00840
typedef struct {
	uint64_t result; //0x0
	SceUID sema_id; //0x8
	int32_t unk_C; //0xC
	uint64_t *response; //0x10
	uint64_t unk_1C; //0x1C
} SceKermitResponse; //0x24 or 0x30????

// 0xBFC008C0
typedef struct {
	int32_t unk_0; //0x0
	int32_t unk_4; //0x4
} SceKermitInterrupt; //0x8

enum SaveStateModes {
	SAVESTATE_MODE_NONE,
	SAVESTATE_MODE_SAVE,
	SAVESTATE_MODE_LOAD,
};

enum AdrenalinePspCommands {
	ADRENALINE_PSP_CMD_NONE,
	ADRENALINE_PSP_CMD_REINSERT_MS,
	ADRENALINE_PSP_CMD_SAVESTATE,
	ADRENALINE_PSP_CMD_LOADSTATE,
};

enum AdrenalineVitaCommands {
	ADRENALINE_VITA_CMD_NONE,
	ADRENALINE_VITA_CMD_SAVESTATE,
	ADRENALINE_VITA_CMD_LOADSTATE,
	ADRENALINE_VITA_CMD_GET_USB_STATE,
	ADRENALINE_VITA_CMD_START_USB,
	ADRENALINE_VITA_CMD_STOP_USB,
	ADRENALINE_VITA_CMD_PAUSE_POPS,
	ADRENALINE_VITA_CMD_RESUME_POPS,
	ADRENALINE_VITA_CMD_POWER_SHUTDOWN,
	ADRENALINE_VITA_CMD_POWER_REBOOT,
};

enum AdrenalineVitaResponse {
	ADRENALINE_VITA_RESPONSE_NONE,
	ADRENALINE_VITA_RESPONSE_SAVED,
	ADRENALINE_VITA_RESPONSE_LOADED,
};

typedef struct {
	int savestate_mode;
	int num;
	unsigned int sp;
	unsigned int ra;

	int pops_mode;
	int draw_psp_screen_in_pops;
	char title[128];
	char titleid[12];
	char filename[256];

	int psp_cmd;
	int vita_cmd;
	int psp_response;
	int vita_response;
} SceAdrenaline;

#define ADRENALINE_SIZE 0x2000
#define ADRENALINE_ADDRESS 0xABCDE000

#define DRAW_NATIVE 0xABCDEF00
#define NATIVE_FRAMEBUFFER 0x0A400000

#define CONVERT_ADDRESS(addr) (0x68000000 + (addr & 0x0FFFFFFF))

#endif
