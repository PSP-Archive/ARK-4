#include <pspsdk.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <pspsuspend.h>
#include <psppower.h>
#include <pspreg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include <globals.h>
#include <libpsardumper.h>
#include <pspdecrypt.h>
#include <kubridge.h>
#include <vlf.h>

#include <loader/dc/msipl/mainbinex/payload.h>
#include <loader/dc/tmctrl/tmctrl.h>
#include "tm_msipl.h"
#include "tm_mloader.h"

#include "pspbtcnf_dc.h"
#include "pspbtcnf_02g_dc.h"
#include "dcman.h"
#include "ipl_update.h"
#include "iop.h"
#include "pspdecryptmod.h"
#include "intrafont.h"
#include "resurrection.h"
#include "vlf.h"

PSP_MODULE_INFO("VResurrection_Manager", 0x800, 2, 0);
PSP_MAIN_THREAD_ATTR(0);

#define UPDATER_VER_STR "6.61"
#define UPDATER "661.PBP"

#define PRX_SIZE_661				5718512
#define LFLASH_FATFMT_UPDATER_SIZE	0x28A0
#define NAND_UPDATER_SIZE			0x39D0
#define LFATFS_UPDATER_SIZE			0xE970

#define PSAR_SIZE_661	26848656

char boot_path[256];

char error_msg[256];
static int g_running = 0, g_cancel = 0;
static SceUID install_thid = -1;

////////////////////////////////////////////////////////////////////
// big buffers for data. Some system calls require 64 byte alignment

// big enough for the full PSAR file
static u8 *g_dataPSAR;
static u8 *g_dataOut;
static u8 *g_dataOut2;   

static int PSAR_BUFFER_SIZE;
static const int SMALL_BUFFER_SIZE = 2500000;

static char flash_table[4][0x4000];
static int flash_table_size[4];

// Gui vars
int begin_install_text;
int install_status;
char st_text[256];
int progress_bar, progress_text;
char pg_text[20];

void ErrorExit(int milisecs, char *fmt, ...)
{
	va_list list;
	char msg[256];	

	va_start(list, fmt);
	vsprintf(msg, fmt, list);
	va_end(list);

	if (!g_running)
	{
		vlfGuiMessageDialog(msg, VLF_MD_TYPE_ERROR | VLF_MD_BUTTONS_NONE);
		sceKernelDelayThread(milisecs*1000);
		sceKernelExitGame();
	}
	else
	{
		strcpy(error_msg, msg);
		g_running = 0;
	}
	
	sceKernelSleepThread();
}

////////////////////////////////////////////////////////////////////
// File helpers

int ReadFile(char *file, int seek, void *buf, int size);
int WriteFile(char *file, void *buf, int size);
void *malloc64(int size);

static int FindTablePath(char *table, int table_size, char *number, char *szOut) {
	int i, j, k;
	for (i = 0; i < table_size - 5; i++) {
		if (strncmp(number, table+i, 5) == 0) {
			for (j = 0, k = 0; ; j++, k++) {
				if (table[i+j+6] < 0x20) {
					szOut[k] = 0;
					break;
				}

				if (table[i+5] == '|' && strncmp(table+i+6, "flash", 5) == 0 && j == 6) {
					szOut[6] = ':';
					szOut[7] = '/';
					k++;
				} else if (table[i+5] == '|' && strncmp(table+i+6, "ipl", 3) == 0 && j == 3) {
					szOut[3] = ':';
					szOut[4] = '/';
					k++;
				} else
				{
					szOut[k] = table[i+j+6];
				}
			}

			return 1;
		}
	}

	return 0;
}

int get_registry_value(const char *dir, const char *name, int *val);


int LoadStartModule(char *module, int partition)
{
	SceUID mod = kuKernelLoadModule(module, 0, NULL);

	if (mod < 0)
		return mod;

	return sceKernelStartModule(mod, 0, NULL, NULL, NULL);
}

int last_percentage = -1;
int last_time;

int DoProgressUpdate(void *param)
{
	vlfGuiProgressBarSetProgress(progress_bar, last_percentage);
	vlfGuiSetText(progress_text, pg_text);
	
	return VLF_EV_RET_REMOVE_HANDLERS;
}

void ClearProgress()
{
	last_time = 0;
	last_percentage = -1;
}

void SetProgress(int percentage, int force)
{
	int st =  sceKernelGetSystemTimeLow();
	
	if (force || (percentage > last_percentage && st >= (last_time+520000)))
	{		
		sprintf(pg_text, "%d%%", percentage);
		last_percentage = percentage;
		last_time = st;
		vlfGuiAddEventHandler(0, -2, DoProgressUpdate, NULL);
	}
}

int DoStatusUpdate(void *param)
{
	vlfGuiSetText(install_status, st_text);
	return VLF_EV_RET_REMOVE_HANDLERS;
}

void SetStatus(char *status)
{
	strcpy(st_text, status);
	vlfGuiAddEventHandler(0, -2, DoStatusUpdate, NULL);
}

void SetPSARProgress(int value, int max, int force)
{
	u32 prog;

	prog = ((88 * value) / max) + 7;

	SetProgress(prog, force);
}

void CancelInstall()
{
	install_thid = -1;
	g_running = 0;
	sceKernelExitDeleteThread(0);
}

int is5Dnum(char *str) {
	int len = strlen(str);
	if (len != 5)
		return 0;

	int i;
	for (i = 0; i < len; i++) {
		if (str[i] < '0' || str[i] > '9')
			return 0;
	}

	return 1;
}

// TODO don't duplicate this
int isVitaFile(char* filename){
    return (strstr(filename, "psv")!=NULL // PS Vita btcnf replacement, not used on PSP
            || strstr(filename, "660")!=NULL // PSP 6.60 modules can be used on Vita, not needed for PSP
            || strstr(filename, "vita")!=NULL // Vita modules
            || strcmp(filename, "/fake.cso")==0 // fake.cso used on Vita to simulate UMD drive when no ISO available
    );
}

void extractFlash0Archive()
{
	int buf_size = 16 * 1024;
	char *archive = DEFAULT_ARK_FOLDER "/" FLASH0_ARK;
    char* dest_path = ARK_DC_PATH;
    unsigned char buf[buf_size];
    int path_len = strlen(dest_path)+1;
    static char filepath[ARK_PATH_SIZE];
    static char filename[ARK_PATH_SIZE];
    strcpy(filepath, dest_path);
	strcat(filepath, "/");
    
    int fdr = sceIoOpen(archive, PSP_O_RDONLY, 0777);
    
    if (fdr>=0){
        int filecount;
        sceIoRead(fdr, &filecount, sizeof(filecount));
        for (int i=0; i<filecount; i++){
            filepath[path_len] = '\0';
            int filesize;
            sceIoRead(fdr, &filesize, sizeof(filesize));

            unsigned char namelen;
            sceIoRead(fdr, &namelen, sizeof(namelen));

            sceIoRead(fdr, filename, namelen);
            filename[namelen] = '\0';
            
			if (isVitaFile(filename)){ // check if file is not needed on PSP
                sceIoLseek(fdr, filesize, 1); // skip file
            }
            else{
				strcat(filepath, (filename[0]=='/')?filename+1:filename);
				int fdw = sceIoOpen(filepath, PSP_O_WRONLY|PSP_O_CREAT|PSP_O_TRUNC, 0777);
				if (fdw < 0){
					sceIoClose(fdr);
					ErrorExit(1000, "Error writing %s.\n", filepath);
				}
				while (filesize>0){
					int read = sceIoRead(fdr, buf, (filesize>buf_size)?buf_size:filesize);
					sceIoWrite(fdw, buf, read);
					filesize -= read;
				}
				sceIoClose(fdw);

				if (i == filecount/2)
				{
					SetProgress(96, 1);
				}
				else if (i == filecount - 1)
				{
					SetProgress(97, 1);
				}
			}
        }
        sceIoClose(fdr);
    }
    else{
        ErrorExit(1000, "Unable to open " FLASH0_ARK "\n");
    }
}

void CopyFile(char *src, char *dest)
{
	SceUID fdi = sceIoOpen(src, PSP_O_RDONLY, 0);
	if (fdi < 0)
	{
		ErrorExit(1000, "Error opening %s: 0x%08X", src, fdi);
	}
	
	SceUID fdo = sceIoOpen(dest, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	if (fdo < 0)
	{
		ErrorExit(1000, "Error opening %s: 0x%08X", dest, fdo);
	}

	int read = sceIoRead(fdi, g_dataOut, SMALL_BUFFER_SIZE);
	if (read < 0)
	{
		ErrorExit(1000, "Error reading %s", src);
	}
	if (!read)
		goto exit;

	if (sceIoWrite(fdo, g_dataOut, read) < 0)
	{
		ErrorExit(1000, "Error writing %s", dest);
	}

exit:	
	sceIoClose(fdi);
	sceIoClose(fdo);
}

void WriteSavedataFiles()
{
	char src[260];
	char dest[260];
	int fd, ret;
	SceIoDirent curFile;

	strcpy(src, boot_path);
	strcat(src, "/" DEFAULT_ARK_FOLDER);

	sceIoMkdir(ARK_DC_PATH "/" DEFAULT_ARK_FOLDER, 0777);

	fd = sceIoDopen(src);
	if(fd >= 0)
	{
		do {
			memset(&curFile, 0, sizeof(SceIoDirent));

			ret = sceIoDread(fd, &curFile);

			if (ret > 0) {
				if (FIO_S_ISREG(curFile.d_stat.st_mode)) {	
					strcpy(src, boot_path);
					strcat(src, "/" DEFAULT_ARK_FOLDER "/");
					strcat(src, curFile.d_name);

					strcpy(dest, ARK_DC_PATH "/" DEFAULT_ARK_FOLDER "/");
					strcat(dest, curFile.d_name);

					CopyFile(src, dest);
				}
			}
		} while (ret > 0);
	}
	else {
		ErrorExit(1000, "Unable to copy ARK Savedata folder!.\n");
	}
}

void ExtractPrxs(int cbFile, SceUID fd)
{
	int psar_pos = 0;
	
	if (pspPSARInit(g_dataPSAR, g_dataOut, g_dataOut2) < 0)
	{
		ErrorExit(1000, "pspPSARInit failed!.\n");
	}   

	int error = 0;

    while (1)
	{
		char name[128];
		int cbExpanded;
		int pos;
		int signcheck;

		int res = pspPSARGetNextFile(g_dataPSAR, cbFile, g_dataOut, g_dataOut2, name, &cbExpanded, &pos, &signcheck);

		if (res < 0)
		{
			if (error)			
				ErrorExit(1000, "PSAR decode error, pos=0x%08X.\n", pos);

			int dpos = pos-psar_pos;
			psar_pos = pos;
			
			error = 1;
			memmove(g_dataPSAR, g_dataPSAR+dpos, PSAR_BUFFER_SIZE-dpos);

			if (sceIoRead(fd, g_dataPSAR+(PSAR_BUFFER_SIZE-dpos), dpos) <= 0)
			{
				ErrorExit(1000, "Error reading PBP.\n");
			}

			pspPSARSetBufferPosition(psar_pos);

			continue;
		}
		else if (res == 0) /* no more files */
		{
			break;
		}

		if (cbExpanded > 0)
		{
			char szDataPath[128];

			if (is5Dnum(name)) {
				int num = atoi(name);

				// Files from 01g-02g
				if (num >= 1 && num <= 2) {
					flash_table_size[num] = pspDecryptTable(g_dataOut2, g_dataOut, cbExpanded, 4);
					if (flash_table_size[num] <= 0) {
						ErrorExit(1000, "Cannot decrypt %02dg table.\n", num);
					}

					memcpy(flash_table[num], g_dataOut2, flash_table_size[num]);

					error = 0;
					continue;
				} else {
					if (num > 12) {
						int found = 0;

						int i;
						for (i = 1; i <= 3; i++) {
							if (flash_table_size[i] > 0) {
								found = FindTablePath(flash_table[i], flash_table_size[i], name, name);
								if (found)
									break;
							}
						}

						if (found) {
							if (strncmp(name, "flash0", 6) == 0) {
								sprintf(szDataPath, ARK_DC_PATH "/%s", name + 8);

								res = WriteFile(szDataPath, g_dataOut2, cbExpanded);
								if (res != cbExpanded) {
									ErrorExit(1000, "Error writing %s.\n", szDataPath);
								}
							}
							else if (strstr(name, "ipl") == name)
							{
								int is2g = (strstr(name, "02g") != NULL);

								if (is2g)
								{
									cbExpanded = pspDecryptPRX(g_dataOut2, g_dataOut, cbExpanded);
									if (cbExpanded <= 0)
									{
										ErrorExit(1000, "Cannot pre-decrypt 2000 IPL\n");
									}
									else
									{
										memcpy(g_dataOut2, g_dataOut, cbExpanded);
									}	
								}
								else {
									int is1g = (strstr(name, "01g") != NULL);

									if (!is1g) {
										ErrorExit(1000, "Unexpected ipl! %s\n", name);
									}
								}

								if (is2g)
								{
									if (WriteFile(ARK_DC_PATH "/ipl_02g.bin", g_dataOut2, cbExpanded) != (cbExpanded))
									{
										ErrorExit(1000, "Error writing 02g ipl.\n");
									}
								}
								else
								{
									if (WriteFile(ARK_DC_PATH "/ipl_01g.bin", g_dataOut2, cbExpanded) != (cbExpanded))
									{
										ErrorExit(1000, "Error writing 01g ipl.\n");
									}
								}

								int cb1 = pspDecryptIPL1(g_dataOut2, g_dataOut, cbExpanded);
								if (cb1 < 0)
								{
									ErrorExit(1000, "Error in IPL decryption.\n");
								}

								int cb2 = pspLinearizeIPL2(g_dataOut, g_dataOut2, cb1);
								if (cb2 < 0)
								{
									ErrorExit(1000, "Error in IPL Linearize.\n");
								}

								sceKernelDcacheWritebackAll();

								if (is2g)
								{
									if (WriteFile(ARK_DC_PATH "/nandipl_02g.bin", g_dataOut2, cb2) != (cb2))
									{
										ErrorExit(1000, "Error writing 02g ipl.\n");
									}
								}
								else
								{
									if (WriteFile(ARK_DC_PATH "/nandipl_01g.bin", g_dataOut2, cb2) != (cb2))
									{
										ErrorExit(1000, "Error writing 01g ipl.\n");
									}
								}
							}
						} else {
							error = 0;
							continue;
						}
					} else {
						error = 0;
						continue;
					}
				}
			}		
		}

		SetPSARProgress(pos, cbFile, 0);
		scePowerTick(0);
		error = 0;

		if (g_cancel)
		{
			sceIoClose(fd);
			CancelInstall();
			return;
		}
	}

	SetPSARProgress(1, 1, 1);
}

static void DescrambleUpdaterModule(u8 *inBuf, u8 *outBuf, int size)
{
	int a = 11;
	int b = 7;
	int c = 6;
	for (int i = 0; i < size; i++)
	{
		int d = (a + b) & 0xff;
		if (c + 1 == 0) {
			ErrorExit(1000, "Error descrambling updater module\n");
		}

		outBuf[i] = (u8)d ^ inBuf[i];
		a = b % (c + 1) & 0xff;
		b = c;
		c = d;
	}
}

static void Extract661Modules()
{
	int size;
	u8 pbp_header[0x28];
	u8 *mod_buf;

	SetStatus("Extracting " UPDATER_VER_STR " updater modules...");
		
	if (ReadFile("ms0:/" UPDATER, 0, pbp_header, sizeof(pbp_header)) != sizeof(pbp_header))
	{
		ErrorExit(1000, "Error reading " UPDATER " at root.\n");
	}

	if (g_cancel)
	{
		CancelInstall();
	}

	if (ReadFile("ms0:/" UPDATER, *(u32 *)&pbp_header[0x20], g_dataPSAR, PRX_SIZE_661) != PRX_SIZE_661)
	{
		ErrorExit(1000, "Invalid " UPDATER ".\n");
	}

	if (g_cancel)
		CancelInstall();

	sceKernelDelayThread(10000);

	size = pspDecryptPRX(g_dataPSAR, g_dataPSAR, PRX_SIZE_661);
	if (size <= 0)
	{
		ErrorExit(1000, "Error decrypting " UPDATER_VER_STR " updater.\n");
	}

	if (g_cancel)
		CancelInstall();

	sceKernelDelayThread(10000);

	size = pspDecryptPRX(g_dataPSAR+0x4E6380, g_dataOut, LFLASH_FATFMT_UPDATER_SIZE);
	if (size <= 0)
	{
		ErrorExit(1000, "Error decoding lflash_fatfmt_updater.prx\n");
	}

	if (WriteFile(ARK_DC_PATH "/kd/lflash_fatfmt_updater.prx", g_dataOut, size) != size)
	{
		ErrorExit(1000, "Error writing lflash_fatfmt_updater.prx.\n");
	}

	mod_buf = (u8 *)g_dataPSAR+0x21880;

	DescrambleUpdaterModule(mod_buf, mod_buf, LFATFS_UPDATER_SIZE);

	if (g_cancel)
		CancelInstall();

	sceKernelDelayThread(10000);

	size = pspDecryptPRX(mod_buf, g_dataOut, LFATFS_UPDATER_SIZE);
	if (size <= 0)
	{
		ErrorExit(1000, "Error decoding lfatfs_updater.prx\n");
	}

	if (WriteFile(ARK_DC_PATH "/kd/lfatfs_updater.prx", g_dataOut, size) != size)
	{
		ErrorExit(1000, "Error writing lfatfs_updater.prx.\n");
	}

	mod_buf = (u8 *)g_dataPSAR+0x30200;

	DescrambleUpdaterModule(mod_buf, mod_buf, NAND_UPDATER_SIZE);

	if (g_cancel)
		CancelInstall();

	sceKernelDelayThread(10000);

	size = pspDecryptPRX(mod_buf, g_dataOut, NAND_UPDATER_SIZE);
	if (size <= 0)
	{
		ErrorExit(1000, "Error decoding emc_sm_updater.prx\n");
	}
	
	if (WriteFile(ARK_DC_PATH "/kd/emc_sm_updater.prx", g_dataOut, size) != size)
	{
		ErrorExit(1000, "Error writing emc_sm_updater.prx.\n");
	}
}

static void Extract661PSAR()
{
	SceUID fd;
	
	SetStatus("Extracting " UPDATER_VER_STR " modules... ");
	
	fd = sceIoOpen("ms0:/" UPDATER, PSP_O_RDONLY, 0);
	if (fd < 0)
	{
		ErrorExit(1000, "Incorrect or inexistant " UPDATER " at root.\n");
	}
	
	sceIoLseek32(fd, 0x577635, PSP_SEEK_SET);
	sceIoRead(fd, g_dataPSAR, PSAR_BUFFER_SIZE);	

	if (g_cancel)
	{
		sceIoClose(fd);
		CancelInstall();
	}

	sceKernelDelayThread(10000);

	if (g_cancel)
	{
		sceIoClose(fd);
		CancelInstall();
	}

	ExtractPrxs(PSAR_SIZE_661, fd);
	sceIoClose(fd);
}

static void WriteTimeMachineFiles()
{
	if (WriteFile(ARK_DC_PATH "/tmctrl.prx", tmctrl, size_tmctrl) != size_tmctrl)
		ErrorExit(1000, "Error writing tmctrl.prx");

	if (WriteFile(ARK_DC_PATH "/payload_01g.bin", ms_ipl_payload, size_ms_ipl_payload) != size_ms_ipl_payload)
		ErrorExit(1000, "Error writing payload_01g.bin");

	if (WriteFile(ARK_DC_PATH "/payload_02g.bin", ms_ipl_payload, size_ms_ipl_payload) != size_ms_ipl_payload)
		ErrorExit(1000, "Error writing payload_02g.bin");

	if (WriteFile(ARK_DC_PATH "/tm_mloader.bin", tm_mloader, size_tm_mloader) != size_tm_mloader)
		ErrorExit(1000, "Error writing payload_02g.bin");
}

static void WriteDCFiles()
{
	if (WriteFile(ARK_DC_PATH "/kd/pspbtcnf_dc.bin", pspbtcnf_dc, size_pspbtcnf_dc) != size_pspbtcnf_dc)
		ErrorExit(1000, "Error writing pspbtcnf_01g_dc.bin");

	if (WriteFile(ARK_DC_PATH "/kd/pspbtcnf_02g_dc.bin", pspbtcnf_02g_dc, size_pspbtcnf_02g_dc) != size_pspbtcnf_02g_dc)
		ErrorExit(1000, "Error writing pspbtcnf_02g_dc.bin");

	if (WriteFile(ARK_DC_PATH "/kd/dcman.prx", dcman, size_dcman) != size_dcman)
		ErrorExit(1000, "Error writing dcman.prx");

	if (WriteFile(ARK_DC_PATH "/kd/ipl_update.prx", ipl_update, size_ipl_update) != size_ipl_update)
		ErrorExit(1000, "Error writing ipl_update.prx");

	if (WriteFile(ARK_DC_PATH "/kd/iop.prx", iop, size_iop) != size_iop)
		ErrorExit(1000, "Error writing iop.prx");

	if (WriteFile(ARK_DC_PATH "/kd/pspdecrypt.prx", pspdecrypt, size_pspdecrypt) != size_pspdecrypt)
		ErrorExit(1000, "Error writing pspdecrypt.prx");

	if (WriteFile(ARK_DC_PATH "/vsh/module/intrafont.prx", intrafont, size_intrafont) != size_intrafont)
		ErrorExit(1000, "Error writing intrafont.prx");

	if (WriteFile(ARK_DC_PATH "/vsh/module/resurrection.prx", resurrection, size_resurrection) != size_resurrection)
		ErrorExit(1000, "Error writing resurrection.prx");
	
	if (WriteFile(ARK_DC_PATH "/vsh/module/vlf.prx", vlf, size_vlf) != size_vlf)
		ErrorExit(1000, "Error writing vlf.prx");
}

int ReadSector(int sector, void *buf, int count)
{
	SceUID fd = sceIoOpen("msstor0:", PSP_O_RDONLY, 0);
	if (fd < 0)
		return fd;

	sceIoLseek(fd, sector*512, PSP_SEEK_SET);
	int read = sceIoRead(fd, buf, count*512);

	if (read > 0)
		read /= 512;

	sceIoClose(fd);
	return read;
}

int WriteSector(int sector, void *buf, int count)
{
	SceUID fd = sceIoOpen("msstor0:", PSP_O_WRONLY, 0);
	if (fd < 0)
		return fd;

	sceIoLseek(fd, sector*512, PSP_SEEK_SET);
	int written = sceIoWrite(fd, buf, count*512);

	if (written > 0)
		written /= 512;

	sceIoClose(fd);
	return written;
}

char *GetKeyName(u32 key)
{
	if (key == PSP_CTRL_SELECT)
		return "SELECT";

	if (key == PSP_CTRL_START)
		return "START";

	if (key == PSP_CTRL_UP)
		return "UP";

	if (key == PSP_CTRL_DOWN)
		return "DOWN";

	if (key == PSP_CTRL_LEFT)
		return "LEFT";

	if (key == PSP_CTRL_RIGHT)
		return "RIGHT";

	if (key == PSP_CTRL_SQUARE)
		return "SQUARE";

	if (key == PSP_CTRL_CIRCLE)
		return "CIRCLE";

	if (key == PSP_CTRL_CROSS)
		return "CROSS";

	if (key == PSP_CTRL_LTRIGGER)
		return "L";

	if (key == PSP_CTRL_TRIANGLE)
		return "TRIANGLE";

	if (key == PSP_CTRL_RTRIGGER)
		return "R";

	return NULL;
}

int install_iplloader()
{
	int res;
	int abs_sec,ttl_sec;
	int signature;
	int type;
	int free_sectors;

	SetStatus("Installing iplloader... ");

	res = ReadSector(0, g_dataOut, 1);
	if (res != 1)
	{
		ErrorExit(1000, "Error 0x%08X in ReadSector.\n", res);
	}

	abs_sec   = g_dataOut[0x1c6]|(g_dataOut[0x1c7]<<8)|(g_dataOut[0x1c8]<<16)|(g_dataOut[0x1c9]<<24);
	ttl_sec   = g_dataOut[0x1ca]|(g_dataOut[0x1cb]<<8)|(g_dataOut[0x1cc]<<16)|(g_dataOut[0x1cd]<<24);
	signature = g_dataOut[0x1fe]|(g_dataOut[0x1ff]<<8);
	type      = g_dataOut[0x1c2];

	if (signature != 0xAA55)
	{
		ErrorExit(1000, "Invalid signature 0x%04X\n", signature);
	}

	free_sectors = (abs_sec-0x10);
	if (free_sectors < 32)
	{
		ErrorExit(1000, "The install failed.\n"
			            "Contact the Sony Computer Entertainment technical support line for assistance.\n"
						"(ffffffff)\n\n"
						"Just kidding. Use Team C+D mspformat pc tool and run the installer again.");
	}

	if (g_cancel)
		CancelInstall();

	res = WriteSector(0x10, tm_msipl, 32);
	if (res != 32)
	{
		ErrorExit(1000, "Error 0x%08X in WriteSector.\n", res);
	}

	char *default_config = "NOTHING = \"ms0:/TM/DCARK/tm_mloader.bin\";";

	SceIoStat stat;

	if (sceIoGetstat("ms0:/TM/config.txt", &stat) < 0)
	{
		WriteFile("ms0:/TM/config.txt", default_config, strlen(default_config));
		return 0;
	}
	
	return 1;
}

int OnProgramFinish(int enter)
{
	if (enter)
	{
		g_running = 0;
		return VLF_EV_RET_REMOVE_HANDLERS;
	}

	return VLF_EV_RET_NOTHING;
}

char text[256];

int OnInstallFinish(void *param)
{
	int key_install = *(int *)param;
	
	vlfGuiCancelBottomDialog();
	vlfGuiRemoveProgressBar(progress_bar);
	vlfGuiRemoveText(progress_text);
	vlfGuiRemoveText(install_status);

	if (key_install)
	{
		install_status = vlfGuiAddText(80, 90, "");
	}
	else
	{
		install_status = vlfGuiAddText(150, 90, "");
	}	
	
	
	vlfGuiSetText(install_status, text);
	vlfGuiCustomBottomDialog(NULL, "Exit", 1, 0, VLF_DEFAULT, OnProgramFinish);
	return VLF_EV_RET_REMOVE_HANDLERS;
}

int OnPaintListenKeys(void *param)
{
	vlfGuiCancelBottomDialog();
	vlfGuiRemoveProgressBar(progress_bar);
	vlfGuiRemoveText(progress_text);
	vlfGuiRemoveText(install_status);

	install_status = vlfGuiAddText(80, 90, "");
	vlfGuiSetText(install_status, text);

	progress_bar = -1;
	progress_text = -1;

	return VLF_EV_RET_REMOVE_HANDLERS;
}

int install_thread(SceSize args, void *argp)
{
	int i;

	sceIoChdir(boot_path);
	
	vlfGuiRemoveText(begin_install_text);
	install_status = vlfGuiAddText(80, 100, "Creating directories...");

	progress_bar = vlfGuiAddProgressBar(136);	
	progress_text = vlfGuiAddText(240, 148, "0%");
	vlfGuiSetTextAlignment(progress_text, VLF_ALIGNMENT_CENTER);

	sceIoMkdir("ms0:/TM", 0777);
	sceIoMkdir(ARK_DC_PATH, 0777);
	sceIoMkdir(ARK_DC_PATH "/codepage", 0777);
	sceIoMkdir(ARK_DC_PATH "/data", 0777);
	sceIoMkdir(ARK_DC_PATH "/data/cert", 0777);
	sceIoMkdir(ARK_DC_PATH "/data/cert", 0777);
	sceIoMkdir(ARK_DC_PATH "/dic", 0777);
	sceIoMkdir(ARK_DC_PATH "/font", 0777);
	sceIoMkdir(ARK_DC_PATH "/gps", 0777);
	sceIoMkdir(ARK_DC_PATH "/kd", 0777);
	sceIoMkdir(ARK_DC_PATH "/kd/resource", 0777);
	sceIoMkdir(ARK_DC_PATH "/net", 0777);
	sceIoMkdir(ARK_DC_PATH "/net/http", 0777);
	sceIoMkdir(ARK_DC_PATH "/vsh", 0777);
	sceIoMkdir(ARK_DC_PATH "/vsh/etc", 0777);
	sceIoMkdir(ARK_DC_PATH "/vsh/module", 0777);
	sceIoMkdir(ARK_DC_PATH "/vsh/resource", 0777);
	sceIoMkdir(ARK_DC_PATH "/vsh/theme", 0777);
	sceIoMkdir(ARK_DC_PATH "/registry", 0777);

	if (g_cancel)
		CancelInstall();
	
	sceKernelDelayThread(800000);
	SetProgress(1, 1);	

	Extract661Modules();
	if (g_cancel)
		CancelInstall();

	sceKernelDelayThread(250000);
	SetProgress(7, 1);

	Extract661PSAR();
	if (g_cancel)
		CancelInstall();

	sceKernelDelayThread(250000);
	SetProgress(95, 1);

	SetStatus("Writing custom modules...");

	extractFlash0Archive();

	sceKernelDelayThread(250000);
	SetProgress(98, 1);

	SetStatus("Writing Time Machine files...");

	WriteTimeMachineFiles();

	SetStatus("Writing DC files...");

	WriteDCFiles();

	SetStatus("Writing Savedata files...");

	WriteSavedataFiles();

	SetStatus("Copying registry...");

	i = ReadFile("flash1:/registry/system.dreg", 0, g_dataOut, SMALL_BUFFER_SIZE);
	if (i <= 0)
	{
		ErrorExit(1000, "Error reading system.dreg.\n");
	}

	if (g_cancel)
		CancelInstall();

	if (WriteFile(ARK_DC_PATH "/registry/system.dreg", g_dataOut, i) != i)
	{
		ErrorExit(1000, "Error writing system.dreg.\n");
	}

	i = ReadFile("flash1:/registry/system.ireg", 0, g_dataOut, SMALL_BUFFER_SIZE);
	if (i <= 0)
	{
		ErrorExit(1000, "Error reading system.ireg.\n");
	}

	if (g_cancel)
		CancelInstall();
	
	if (WriteFile(ARK_DC_PATH "/registry/system.ireg", g_dataOut, i) != i)
	{
		ErrorExit(1000, "Error writing system.ireg.\n");
	}

	i = ReadFile("flash2:/act.dat", 0, g_dataOut, SMALL_BUFFER_SIZE);
	if (i > 0)
	{
		if (WriteFile(ARK_DC_PATH "/act.dat", g_dataOut, i) != i)
		{
			ErrorExit(1000, "Error writing act.dat.\n");
		}
	}

	if (g_cancel)
		CancelInstall();

	sceKernelDelayThread(380000);
	SetProgress(99, 1);

	int key_install = install_iplloader();
	if (g_cancel)
		CancelInstall();

	install_thid = -1;
	sceKernelDelayThread(380000);
	SetProgress(100, 1);
	sceKernelDelayThread(1200000);

	strcpy(text, "");	

	if (key_install)
	{
		SceCtrlData pad;
		char buf[256];		
		
		strcpy(text, "Please keep pressed for some seconds\n"
			         "the key/s which you want to use to\n"
					 "boot DC-ARK... ");
		
		vlfGuiAddEventHandler(0, -1, OnPaintListenKeys, NULL);	
		
		while (1)
		{		
			sceKernelDelayThread(3000000);
			
			sceCtrlPeekBufferPositive(&pad, 1);
			
			if (pad.Buttons != 0)
				break;
		}

		strcpy(buf, "");

		int first = 1;

		for (i = 0; i < 32; i++)
		{
			if ((pad.Buttons & (1 << i)))
			{
				if (GetKeyName(1 << i))
				{
					if (!first)
						strcat(buf, "+");
					else
						first = 0;
					
					strcat(buf, GetKeyName(1 << i));
				}
			}
		}

		strcat(text, buf);
		SetStatus(text);
		sceKernelDelayThread(850000);

		strcat(buf, " = \"ms0:/TM/DCARK/tm_mloader.bin\";\r\n");
		memcpy(g_dataOut, buf, strlen(buf));

		int size = ReadFile("ms0:/TM/config.txt", 0, g_dataOut+strlen(buf), SMALL_BUFFER_SIZE);

		if (size >= 0)
		{
			WriteFile("ms0:/TM/config.txt", g_dataOut, size+strlen(buf));
		}
		
		strcat(text, "\n");
		sceKernelDelayThread(350000);
	}
	
	strcat(text, "\n");
	strcat(text, "Installation completed.");
	sceKernelDelayThread(100000);
	vlfGuiAddEventHandler(0, -1, OnInstallFinish, &key_install);
	sceKernelDelayThread(800000);

	install_thid = -1;
	return sceKernelExitDeleteThread(0);
}

int OnCancelInstall(int enter)
{
	if (!enter)
	{
		if (vlfGuiMessageDialog("Are you sure you want to cancel?", VLF_MD_TYPE_NORMAL | VLF_MD_BUTTONS_YESNO | VLF_MD_INITIAL_CURSOR_NO) == VLF_MD_YES)
		{
			if (install_thid >= 0)
			{
				int is = install_status;

				install_status = -1;				
				vlfGuiSetText(is, "Cancelling...");
				g_cancel = 1;				
			}
			else
			{
				g_running = 0;
			}

			return VLF_EV_RET_REMOVE_HANDLERS;
		}
	}
	
	return VLF_EV_RET_NOTHING;
}

int OnInstallBegin(void *param)
{
	install_thid = sceKernelCreateThread("install_thread", install_thread, 0x18, 0x10000, 0, NULL);
	if (install_thid >= 0)
	{
		sceKernelStartThread(install_thid, 0, 0);
	}

	return VLF_EV_RET_REMOVE_HANDLERS;
}

int app_main()
{
	int theme;
	
	vlfGuiSystemSetup(1, 1, 1);

	if (sceKernelDevkitVersion() < 0x02070110)
	{
		ErrorExit(1000, "This program requires 2.71 or higher.\n",
			             "If you are in a cfw, please reexecute psardumper on the higher kernel.\n");
	}

	SceUID mod = LoadStartModule("libpsardumper.prx", PSP_MEMORY_PARTITION_KERNEL);
	if (mod < 0)
	{
		ErrorExit(1000, "Error 0x%08X loading/starting libpsardumper.prx.\n", mod);
	}

	mod = LoadStartModule("pspdecrypt.prx", PSP_MEMORY_PARTITION_KERNEL);
	if (mod < 0)
	{
		ErrorExit(1000, "Error 0x%08X loading/starting pspdecrypt.prx.\n", mod);
	}

	if (get_registry_value("/CONFIG/SYSTEM/XMB/THEME", "custom_theme_mode", &theme))
	{	
		if (theme != 0)
		{
			ErrorExit(1000, "Your psp has a custom theme set.\n"
							"Turn the theme off before running this program.\n");
		}
	}

	memset(flash_table, 0, sizeof(flash_table));
	memset(flash_table_size, 0, sizeof(flash_table_size));

	g_dataOut = malloc64(SMALL_BUFFER_SIZE);
	g_dataOut2 = malloc64(SMALL_BUFFER_SIZE);

	if (!g_dataOut || !g_dataOut2)
	{
		ErrorExit(1000, "There is not enough RAM memory.\n");
	}

	PSAR_BUFFER_SIZE = (28*1024*1024);
	g_dataPSAR = malloc64(PSAR_BUFFER_SIZE);

	if (!g_dataPSAR)
	{
		PSAR_BUFFER_SIZE = 8*1024*1024;
		g_dataPSAR = malloc64(PSAR_BUFFER_SIZE);
	}

	if (!g_dataPSAR)
	{
		ErrorExit(1000, "There is not enough RAM memory.\n");
	}

	int tt = vlfGuiAddText(0, 0, "DC-ARK");
	int tp = vlfGuiAddPictureResource("update_plugin", "tex_update_icon", 0, 0);
	//int tp = vlfGuiAddPictureFile("m33.tga", 0, 0);
	vlfGuiChangeCharacterByButton('*', VLF_ENTER);
	begin_install_text = vlfGuiAddText(110, 118, "Press * to begin the installation");

	vlfGuiSetTitleBar(tt, tp, 1, 0);
	vlfGuiBottomDialog(VLF_DI_CANCEL, -1, 1, 0, VLF_DEFAULT, OnCancelInstall);
	vlfGuiAddEventHandler(PSP_CTRL_ENTER, 0, OnInstallBegin, NULL);

	g_running = 1;

	while (g_running)
	{
		vlfGuiDrawFrame();
	}

	if (error_msg[0])
	{
		if (install_thid >= 0)
		{
			sceKernelDeleteThread(install_thid);
			install_thid = -1;
		}
		
		vlfGuiMessageDialog(error_msg, VLF_MD_TYPE_ERROR | VLF_MD_BUTTONS_NONE);
		sceKernelDelayThread(1000*1000);
	}

	sceKernelExitGame();

   	return 0;
}


