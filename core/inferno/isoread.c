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
#include <zlib.h>
#include "systemctrl.h"
#include "systemctrl_se.h"
#include "systemctrl_private.h"
#include "inferno.h"
#include "lz4.h"
#include "globals.h"
#include "macros.h"

#define CSO_MAGIC 0x4F534943 // CISO
#define ZSO_MAGIC 0x4F53495A // ZISO
#define DAX_MAGIC 0x00584144 // DAX
#define JSO_MAGIC 0x4F53494A // JISO

#define DAX_BLOCK_SIZE 0x2000
#define DAX_COMP_BUF 0x2400

#define CISO_IDX_MAX_ENTRIES 4096
// 0x00002784
struct IoReadArg g_read_arg;

SceUID heapid = -1;

// 0x0000248C
int g_iso_opened = 0;

// 0x000023D0
SceUID g_iso_fd = -1;

// 0x000023D4
int g_total_sectors = -1;

// 0x000024C0
static u8 *g_ciso_block_buf = NULL;

// 0x000024C4, size CISO_DEC_BUFFER_SIZE + (1 << g_CISO_hdr.align), align 64
static u8 *g_ciso_dec_buf = NULL;

// 0x00002704
static int g_CISO_cur_idx = 0;

// 0x00002700
static u32 g_ciso_dec_buf_offset = (u32)-1;

static int g_ciso_dec_buf_size = 0;

// 0x00002720
static u32 g_ciso_total_block = 0;

struct CISO_header {
    uint32_t magic;  // 0
    u32 header_size;  // 4
    u64 total_bytes; // 8
    u32 block_size; // 16
    u8 ver; // 20
    u8 align;  // 21
    u8 rsv_06[2];  // 22
};

typedef struct{ 
    uint32_t magic;
    uint32_t uncompressed_size;
    uint32_t version; 
    uint32_t nc_areas; 
    uint32_t unused[4]; 
} DAXHeader;

typedef struct _JisoHeader {
    uint32_t magic; // [0x000] 'JISO'
    uint8_t unk_x001; // [0x004] 0x03?
    uint8_t unk_x002; // [0x005] 0x01?
    uint16_t block_size; // [0x006] Block size, usually 2048.
    // TODO: Are block_headers and method 8-bit or 16-bit?
    uint8_t block_headers; // [0x008] Block headers. (1 if present; 0 if not.)
    uint8_t unk_x009; // [0x009]
    uint8_t method; // [0x00A] Method. (See JisoAlgorithm_e.)
    uint8_t unk_x00b; // [0x00B]
    uint32_t uncompressed_size; // [0x00C] Uncompressed data size.
    uint8_t md5sum[16]; // [0x010] MD5 hash of the original image.
    uint32_t header_size; // [0x020] Header size? (0x30)
    uint8_t unknown[12]; // [0x024]
} JisoHeader;

typedef enum {
	JISO_METHOD_LZO		= 0,
	JISO_METHOD_ZLIB	= 1,
} JisoMethod;

static struct CISO_header g_CISO_hdr __attribute__((aligned(64)));
static DAXHeader* dax_header = (DAXHeader*)&g_CISO_hdr;
static JisoHeader* jiso_header = (JisoHeader*)&g_CISO_hdr;

static u32 *g_cso_idx_cache = NULL;
static u32 g_cso_idx_start_block = -1;

static int (*read_iso_data)(u8* addr, u32 size, u32 offset);
static int read_raw_data(u8* addr, u32 size, u32 offset);
static int read_ciso_data(u8* addr, u32 size, u32 offset);
static int read_ziso_data(u8* addr, u32 size, u32 offset);
static int read_jiso_data(u8* addr, u32 size, u32 offset);
static int read_ciso2_data(u8* addr, u32 size, u32 offset);
static int read_dax_data(u8* addr, u32 size, u32 offset);


// 0x00000368
static void wait_until_ms0_ready(void)
{
    int ret, status = 0, bootfrom;
    const char *drvname;

    drvname = "mscmhc0:";

    if(psp_model == PSP_GO) {
        bootfrom = sceKernelBootFrom();
        printk("%s: bootfrom: 0x%08X\n", __func__, bootfrom);

        if(bootfrom == 0x50) {
            drvname = "mscmhcemu0:";
        } else {
            // vsh mode?
            return;
        }
    }

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

// 0x00000E58
static int get_nsector(void)
{
    if(g_ciso_total_block <= 0) {
        SceOff off, total;

        off = sceIoLseek(g_iso_fd, 0, PSP_SEEK_CUR);
        total = sceIoLseek(g_iso_fd, 0, PSP_SEEK_END);
        sceIoLseek(g_iso_fd, off, PSP_SEEK_SET);

        g_ciso_total_block = total / ISO_SECTOR_SIZE;
    }

    return g_ciso_total_block;
}

// 0x00000F00
static int is_ciso(SceUID fd)
{
    int ret;

    g_CISO_hdr.magic = 0;
    g_ciso_dec_buf_offset = (u32)-1;
    g_ciso_dec_buf_size = 0;

    sceIoLseek(fd, 0, PSP_SEEK_SET);
    ret = sceIoRead(fd, &g_CISO_hdr, sizeof(g_CISO_hdr));

    if(ret != sizeof(g_CISO_hdr)) {
        ret = -1;
        printk("%s: -> %d\n", __func__, ret);
        goto exit;
    }

    u32 magic = g_CISO_hdr.magic;

    if(magic == CSO_MAGIC || magic == ZSO_MAGIC || magic == DAX_MAGIC || magic == JSO_MAGIC) { // CISO or ZISO or JISO or DAX
        g_CISO_cur_idx = -1;
        
        u32 dec_size = 0;
        u32 com_size = 0;
        
        if (magic == DAX_MAGIC){
            g_ciso_total_block = dax_header->uncompressed_size / DAX_BLOCK_SIZE;
            dec_size = DAX_BLOCK_SIZE;
            com_size = DAX_COMP_BUF;
            read_iso_data = &read_dax_data;
        }
        else if (magic == JSO_MAGIC){
            g_ciso_total_block = jiso_header->uncompressed_size / jiso_header->block_size;
            dec_size = jiso_header->block_size;
            com_size = jiso_header->block_size + ISO_SECTOR_SIZE/4;
            read_iso_data = &read_jiso_data;
        }
        else{
            g_ciso_total_block = g_CISO_hdr.total_bytes / g_CISO_hdr.block_size;
            dec_size = g_CISO_hdr.block_size;
            com_size = dec_size + (1 << g_CISO_hdr.align);
            if (g_CISO_hdr.ver == 2) read_iso_data = &read_ciso2_data;
            else read_iso_data = (magic == ZSO_MAGIC)? &read_ziso_data : &read_ciso_data;
        }
        if (heapid<0){
            heapid = sceKernelCreateHeap(PSP_MEMORY_PARTITION_KERNEL, dec_size + com_size + (CISO_IDX_MAX_ENTRIES * 4) + 256, 1, "InfernoHeap");
            if (heapid<0){
                ret = -5;
                goto exit;
            }
        }
        if(g_ciso_dec_buf == NULL) {
            g_ciso_dec_buf = sceKernelAllocHeapMemory(heapid, dec_size+64); //oe_malloc(dec_size + 64);
            if(g_ciso_dec_buf == NULL) {
                ret = -2;
                goto exit;
            }
            if((u32)g_ciso_dec_buf & 63) // align 64
                g_ciso_dec_buf = (void*)(((u32)g_ciso_dec_buf & (~63)) + 64);
        }
        if(g_ciso_block_buf == NULL) {
            g_ciso_block_buf = sceKernelAllocHeapMemory(heapid, com_size+64); //oe_malloc(com_size + 64);
            if(g_ciso_block_buf == NULL) {
                ret = -3;
                goto exit;
            }
            if((u32)g_ciso_block_buf & 63) // align 64
                g_ciso_block_buf = (void*)(((u32)g_ciso_block_buf & (~63)) + 64);
        }
        if (g_cso_idx_cache == NULL) {
            g_cso_idx_cache = sceKernelAllocHeapMemory(heapid, (CISO_IDX_MAX_ENTRIES * 4) + 64); //oe_malloc((CISO_IDX_MAX_ENTRIES * 4) + 64);
            if (g_cso_idx_cache == NULL) {
                ret = -4;
                goto exit;
            }
            if((u32)g_cso_idx_cache & 63) // align 64
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
int iso_open(void)
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

    if (is_ciso(g_iso_fd) == 0x8002012F)
        read_iso_data = &read_raw_data;

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
            iso_open();
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
            iso_open();
        }
    }

    if(i == 16) {
        ret = 0x80010013;
        goto exit;
    }

exit:
    return ret;
}

static int read_compressed_data_generic(u8* addr, u32 size, u32 offset,
    u32 header_size, u32 block_size, u32 uncompressed_size, u32 block_skip, u32 align,
    void (*decompress)(void* src, int src_len, void* dst, int dst_len, u32 is_nc)
)
{
    u32 cur_block;
    u32 pos, ret, read_bytes;
    u32 o_offset = offset;
    
    u8* com_buf = g_ciso_block_buf;
    u8* dec_buf = g_ciso_dec_buf;
    
    if(offset > uncompressed_size) {
        // return if the offset goes beyond the iso size
        return 0;
    }
    else if(offset + size > uncompressed_size) {
        // adjust size if it tries to read beyond the game data
        size = uncompressed_size - offset;
    }
    
    // refresh index table if needed
    u32 starting_block = o_offset / block_size;
    u32 ending_block = ((o_offset+size) / block_size) + 1;
    if (g_cso_idx_start_block < 0 || starting_block < g_cso_idx_start_block || ending_block >= g_cso_idx_start_block + CISO_IDX_MAX_ENTRIES){
        u32 idx_size = 0;
        if (starting_block + CISO_IDX_MAX_ENTRIES > g_total_sectors) {
            idx_size = (g_total_sectors - starting_block + 1) * 4;
        } else {
            idx_size = CISO_IDX_MAX_ENTRIES * 4;
        }
        read_raw_data(g_cso_idx_cache, idx_size, starting_block * 4 + header_size);
        g_cso_idx_start_block = starting_block;
    }
    
    while(size > 0) {
        // calculate block number and offset within block
        cur_block = offset / block_size;
        pos = offset & (block_size - 1);

        if(cur_block >= g_total_sectors) {
            // EOF reached
            break;
        }
        
        if (cur_block>=g_cso_idx_start_block+CISO_IDX_MAX_ENTRIES){
            // refresh index cache
            read_raw_data(g_cso_idx_cache, CISO_IDX_MAX_ENTRIES, cur_block * 4 + header_size);
            g_cso_idx_start_block = cur_block;
        }
        
        // read compressed block offset
        u32 b_offset = g_cso_idx_cache[cur_block-g_cso_idx_start_block];
        u32 b_size = g_cso_idx_cache[cur_block-g_cso_idx_start_block+1];
        u32 is_nc = b_offset>>31;
        b_offset &= 0x7FFFFFFF;
        b_size &= 0x7FFFFFFF;
        b_size -= b_offset;

        if (cur_block == g_total_sectors-1 && header_size == sizeof(DAXHeader))
            b_size = DAX_COMP_BUF; // fix for last DAX block

        if (align){
            b_size += 1 << align;
            b_offset = b_offset << align;
        }

        // read block, skipping header if needed
        b_size = read_raw_data(com_buf, b_size, b_offset + block_skip);

        // decompress block
        decompress(com_buf, b_size, dec_buf, block_size, is_nc);

        // read data from block into buffer
        read_bytes = MIN(size, (block_size - pos));
        memcpy(addr, dec_buf + pos, read_bytes);
        size -= read_bytes;
        addr += read_bytes;
        offset += read_bytes;
    }

    u32 res = offset - o_offset;
    
    return res;
}

static void decompress_zlib(void* src, int src_len, void* dst, int dst_len, u32 is_nc){
    sceKernelDeflateDecompress(dst, dst_len, src, 0); // use raw inflate
}

static void decompress_dax1(void* src, int src_len, void* dst, int dst_len, u32 is_nc){
    if (src_len >= dst_len) memcpy(dst, src, dst_len); // check for NC area
    else sceKernelDeflateDecompress(dst, dst_len, src, 0); // use raw inflate
}

static void decompress_lzo(void* src, int src_len, void* dst, int dst_len, u32 is_nc){
    if (src_len >= dst_len) memcpy(dst, src, dst_len); // check for NC area
    else lzo1x_decompress(src, src_len, dst, &dst_len, 0); // use lzo
}

static void decompress_ciso(void* src, int src_len, void* dst, int dst_len, u32 is_nc){
    if (is_nc) memcpy(dst, src, dst_len); // check for NC area
    else sceKernelDeflateDecompress(dst, dst_len, src, 0);
}

static void decompress_ziso(void* src, int src_len, void* dst, int dst_len, u32 is_nc){
    if (is_nc) memcpy(dst, src, dst_len); // check for NC area
    else LZ4_decompress_fast(src, dst, dst_len);
}

static void decompress_cso2(void* src, int src_len, void* dst, int dst_len, u32 is_nc){
    if (src_len >= dst_len) memcpy(dst, src, dst_len); // check for NC area
    else if (is_nc) LZ4_decompress_fast(src, dst, dst_len);
    else sceKernelDeflateDecompress(dst, dst_len, src, 0);
}

static int read_ciso_data(u8* addr, u32 size, u32 offset){
    // CISO
    return read_compressed_data_generic(
        addr, size, offset,
        sizeof(struct CISO_header), g_CISO_hdr.block_size,
        g_CISO_hdr.total_bytes, 0, g_CISO_hdr.align,
        &decompress_ciso
    );
}

static int read_ziso_data(u8* addr, u32 size, u32 offset){
    // ZISO
    return read_compressed_data_generic(
        addr, size, offset,
        sizeof(struct CISO_header), g_CISO_hdr.block_size,
        g_CISO_hdr.total_bytes, 0, g_CISO_hdr.align,
        &decompress_ziso
    );
}

static int read_jiso_data(u8* addr, u32 size, u32 offset){
    // JISO
    return read_compressed_data_generic(
        addr, size, offset,
        sizeof(JisoHeader), jiso_header->block_size,
        jiso_header->uncompressed_size, 4*jiso_header->block_headers, 0,
        (jiso_header->method)? &decompress_dax1 : &decompress_lzo
    );
}

static int read_ciso2_data(u8* addr, u32 size, u32 offset){
    // CISOv2
    return read_compressed_data_generic(
        addr, size, offset,
        sizeof(struct CISO_header), g_CISO_hdr.block_size,
        g_CISO_hdr.total_bytes, 0, g_CISO_hdr.align,
        &decompress_cso2
    );
}

static int read_dax_data(u8* addr, u32 size, u32 offset){
    // DAX
    return read_compressed_data_generic(
        addr, size, offset,
        sizeof(DAXHeader), DAX_BLOCK_SIZE,
        dax_header->uncompressed_size, 2, 0,
        // for DAX Version 1 we can skip parsing NC-Areas and just use the block_size trick as in JSO and CSOv2
        (dax_header->version >= 1)? &decompress_dax1 : &decompress_zlib
    );
}

// 0x00000C7C
int iso_read(struct IoReadArg *args)
{
    return read_iso_data(args->address, args->size, args->offset);
}

// 0x000003E0
int iso_read_with_stack(u32 offset, void *ptr, u32 data_len)
{
    int ret, retv;

    ret = sceKernelWaitSema(g_umd9660_sema_id, 1, 0);

    if(ret < 0) {
        return -1;
    }

    g_read_arg.offset = offset;
    g_read_arg.address = ptr;
    g_read_arg.size = data_len;
    retv = sceKernelExtendKernelStack(0x2000, (void*)&iso_cache_read, &g_read_arg);

    ret = sceKernelSignalSema(g_umd9660_sema_id, 1);

    if(ret < 0) {
        return -1;
    }

    return retv;
}
