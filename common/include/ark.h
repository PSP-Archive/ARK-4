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

#ifndef _ARK_H_
#define _ARK_H_

#include <pspsdk.h>

#ifdef __cplusplus
extern "C" {
#endif

// ARK Version
#define ARK_MAJOR_VERSION 4
#define ARK_MINOR_VERSION 20
#define ARK_MICRO_VERSION 69
#define ARK_REVISION      176

// Pointers and sizes
#define ARK_PATH_SIZE 128
#define ARK_CONFIG 0x08800010 // ARK Runtime configuration backup address
#define ARK_CONFIG_MAGIC 0xB00B1E55 // generic magic number
#define USER_BASE 0x08800000 // user partition (p2)
#define KERNEL_BASE 0x88000000 // kernel partition (p1)
#define GAME_TEXT (USER_BASE + 0x4000) // game's main elf load address
#define SYSMEM_TEXT KERNEL_BASE // sysmem.prx load address (start of kernel ram)
#define REBOOT_TEXT (KERNEL_BASE + 0x600000) // reboot.bin load address
#define REBOOTEX_TEXT (KERNEL_BASE + 0xFC0000) // rebootex load address
#define LOADER_TEXT (0x040EC000) // cIPL load address
#define MAINBIN_TEXT (0x04000000) // IPL load address
#define EXTRA_RAM 0x8A000000 // extra RAM (on 2k+ or vita)
#define FLASH_SONY 0x8B000000 // flash ramfs on vita
#define ARK_FLASH 0x8BA00000 // ark's flash ramfs on vita
#define BOOT_KEY_BUFFER (KERNEL_BASE + 0xFB0000) // controller input in payloadex (cIPL)
#define ARK_BIN_MAX_SIZE 0x8000 // max size of ARK4.BIN
#define MAX_FLASH0_SIZE 0x32000 // max size of FLASH0.ARK
#define USER_SIZE (24 * 1024 * 1024) // user partition size
#define KERNEL_SIZE (4 * 1024 * 1024) // kernel partition size
#define VITA_FLASH_SIZE 0x01000000 // vita flash ramfs size
#define EXTRA_RAM_SIZE (32 * 1024 * 1024) // size of extra ram (2k+)
#define MAX_HIGH_MEMSIZE 55 // max ram that can be given to user
#define FAKE_UID 0x0B00B500

// Paths and other global strings
#define VBOOT_PBP "VBOOT.PBP" // default launcher
#define ARK_XMENU "XBOOT.PBP" // PS1 launcher
#define ARK_RECOVERY "RECOVERY.PBP" // recovery app
#define RECOVERY_PRX "RECOVERY.PRX" // Classic Recovery
#define FLASH0_ARK "FLASH0.ARK" // ARK flash0 package
#define VSH_MENU "VSHMENU.PRX" // ARK VSH Menu for XMB
#define XMBCTRL_PRX "XMBCTRL.PRX" // XMB Control
#define IDSREG_PRX "IDSREG.PRX" // idsRegeneration
#define USBDEV_PRX "USBDEV.PRX" // Custom USB Device
#define PS1SPU_PRX "PS1SPU.PRX" // PS1 SPU Plugin
#define RECOVERY_PRX_FLASH "flash0:/vsh/module/ark_recovery.prx" // Classic Recovery flash0 path
#define IDSREG_PRX_FLASH "flash0:/kd/ark_idsreg.prx" // idsRegeneration flash0 path
#define XMBCTRL_PRX_FLASH "flash0:/kd/ark_xmbctrl.prx" // XMB Control flash0 path
#define USBDEV_PRX_FLASH "flash0:/vsh/module/ark_usbdev.prx" // USBDEV flash0 path
#define VSH_MENU_FLASH "flash0:/vsh/module/ark_satelite.prx" // VSH Menu flash0 path
#define H_FILE "H.BIN" // user exploit binloader
#define K_FILE "K.BIN" // kernel exploit file for Live loaders
#define UPDATER_FILE "UPDATER.TXT" // Update Server URL file
#define ARK_SETTINGS "SETTINGS.TXT" // CFW Settings file
#define MENU_SETTINGS "ARKMENU.BIN" // Settings file for CL and VSH Menu
#define ARK_SETTINGS_FLASH "flash1:/"ARK_SETTINGS
#define UPDATER_FILE_FLASH "flash1:/"UPDATER_FILE // Update Server URL file flash1 path
#define PLUGINS_FILE "PLUGINS.TXT" // plugins config file
#define SEPLUGINS_MS0 "ms0:/SEPLUGINS/" // plugins folder
#define SEPLUGINS_EF0 "ef0:/SEPLUGINS/" // plugins folder (pspgo internal)
#define PLUGINS_PATH SEPLUGINS_MS0 PLUGINS_FILE
#define PLUGINS_PATH_GO SEPLUGINS_EF0 PLUGINS_FILE
#define PLUGINS_PATH_FLASH "flash0:/"PLUGINS_FILE
#define ARK_THEME_FILE "THEME.ARK" // theme file for arkMenu
#define ARK_LANG_FILE "LANG.ARK" // language files
#define ARK_BIN "ARK.BIN" // ARK-2 payload
#define ARK4_BIN "ARK4.BIN" // ARK-4 payload
#define ARKX_BIN "ARKX.BIN" // ARK-X payload
#define LIVE_EXPLOIT_ID "Live" // default loader name
#define CIPL_EXPLOIT_ID "cIPL" // loader name for Custom IPL
#define DC_EXPLOIT_ID "DC" // loader name for Despertar del Cementerio
#define DEFAULT_ARK_FOLDER "ARK_01234"
#define SAVEDATA_MS0 "ms0:/PSP/SAVEDATA/"
#define SAVEDATA_EF0 "ef0:/PSP/SAVEDATA/"
#define DEFAULT_ARK_PATH SAVEDATA_MS0 DEFAULT_ARK_FOLDER "/" // default path for ARK files
#define DEFAULT_ARK_PATH_GO SAVEDATA_EF0 DEFAULT_ARK_FOLDER "/" // default path for ARK files
#define ARK_DC_PATH "ms0:/TM/DCARK"
#define TM_PATH_W L"\\TM\\DCARK\\"

// Different firmware versions
#define FW_661 0x06060110
#define FW_660 0x06060010
#define FW_150 0x01050001

// Syscon mem address used for 1.50 resume support
#define SYSCON_SCRATCHPAD_RESUME_FW_ADDR 0x4

/*
Device identifier. Used as extension of psp_model.
First two bits identify the device (PSP or PS Vita)
Second two bits identify special cases (PSP, Vita, Vita PSX, etc)
Dev Sub
00  00 -> unknown device (attempt to autodetect)
01  00 -> psp
01  01 -> psp toolkit
01  10 -> unused
10  00 -> ps vita
10  01 -> vita adrenaline
10  10 -> vita pops
11  00 -> device mask
*/
typedef enum{
    DEV_UNK = 0b0000,
    PSP_ORIG = 0b0100,
    PSP_TOOL = 0b0101,
    PS_VITA = 0b1000,
    PSV_ADR = 0b1001,
    PSV_POPS = 0b1010,
    DEV_MASK = 0b1100,
}ExecMode;

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

// macros for device checking
#define IS_PSP(ark_config) ((ark_config->exec_mode&DEV_MASK)==PSP_ORIG)
#define IS_VITA(ark_config) ((ark_config->exec_mode&DEV_MASK)==PS_VITA)
#define IS_VITA_ADR(ark_config) (ark_config->exec_mode==PSV_ADR)
#define IS_VITA_POPS(ark_config) (ark_config->exec_mode==PSV_POPS)

// Function to obtain ARK's execution environment configuration
void* sctrlHENGetArkConfig(ARKConfig* conf);

#ifdef __cplusplus
}
#endif

#endif

