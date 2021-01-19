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

#include <macros.h>
#include "flashpatch.h"
#include "functions.h"

extern ARKConfig* ark_config;

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

    if (ark_config==NULL) return 0;
    
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

	char path[ARK_PATH_SIZE];
	strcpy(path, ark_config->arkpath);
	strcat(path, FLASH0_ARK);

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

#ifndef PRTSTR
#define PRTSTR11(text, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11) g_tbl->prtstr(text, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11)
#define PRTSTR10(text, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10) PRTSTR11(text, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, 0)
#define PRTSTR9(text, x1, x2, x3, x4, x5, x6, x7, x8, x9) PRTSTR10(text, x1, x2, x3, x4, x5, x6, x7, x8, x9, 0)
#define PRTSTR8(text, x1, x2, x3, x4, x5, x6, x7, x8) PRTSTR9(text, x1, x2, x3, x4, x5, x6, x7, x8, 0)
#define PRTSTR7(text, x1, x2, x3, x4, x5, x6, x7) PRTSTR8(text, x1, x2, x3, x4, x5, x6, x7, 0)
#define PRTSTR6(text, x1, x2, x3, x4, x5, x6) PRTSTR7(text, x1, x2, x3, x4, x5, x6, 0)
#define PRTSTR5(text, x1, x2, x3, x4, x5) PRTSTR6(text, x1, x2, x3, x4, x5, 0)
#define PRTSTR4(text, x1, x2, x3, x4) PRTSTR5(text, x1, x2, x3, x4, 0)
#define PRTSTR3(text, x1, x2, x3) PRTSTR4(text, x1, x2, x3, 0)
#define PRTSTR2(text, x1, x2) PRTSTR3(text, x1, x2, 0)
#define PRTSTR1(text, x1) PRTSTR2(text, x1, 0)
#define PRTSTR(text) PRTSTR1(text, 0)
#endif

void open_flash(){
    while(k_tbl->IoUnassign("flash0:") < 0) {
		k_tbl->KernelDelayThread(500000);
	}
	while (k_tbl->IoAssign("flash0:", "lflash0:0,0", "flashfat0:", 0, NULL, 0)<0){
	    k_tbl->KernelDelayThread(500000);
	}
	/*
	while(k_tbl->IoUnassign("flash1:") < 0) {
		k_tbl->KernelDelayThread(500000);
	}
	while (k_tbl->IoAssign("flash1:", "lflash0:0,1", "flashfat1:", 0, NULL, 0)<0){
	    k_tbl->KernelDelayThread(500000);
	}
	*/
}

void close_flash(){
    /*
    int (*IoSync)(const char *, unsigned int) = (void*)FindFunction("sceIOFileManager", "IoFileMgrForKernel", 0xAB96437F);
    IoSync("flashfat0:", 0);
    IoSync("msfat0:", 0);
    while(k_tbl->IoUnassign("flash0:") < 0) {
		k_tbl->KernelDelayThread(500000);
	}
	while (k_tbl->IoAssign("flash0:", "lflash0:0,0", "flashfat0:", 1, NULL, 0)<0){
	    k_tbl->KernelDelayThread(500000);
	}
	while(k_tbl->IoUnassign("flash1:") < 0) {
		k_tbl->KernelDelayThread(500000);
	}
	while (k_tbl->IoAssign("flash1:", "lflash0:0,1", "flashfat1:", 1, NULL, 0)<0){
	    k_tbl->KernelDelayThread(500000);
	}
	*/
}

void extractFlash0Archive(){

    if (ark_config==NULL) return;

    char* dest_path = "flash0:/";
    unsigned char buf[0x10000];
    int bufsize = sizeof(buf);
    int path_len = strlen(dest_path);
    static char archive[ARK_PATH_SIZE];
    static char filepath[ARK_PATH_SIZE];
    static char filename[ARK_PATH_SIZE];
    strcpy(filepath, dest_path);
    strcpy(archive, ark_config->arkpath);
    strcat(archive, FLASH0_ARK);

    PRTSTR1("Extracting %s", archive);
    
    int fdr = k_tbl->KernelIOOpen(archive, PSP_O_RDONLY, 0777);
    
    if (fdr>=0){
        open_flash();
        int filecount;
        k_tbl->KernelIORead(fdr, &filecount, sizeof(filecount));
        PRTSTR1("Extracting %d files", filecount);
        for (int i=0; i<filecount; i++){
            filepath[path_len] = '\0';
            int filesize;
            k_tbl->KernelIORead(fdr, &filesize, sizeof(filesize));

            char namelen;
            k_tbl->KernelIORead(fdr, &namelen, sizeof(namelen));

            k_tbl->KernelIORead(fdr, filename, namelen);
            filename[namelen] = '\0';
            
            if (strstr(filename, "psvbt")!=NULL // PS Vita btcnf replacement, not used on PSP
                    || strstr(filename, "660")!=NULL // PSP 6.60 modules are used on Vita for better compatibility, not needed on PSP
                    || strcmp(filename, "/fake.cso")==0){ // fake.cso used on Vita to simulate UMD drive
                k_tbl->IoLseek(fdr, filesize, 1); // skip file
            }
            else{
                strcat(filepath, (filename[0]=='/')?filename+1:filename);
                PRTSTR1("Extracting file %s", filepath);
                int fdw = k_tbl->KernelIOOpen(filepath, PSP_O_WRONLY|PSP_O_CREAT|PSP_O_TRUNC, 0777);
                if (fdw < 0){
                    PRTSTR("ERROR: could not open file for writing");
                    k_tbl->KernelIOClose(fdr);
                    while(1){};
                    return;
                }
                while (filesize>0){
                    int read = k_tbl->KernelIORead(fdr, buf, (filesize>bufsize)?bufsize:filesize);
                    k_tbl->KernelIOWrite(fdw, buf, read);
                    filesize -= read;
                }
                k_tbl->KernelIOClose(fdw);
            }
        }
        k_tbl->KernelIOClose(fdr);
        k_tbl->KernelIORemove(archive);
        close_flash();
    }
    else{
        PRTSTR("Nothing to be done");
    }
    PRTSTR("Done");
    k_tbl->KernelExitThread(0);
    //doBreakpoint();
}

void flashPatch(){
	if (IS_PSP(ark_config->exec_mode)){ // on PSP, extract FLASH0.ARK into flash0
	    SceUID kthreadID = k_tbl->KernelCreateThread( "arkflasher", (void*)KERNELIFY(&extractFlash0Archive), 25, 0x12000, 0, NULL);
	    if (kthreadID >= 0){
		    k_tbl->KernelStartThread(kthreadID, 0, NULL);
		    k_tbl->waitThreadEnd(kthreadID, NULL);
		    k_tbl->KernelDeleteThread(kthreadID);
	    }
    }
    else{ // Patching flash0 on Vita
	    // Redirect KERMIT_CMD_ERROR_EXIT loadFlash function
	    u32 knownnids[2] = { 0x3943440D, 0x0648E1A3 /* 3.3X */ };
	    u32 swaddress = 0;
	    int i;
	    for (i = 0; i < 2; i++){
		    swaddress = findFirstJALForFunction("sceKermitPeripheral_Driver", "sceKermitPeripheral_driver", knownnids[i]);
		    if (swaddress != 0)
			    break;
	    }
	    if (swaddress){
	        _sw(JUMP(flashLoadPatch), swaddress);
	        _sw(NOP, swaddress+4);
	    }
	}
}
