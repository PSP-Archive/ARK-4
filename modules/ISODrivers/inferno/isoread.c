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

#include <pspkernel.h>
#include <pspreg.h>
#include <stdio.h>
#include <string.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <pspsysmem_kernel.h>
#include <psprtc.h>
#include <psputilsforkernel.h>
#include <pspthreadman_kernel.h>
#include <macros.h>
#include "systemctrl.h"
#include "systemctrl_se.h"
#include "systemctrl_private.h"
#include "inferno.h"

// 0x00002784
struct IoReadArg g_read_arg;

// 0x00002484
void *g_sector_buf = NULL;

// 0x0000248C
int g_iso_opened = 0;

// 0x000023D0
SceUID g_iso_fd = -1;

// 0x000023D4
int g_total_sectors = -1;

// 0x00002488
static int g_is_ciso = 0;

// 0x000024C0
static void *g_ciso_block_buf = NULL;

// 0x000024C4, size CISO_DEC_BUFFER_SIZE + (1 << g_CISO_hdr.align), align 64
static void *g_ciso_dec_buf = NULL;

// 0x00002704
static int g_CISO_cur_idx = 0;

// 0x00002700
static unsigned int g_ciso_dec_buf_offset = (unsigned int)-1;

static int g_ciso_dec_buf_size = 0;

// 0x00002720
static unsigned int g_ciso_total_block = 0;

struct CISO_header {
	unsigned char magic[4];  // 0
	unsigned int header_size;  // 4
	u64 total_bytes; // 8
	unsigned int block_size; // 16
	unsigned char ver; // 20
	unsigned char align;  // 21
	unsigned char rsv_06[2];  // 22
} __attribute__((packed));

// 0x00002708
static struct CISO_header g_CISO_hdr __attribute__((aligned(64)));

// 0x00002500
static unsigned int g_CISO_idx_cache[CISO_IDX_BUFFER_SIZE/4] __attribute__((aligned(64)));

// 0x00000368
static void wait_until_ms0_ready(void)
{
	int ret, status = 0;
	const char *drvname;

	drvname = "mscmhc0:";

	while( 1 )
	{
		ret = sceIoDevctl(drvname, 0x02025801, 0, 0, &status, sizeof(status));

		if(ret < 0)
		{
			sceKernelDelayThread(20000);
			continue;
		}

		if(status == 4)
		{
			break;
		}

		sceKernelDelayThread(20000);
	}
}

// 0x00000EE4
static int ciso_get_nsector(SceUID fd)
{
//	return g_CISO_hdr.total_bytes / g_CISO_hdr.block_size;;
	return g_ciso_total_block;
}

// 0x00000E58
static int iso_get_nsector(SceUID fd)
{
	SceOff off, total;

	off = sceIoLseek(fd, 0, PSP_SEEK_CUR);
	total = sceIoLseek(fd, 0, PSP_SEEK_END);
	sceIoLseek(fd, off, PSP_SEEK_SET);

	return total / ISO_SECTOR_SIZE;
}

// 0x00000E58
static int get_nsector(void)
{
	if(g_is_ciso)
	{
		return ciso_get_nsector(g_iso_fd);
	}

	return iso_get_nsector(g_iso_fd);
}

// 0x00000F00
static int is_ciso(SceUID fd)
{
	int ret;
	unsigned int *magic;

	g_CISO_hdr.magic[0] = '\0';
	g_ciso_dec_buf_offset = (unsigned int)-1;
	g_ciso_dec_buf_size = 0;

	sceIoLseek(fd, 0, PSP_SEEK_SET);
	ret = sceIoRead(fd, &g_CISO_hdr, sizeof(g_CISO_hdr));

	if(ret != sizeof(g_CISO_hdr))
	{
		ret = -1;
		printk("%s: -> %d\r\n", __func__, ret);
		goto exit;
	}

	magic = (unsigned int*)g_CISO_hdr.magic;

	if(*magic == 0x4F534943)
	{ // CISO
		g_CISO_cur_idx = -1;
		g_ciso_total_block = g_CISO_hdr.total_bytes / g_CISO_hdr.block_size;
		printk("%s: total block %d\r\n", __func__, (int)g_ciso_total_block);

		if(g_ciso_dec_buf == NULL)
		{
			g_ciso_dec_buf = oe_malloc(CISO_DEC_BUFFER_SIZE + (1 << g_CISO_hdr.align) + 64);

			if(g_ciso_dec_buf == NULL)
			{
				ret = -2;
				printk("%s: -> %d\r\n", __func__, ret);
				goto exit;
			}

			if((unsigned int)g_ciso_dec_buf & 63)
				g_ciso_dec_buf = (void*)(((unsigned int)g_ciso_dec_buf & (~63)) + 64);
		}

		if(g_ciso_block_buf == NULL)
		{
			g_ciso_block_buf = oe_malloc(ISO_SECTOR_SIZE + 64);

			if(g_ciso_block_buf == NULL)
			{
				ret = -3;
				printk("%s: -> %d\r\n", __func__, ret);
				goto exit;
			}

			if((unsigned int)g_ciso_block_buf & 63)
				g_ciso_block_buf = (void*)(((unsigned int)g_ciso_block_buf & (~63)) + 64);
		}

		ret = 0;
	}
	else
	{
		ret = 0x8002012F;
	}

exit:
	return ret;
}

// 0x000009D4
int isoOpen(void)
{
	int ret, retries;

	wait_until_ms0_ready();
	sceIoClose(g_iso_fd);
	g_iso_opened = 0;
	retries = 0;

	do {
		g_iso_fd = sceIoOpen(g_iso_fn, 0x000F0001, 0777);

		if(g_iso_fd < 0)
		{
			if(++retries >= 16)
			{
				return -1;
			}

			sceKernelDelayThread(20000);
		}
	} while(g_iso_fd < 0);

	if(g_iso_fd < 0)
	{
		return -1;
	}

	g_is_ciso = 0;
	ret = is_ciso(g_iso_fd);

	if(ret >= 0)
	{
		g_is_ciso = 1;
	}

	g_iso_opened = 1;
	g_total_sectors = get_nsector();

	return 0;
}

// 0x00000BB4
static int read_raw_data(unsigned char* addr, unsigned int size, unsigned int offset)
{
	int ret, i;
	SceOff ofs;

	i = 0;

	do {
		i++;
		ofs = sceIoLseek(g_iso_fd, offset, PSP_SEEK_SET);

		if(ofs >= 0)
		{
			i = 0;
			break;
		}
		else 
		{
			printk("%s: lseek retry %d error 0x%08X\r\n", __func__, i, (int)ofs);
			isoOpen();
		}
	} while(i < 16);

	if(i == 16)
	{
		ret = 0x80010013;
		goto exit;
	}

	for(i=0; i<16; ++i)
	{
		ret = sceIoRead(g_iso_fd, addr, size);

		if(ret >= 0)
		{
			i = 0;
			break;
		}
		else 
		{
			printk("%s: read retry %d error 0x%08X\r\n", __func__, i, ret);
			isoOpen();
		}
	}

	if(i == 16)
	{
		ret = 0x80010013;
		goto exit;
	}

exit:
	return ret;
}

// 0x00001018
static int read_cso_sector(unsigned char *addr, int sector)
{
	int ret;
	int n_sector;
	unsigned int offset, next_offset;
	int size;

	n_sector = sector - g_CISO_cur_idx;

	// not within sector idx cache?
	if(g_CISO_cur_idx == -1 || n_sector < 0 || n_sector >= NELEMS(g_CISO_idx_cache))
	{
		ret = read_raw_data((unsigned char*)g_CISO_idx_cache, sizeof(g_CISO_idx_cache), (sector << 2) + sizeof(struct CISO_header));

		if(ret < 0)
		{
			ret = -4;
			printk("%s: -> %d\r\n", __func__, ret);

			return ret;
		}

		g_CISO_cur_idx = sector;
		n_sector = 0;
	}

	offset = (g_CISO_idx_cache[n_sector] & 0x7FFFFFFF) << g_CISO_hdr.align;

	// is plain?
	if(g_CISO_idx_cache[n_sector] & 0x80000000)
	{
		return read_raw_data(addr, ISO_SECTOR_SIZE, offset);
	}

	sector++;
	n_sector = sector - g_CISO_cur_idx;

	if(g_CISO_cur_idx == -1 || n_sector < 0 || n_sector >= NELEMS(g_CISO_idx_cache))
	{
		ret = read_raw_data((unsigned char*)g_CISO_idx_cache, sizeof(g_CISO_idx_cache), (sector << 2) + sizeof(struct CISO_header));

		if(ret < 0)
		{
			ret = -5;
			printk("%s: -> %d\r\n", __func__, ret);

			return ret;
		}

		g_CISO_cur_idx = sector;
		n_sector = 0;
	}

	next_offset = (g_CISO_idx_cache[n_sector] & 0x7FFFFFFF) << g_CISO_hdr.align;
	size = next_offset - offset;
	
	if(g_CISO_hdr.align)
		size += 1 << g_CISO_hdr.align;

	if(size <= ISO_SECTOR_SIZE)
		size = ISO_SECTOR_SIZE;

	if(g_ciso_dec_buf_offset == (unsigned int)-1 || offset < g_ciso_dec_buf_offset || offset + size >= g_ciso_dec_buf_offset + g_ciso_dec_buf_size)
	{
		ret = read_raw_data(g_ciso_dec_buf, size, offset);

		/* May not reach CISO_DEC_BUFFER_SIZE */	
		if(ret < 0)
		{
			g_ciso_dec_buf_offset = (unsigned int)-1;
			ret = -6;
			printk("%s: -> %d\r\n", __func__, ret);

			return ret;
		}

		g_ciso_dec_buf_offset = offset;
		g_ciso_dec_buf_size = ret;
	}

	ret = sceKernelDeflateDecompress(addr, ISO_SECTOR_SIZE, g_ciso_dec_buf + offset - g_ciso_dec_buf_offset, 0);

	return ret < 0 ? ret : ISO_SECTOR_SIZE;
}

static int read_cso_data(unsigned char* addr, unsigned int size, unsigned int offset)
{
	unsigned int cur_block;
	int pos, ret, read_bytes;
	unsigned int o_offset = offset;

	while(size > 0)
	{
		cur_block = offset / ISO_SECTOR_SIZE;
		pos = offset & (ISO_SECTOR_SIZE - 1);

		if(cur_block >= g_total_sectors)
		{
			// EOF reached
			break;
		}

		ret = read_cso_sector(g_ciso_block_buf, cur_block);

		if(ret != ISO_SECTOR_SIZE)
		{
			ret = -7;
			printk("%s: -> %d\r\n", __func__, ret);

			return ret;
		}

		read_bytes = MIN(size, (ISO_SECTOR_SIZE - pos));
		memcpy(addr, g_ciso_block_buf + pos, read_bytes);
		size -= read_bytes;
		addr += read_bytes;
		offset += read_bytes;
	}

	return offset - o_offset;
}

// 0x00000C7C
int isoRead(struct IoReadArg *args)
{
	int ret;

	if(g_is_ciso != 0)
	{
		ret = read_cso_data(args->address, args->size, args->offset);
	}
	else
	{
		ret = read_raw_data(args->address, args->size, args->offset);
	}

	enum
	{
		PSP_ISO_ID_ADDR = 0x8000,
		PSP_ISO_ID_LENGTH = 32,
	};

	// Fix PSP ISO ID for those FW below 1.8x.
	// It seems 2.00 FW won't need this
	if(ret >= 0 && args->offset < PSP_ISO_ID_ADDR + PSP_ISO_ID_LENGTH && args->offset + args->size > PSP_ISO_ID_ADDR)
	{
		// Grab from lovelyMonkey's PSP ISO
		unsigned char sample[PSP_ISO_ID_LENGTH] = {
			0x01, 0x43, 0x44, 0x30, 0x30, 0x31, 0x01, 0x00, 0x50, 0x53, 0x50, 0x20, 0x47, 0x41, 0x4D, 0x45, 
			0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20
		};
		int len, start;

		printk("Patching ISO header\r\n");
		start = MAX(PSP_ISO_ID_ADDR, args->offset);
		len = MIN(PSP_ISO_ID_ADDR + PSP_ISO_ID_LENGTH, args->offset + args->size) - start;
		memcpy(args->address + start - args->offset, sample + start - PSP_ISO_ID_ADDR, len);
	}

	return ret;
}

// 0x000003E0
int isoReadStack(unsigned int offset, void *ptr, unsigned int data_len)
{
	int ret, retv;

	ret = sceKernelWaitSema(g_umd9660_sema_id, 1, 0);

	if(ret < 0)
	{
		return -1;
	}

	g_read_arg.offset = offset;
	g_read_arg.address = ptr;
	g_read_arg.size = data_len;
	retv = sceKernelExtendKernelStack(0x2000, (void*)&isoRead, &g_read_arg);

	ret = sceKernelSignalSema(g_umd9660_sema_id, 1);

	if(ret < 0)
	{
		return -1;
	}

	return retv;
}
