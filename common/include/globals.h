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
#define LOADER_TEXT (0x040EC000)
#define MAINBIN_TEXT (0x04000000)
#define EXTRA_RAM 0x8A000000
#define FLASH_SONY 0x8B000000
#define ARK_FLASH 0x8BA00000
#define BOOT_KEY_BUFFER (KERNEL_BASE + 0xFB0000)

// Memory Partition Size
#define USER_SIZE (24 * 1024 * 1024)
#define KERNEL_SIZE (4 * 1024 * 1024)
#define VITA_FLASH_SIZE 0x01000000 // 16MiB
#define VITA_EXTRA_RAM_SIZE (16 * 1024 * 1024)
#define MAX_HIGH_MEMSIZE 55

// ARK_CONFIG
#define ARK_PATH_SIZE 128
#define ARK_MENU "VBOOT.PBP" // default launcher
#define ARK_XMENU "XBOOT.PBP" // PS1 launcher
#define ARK_RECOVERY "RECOVERY.PBP" // recovery app
#define FLASH0_ARK "FLASH0.ARK" // ARK flash0 package
#define VSH_MENU "VSHMENU.PRX" // ARK VSH Menu for XMB
#define XMBCTRL_PRX "XMBCTRL.PRX" // XMB Control
#define K_FILE "K.BIN" // kernel exploit file for Live loaders
#define ARK2_BIN "ARK.BIN" // ARK-2 payload
#define ARK4_BIN "ARK4.BIN" // ARK-4 payload
#define ARK_BIN_MAX_SIZE 0x8000
#define ARK_MAJOR_VERSION 4
#define ARK_MINOR_VERSION 20
#define ARK_MICRO_VERSION 55
#define PRO_VERSION 0x4000 // identify as ARK
#define MAX_FLASH0_SIZE 0x32000

/*
First two bits identify the device (PSP or PS Vita)
Second two bits identify special cases (PSP, Vita, Vita PSX, etc)
Dev Sub
00  00 -> unknown device (attempt to autodetect)
01  00 -> psp
01  01 -> unused
01  10 -> unused
10  00 -> ps vita
10  01 -> vita adrenaline
10  10 -> vita pops
11  00 -> device mask
*/
typedef enum{
    DEV_UNK = 0b0000,
    PSP_ORIG = 0b0100,
    PS_VITA = 0b1000,
    PSV_ADR = 0b1001,
    PSV_POPS = 0b1010,
    DEV_MASK = 0b1100,
}ExecMode;

// Different PSP models
enum {
    PSP_1000 = 0,   // 01g
    PSP_2000 = 1,   // 02g
    PSP_3000 = 2,   // 03g
    PSP_4000 = 3,   // 04g
    PSP_GO   = 4,   // 05g
    PSP_7000 = 6,   // 07g
    PSP_9000 = 8,   // 09g
    PSP_11000 = 10, // 11g
};

// Different firmware versions
#define FW_661 0x06060110
#define FW_660 0x06060010
#define FW_150 0x01050003

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
#define DEFAULT_ARK_FOLDER "ARK_01234"
#define DEFAULT_ARK_PATH "ms0:/PSP/SAVEDATA/" DEFAULT_ARK_FOLDER "/" // default path for ARK files
#define ARK_DC_PATH "ms0:/TM/DCARK"
#define TM_PATH_W L"\\TM\\DCARK\\"

#define IS_PSP(ark_config) ((ark_config->exec_mode&DEV_MASK)==PSP_ORIG)
#define IS_VITA(ark_config) ((ark_config->exec_mode&DEV_MASK)==PS_VITA)
#define IS_VITA_ADR(ark_config) (ark_config->exec_mode==PSV_ADR)
#define IS_VITA_POPS(ark_config) (ark_config->exec_mode==PSV_POPS)

#endif

