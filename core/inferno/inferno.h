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

#define MAX_FILES_NR 8

#define SAFE_FREE(p) \
do { \
    if(p != NULL) { \
        oe_free(p); \
        p = NULL; \
    } \
} while ( 0 )

struct IoReadArg {
    u32 offset; // 0
    u8 *address; // 4
    u32 size; // 8
};

extern u32 psp_fw_version;
extern PspIoDrv g_iodrv;

extern SceUID g_umd_cbid;
extern int g_umd_error_status;
extern int g_drive_status;

extern char g_iso_fn[255];
extern int g_game_fix_type;
extern SceUID g_drive_status_evf;
extern void *g_sector_buf;
extern SceUID g_umd9660_sema_id;
extern int g_iso_opened;
extern SceUID g_iso_fd;
extern int g_total_sectors;
extern struct IoReadArg g_read_arg;
extern int g_disc_type;
extern unsigned char umd_seek;
extern unsigned char umd_speed;
extern u32 last_read_offset;
extern u32 cur_offset;

extern void sceUmdSetDriveStatus(int status);

extern int power_event_handler(int ev_id, char *ev_name, void *param, int *result);

extern int iso_open(void);
extern int iso_read(struct IoReadArg *args);
extern int iso_cache_read(struct IoReadArg *args);
extern int iso_read_with_stack(u32 offset, void *ptr, u32 data_len);

extern int infernoSetDiscType(int type);
extern int infernoCacheInit(int cache_size, int cache_num, int partition);
extern int infernoCacheAdd(u32 pos, int len);
extern void infernoCacheSetPolicy(int policy);

#endif
