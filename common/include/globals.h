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
#define MAX_HIGH_MEMSIZE 55

// ARK_CONFIG
#define ARK_PATH_SIZE 128
//#define CONF_ADDR 0x883ffe00 //0x8BFFFF00
#define ARK_MENU "MENU.PBP"
#define ARK_BIN "ARK.BIN"
#define FLASH0_ARK "FLASH0.ARK"
#define VSH_MENU "VSHMENU.PRX"
#define K_FILE "K.BIN"
#define ARK_BIN_MAX_SIZE 0x8000

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
11  11 -> subdevice mask
*/
typedef enum{
    DEV_UNK = 0b0000,
    PSP_ORIG = 0b0100,
    //PSP_SLIM = 0b0101,
    //PSP_GO = 0b0111,
    PS_VITA = 0b1000,
    //PSV_MINIS = 0b1001,
    PSV_POPS = 0b1010,
    DEV_MASK = 0b1100,
    SUB_DEV_MARK = 0b1111,
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


typedef struct ARKConfig{
    char arkpath[ARK_PATH_SIZE-20]; // leave enough room to concatenate files
    char exploit_id[20];
    unsigned char exec_mode;
    unsigned char recovery;
} ARKConfig;

#define IS_PSP(exec_mode) ((exec_mode&DEV_MASK)==PSP_ORIG)
#define IS_VITA(exec_mode) ((exec_mode&DEV_MASK)==PS_VITA)
#define IS_VITA_POPS(exec_mode) (exec_mode==PSV_POPS)

// Memory Partition Size
#define USER_SIZE (24 * 1024 * 1024)
#define KERNEL_SIZE (4 * 1024 * 1024)
#define FLASH_SIZE 0x01000000

#endif

