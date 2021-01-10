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

#include "flashpatch.h"
#include "functions.h"

// kermit_peripheral's sub_000007CC clone, called by loadexec + 0x0000299C with a0=8 (was a0=7 for fw <210)
// Returns 0 on success
u64 kermit_flash_load(int cmd)
{
	u8 buf[128];
	u64 resp;
	void *alignedBuf = (void*)ALIGN_64((int)buf + 63);
	k_tbl->KernelDcacheInvalidateRange(alignedBuf, 0x40);
	
	KermitPacket *packet = (KermitPacket *)KERMIT_PACKET((int)alignedBuf);


	u32 argc = 0;
	k_tbl->Kermit_driver_4F75AA05(packet, KERMIT_MODE_PERIPHERAL, cmd, argc, KERMIT_CALLBACK_DISABLE, &resp);

	return resp;
}

int flash_load()
{
	int ret = kermit_flash_load(KERMIT_CMD_ERROR_EXIT);

	// Wait for flash to load
	k_tbl->KernelDelayThread(10000);

	return ret;
}

int flashLoadPatch(int cmd)
{

	int ret = kermit_flash_load(cmd);

	// Custom handling on loadFlash mode, else nothing
	if ( cmd == KERMIT_CMD_ERROR_EXIT || cmd == KERMIT_CMD_ERROR_EXIT_2 )
	{
		int linked;

		// Wait for flash to load
		k_tbl->KernelDelayThread(10000);

		// Patch flash0 Filesystem Driver
		linked = patchFlash0Archive();
		k_tbl->KernelIcacheInvalidateAll();
		k_tbl->KernelDcacheWritebackInvalidateAll();
	}
	
	return ret;
}

int patchFlash0Archive()
{
	int fd;

	// Base Address
	uint32_t procfw = 0x8BA00000;
	uint32_t sony = FLASH_SONY;

	// Cast PROCFW flash0 Filesystem
	VitaFlashBufferFile * prof0 = (VitaFlashBufferFile *)procfw;
	
	// Cast Sony flash0 Filesystem
	VitaFlashBufferFile * f0 = (VitaFlashBufferFile *)sony;

	// flash0 Filecounts
	uint32_t procfw_filecount = 0;
	uint32_t flash0_filecount = 0;

    /*	
	if (!NEWER_FIRMWARE){
		// Prevent Double Tapping
		if(prof0[0].name == (char*)f0) return 0;
	}
	*/

	char path[SAVE_PATH_SIZE];
	strcpy(path, ark_config->arkpath);
	strcat(path, "FLASH0.ARK");

	fd = k_tbl->KernelIOOpen(path, PSP_O_RDONLY, 0777);

	if(fd < 0)
		return fd;

	k_tbl->KernelIORead(fd, &procfw_filecount, sizeof(procfw_filecount));
	k_tbl->KernelIOClose(fd);

	// Count Sony flash0 Files
	while(f0[flash0_filecount].content != NULL) flash0_filecount++;

	// Copy Sony flash0 Filesystem into PROCFW flash0
	memcpy(&prof0[procfw_filecount], f0, (flash0_filecount + 1) * sizeof(VitaFlashBufferFile));
	
	// Cast Namebuffer
	char * namebuffer = (char *)sony;
	
	// Cast Contentbuffer
	unsigned char * contentbuffer = (unsigned char *)&prof0[procfw_filecount + flash0_filecount + 1];
	
	// Ammount of linked in Files
	unsigned int linked = 0;
	
	fd = k_tbl->KernelIOOpen(path, PSP_O_RDONLY, 0777);

	if(fd < 0)
		return fd;

	int fileSize, ret, i;
	unsigned char lenFilename;

	// skip file counter
	k_tbl->KernelIORead(fd, &fileSize, sizeof(fileSize));

	for(i=0; i<procfw_filecount; ++i)
	{
		ret = k_tbl->KernelIORead(fd, &fileSize, sizeof(fileSize));

		if(ret != sizeof(fileSize))
			break;

		ret = k_tbl->KernelIORead(fd, &lenFilename, sizeof(lenFilename));

		if(ret != sizeof(lenFilename))
			break;

		ret = k_tbl->KernelIORead(fd, namebuffer, lenFilename);

		if(ret != lenFilename)
			break;

		namebuffer[lenFilename] = '\0';

		// Content Buffer 64 Byte Alignment required
		// (if we don't align this buffer by 64 PRXDecrypt will fail on 1.67+ FW!)
		if((((unsigned int)contentbuffer) % 64) != 0)
		{
			// Align Content Buffer
			contentbuffer += 64 - (((unsigned int)contentbuffer) % 64);
		}
		
		ret = k_tbl->KernelIORead(fd, contentbuffer, fileSize);

		if(ret != fileSize)
			break;

		// Link File into virtual flash0 Filesystem
		prof0[linked].name = namebuffer;
		prof0[linked].content = contentbuffer;
		prof0[linked++].size = fileSize;

		// Move Buffer
		namebuffer += lenFilename + 1;
		contentbuffer += fileSize;
	}

	k_tbl->KernelIOClose(fd);

	// Injection Error
	if(procfw_filecount == 0 || linked != procfw_filecount) return -1;
	
	// Return Number of Injected Files
	return linked;
}
