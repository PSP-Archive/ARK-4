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

#ifndef _OFFSETS_H_
#define _OFFSETS_H_

#include <kermit.h>

// Generic Offsets
#define USER_BASE 0x08800000
#define KERNEL_BASE 0x88000000
#define GAME_TEXT (USER_BASE + 0x4000)
#define SYSMEM_TEXT KERNEL_BASE
#define REBOOT_TEXT (KERNEL_BASE + 0x600000)
#define REBOOTEX_TEXT (KERNEL_BASE + 0xFC0000)
#define FLASH_SONY 0x8B000000

// ARK_CONFIG
#define ARK_PATH_SIZE 128
//#define CONF_ADDR 0x883ffe00 //0x8BFFFF00
#define ARK_MENU "MBOOT.PBP"
#define ARK_BIN "ARK.BIN"
#define FLASH0_ARK "FLASH0.ARK"
#define K_FILE "K.BIN"
#define ARK_BIN_MAX_SIZE 0x8000

/*
First two bits identify the device (PSP or PS Vita)
Second two bits identify special cases (PSP Go, PSP Minis and PSX games on Vita)
Dev Sub
00  00 -> unk
01  00 -> psp
01  01 -> psp with extra ram (2k, 3k, street)
01  11 -> psp go
10  00 -> ps vita
10  01 -> vita minis
10  10 -> vita pops
11  00 -> device mask
11  11 -> subdevice mask
*/
typedef enum{
    DEV_UNK = 0b0000,
	PSP_ORIG = 0b0100,
	PSP_EXTRA = 0b0101,
	PSP_GO = 0b0111,
	PS_VITA = 0b1000,
	PSV_MINIS = 0b1001,
	PSV_POPS = 0b1010,
	DEV_MASK = 0b1100,
	SUB_DEV_MARK = 0b1111,
}ExecMode;



typedef struct{ /* PEOPS SPU configuration */
	int enablepeopsspu;
	int volume;
	int reverb;
	int interpolation;
	int enablexaplaying;
	int changexapitch;
	int spuirqwait;
	int spuupdatemode;
	int sputhreadpriority;
} PeopsConfig;

typedef struct ARKConfig{
	char arkpath[ARK_PATH_SIZE-20]; // leave enough room to concatenate files
	char exploit_id[20];
	unsigned char exec_mode;
	unsigned char override_peops_config; // remove?
	PeopsConfig peops_config;
} ARKConfig;

//#define ark_config ((ARKConfig*)CONF_ADDR)
//#define ARKPATH (ark_config->arkpath)
//#define EXPLOIT_ID (ark_config->exploit_id)
#define IS_PSP(exec_mode) ((exec_mode&DEV_MASK)==PSP_ORIG)
#define IS_VITA(exec_mode) ((exec_mode&DEV_MASK)==PS_VITA)
#define IS_VITA_POPS(exec_mode) (exec_mode==PSV_POPS)
//#define IS_PSP _IS_PSP(ark_config)
//#define IS_VITA _IS_VITA(ark_config)
//#define IS_VITA_POPS _IS_VITA_POPS(ark_config)

// Memory Partition Size
#define USER_SIZE (24 * 1024 * 1024)
#define KERNEL_SIZE (4 * 1024 * 1024)
#define FLASH_SIZE 0x01000000

// 6.60 modulemgr.prx
#define MODULEMGR_PARTITION_CHECK 0x7FD0
#define MODULEMGR_PROLOGUE_MODULE 0x8124
#define MODULEMGR_INIT_APITYPE_FIELD 0x99A0
#define MODULEMGR_INIT_FILENAME_FIELD 0x99A4
#define MODULEMGR_INIT_APPLICATION_TYPE_FIELD 0x99FC
#define MODULEMGR_DEVICE_CHECK_1 0x760
#define MODULEMGR_DEVICE_CHECK_2 0x7C0
#define MODULEMGR_DEVICE_CHECK_3 0x30B0
#define MODULEMGR_DEVICE_CHECK_4 0x310C
#define MODULEMGR_DEVICE_CHECK_5 0x313A
#define MODULEMGR_DEVICE_CHECK_6 0x3444
#define MODULEMGR_DEVICE_CHECK_7 0x349C
#define MODULEMGR_DEVICE_CHECK_8 0x34CA
#define MODULEMGR_CHECK_EXEC_STUB 0x8884
#define MODULEMGR_PARTITION_CHECK_CALL_1 0x651C
#define MODULEMGR_PARTITION_CHECK_CALL_2 0x6898
#define MODULEMGR_PROLOGUE_MODULE_CALL 0x7048
#define MODULEMGR_PROBE_EXEC_3 0x8824
#define MODULEMGR_PROBE_EXEC_3_CALL 0x7C5C


// mediasync.prx
// 6.60 mediasync offsets
#define MEDIASYNC_KD_FOLDER_PATCH 0xC8
#define MEDIASYNC_MS_CHECK_MEDIA 0x744
#define MEDIASYNC_DISC_MEDIA_CHECK_1 0x3C4
#define MEDIASYNC_DISC_MEDIA_CHECK_2 0xDC8
#define MEDIASYNC_MS_SYSTEM_FILE_CHECK 0x10B4
#define MEDIASYNC_DISC_ID_CHECK_1 0xFC0
#define MEDIASYNC_DISC_ID_CHECK_2 0xFDC

// 6.60 np9660.prx
#define NP9660_INIT_FOR_KERNEL_CALL 0x3C5C
#define NP9660_INIT_ISOFS_CALL 0x3C78
#define NP9660_READ_DISC_SECTOR_CALL_1 0x4414
#define NP9660_READ_DISC_SECTOR_CALL_2 0x596C
#define NP9660_IO_CLOSE_STUB 0x7D68
#define NP9660_INIT 0x36A8
// These need code upgrade to point to BSS properly, right now they are relative to Text
// See 0x3394
// NP9660_ISO_FD at the start of BSS
#define NP9660_DATA_1 (0x00005BB4 - 0x00005BA4 + 0x00000188 + 0x00008900)
#define NP9660_DATA_2 (0x00005BBC - 0x00005BA4 + 0x00000188 + 0x00008900)
#define NP9660_DATA_3 (0x00005BD0 - 0x00005BA4 + 0x00000188 + 0x00008900)
#define NP9660_DATA_4 (0x00005BD8 - 0x00005BA4 + 0x00000188 + 0x00008900)
#define NP9660_DATA_5 (0x00000114 + 0x00008900)
#define NP9660_ISO_FD (0x00000188 + 0x00008900)
#define NP9660_READ_DISC_SECTOR 0x4FEC
#define NP9660_READ_SECTOR_FLUSH 0x505C

// isofs.prx
#define ISOFS_WAIT_SEMA_CALL     0x3FEC
#define ISOFS_SIGNAL_SEMA_CALL_1 0x4024
#define ISOFS_SIGNAL_SEMA_CALL_2 0x40D8
#define ISOFS_SIGNAL_SEMA_CALL_3 0x42B4
#define ISOFS_GLOBAL_FIX 0x7EF4

#endif

