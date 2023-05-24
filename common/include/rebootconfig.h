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
#include <systemctrl_se.h>
#include <functions.h>
#include "ansi_c_functions.h"
#include "globals.h"

#define REBOOTEX_MAX_SIZE 0x4000
#define BTCNF_MAGIC 0x0F803001
#define BOOTCONFIG_TEMP_BUFFER 0x88FB0200

// PROCFW Reboot Buffer Configuration Address
#define REBOOTEX_CONFIG (REBOOTEX_TEXT - 0x10000)

// PROCFW Reboot Buffer ISO Path (so we don't lose that information)
#define REBOOTEX_CONFIG_ISO_PATH_MAXSIZE 0x100

/**
    Originally ARK used a similar rebootex config as PRO with the same magic number.
    Yet it wasn't fully binary compatible with PRO's rebootex config.
    Now we are using a different magic number to identify ARK's rebootconfig vs PRO's.
    This allows ARK SystemControl to work with PRO's reboot buffer configuration.
    
    If ARK can't detect rebootex configuration, several CFW functions that work with it will be disabled
        (such as being able to change ISO driver, ISO path, reboot module and such). 
*/

// ARK Rebootex config
typedef struct RebootConfigARK {
    unsigned int magic;
    unsigned int reboot_buffer_size;
    unsigned char iso_mode;
    unsigned char iso_disc_type;
    char iso_path[REBOOTEX_CONFIG_ISO_PATH_MAXSIZE]; // inferno ISO path
    struct { // runtime module
        char *before;
        void *buffer;
        u32 size;
        u32 flags;
    } rtm_mod;
} RebootConfigARK;

#define IS_ARK_CONFIG(config) (*((u32*)config) == ARK_CONFIG_MAGIC)

#endif

