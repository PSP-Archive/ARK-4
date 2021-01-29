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
#include "systemctrl.h"
#include "systemctrl_se.h"
#include "systemctrl_private.h"
#include "inferno.h"
#include "lz4.h"

#define CISO_IDX_MAX_ENTRIES 4096

#define REMAINDER(a,b) ((a) % (b))
#define IS_DIVISIBLE(a,b) (REMAINDER(a,b) == 0)
#define GET_CSO_OFFSET(block) ((g_cso_idx_cache[block] & 0x7FFFFFFF) << g_CISO_hdr.align)

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
static u32 g_ciso_dec_buf_offset = (u32)-1;

static int g_ciso_dec_buf_size = 0;

// 0x00002720
static u32 g_ciso_total_block = 0;

static int lz4_compressed = 0;

struct CISO_header {
	u8 magic[4];  // 0
	u32 header_size;  // 4
	u64 total_bytes; // 8
	u32 block_size; // 16
	u8 ver; // 20
	u8 align;  // 21
	u8 rsv_06[2];  // 22
};

// 0x00002708
static struct CISO_header g_CISO_hdr __attribute__((aligned(64)));

// 0x00002500
static u32 g_CISO_idx_cache[CISO_IDX_BUFFER_SIZE/4] __attribute__((aligned(64)));

static u32 *g_cso_idx_cache = NULL;

u32 g_cso_idx_start_block = -1;

// 0x00000368
static void wait_until_ms0_ready(void)
{
	int ret, status = 0, bootfrom;
	const char *drvname;

	drvname = "mscmhc0:";

    /*
	if(psp_model == PSP_GO) {
		bootfrom = sctrlKernelBootFrom();
		printk("%s: bootfrom: 0x%08X\n", __func__, bootfrom);

		if(bootfrom == 0x50) {
			drvname = "mscmhcemu0:";
		} else {
			// vsh mode?
			return;
		}
	}
	*/

	while( 1 ) {
		ret = sceIoDevctl(drvname, 0x02025801, 0, 0, &status, sizeof(status));

		if(ret < 0) {
			sceKernelDelayThread(20000);
			continue;
		}

		if(status == 4) {
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
	if(g_is_ciso) {
		return ciso_get_nsector(g_iso_fd);
	}

	return iso_get_nsector(g_iso_fd);
}

// 0x00000F00
static int is_ciso(SceUID fd)
{
	int ret;
	u32 *magic;

	g_CISO_hdr.magic[0] = '\0';
	g_ciso_dec_buf_offset = (u32)-1;
	g_ciso_dec_buf_size = 0;

	sceIoLseek(fd, 0, PSP_SEEK_SET);
	ret = sceIoRead(fd, &g_CISO_hdr, sizeof(g_CISO_hdr));

	if(ret != sizeof(g_CISO_hdr)) {
		ret = -1;
		printk("%s: -> %d\n", __func__, ret);
		goto exit;
	}

	magic = (u32*)g_CISO_hdr.magic;

	if(*magic == 0x4F534943 || *magic == 0x4F53495A) { // CISO or ZISO
		lz4_compressed = (*magic == 0x4F53495A) ? 1 : 0;
		g_CISO_cur_idx = -1;
		g_ciso_total_block = g_CISO_hdr.total_bytes / g_CISO_hdr.block_size;
		printk("%s: total block %d\n", __func__, (int)g_ciso_total_block);

		if(g_ciso_dec_buf == NULL) {
			g_ciso_dec_buf = oe_malloc(CISO_DEC_BUFFER_SIZE + (1 << g_CISO_hdr.align) + 64);

			if(g_ciso_dec_buf == NULL) {
				ret = -2;
				printk("%s: -> %d\n", __func__, ret);
				goto exit;
			}

			if((u32)g_ciso_dec_buf & 63)
				g_ciso_dec_buf = (void*)(((u32)g_ciso_dec_buf & (~63)) + 64);
		}

		if(g_ciso_block_buf == NULL) {
			g_ciso_block_buf = oe_malloc(ISO_SECTOR_SIZE + 64);

			if(g_ciso_block_buf == NULL) {
				ret = -3;
				printk("%s: -> %d\n", __func__, ret);
				goto exit;
			}

			if((u32)g_ciso_block_buf & 63)
				g_ciso_block_buf = (void*)(((u32)g_ciso_block_buf & (~63)) + 64);
		}
		
		if (g_cso_idx_cache == NULL) {
			g_cso_idx_cache = oe_malloc((CISO_IDX_MAX_ENTRIES * 4) + 64);
			if (g_cso_idx_cache == NULL) {
				ret = -3;
				printk("%s: -> %d\n", __func__, ret);
				goto exit;
			}

			if((u32)g_ciso_block_buf & 63)
				g_cso_idx_cache = (void*)(((u32)g_cso_idx_cache & (~63)) + 64);

		}

		ret = 0;
	} else {
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

		if(g_iso_fd < 0) {
			if(++retries >= 16) {
				return -1;
			}

			sceKernelDelayThread(20000);
		}
	} while(g_iso_fd < 0);

	if(g_iso_fd < 0) {
		return -1;
	}

	g_is_ciso = 0;
	ret = is_ciso(g_iso_fd);

	if(ret >= 0) {
		g_is_ciso = 1;
	}

	g_iso_opened = 1;
	g_total_sectors = get_nsector();

	return 0;
}

// 0x00000BB4
static int read_raw_data(u8* addr, u32 size, u32 offset)
{
	int ret, i;
	SceOff ofs;

	i = 0;

	do {
		i++;
		ofs = sceIoLseek(g_iso_fd, offset, PSP_SEEK_SET);

		if(ofs >= 0) {
			i = 0;
			break;
		} else {
			printk("%s: lseek retry %d error 0x%08X\n", __func__, i, (int)ofs);
			isoOpen();
		}
	} while(i < 16);

	if(i == 16) {
		ret = 0x80010013;
		goto exit;
	}

	for(i=0; i<16; ++i) {
		ret = sceIoRead(g_iso_fd, addr, size);

		if(ret >= 0) {
			i = 0;
			break;
		} else {
			printk("%s: read retry %d error 0x%08X\n", __func__, i, ret);
			isoOpen();
		}
	}

	if(i == 16) {
		ret = 0x80010013;
		goto exit;
	}

exit:
	return ret;
}

// 0x00001018
static int read_cso_sector(u8 *addr, int sector)
{
	int ret;
	int n_sector;
	u32 offset, next_offset;
	int size;

	n_sector = sector - g_CISO_cur_idx;

	// not within sector idx cache?
	if(g_CISO_cur_idx == -1 || n_sector < 0 || n_sector >= NELEMS(g_CISO_idx_cache)) {
		ret = read_raw_data((u8*)g_CISO_idx_cache, sizeof(g_CISO_idx_cache), (sector << 2) + sizeof(struct CISO_header));

		if(ret < 0) {
			ret = -4;
			printk("%s: -> %d\n", __func__, ret);

			return ret;
		}

		g_CISO_cur_idx = sector;
		n_sector = 0;
	}

	offset = (g_CISO_idx_cache[n_sector] & 0x7FFFFFFF) << g_CISO_hdr.align;

	// is plain?
	if(g_CISO_idx_cache[n_sector] & 0x80000000) {
		return read_raw_data(addr, ISO_SECTOR_SIZE, offset);
	}

	sector++;
	n_sector = sector - g_CISO_cur_idx;

	if(g_CISO_cur_idx == -1 || n_sector < 0 || n_sector >= NELEMS(g_CISO_idx_cache)) {
		ret = read_raw_data((u8*)g_CISO_idx_cache, sizeof(g_CISO_idx_cache), (sector << 2) + sizeof(struct CISO_header));

		if(ret < 0) {
			ret = -5;
			printk("%s: -> %d\n", __func__, ret);

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

	if(g_ciso_dec_buf_offset == (u32)-1 || offset < g_ciso_dec_buf_offset || offset + size >= g_ciso_dec_buf_offset + g_ciso_dec_buf_size) {
		ret = read_raw_data(g_ciso_dec_buf, size, offset);

		/* May not reach CISO_DEC_BUFFER_SIZE */	
		if(ret < 0) {
			g_ciso_dec_buf_offset = (u32)-1;
			ret = -6;
			printk("%s: -> %d\n", __func__, ret);

			return ret;
		}

		g_ciso_dec_buf_offset = offset;
		g_ciso_dec_buf_size = ret;
	}

	if(!lz4_compressed) {
		ret = sceKernelDeflateDecompress(addr, ISO_SECTOR_SIZE, g_ciso_dec_buf + offset - g_ciso_dec_buf_offset, 0);
	} else {
		ret = LZ4_decompress_fast(g_ciso_dec_buf + offset - g_ciso_dec_buf_offset, (char *)addr, ISO_SECTOR_SIZE);
		if(ret < 0) {
			ret = -20;
			printk("%s: -> %d\n", __func__, ret);
		}
	}

	return ret < 0 ? ret : ISO_SECTOR_SIZE;
}

static int read_cso_data(u8* addr, u32 size, u32 offset)
{
	u32 cur_block;
	int pos, ret, read_bytes;
	u32 o_offset = offset;

	while(size > 0) {
		cur_block = offset / ISO_SECTOR_SIZE;
		pos = offset & (ISO_SECTOR_SIZE - 1);

		if(cur_block >= g_total_sectors) {
			// EOF reached
			break;
		}

		ret = read_cso_sector(g_ciso_block_buf, cur_block);

		if(ret != ISO_SECTOR_SIZE) {
			ret = -7;
			printk("%s: -> %d\n", __func__, ret);

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

/**
 * decompress (if necessary) a compressed block and copy it to the destination
 * If offset_shift is nonzero then the decompressed data will by used from this offset.
 * The block_num is used to know if it needs to be decompressed
 */
static int decompress_block(u8 *dst, u8 *src, int size, int block_num, int offset_shift)
{
	int ret;

	if(g_cso_idx_cache[block_num] & 0x80000000) {
		// do not copy if the block is already in place
		if(offset_shift > 0 || src != dst) {
			// no decompression is needed, copy the data from the end of the buffer to the start
			memmove(dst, src + offset_shift, size);
		}
		ret = size;
	} else {
		if(!lz4_compressed) {
			// gzip decompress
			ret = sceKernelDeflateDecompress(g_ciso_dec_buf, ISO_SECTOR_SIZE, src, 0);
		} else {
			// LZ4 decompress
			ret = LZ4_decompress_fast((char *)src, (char *)g_ciso_dec_buf, ISO_SECTOR_SIZE);
		}
		if(ret < 0) {
			ret = -20;
			printk("%s: -> %d\n", __func__, ret);
			return ret;
		}
		// copy the decompressed data to the destination buffer
		memcpy(dst, g_ciso_dec_buf + offset_shift, size);
	}

	return ret;
}

static int refresh_cso_index(u32 size, u32 offset) {
	// seek the first block offset
	u32 starting_block = offset / ISO_SECTOR_SIZE;

	// calculate the last needed block and read the index
	u32 ending_block = (offset + size) / ISO_SECTOR_SIZE + 1;

	u32 idx_size = (ending_block - starting_block + 1) * 4;

	if (idx_size > CISO_IDX_MAX_ENTRIES * 4) {
		// the requested index size is too big
		return -1;
	}

	// out of scope, read cso index table again
	if (starting_block < g_cso_idx_start_block|| ending_block >= g_cso_idx_start_block + CISO_IDX_MAX_ENTRIES) {

		u32 total_blocks = g_CISO_hdr.total_bytes / g_CISO_hdr.block_size;

		if (starting_block > total_blocks) {
			// the requested block goes beyond the max block number
			return -1;
		}

		if (starting_block + 4096 > total_blocks) {
			idx_size = (total_blocks - starting_block + 1) * 4;
		} else {
			idx_size = CISO_IDX_MAX_ENTRIES * 4;
		}

		int ret = read_raw_data(g_cso_idx_cache, idx_size, starting_block * 4 + 24);
		if(ret < 0) {
		    return ret;
		}

		g_cso_idx_start_block = starting_block;
		return 0;
	}

	return starting_block - g_cso_idx_start_block;
}

static int read_cso_data_ng(u8 *addr, u32 size, u32 offset)
{
	u32 cso_block;

	u32 start_blk = 0;
	u32 first_block_size = 0;
	u32 last_block_size = 0;

	u32 cso_read_offset, cso_read_size;

	if(offset > g_CISO_hdr.total_bytes) {
		// return if the offset goes beyond the iso size
		return 0;
	} else if(offset + size > g_CISO_hdr.total_bytes) {
		// adjust size if it tries to read beyond the game data
		size = g_CISO_hdr.total_bytes - offset;
	}

	if ((start_blk = refresh_cso_index(size, offset)) < 0) {
		//FIXME: fallback to slower read, try to get a bigger block instead
		// a game shouldn't request more than 8MiB in a single read so this
		// isn't executed in normal cases
		printk("Index for read of size %i is greater that allowed maximum\n", size);
		return read_cso_data(addr, size, offset);
	}

	// check if the first read is in the middle of a compressed block or if there is only one block
	if(!IS_DIVISIBLE(offset, ISO_SECTOR_SIZE) || size <= ISO_SECTOR_SIZE) {
		// calculate the offset and size of the compressed data
		cso_read_offset = GET_CSO_OFFSET(start_blk);
		cso_read_size = GET_CSO_OFFSET(start_blk + 1) - cso_read_offset;

		// READ #2 (only if the first block is a partial one)
		read_raw_data(g_ciso_block_buf, cso_read_size, cso_read_offset);

		u32 offset_shift = REMAINDER(offset, ISO_SECTOR_SIZE);

		// calculate the real size needed from the decompressed block
		if(offset_shift + size <= ISO_SECTOR_SIZE) {
			// if the size + offset shift is less than the sector size then
			// use the value directly for the first block size
			first_block_size = size;
		} else {
			// else use the remainder
			first_block_size = ISO_SECTOR_SIZE - offset_shift;
		}

		// decompress (if required)
		if(decompress_block(addr, g_ciso_block_buf, first_block_size, start_blk, offset_shift) < 0) {
			return -2;
		}

		// update size
		size -= first_block_size;

		// only one block to read, return early
		if(size == 0) {
			return first_block_size;
		}

		// update offset and addr
		offset += first_block_size;
		addr += first_block_size;

		start_blk++;
	}

	{
		// calculate the last block (or the remaining one)
		cso_block = size / 2048 + start_blk;

		// don't go over the next block if the read size occupies all of it
		if(IS_DIVISIBLE(size, ISO_SECTOR_SIZE)) {
			cso_block--;
		}

		cso_read_offset = GET_CSO_OFFSET(cso_block);

		// get the compressed block size
		cso_read_size = GET_CSO_OFFSET(cso_block + 1) - cso_read_offset;

		// READ #3 (only if the last block is a partial one)
		read_raw_data(g_ciso_block_buf, cso_read_size, cso_read_offset);

		// calculate the partial decompressed block size
		last_block_size = size % 2048;

		// update size
		size -= last_block_size;

		// calculate the offset to place the last decompressed block
		void *last_offset = addr + ((size / ISO_SECTOR_SIZE) * ISO_SECTOR_SIZE);

		if(decompress_block(last_offset, g_ciso_block_buf, last_block_size, cso_block, 0) < 0) {
			return -3;
		}

		// no more blocks
		if(size == 0) {
			return first_block_size +last_block_size;
		}
	}

	// calculate the needed blocks
	if(IS_DIVISIBLE(size, 2048)) {
		cso_block = size / 2048;
	} else {
		cso_block = size / 2048 + 1;
	}

	cso_read_offset = GET_CSO_OFFSET(start_blk);
	cso_read_size = GET_CSO_OFFSET(start_blk + cso_block) - cso_read_offset;

	// place the compressed blocks at the end of the provided buffer
	// so it can be reused in the decompression without overlap
	u32 shifted_offset = cso_block * 2048 - cso_read_size;

	// READ #4 (main section of compressed blocks)
	read_raw_data(addr + shifted_offset, cso_read_size, cso_read_offset);

	int i;
	u32 read_size = 0;

	// process every compressed block
	for(i = 0; i < cso_block ; i++) {
		// shift the source with the size of the last read
		void *src = addr + shifted_offset + read_size;

		// shift the destination, block by block
		void *dst = addr + i * ISO_SECTOR_SIZE;

		// calculate a size in case last block is a partial one and its
		// size is less that the sector size
		int dec_size = size < ISO_SECTOR_SIZE ? size : ISO_SECTOR_SIZE;

		if(decompress_block(dst, src, dec_size, i + start_blk, 0) < 0) {
			return -4;
		}

		cso_read_offset = GET_CSO_OFFSET(start_blk + i);
		u32 decompressed_size = GET_CSO_OFFSET(start_blk + i + 1) - cso_read_offset;
		read_size += decompressed_size;
	}

	return size + first_block_size + last_block_size;
}

// 0x00000C7C
int isoRead(struct IoReadArg *args)
{
	if(g_is_ciso != 0) {
		return read_cso_data_ng(args->address, args->size, args->offset);
	}

	return read_raw_data(args->address, args->size, args->offset);
}

// 0x000003E0
int isoReadStack(u32 offset, void *ptr, u32 data_len)
{
	int ret, retv;

	ret = sceKernelWaitSema(g_umd9660_sema_id, 1, 0);

	if(ret < 0) {
		return -1;
	}

	g_read_arg.offset = offset;
	g_read_arg.address = ptr;
	g_read_arg.size = data_len;
	retv = sceKernelExtendKernelStack(0x2000, (void*)&isoRead, &g_read_arg);

	ret = sceKernelSignalSema(g_umd9660_sema_id, 1);

	if(ret < 0) {
		return -1;
	}

	return retv;
}

