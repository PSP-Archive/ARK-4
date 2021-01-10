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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pspsdk.h>
#include <pspthreadman_kernel.h>
#include <pspumd.h>
#include "systemctrl.h"
#include "systemctrl_se.h"
#include "systemctrl_private.h"
#include "isoreader.h"
#include "printk.h"
#include "utils.h"
#include "strsafe.h"
#include "virtual_pbp.h"
#include "main.h"
#include "dirent_track.h"

typedef struct __attribute__((packed))
{
	u32 signature;
	u32 version;
	u32 fields_table_offs;
	u32 values_table_offs;
	int nitems;
} SFOHeader;

typedef struct __attribute__((packed))
{
	u16 field_offs;
	u8  unk;
	u8  type; // 0x2 -> string, 0x4 -> number
	u32 unk2;
	u32 unk3;
	u16 val_offs;
	u16 unk4;
} SFODir;

typedef struct _PBPEntry {
	u32 enabled;
	char *name;
} PBPEntry;

static PBPEntry pbp_entries[8] = {
	{ 1, "/PSP_GAME/PARAM.SFO" },
	{ 1, "/PSP_GAME/ICON0.PNG" },
	{ 1, "/PSP_GAME/ICON1.PMF" },
	{ 1, "/PSP_GAME/PIC0.PNG"  },
	{ 1, "/PSP_GAME/PIC1.PNG"  },
	{ 1, "/PSP_GAME/SND0.AT3"  },
	{ 0, "DATA.PSP"  }, // placeholder, never enable or delete it
	{ 0, "DATA.PSAR" }, // placeholder, never enable or delete it
};

static u8 virtualsfo[408] = {
	0x00, 0x50, 0x53, 0x46, 0x01, 0x01, 0x00, 0x00,
	0x94, 0x00, 0x00, 0x00, 0xE8, 0x00, 0x00, 0x00, 
	0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 
	0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x04, 0x02, 
	0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
	0x04, 0x00, 0x00, 0x00, 0x12, 0x00, 0x04, 0x02, 
	0x0A, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 
	0x08, 0x00, 0x00, 0x00, 0x1A, 0x00, 0x04, 0x02, 
	0x05, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 
	0x18, 0x00, 0x00, 0x00, 0x27, 0x00, 0x04, 0x04, 
	0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 
	0x20, 0x00, 0x00, 0x00, 0x36, 0x00, 0x04, 0x02,
	0x05, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 
	0x24, 0x00, 0x00, 0x00, 0x45, 0x00, 0x04, 0x04, 
	0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 
	0x2C, 0x00, 0x00, 0x00, 0x4C, 0x00, 0x04, 0x02, 
	0x40, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 
	0x30, 0x00, 0x00, 0x00, 0x42, 0x4F, 0x4F, 0x54, 
	0x41, 0x42, 0x4C, 0x45, 0x00, 0x43, 0x41, 0x54, 
	0x45, 0x47, 0x4F, 0x52, 0x59, 0x00, 0x44, 0x49, 
	0x53, 0x43, 0x5F, 0x49, 0x44, 0x00, 0x44, 0x49, 
	0x53, 0x43, 0x5F, 0x56, 0x45, 0x52, 0x53, 0x49,
	0x4F, 0x4E, 0x00, 0x50, 0x41, 0x52, 0x45, 0x4E, 
	0x54, 0x41, 0x4C, 0x5F, 0x4C, 0x45, 0x56, 0x45, 
	0x4C, 0x00, 0x50, 0x53, 0x50, 0x5F, 0x53, 0x59, 
	0x53, 0x54, 0x45, 0x4D, 0x5F, 0x56, 0x45, 0x52, 
	0x00, 0x52, 0x45, 0x47, 0x49, 0x4F, 0x4E, 0x00, 
	0x54, 0x49, 0x54, 0x4C, 0x45, 0x00, 0x00, 0x00, 
	0x01, 0x00, 0x00, 0x00, 0x4D, 0x47, 0x00, 0x00, 
	0x55, 0x43, 0x4A, 0x53, 0x31, 0x30, 0x30, 0x34, 
	0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x31, 0x2E, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 
	0x01, 0x00, 0x00, 0x00, 0x31, 0x2E, 0x30, 0x30, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 
	0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 
	0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 
	0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 
	0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 
	0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 
	0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 
	0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 
	0x37, 0x38, 0x39, 0x30, 0x31, 0x32, 0x33, 0x34, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

VirtualPBP *g_vpbps = NULL;
int g_vpbps_cnt = 0;
static int g_sema = -1;
static VirtualPBP *g_caches = NULL;
static u32 g_caches_cnt;
static u8 g_referenced[32];
static u8 g_need_update = 0;

static inline u32 get_isocache_magic(void)
{
	u32 version;

	version = sctrlHENGetVersion() & 0xF;
	version = (version << 16) | sctrlHENGetMinorVersion();
	version += 0xC01DB15D;

	return version;
}

static int is_iso(SceIoDirent * dir)
{
	//result
	int result = 0;

	// is it a file?
	if (!FIO_S_ISREG(dir->d_stat.st_mode)) {
		return 0;
	}

	//grab extension
	char * ext = dir->d_name + strlen(dir->d_name) - 4;

	//filename length check
	if (ext > dir->d_name) {
		//check extension
		if (stricmp(ext, ".iso") == 0 || stricmp(ext, ".cso") == 0 || stricmp(ext, ".zso") == 0) {
			result = 1;
		}
	}

	return result;
}

static VirtualPBP* get_vpbp_by_path(const char *path)
{
	char *p;
	int isoindex;

	if (g_vpbps == NULL) {
		return NULL;
	}

	p = strstr(path, ISO_ID);

	if (p == NULL) {
		return NULL;
	}

	p = strrchr(path, '@') + 1;
	isoindex = strtol(p, NULL, 16);

	if (isoindex < 0 || isoindex >= g_vpbps_cnt) {
		return NULL;
	}

	return &g_vpbps[isoindex];
}

static inline void lock(void)
{
	sceKernelWaitSema(g_sema, 1, 0);
}

static inline void unlock(void)
{
	sceKernelSignalSema(g_sema, 1);
}

static VirtualPBP* get_vpbp_by_fd(SceUID fd)
{
	fd -= MAGIC_VPBP_FD;

	if (fd < 0 || fd >= g_vpbps_cnt || g_vpbps == NULL) {
		return NULL;
	}

	return &g_vpbps[fd];
}

static int get_sfo_string(const char *sfo, const char *name, char *output, int output_size)
{
	SFOHeader *header = (SFOHeader *)sfo;
	SFODir *entries = (SFODir *)(sfo+0x14);
	int i;

	if(header->signature != 0x46535000) {
		return -39;
	}

	for (i=0; i<header->nitems; i++) {
		if (0 == strcmp(sfo+header->fields_table_offs+entries[i].field_offs, name)) {
			if(entries[i].type != 0x02) {
				return -41;
			}

			memset(output, 0, output_size);
			strncpy(output, sfo+header->values_table_offs+entries[i].val_offs, output_size);
			output[output_size-1] = '\0';

			return 0;
		}
	}

	return  -40;
}

static int get_sfo_u32(const char *sfo, const char *name, u32 *output)
{
	SFOHeader *header = (SFOHeader *)sfo;
	SFODir *entries = (SFODir *)(sfo+0x14);
	int i;

	if(header->signature != 0x46535000) {
		return -39;
	}

	for (i=0; i<header->nitems; i++) {
		if (0 == strcmp(sfo+header->fields_table_offs+entries[i].field_offs, name)) {
			if(entries[i].type != 0x04) {
				return -41;
			}

			*output = *(u32*)(sfo+header->values_table_offs+entries[i].val_offs);

			return 0;
		}
	}

	return  -40;
}

static int add_cache(VirtualPBP *vpbp)
{
	int i;

	if (vpbp == NULL || !vpbp->enabled || g_caches == NULL) {
		return -22;
	}

	for(i=0; i<g_caches_cnt; ++i) {
		if(!g_caches[i].enabled) {
			memcpy(&g_caches[i], vpbp, sizeof(*vpbp));
			g_referenced[i] = 1;
			g_need_update = 1;

			return 1;
		}
	}

	return 0;
}

static int load_cache(void)
{
	int i, fd, ret;
	u32 magic;

	if (g_caches == NULL) {
		return -32;
	}

	memset(g_caches, 0, sizeof(g_caches[0]) * g_caches_cnt);

	for(i=0; i<3; ++i) {
		fd = sceIoOpen(PSP_CACHE_PATH, PSP_O_RDONLY, 0777);

		if (fd >= 0) {
			break;
		}

		printk("%s: open %s -> 0x%08X\n", __func__, PSP_CACHE_PATH, fd);

		fd = sceIoOpen(PSPGO_CACHE_PATH, PSP_O_RDONLY, 0777);

		if (fd >= 0) {
			break;
		}

		printk("%s: open %s -> 0x%08X\n", __func__, PSPGO_CACHE_PATH, fd);
	}

	if (fd < 0) {
		return -24;
	}

	ret = sceIoRead(fd, &magic, sizeof(magic));

	if (ret != sizeof(magic) && magic != get_isocache_magic()) {
		return -25;
	}

	sceIoRead(fd, g_caches, g_caches_cnt*sizeof(g_caches[0]));
	memset(g_referenced, 0, sizeof(g_referenced));
	sceIoClose(fd);

	return 0;
}

static int save_cache(void)
{
	int i;
	SceUID fd;
	u32 magic = get_isocache_magic();

	if (g_caches == NULL) {
		return -33;
	}

	for(i=0; i<g_caches_cnt; ++i) {
		if (g_caches[i].enabled && !g_referenced[i]) {
			g_need_update = 1;
			memset(&g_caches[i], 0, sizeof(g_caches[i]));
		}
	}

	if(!g_need_update) {
		printk("%s: no need to update\n", __func__);

		return 0;
	}

	for(i=0; i<3; ++i) {
		fd = sceIoOpen(PSP_CACHE_PATH, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

		if (fd >= 0) {
			break;
		}

		printk("%s: open %s -> 0x%08X\n", __func__, PSP_CACHE_PATH, fd);

		fd = sceIoOpen(PSPGO_CACHE_PATH, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

		if (fd >= 0) {
			break;
		}

		printk("%s: open %s -> 0x%08X\n", __func__, PSPGO_CACHE_PATH, fd);
	}

	if (fd < 0) {
		return -21;
	}

	sceIoWrite(fd, &magic, sizeof(magic));
	sceIoWrite(fd, g_caches, sizeof(g_caches[0])*g_caches_cnt);
	sceIoClose(fd);

	g_need_update = 0;

	return 0;
}

static int get_iso_file_size(const char *path, u32 *file_size)
{
	int ret;
	SceIoStat stat;

	ret = sceIoGetstat(path, &stat);

	if (ret < 0)
		return ret;

	*file_size = stat.st_size;

	return 0;
}

static int get_cache(const char *file, ScePspDateTime *mtime, VirtualPBP* pbp)
{
	int i, ret;
	u32 file_size;

	if (g_caches == NULL) {
		return -34;
	}

	ret = get_iso_file_size(file, &file_size);

	if(ret < 0) {
		return -37;
	}

	for(i=0; i<g_caches_cnt; ++i) {
		if(g_caches[i].enabled && 0 == strcmp(g_caches[i].name, file)) {
			if (file_size == g_caches[i].iso_total_size &&
				   	memcmp(&g_caches[i].mtime, mtime, sizeof(*mtime)) == 0) {
				memcpy(pbp, &g_caches[i], sizeof(*pbp));
				g_referenced[i] = 1;

				return 0;
			}
		}
	}

	return -23;
}

static int build_vpbp(VirtualPBP *vpbp)
{
	int ret, i;
	u32 off;

	printk("Need to build vpbp %s\n", vpbp->name);
	memset(vpbp->header, 0, sizeof(vpbp->header));
	memset(vpbp->sects, 0, sizeof(vpbp->sects));
	vpbp->enabled = 1;
	vpbp->file_pointer = 0;
	vpbp->header[0] = 0x50425000; // PBP magic
	vpbp->header[1] = 0x10000; // version

	// fill vpbp offsets
	off = 0x28;

	ret = isoOpen(vpbp->name);

	if (ret < 0) {
		printk("%s: isoOpen -> %d\n", __func__, ret);
		ret = add_cache(vpbp);

		return ret;
	}

	for(i=0; i<NELEMS(pbp_entries); ++i) {
		vpbp->header[i+2] = off;

		if (pbp_entries[i].enabled) {
			PBPSection *sec = &vpbp->sects[i];

			ret = isoGetFileInfo(pbp_entries[i].name, &sec->size, &sec->lba);

			if (ret < 0) {
				if (i == 0) {
					// no PARAM.SFO?
					// then it's a bad ISO
					isoClose();

					return -36;
				} else {
					continue;
				}
			}

			if (i == 0) {
				off += sizeof(virtualsfo);
			} else {
				off += sec->size;
			}
		}
	}

	vpbp->pbp_total_size = vpbp->header[9];
	get_iso_file_size(vpbp->name, &vpbp->iso_total_size);
	ret = add_cache(vpbp);
	printk("%s: add_cache -> %d\n", __func__, ret);
	isoClose();

	return ret;
}

void *oe_realloc(void *ptr, int size)
{
	void *p;
	
	p = oe_malloc(size);

	if(p != NULL && ptr != NULL) {
		memcpy(p, ptr, size);
		oe_free(ptr);
	}

	return p;
}

static VirtualPBP *vpbp_realloc(VirtualPBP *vpbp, int size)
{
	VirtualPBP *v;

	v = (VirtualPBP*) oe_realloc(vpbp, size * sizeof(vpbp[0]));

	return v;
}

static int get_header_section(VirtualPBP *vpbp, u32 remaining, void *data)
{
	int re;

	re = MIN(remaining, vpbp->header[2] - vpbp->file_pointer);
	memcpy(data, vpbp->header+vpbp->file_pointer, re);

	return re;
}

static int get_sfo_section(VirtualPBP *vpbp, u32 remaining, void *data)
{
	int re, ret;
	void *buf, *buf_64;
	char sfotitle[64];
	char disc_id[12];
	u32 parental_level = 1;

	buf = oe_malloc(SECTOR_SIZE+64);

	if (buf == NULL) {
		printk("%s: buf cannot allocate\n", __func__);

		return -13;
	}

	buf_64 = PTR_ALIGN_64(buf);
	ret = isoRead(buf_64, vpbp->sects[0].lba, 0, SECTOR_SIZE);

	if (ret < 0) {
		printk("%s: isoRead -> 0x%08X\n", __func__, ret);
		oe_free(buf);

		return -37;
	}

	ret = get_sfo_string(buf_64, "TITLE", sfotitle, sizeof(sfotitle));

	if (ret < 0) {
		oe_free(buf);

		return ret;
	}

	ret = get_sfo_string(buf_64, "DISC_ID", disc_id, sizeof(disc_id));

	if (ret < 0) {
		oe_free(buf);

		return ret;
	}

	get_sfo_u32(buf_64, "PARENTAL_LEVEL", &parental_level);

	oe_free(buf);
	memcpy(virtualsfo+0x118, sfotitle, 64);
	memcpy(virtualsfo+0xf0, disc_id, 12);
	memcpy(virtualsfo+0x108, &parental_level, sizeof(parental_level));
	re = MIN(remaining, sizeof(virtualsfo) - (vpbp->file_pointer - vpbp->header[2]));
	memcpy(data, virtualsfo+vpbp->file_pointer-vpbp->header[2], re);

	return re;
}

static int get_pbp_section(VirtualPBP *vpbp, u32 remaining, int idx, void *data)
{
	void *buf, *buf_64;
	int rest, pos_section, re, total_re, buf_size = 8 * SECTOR_SIZE;
	int pos;

	total_re = re = 0;
	pos = vpbp->file_pointer;

	if(remaining == 0) {
		goto out;
	}

	if(!pbp_entries[idx].enabled) {
		goto out;
	}

	if (pos < vpbp->header[2]) {
		return get_header_section(vpbp, remaining, data);
	}

	if (pos >= vpbp->header[2] && pos < vpbp->header[3]) {
		return get_sfo_section(vpbp, remaining, data);
	}

	if(!(pos >= vpbp->header[2+idx] && pos < vpbp->header[2+1+idx])) {
		goto out;
	}

	buf = oe_malloc(buf_size + 64);

	if(buf == NULL) {
		printk("%s: buf(2) cannot allocate\n", __func__);

		return -5;
	}

	pos_section = pos - vpbp->header[2+idx];
	rest = MIN(remaining, vpbp->sects[idx].size - pos_section);
	buf_64 = PTR_ALIGN_64(buf);

	while (rest > 0) {
		int ret;

		re = MIN(rest, buf_size);
		ret = isoRead(buf_64, vpbp->sects[idx].lba, pos_section, re);

		if (ret < 0) {
			printk("%s: isoRead -> 0x%08X\n", __func__, ret);

			return -38;
		}

		memcpy(data, buf_64, re);
		rest -= re;
		pos_section += re;
		data += re;
		remaining -= re;
		total_re += re;
		pos += re;
	}

	oe_free(buf);

out:
	return total_re;
}

int vpbp_init(void)
{
	if (g_sema >= 0) {
		sceKernelDeleteSema(g_sema);
	}

	g_sema = sceKernelCreateSema("VPBPSema", 0, 1, 1, NULL);

	if (g_caches != NULL) {
		oe_free(g_caches);
		g_caches_cnt = 0;
	}

	g_caches_cnt = CACHE_MAX_SIZE;
	g_caches = oe_malloc(sizeof(g_caches[0]) * g_caches_cnt);

	if (g_caches == NULL) {
		g_caches_cnt = 0;
		printk("%s: g_cache cannot allocate\n", __func__);

		return -27;
	}

	memset(g_caches, 0, sizeof(g_caches[0]) * g_caches_cnt);
	
	return 0;
}

SceUID vpbp_open(const char * file, int flags, SceMode mode)
{
	int ret;
	VirtualPBP *vpbp;

	lock();

	if (flags & (PSP_O_WRONLY | PSP_O_TRUNC | PSP_O_CREAT) || !(flags & PSP_O_RDONLY)) {
		printk("%s: bad flags 0x%08X\n", __func__, flags);
		unlock();

		return -6;
	}
	
	vpbp = get_vpbp_by_path(file);

	if (vpbp == NULL) {
		printk("%s: Unknown file %s in vpbp list\n", __func__, file);
		unlock();

		return -12;
	}

	if (vpbp->enabled) {
		vpbp->file_pointer = 0;
		ret = isoOpen(vpbp->name);

		if (ret < 0) {
			printk("%s: isoOpen -> %d\n", __func__, ret);
			unlock();

			return -29;
		}

		unlock();

		return MAGIC_VPBP_FD+(vpbp-&g_vpbps[0]);
	}

	unlock();

	return -26;
}

SceOff vpbp_lseek(SceUID fd, SceOff offset, int whence)
{
	VirtualPBP *vpbp;

	lock();
	vpbp = get_vpbp_by_fd(fd);
	
	if (vpbp == NULL) {
		printk("%s: unknown fd 0x%08X\n", __func__, fd);
		unlock();

		return -3;
	}

	switch(whence) {
		case PSP_SEEK_SET:
			vpbp->file_pointer = (int)offset;
			break;
		case PSP_SEEK_CUR:
			vpbp->file_pointer += (int)offset;
			break;
		case PSP_SEEK_END:
			vpbp->file_pointer = vpbp->pbp_total_size + (int)offset;
			break;
		default:
			break;
	}

	unlock();

	return vpbp->file_pointer;
}

int vpbp_read(SceUID fd, void * data, SceSize size)
{
	VirtualPBP *vpbp;
	u32 remaining;
	int ret, idx;
	
	lock();
	vpbp = get_vpbp_by_fd(fd);

	if (vpbp == NULL) {
		printk("%s: unknown fd 0x%08X\n", __func__, fd);
		unlock();

		return -4;
	}

	remaining = size;

	while(remaining > 0) {
		for(idx=0; idx<NELEMS(pbp_entries)-2; ++idx) {
			ret = get_pbp_section(vpbp, remaining, idx, data);

			if(ret >= 0) {
				data += ret;
				vpbp->file_pointer += ret;
				remaining -= ret;
			} else {
				unlock();

				return ret;
			}
		}

		if (vpbp->file_pointer >= vpbp->pbp_total_size)
			break;
	}

	unlock();

	return size - remaining;
}

int vpbp_close(SceUID fd)
{
	VirtualPBP *vpbp;
	
	lock();
	vpbp = get_vpbp_by_fd(fd);

	if (vpbp == NULL) {
		printk("%s: unknown fd 0x%08X\n", __func__, fd);
		unlock();

		return -7;
	}

	isoClose();
	unlock();

	return 0;
}

int vpbp_disable_all_caches(void)
{
	VirtualPBP *vpbp;

	lock();

	if (g_vpbps == NULL) {
		unlock();

		return -35;
	}

	for(vpbp=g_vpbps; vpbp != &g_vpbps[g_vpbps_cnt]; vpbp++) {
		vpbp->enabled = 0;
	}

	unlock();

	return 0;
}

int vpbp_remove(const char * file)
{
	int ret;
	VirtualPBP* vpbp;

	lock();
	vpbp = get_vpbp_by_path(file);

	if (vpbp == NULL) {
		printk("%s: Unknown file %s in vpbp list\n", __func__, file);
		unlock();

		return -14;
	}

	ret = sceIoRemove(vpbp->name);
	vpbp->enabled = 0;
	unlock();

	return ret;
}

int vpbp_is_fd(SceUID fd)
{
	VirtualPBP *vpbp;
	int ret;
	
	lock();
	vpbp = get_vpbp_by_fd(fd);

	if (vpbp != NULL) {
		ret = 1;
	} else {
		ret = 0;
	}

	unlock();

	return ret;
}

int vpbp_getstat(const char * file, SceIoStat * stat)
{
	int ret;
	VirtualPBP *vpbp;

	lock();
	vpbp = get_vpbp_by_path(file);

	if (vpbp == NULL) {
		printk("%s: Unknown file %s in vpbp list\n", __func__, file);
		unlock();

		return -30;
	}

	ret = sceIoGetstat(vpbp->name, stat);
	stat->st_mode = 0x21FF;
	stat->st_attr = 0x20;
	stat->st_size = vpbp->iso_total_size;
	memcpy(&stat->st_ctime, &vpbp->ctime, sizeof(ScePspDateTime));
	memcpy(&stat->st_mtime, &vpbp->ctime, sizeof(ScePspDateTime));
	memcpy(&stat->st_atime, &vpbp->ctime, sizeof(ScePspDateTime));
	unlock();

	return ret;
}

static int has_prometheus_module(VirtualPBP *vpbp)
{
	int ret;
	u32 size, lba;
	
	ret = isoOpen(vpbp->name);

	if (ret < 0) {
		return 0;
	}

	ret = isoGetFileInfo("/PSP_GAME/SYSDIR/EBOOT.OLD", &size, &lba);
	ret = (ret >= 0) ? 1 : 0;

	isoClose();

	return ret;
}

typedef struct _pspMsPrivateDirent {
	SceSize size;
	char s_name[16];
	char l_name[1024];
} pspMsPrivateDirent;

static int get_ISO_shortname(char *s_name, u32 size, const char *l_name)
{
	const char *p;
	SceUID fd;
	char *prefix = NULL;
	int result = -7;
	pspMsPrivateDirent *pri_dirent = NULL;
	SceIoDirent *dirent = NULL;

	if (s_name == NULL || l_name == NULL) {
		result = -1;
		goto exit;
	}

	p = strrchr(l_name, '/');

	if (p == NULL) {
		result = -2;
		goto exit;
	}

	prefix = oe_malloc(512);
	pri_dirent = oe_malloc(sizeof(*pri_dirent));
	dirent = oe_malloc(sizeof(*dirent));

	if(prefix == NULL) {
		result = -3;
		goto exit;
	}

	if(pri_dirent == NULL) {
		result = -4;
		goto exit;
	}

	if(dirent == NULL) {
		result = -5;
		goto exit;
	}

	strncpy_s(prefix, 512, l_name, p + 1 - l_name);
	prefix[MIN(p + 1 - l_name, 512-1)] = '\0';
	printk("%s: prefix %s\n", __func__, prefix);
	fd = sceIoDopen(prefix);

	if (fd >= 0) {
		int ret;

		do {
			memset(dirent, 0, sizeof(*dirent));
			memset(pri_dirent, 0, sizeof(*pri_dirent));
			pri_dirent->size = sizeof(*pri_dirent);
			dirent->d_private = (void*)pri_dirent;
			ret = sceIoDread(fd, dirent);

			if (ret >= 0) {
				if (!strcmp(dirent->d_name, p+1)) {
					strncpy(s_name, l_name, MIN(p + 1 - l_name, size));
					s_name[MIN(p + 1 - l_name, size-1)] = '\0';
					strcat(s_name, pri_dirent->s_name);
					printk("%s: final %s\n", __func__, s_name);
					result = 0;

					break;
				}
			}
		} while (ret > 0);

		sceIoDclose(fd);
	} else {
		printk("%s: sceIoDopen %s -> 0x%08X\n", __func__, prefix, fd);
		result = -6;
		goto exit;
	}

exit:
	oe_free(prefix);
	oe_free(pri_dirent);
	oe_free(dirent);

	return result;
}

int vpbp_loadexec(char * file, struct SceKernelLoadExecVSHParam * param)
{
	int ret;
	SEConfig config;
	VirtualPBP *vpbp;
	int apitype;
	const char *loadexec_file;

	lock();
	vpbp = get_vpbp_by_path(file);

	if (vpbp == NULL) {
		printk("%s: Unknown file %s in vpbp list\n", __func__, file);
		unlock();

		return -31;
	}

	sctrlSEGetConfig(&config);
	
	if(config.chn_iso) {
		get_ISO_shortname(vpbp->name, sizeof(vpbp->name), vpbp->name);
	}

	//set iso file for reboot
	sctrlSESetUmdFile(vpbp->name);

	//set iso mode for reboot
	sctrlSESetBootConfFileIndex(config.umdmode);
	sctrlSESetDiscType(PSP_UMD_TYPE_GAME);

	printk("%s: ISO %s, UMD mode %d\n", __func__, vpbp->name, config.umdmode);
	
	//reset and configure reboot parameter
	memset(param, 0, sizeof(param));
	param->size = sizeof(param);

	if (has_prometheus_module(vpbp)) {
		printk("%s: prometheus module detected, use EBOOT.OLD\n", __func__);
		param->argp = "disc0:/PSP_GAME/SYSDIR/EBOOT.OLD";
	} else {
		param->argp = "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN";
	}

	param->args = strlen(param->argp) + 1;

	if (psp_model == PSP_GO) {
		char devicename[20];

		ret = get_device_name(devicename, sizeof(devicename), vpbp->name);

		if(ret == 0 && 0 == stricmp(devicename, "ef0:")) {
			apitype = 0x125;
		} else {
			apitype = 0x123;
		}

		param->key = "umdemu";
		loadexec_file = vpbp->name;
	} else {
		if(config.umdmode == MODE_MARCH33) {
			param->key = "game";
		} else {
			param->key = "umdemu";
		}

		apitype = 0x120;
		loadexec_file = param->argp;
	}

	//start game image
	return sctrlKernelLoadExecVSHWithApitype(apitype, loadexec_file, param);

	unlock();

	return ret;
}

SceUID vpbp_dopen(const char * dirname)
{
	SceUID result;

	lock();
	result = sceIoDopen(dirname);

	if (result >= 0 && strlen(dirname) > 4 && 0 == stricmp(dirname+4, "/ISO")) {
		load_cache();
	}

	unlock();

	return result;
}

static int add_fake_dirent(SceIoDirent *dir, int vpbp_idx)
{
	VirtualPBP *vpbp;

	vpbp = &g_vpbps[vpbp_idx];
	sprintf(dir->d_name, "%s%08X", ISO_ID, vpbp_idx);
	printk("%s: ISO %s -> %s added\n", __func__, vpbp->name, dir->d_name);

	dir->d_stat.st_mode = 0x11FF;
	dir->d_stat.st_attr = 0x10;

	memcpy(&dir->d_stat.st_ctime, &vpbp->ctime, sizeof(ScePspDateTime));
	memcpy(&dir->d_stat.st_mtime, &vpbp->ctime, sizeof(ScePspDateTime));
	memcpy(&dir->d_stat.st_atime, &vpbp->ctime, sizeof(ScePspDateTime));

	return 1;
}

int vpbp_dread(SceUID fd, SceIoDirent * dir)
{
	int result, cur_idx, ret;
	struct IoDirentEntry *entry;

	lock();

	entry = dirent_search(fd);

	if(entry == NULL) {
		result = -44;
		goto exit;
	}
	
	result = sceIoDread(entry->iso_dfd, dir);

	if(sceKernelFindModuleByName("Game_Categories_Light") == NULL) {
		while(result > 0 && !is_iso(dir)) {
			result = sceIoDread(entry->iso_dfd, dir);
		}
	}

	if (result > 0 && is_iso(dir)) {
		VirtualPBP *vpbp;

		vpbp = vpbp_realloc(g_vpbps, g_vpbps_cnt+1);

		if(vpbp == NULL) {
			result = -42;
			goto exit;
		}

		g_vpbps = vpbp;
		g_vpbps_cnt++;
		cur_idx = g_vpbps_cnt-1;
		vpbp = &g_vpbps[cur_idx];
		STRCPY_S(vpbp->name, entry->path);
		vpbp->name[4] = '\0';
		STRCAT_S(vpbp->name, "/ISO");
		STRCAT_S(vpbp->name, entry->path + sizeof("xxx:/PSP/GAME") - 1);
		STRCAT_S(vpbp->name, "/");
		STRCAT_S(vpbp->name, dir->d_name);
		memcpy(&vpbp->ctime, &dir->d_stat.st_ctime, sizeof(vpbp->ctime));
		memcpy(&vpbp->mtime, &dir->d_stat.st_mtime, sizeof(vpbp->mtime));

		ret = get_cache(vpbp->name, &vpbp->mtime, vpbp);

		if (ret < 0) {
			ret = build_vpbp(vpbp);

			if (ret < 0) {
				result = -43;
				goto exit;
			}
		}

		result = add_fake_dirent(dir, cur_idx);
	}

exit:
	unlock();

	return result;
}

int vpbp_dclose(SceUID fd)
{
	int result;
	struct IoDirentEntry *entry;

	lock();
	entry = dirent_search(fd);

	if (entry != NULL && strlen(entry->path) > 4 && 0 == stricmp(entry->path+4, "/PSP/GAME")) {
		save_cache();
	}

	result = sceIoDclose(fd);
	unlock();

	return result;
}

int vpbp_reset(int cache)
{
	if (g_vpbps != NULL) {
		oe_free(g_vpbps);
		g_vpbps = NULL;
		g_vpbps_cnt = 0;
	}

	if(cache == 1) {
		if (g_caches != NULL) {
			oe_free(g_caches);
			g_caches = NULL;
			g_caches_cnt = 0;
		}
	}

	return 0;
}
