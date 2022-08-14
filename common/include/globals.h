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

#include <pspsdk.h>

// Generic Offsets
#define USER_BASE 0x08800000
#define KERNEL_BASE 0x88000000
#define GAME_TEXT (USER_BASE + 0x4000)
#define SYSMEM_TEXT KERNEL_BASE
#define REBOOT_TEXT (KERNEL_BASE + 0x600000)
#define REBOOTEX_TEXT (KERNEL_BASE + 0xFC0000)
#define FLASH_SONY 0x8B000000
#define ARK_FLASH 0x8BA00000
#define MAX_HIGH_MEMSIZE 55

// ARK_CONFIG
#define ARK_PATH_SIZE 128
#define ARK_MENU "MENU.PBP" // default launcher
#define ARK_XMENU "XMENU.PBP" // PS1 launcher
#define ARK_INGAME "INGAME.PRX" // in-game menu
#define ARK_RECOVERY "RECOVERY.PBP" // recovery app
#define FLASH0_ARK "FLASH0.ARK" // ARK flash0 package
#define VSH_MENU "VSHMENU.PRX" // ARK VSH Menu for XMB
#define K_FILE "K.BIN" // kernel exploit file for Live loaders
#define ARK2_BIN "ARK.BIN" // ARK-2 payload
#define ARK4_BIN "ARK4.BIN" // ARK-4 payload
#define ARK_BIN_MAX_SIZE 0x8000
#define ARK_MAJOR_VERSION 4
#define ARK_MINOR_VERSION 18
#define ARK_MICRO_VERSION 6
#define PRO_VERSION 0x4000 // identify as ARK

/*
First two bits identify the device (PSP or PS Vita)
Second two bits identify special cases (PSP Slim/Go, Vita Minis, Vita PSX, etc)
Dev Sub
00  00 -> unknown device (attempt to autodetect)
01  00 -> psp
01  01 -> psp with extra ram, useless for now (use sceKernelGetModel)
01  11 -> psp go, useless fow now (use sceKernelGetModel)
10  00 -> ps vita
10  01 -> vita minis, useless for now (use better base game!)
10  10 -> vita pops, useless for now (extremely limited in later firmwares, though there are ways around them...)
11  00 -> device mask
*/
typedef enum{
    DEV_UNK = 0b0000,
    PSP_ORIG = 0b0100,
    PS_VITA = 0b1000,
    PSV_POPS = 0b1010,
    DEV_MASK = 0b1100,
}ExecMode;

// Different PSP models
enum {
    PSP_1000 = 0,
    PSP_2000 = 1,
    PSP_3000 = 2,
    PSP_4000 = 3,
    PSP_GO   = 4,
    PSP_7000 = 6,
    PSP_9000 = 8,
    PSP_11000 = 10,
};

// Different firmware versions
#define FW_661 0x06060110
#define FW_660 0x06060010
#define FW_639 0x06030910
#define FW_635 0x06030510
#define FW_620 0x06020010
#define FW_610 0x06010010
#define FW_600 0x06000010
#define FW_551 0x05050110
#define FW_550 0x05050010
#define FW_503 0x05000310
#define FW_501 0x05000110
#define FW_500 0x05000010
#define FW_401 0x04000110
#define FW_396 0x03090610
#define FW_395 0x03090510
#define FW_393 0x03090310
#define FW_390 0x03090010
#define FW_380 0x03080010
#define FW_372 0x03070210
#define FW_371 0x03070110
#define FW_352 0x03050210
#define FW_351 0x03050110
#define FW_350 0x03050010
#define FW_340 0x03040010
#define FW_330 0x03030010
#define FW_311 0x03010110
#define FW_310 0x03010010
#define FW_303 0x03000310
#define FW_302 0x03000210

// These settings should be global and constant during the entire execution of ARK.
// It should not be possible to change these (except for recovery flag).
typedef struct ARKConfig{
    u32 magic;
    char arkpath[ARK_PATH_SIZE-20]; // ARK installation folder, leave enough room to concatenate files
    char exploit_id[12]; // ID of the game exploit, or name of the bootloader
    char launcher[20]; // run ARK in launcher mode if launcher specified
    unsigned char exec_mode; // ARK execution mode (PSP, PS Vita, Vita POPS, etc)
    unsigned char recovery; // run ARK in recovery mode (disables settings, plugins and autoboots RECOVERY.PBP)
} ARKConfig;

// ARK Runtime configuration backup address
#define ARK_CONFIG 0x08800010
#define ARK_CONFIG_MAGIC 0xB00B1E55
#define LIVE_EXPLOIT_ID "Live" // default loader name
#define DEFAULT_ARK_PATH "ms0:/PSP/SAVEDATA/ARK_01234/" // default path for ARK files

#define IS_PSP(ark_config) ((ark_config->exec_mode&DEV_MASK)==PSP_ORIG)
#define IS_VITA(ark_config) ((ark_config->exec_mode&DEV_MASK)==PS_VITA)
#define IS_VITA_POPS(ark_config) (ark_config->exec_mode==PSV_POPS)

// Memory Partition Size
#define USER_SIZE (24 * 1024 * 1024)
#define KERNEL_SIZE (4 * 1024 * 1024)
#define FLASH_SIZE 0x01000000
#define VITA_EXTRA_RAM 11*1024*1024

#endif

