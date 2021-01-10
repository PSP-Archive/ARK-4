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

#include "globals.h"

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

extern RebootBufferConfiguration reboot_config;

#endif

