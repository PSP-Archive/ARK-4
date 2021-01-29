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

#ifndef INFERNO_H
#define INFERNO_H

#define ISO_SECTOR_SIZE 2048
#define CISO_IDX_BUFFER_SIZE 0x200
#define CISO_DEC_BUFFER_SIZE 0x2000

#define MAX_FILES_NR 8

struct IoReadArg
{
	unsigned int offset; // 0
	unsigned char *address; // 4
	unsigned int size; // 8
};

extern unsigned int psp_model;
extern unsigned int psp_fw_version;
extern PspIoDrv g_iodrv;

extern SceUID g_umd_cbid;
extern int g_umd_error_status;
extern int g_drive_status;

extern const char *g_iso_fn;
extern int g_game_fix_type;
extern SceUID g_drive_status_evf;
extern void *g_sector_buf;
extern SceUID g_umd9660_sema_id;
extern int g_iso_opened;
extern SceUID g_iso_fd;
extern int g_total_sectors;
extern struct IoReadArg g_read_arg;
extern int g_disc_type;

extern void sceUmdSetDriveStatus(int status);

extern int powerEvtHandler(int ev_id, char *ev_name, void *param, int *result);

extern int isoOpen(void);
extern int isoRead(struct IoReadArg *args);
extern int isoReadStack(unsigned int offset, void *ptr, unsigned int data_len);

extern int infernoSetDiscType(int type);

#endif
