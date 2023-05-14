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
#include "globals.h"
#include "macros.h"

#define CSO_MAGIC 0x4F534943 // CISO
#define ZSO_MAGIC 0x4F53495A // ZISO
#define DAX_MAGIC 0x00584144 // DAX
#define JSO_MAGIC 0x4F53494A // JISO

#define DAX_BLOCK_SIZE 0x2000
#define DAX_COMP_BUF 0x2400

#define CISO_IDX_MAX_ENTRIES 2048 // will be adjusted according to CSO block_size

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
//static int g_CISO_cur_idx = 0;

static u32 *g_cso_idx_cache = NULL;
static int g_cso_idx_start_block = -1;
static int g_cso_idx_cache_num = 0;

// reader data
static u32 header_size;
static u32 block_size;
static u32 uncompressed_size;
static u32 block_header;
static u32 align;
static u32 g_ciso_total_block;

// reader functions
static int is_compressed = 0;
static void (*ciso_decompressor)(void* src, int src_len, void* dst, int dst_len, u32 topbit) = NULL;

// 0x00000368
static void wait_until_ms0_ready(void)
{
    int ret, status = 0, bootfrom;
    const char *drvname;

    drvname = "mscmhc0:";

    bootfrom = sceKernelBootFrom();
    #ifdef DEBUG
    printk("%s: bootfrom: 0x%08X\n", __func__, bootfrom);
    #endif
    if(bootfrom == 0x50) {
        drvname = "mscmhcemu0:";
    } else {
        // vsh mode?
        return;
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
    if(g_total_sectors <= 0) {
        SceOff off, total;

        off = sceIoLseek(g_iso_fd, 0, PSP_SEEK_CUR);
        total = sceIoLseek(g_iso_fd, 0, PSP_SEEK_END);
        sceIoLseek(g_iso_fd, off, PSP_SEEK_SET);

        g_total_sectors = total / ISO_SECTOR_SIZE;
    }

    return g_total_sectors;
}

#ifdef DEBUG
static int io_calls = 0;
#endif

// 0x00000BB4
static int read_raw_data(u8* addr, u32 size, u32 offset)
{
    int ret, i;
    SceOff ofs;
    i = 0;

    #ifdef DEBUG
    io_calls++;
    #endif
    
    do {
        i++;
        ofs = sceIoLseek(g_iso_fd, offset, PSP_SEEK_SET);

        if(ofs >= 0) {
            i = 0;
            break;
        } else {
            #ifdef DEBUG
            printk("%s: lseek retry %d error 0x%08X\n", __func__, i, (int)ofs);
            #endif
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
            #ifdef DEBUG
            printk("%s: read retry %d error 0x%08X\n", __func__, i, ret);
            #endif
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


/**
    The core of compressed iso reader.
    Abstracted to be compatible with all formats (CSO/ZSO/JSO/DAX).
    
    All compressed formats have the same overall structure:
    - A header followed by an array of block offsets (uint32).
    
    We only need to know the size of the header and some information from it.
    - block size: the size of a block once uncompressed.
    - uncompressed size: total size of the original (uncompressed) ISO file.
    - block header: size of block header if any (zlib header in DAX, JISO block_header, none for CSO/ZSO).
    - align: CISO block alignment (none for others).
    
    Some other Technical Information:
    - Block offsets can use the top bit to represent aditional information for the decompressor (NCarea, compression method, etc).
    - Block size is calculated via the difference with the next block. Works for DAX, allowing us to skip parsing block size array (with correction for last block).
    - Non-Compressed Area can be determined if size of compressed block is equal to size of uncompressed (equal or greater for CSOv2 due to padding).
    - This reader won't work for CSO/ZSO files above 4GB to avoid using 64 bit arithmetic, but can be easily adjustable.
    - This reader can compile and run on PC and other platforms, as long as datatypes are properly defined and read_raw_data() is properly implemented.
    
    Includes IO Speed improvements:
    - a cache for block offsets, so we reduce block offset IO.
    - reading the entire compressed data (or chunks of it) at the end of provided buffer to reduce block IO.

*/
static int read_compressed_data(u8* addr, u32 size, u32 offset)
{

    u32 cur_block;
    u32 pos, ret, read_bytes;
    u32 o_offset = offset;
    u32 g_ciso_total_block = uncompressed_size/block_size;
    u8* com_buf = g_ciso_block_buf;
    u8* dec_buf = g_ciso_dec_buf;
    u8* c_buf = NULL;
    u8* top_addr = addr+size;

    #ifdef DEBUG
    io_calls = 0;
    #endif
    
    if(offset > uncompressed_size) {
        // return if the offset goes beyond the iso size
        return 0;
    }
    else if(offset + size > uncompressed_size) {
        // adjust size if it tries to read beyond the game data
        size = uncompressed_size - offset;
    }

    // IO speedup tricks
    u32 starting_block = o_offset / block_size;
    u32 ending_block = ((o_offset+size)/block_size);
    
    // refresh index table if needed
    if (g_cso_idx_start_block < 0 || starting_block < g_cso_idx_start_block || starting_block-g_cso_idx_start_block+1 >= g_cso_idx_cache_num-1){
        read_raw_data(g_cso_idx_cache, g_cso_idx_cache_num*sizeof(u32), starting_block * sizeof(u32) + header_size);
        g_cso_idx_start_block = starting_block;
    }

    // Calculate total size of compressed data
    u32 o_start = (g_cso_idx_cache[starting_block-g_cso_idx_start_block]&0x7FFFFFFF)<<align;
    // last block index might be outside the block offset cache, better read it from disk
    u32 o_end;
    if (ending_block-g_cso_idx_start_block < g_cso_idx_cache_num-1){
        o_end = g_cso_idx_cache[ending_block-g_cso_idx_start_block];
    }
    else read_raw_data(&o_end, sizeof(u32), ending_block*sizeof(u32)+header_size); // read last two offsets
    o_end = (o_end&0x7FFFFFFF)<<align;
    u32 compressed_size = o_end-o_start;

    #ifdef DEBUG
    printf("(0)compressed size: %d, ", compressed_size);
    #endif

    // try to read at once as much compressed data as possible
    if (size > block_size*2){ // only if going to read more than two blocks
        if (size < compressed_size) compressed_size = size-block_size; // adjust chunk size if compressed data is still bigger than uncompressed
        c_buf = top_addr - compressed_size; // read into the end of the user buffer
        read_raw_data(c_buf, compressed_size, o_start);
    }

    while(size > 0) {
        // calculate block number and offset within block
        cur_block = offset / block_size;
        pos = offset & (block_size - 1);

        // check if we need to refresh index table
        if (cur_block-g_cso_idx_start_block >= g_cso_idx_cache_num-1){
            read_raw_data(g_cso_idx_cache, g_cso_idx_cache_num*sizeof(u32), cur_block*sizeof(u32) + header_size);
            g_cso_idx_start_block = cur_block;
        }
        
        // read compressed block offset and size
        u32 b_offset = g_cso_idx_cache[cur_block-g_cso_idx_start_block];
        u32 b_size = g_cso_idx_cache[cur_block-g_cso_idx_start_block+1];
        u32 topbit = b_offset&0x80000000; // extract top bit for decompressor
        b_offset = (b_offset&0x7FFFFFFF) << align;
        b_size = (b_size&0x7FFFFFFF) << align;
        b_size -= b_offset;

        if (cur_block == g_ciso_total_block-1 && header_size == sizeof(DAXHeader))
            // fix for last DAX block (you can't trust the value of b_size since there's no offset for last_block+1)
            b_size = DAX_COMP_BUF;

        // check if we need to (and can) read another chunk of data
        if (c_buf < addr || c_buf+b_size > top_addr){
            if (size > b_size+block_size){ // only if more than two blocks left, otherwise just use normal reading
                compressed_size = o_end-b_offset; // recalculate remaining compressed data
                if (size < compressed_size) compressed_size = size-block_size; // adjust if still bigger than uncompressed
                if (compressed_size >= b_size){
                    c_buf = top_addr - compressed_size; // read into the end of the user buffer
                    read_raw_data(c_buf, compressed_size, b_offset);
                }
            }
        }

        // read block, skipping header if needed
        if (c_buf >= addr && c_buf+b_size <= top_addr){
            memcpy(com_buf, c_buf+block_header, b_size); // fast read
            c_buf += b_size;
        }
        else{ // slow read
            b_size = read_raw_data(com_buf, b_size, b_offset + block_header);
            if (c_buf) c_buf += b_size;
        }

        // decompress block
        ciso_decompressor(com_buf, b_size, dec_buf, block_size, topbit);
    
        // read data from block into buffer
        read_bytes = MIN(size, (block_size - pos));
        memcpy(addr, dec_buf + pos, read_bytes);
        size -= read_bytes;
        addr += read_bytes;
        offset += read_bytes;
    }

    u32 res = offset - o_offset;

    #ifdef DEBUG
    printf("read %d bytes at %p took %d IO calls\n", res, o_offset, io_calls);
    #endif
    
    return res;
}

static void decompress_dax(void* src, int src_len, void* dst, int dst_len, u32 topbit){
    // use raw inflate with no NCarea check (DAX V0)
    sceKernelDeflateDecompress(dst, DAX_COMP_BUF, src, 0);
}

static void decompress_dax1(void* src, int src_len, void* dst, int dst_len, u32 topbit){
    // for DAX Version 1 we can skip parsing NC-Areas and just use the block_size trick as in JSO and CSOv2
    if (src_len == dst_len) memcpy(dst, src, dst_len); // check for NC area
    else sceKernelDeflateDecompress(dst, dst_len, src, 0); // use raw inflate
}

static void decompress_jiso(void* src, int src_len, void* dst, int dst_len, u32 topbit){
    // while JISO allows for DAX-like NCarea, it by default uses compressed size check
    if (src_len == dst_len) memcpy(dst, src, dst_len); // check for NC area
    else lzo1x_decompress(src, src_len, dst, &dst_len, 0); // use lzo
}

static void decompress_ciso(void* src, int src_len, void* dst, int dst_len, u32 topbit){
    if (topbit) memcpy(dst, src, dst_len); // check for NC area
    else sceKernelDeflateDecompress(dst, dst_len, src, 0);
}

static void decompress_ziso(void* src, int src_len, void* dst, int dst_len, u32 topbit){
    if (topbit) memcpy(dst, src, dst_len); // check for NC area
    else LZ4_decompress_fast(src, dst, dst_len);
}

static void decompress_cso2(void* src, int src_len, void* dst, int dst_len, u32 topbit){
    // in CSOv2, top bit represents compression method instead of NCarea
    if (src_len >= dst_len) memcpy(dst, src, dst_len); // check for NC area (JSO-like, but considering padding, thus >=)
    else if (topbit) LZ4_decompress_fast(src, dst, dst_len);
    else sceKernelDeflateDecompress(dst, dst_len, src, 0);
}

// 0x00000F00
static int is_ciso(SceUID fd)
{
    int ret;
    
    struct CISO_header g_CISO_hdr;

    g_CISO_hdr.magic = 0;

    sceIoLseek(fd, 0, PSP_SEEK_SET);
    ret = sceIoRead(fd, &g_CISO_hdr, sizeof(g_CISO_hdr));

    if(ret != sizeof(g_CISO_hdr)) {
        return -1;
    }

    u32 magic = g_CISO_hdr.magic;

    if(magic == CSO_MAGIC || magic == ZSO_MAGIC || magic == DAX_MAGIC || magic == JSO_MAGIC) { // CISO or ZISO or JISO or DAX
        u32 com_size = 0;
        // set reader and decompressor functions according to format
        if (magic == DAX_MAGIC){
            DAXHeader* dax_header = (DAXHeader*)&g_CISO_hdr;
            header_size = sizeof(DAXHeader);
            block_size = DAX_BLOCK_SIZE; // DAX uses static block size (8K)
            uncompressed_size = dax_header->uncompressed_size;
            block_header = 2; // skip over the zlib header (2 bytes)
            align = 0; // no alignment for DAX
            com_size = DAX_COMP_BUF;
            ciso_decompressor = (dax_header->version >= 1)? &decompress_dax1 : &decompress_dax;
        }
        else if (magic == JSO_MAGIC){
            JisoHeader* jiso_header = (JisoHeader*)&g_CISO_hdr;
            header_size = sizeof(JisoHeader);
            block_size = jiso_header->block_size;
            uncompressed_size = jiso_header->uncompressed_size;
            block_header = 4*jiso_header->block_headers; // if set to 1, each block has a 4 byte header, 0 otherwise
            align = 0; // no alignment for JISO
            com_size = jiso_header->block_size;
            ciso_decompressor = (jiso_header->method)? &decompress_dax1 : &decompress_jiso; //  zlib or lzo, depends on method
        }
        else{ // CSO/ZSO/v2
            header_size = sizeof(struct CISO_header);
            block_size = g_CISO_hdr.block_size;
            uncompressed_size = g_CISO_hdr.total_bytes;
            block_header = 0; // CSO/ZSO uses raw blocks
            align = g_CISO_hdr.align;
            com_size = block_size + (1 << g_CISO_hdr.align);
            if (g_CISO_hdr.ver == 2) ciso_decompressor = &decompress_cso2; // CSOv2 uses both zlib and lz4
            else ciso_decompressor = (magic == ZSO_MAGIC)? &decompress_ziso : &decompress_ciso; // CSO/ZSO v1 (zlib or lz4)
        }
        g_total_sectors = uncompressed_size / ISO_SECTOR_SIZE; // total number of DVD sectors (2K) in the original ISO.
        g_ciso_total_block = uncompressed_size / block_size;
        // for files with higher block sizes, we can reduce block cache size
        int ratio = block_size/ISO_SECTOR_SIZE;
        g_cso_idx_cache_num = CISO_IDX_MAX_ENTRIES/ratio;
        // lets use our own heap so that kram usage depends on game format (less heap needed for systemcontrol; better memory management)
        if (heapid < 0){
            heapid = sceKernelCreateHeap(PSP_MEMORY_PARTITION_KERNEL, (2*com_size) + (g_cso_idx_cache_num * 4) + 128, 1, "InfernoHeap");
            if (heapid<0){
                return -5;
            }
            // allocate buffer for decompressed block
            g_ciso_dec_buf = sceKernelAllocHeapMemory(heapid, com_size+64);
            if(g_ciso_dec_buf == NULL) {
                return -2;
            }
            if((u32)g_ciso_dec_buf & 63) // align 64
                g_ciso_dec_buf = (void*)(((u32)g_ciso_dec_buf & (~63)) + 64);
            // allocate buffer for compressed block
            g_ciso_block_buf = sceKernelAllocHeapMemory(heapid, com_size+64);
            if(g_ciso_block_buf == NULL) {
                return -3;
            }
            if((u32)g_ciso_block_buf & 63) // align 64
                g_ciso_block_buf = (void*)(((u32)g_ciso_block_buf & (~63)) + 64);
            // allocate buffer for block offset cache
            g_cso_idx_cache = sceKernelAllocHeapMemory(heapid, (g_cso_idx_cache_num * 4) + 64);
            if (g_cso_idx_cache == NULL) {
                return -4;
            }
            //if((u32)g_cso_idx_cache & 63) // align 64
            //    g_cso_idx_cache = (void*)(((u32)g_cso_idx_cache & (~63)) + 64);
        }
        return 1;
    } else {
        return 0;
    }
}

// 0x000009D4
int iso_open(void)
{
    int ret, retries;

    if (g_iso_fn == NULL || g_iso_fn[0] == 0) return -1;

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

    is_compressed = is_ciso(g_iso_fd);

    if (is_compressed < 0) return is_compressed;

    g_iso_opened = 1;
    g_total_sectors = get_nsector();

    return 0;
}

// 0x00000C7C
int iso_read(struct IoReadArg *args)
{
    if (is_compressed)
        return read_compressed_data(args->address, args->size, args->offset);
    return read_raw_data(args->address, args->size, args->offset);
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