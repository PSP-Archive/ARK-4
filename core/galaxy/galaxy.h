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

#ifndef _GALAXY_H_
#define _GALAXY_H_

#include <pspsdk.h>
#include <stdio.h>

// Read Argument for Kernel Extend Stack
struct IoReadArg {
	unsigned int offset;
	unsigned char * address;
	unsigned int size;
};

// CSO Header
struct CISO_header {
	unsigned char magic[4];
	unsigned int header_size;
	uint64_t total_bytes;
	unsigned int block_size;
	unsigned char ver;
	unsigned char align;
	unsigned char rsv_06[2];
};

// ISO Sector Size
#define ISO_SECTOR_SIZE 0x800

// CSO Buffer Size
#define CISO_IDX_BUFFER_SIZE 0x200
#define CISO_DEC_BUFFER_SIZE 0x2000

// Get Disc Sector Count
int get_total_block(void);

// Read Raw ISO Data
int read_raw_data(unsigned char * addr, unsigned int size, unsigned int offset);

// Read CSO Disc Sector
int read_cso_sector(unsigned char * addr, int sector);

// Custom CSO Sector Reader
int read_cso_data(unsigned char * addr, unsigned int size, unsigned int offset);

// Custom ISO Sector Reader Wrapper
int iso_read(struct IoReadArg * args);

// Open CSO File (subcall from open_iso)
int cso_open(SceUID fd);

// Open ISO File
int open_iso(void);

// readDiscSectorNP9660 Hook
int readDiscSector(unsigned int sector, unsigned char * buffer, unsigned int size);

// sceIoClose Hook
int myIoClose(int fd);

// Init ISO Emulator
int initEmulator(void);

// sceKernelStartThread Hook
int myKernelStartThread(SceUID thid, SceSize arglen, void * argp);

// sceKernelCreateThread Hook
SceUID myKernelCreateThread(const char * name, SceKernelThreadEntry entry, int initPriority, int stackSize, SceUInt attr, SceKernelThreadOptParam * option);

#endif

