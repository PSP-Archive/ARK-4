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

#ifndef _REBOOTCONFIG_H_
#define _REBOOTCONFIG_H_

#include <pspsdk.h>
#include <macros.h>
#include <rebootconfig.h>
#include <systemctrl_se.h>
#include <functions.h>
#include "ansi_c_functions.h"
#include "globals.h"

#define REBOOT_START 0x88600000
#define REBOOTEX_START 0x88FC0000
#define REBOOTEX_MAX_SIZE 0x4000
#define BTCNF_MAGIC 0x0F803001
#define BOOTCONFIG_TEMP_BUFFER 0x88FB0200

// PROCFW Reboot Buffer Configuration Magic (0xCOLDBIRD)
#define REBOOTEX_CONFIG_MAGIC 0xC01DB15D

// PROCFW Reboot Buffer Configuration Address
#define REBOOTEX_CONFIG (REBOOTEX_TEXT - 0x10000)
#define REBOOTEX_CONFIG_MAXSIZE 0x100

// PROCFW Reboot Buffer ISO Path (so we don't lose that information)
#define REBOOTEX_CONFIG_ISO_PATH (REBOOTEX_CONFIG + REBOOTEX_CONFIG_MAXSIZE)
#define REBOOTEX_CONFIG_ISO_PATH_MAXSIZE 0x100

// PROCFW Reboot Buffer Configuration
typedef struct RebootBufferConfiguration {
    unsigned int magic;
    unsigned int reboot_buffer_size;
    unsigned char psidHash[16];
    unsigned char iso_mode;
    unsigned char iso_disc_type;
} RebootBufferConfiguration;

typedef struct RebootexFunctions{
    void* rebootex_decrypt;
    void* rebootex_checkexec;
    void* orig_decrypt;
    void* orig_checkexec;
}RebootexFunctions;
#define REBOOTEX_FUNCTIONS (RebootexFunctions*)0x08D38000

extern RebootBufferConfiguration reboot_config;

// sceReboot Main Function
extern int (* sceReboot)(int, int, int, int);

// Instruction Cache Invalidator
extern void (* sceRebootIcacheInvalidateAll)(void);

// Data Cache Invalidator
extern void (* sceRebootDacheWritebackInvalidateAll)(void);

// Sony PRX Decrypter Function Pointer
extern int (* SonyPRXDecrypt)(void *, unsigned int, unsigned int *);
extern int (* origCheckExecFile)(unsigned char * addr, void * arg2);

// LfatOpen on PS Vita
extern int (*pspemuLfatOpen)(char** filename, int unk);

// UnpackBootConfig on PSP
extern int (* UnpackBootConfig)(char * buffer, int length);

int _UnpackBootConfig(char **p_buffer, int length);

#endif

