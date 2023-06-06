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

#include "isoreader.h"
#include <string.h>
#include <pspiofilemgr.h>
#include <psputilsforkernel.h>
#include <pspsysmem_kernel.h>
#include "systemctrl_private.h"
#include "lz4.h"
#include "minilzo.h"
#include "macros.h"

#define MAX_RETRIES 8
#define MAX_DIR_LEVEL 8
#define ISO_STANDARD_ID "CD001"

#define CSO_MAGIC 0x4F534943 // CISO
#define ZSO_MAGIC 0x4F53495A // ZISO
#define DAX_MAGIC 0x00584144 // DAX
#define JSO_MAGIC 0x4F53494A // JISO

#define DAX_BLOCK_SIZE 0x2000
#define DAX_COMP_BUF 0x2400
#define CISO_IDX_MAX_ENTRIES 256

typedef struct _CISOHeader {
    uint32_t magic;            /* +00 : 'C','I','S','O'                           */
    u32 header_size;
    u64 total_bytes;    /* +08 : number of original data size              */
    u32 block_size;        /* +10 : number of compressed block size           */
    u8 ver;                /* +14 : version 01                                */
    u8 align;            /* +15 : align of index (offset = index[n]<<align) */
    u8 rsv_06[2];        /* +16 : reserved                                  */
} __attribute__ ((packed)) CISOHeader;

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

typedef unsigned int uint;

// header information
static u32 header_size;
static u32 uncompressed_size;
static u32 block_size;
static u32 block_header;
static u32 align;

// buffers
static u32* g_CISO_idx_cache = NULL; // block offset cache
static u8* ciso_com_buf = NULL; // compressed block
static u8* ciso_dec_buf = NULL; // decompressed block
static char* g_sector_buffer = NULL; // ISO sector

static int g_CISO_cur_idx = -1;
static int ciso_cur_block = -1;
static const char * g_filename = NULL;
static SceUID g_isofd = -1;
static u32 g_total_sectors = 0;

static int (*read_data)(void* addr, u32 size, u32 offset);
static void (*ciso_decompressor)(void* src, int src_len, void* dst, int dst_len, u32 topbit) = NULL;

static Iso9660DirectoryRecord g_root_record;

static void isoAlloc(){
    ciso_dec_buf = my_malloc(DAX_BLOCK_SIZE + 64);
    ciso_com_buf = my_malloc(DAX_COMP_BUF + 64);
    g_sector_buffer = my_malloc(SECTOR_SIZE);
    g_CISO_idx_cache = my_malloc(4*CISO_IDX_MAX_ENTRIES);
}

static void isoFree(){
    my_free(ciso_dec_buf);
    my_free(ciso_com_buf);
    my_free(g_sector_buffer);
    my_free(g_CISO_idx_cache);
}

static inline u32 isoPos2LBA(u32 pos)
{
    return pos / SECTOR_SIZE;
}

static inline u32 isoLBA2Pos(u32 lba, int offset)
{
    return lba * SECTOR_SIZE + offset;
}

static inline u32 isoPos2OffsetInSector(u32 pos)
{
    return pos & (SECTOR_SIZE - 1);
}

static inline u32 isoPos2RestSize(u32 pos)
{
    return SECTOR_SIZE - isoPos2OffsetInSector(pos);
}

static int reOpen(void)
{
    int retries = MAX_RETRIES, fd = -1;

    sceIoClose(g_isofd);

    while(retries -- > 0) {
        fd = sceIoOpen(g_filename, PSP_O_RDONLY, 0777);

        if (fd >= 0) {
            break;
        }

        sceKernelDelayThread(100000);
    }

    if (fd >= 0) {
        g_isofd = fd;
    }

    return fd;
}

static int read_raw_data(void* addr, u32 size, u32 offset)
{
    int ret, i;
    SceOff ofs;

    for(i=0; i<MAX_RETRIES; ++i) {
        ofs = sceIoLseek(g_isofd, offset, PSP_SEEK_SET);

        if (ofs >= 0) {
            break;
        } else {
            #ifdef DEBUG
            printk("%s: sceIoLseek -> 0x%08X\n", __func__, (int)ofs);
            #endif
            reOpen();
        }

        sceKernelDelayThread(100000);
    }

    for(i=0; i<MAX_RETRIES; ++i) {
        ret = sceIoRead(g_isofd, addr, size);

        if(ret >= 0) {
            break;
        } else {
            #ifdef DEBUG
            printk("%s: sceIoRead -> 0x%08X\n", __func__, ret);
            #endif
            reOpen();
            sceIoLseek(g_isofd, offset, PSP_SEEK_SET);
        }

        sceKernelDelayThread(100000);
    }

    return ret;
}

static void decompress_zlib(void* src, int src_len, void* dst, int dst_len, u32 topbit){
    sceKernelDeflateDecompress(dst, dst_len, src, 0); // use raw inflate
}

static void decompress_dax1(void* src, int src_len, void* dst, int dst_len, u32 topbit){
    if (src_len == dst_len) memcpy(dst, src, dst_len); // check for NC area
    else sceKernelDeflateDecompress(dst, dst_len, src, 0); // use raw inflate
}

static void decompress_ciso(void* src, int src_len, void* dst, int dst_len, u32 topbit){
    if (topbit) memcpy(dst, src, dst_len); // check for NC area
    else sceKernelDeflateDecompress(dst, dst_len, src, 0);
}

static void decompress_ziso(void* src, int src_len, void* dst, int dst_len, u32 topbit){
    if (topbit) memcpy(dst, src, dst_len); // check for NC area
    else LZ4_decompress_fast(src, dst, dst_len);
}

static void decompress_jiso(void* src, int src_len, void* dst, int dst_len, u32 topbit){
    if (src_len == dst_len) memcpy(dst, src, dst_len); // check for NC area
    else lzo1x_decompress(src, src_len, dst, &dst_len, 0); // use lzo
}

static void decompress_cso2(void* src, int src_len, void* dst, int dst_len, u32 topbit){
    if (src_len == dst_len) memcpy(dst, src, dst_len); // check for NC area
    else if (topbit) LZ4_decompress_fast(src, dst, dst_len);
    else sceKernelDeflateDecompress(dst, dst_len, src, 0);
}

static int read_compressed_data(u8* addr, u32 size, u32 offset)
{
    u32 cur_block;
    u32 pos, ret, read_bytes;
    u32 o_offset = offset;
    u32 g_ciso_total_block = uncompressed_size/block_size;
    u8* com_buf = PTR_ALIGN_64(ciso_com_buf);
    u8* dec_buf = PTR_ALIGN_64(ciso_dec_buf);
    u8* c_buf = NULL;
    u8* top_addr = addr+size;
    
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
    if (g_CISO_cur_idx < 0 || starting_block < g_CISO_cur_idx || starting_block-g_CISO_cur_idx+1 >= CISO_IDX_MAX_ENTRIES-1){
        read_raw_data(g_CISO_idx_cache, CISO_IDX_MAX_ENTRIES*sizeof(u32), starting_block * sizeof(u32) + header_size);
        g_CISO_cur_idx = starting_block;
    }

    // Calculate total size of compressed data
    u32 o_start = (g_CISO_idx_cache[starting_block-g_CISO_cur_idx]&0x7FFFFFFF)<<align;
    // last block index might be outside the block offset cache, better read it from disk
    u32 o_end;
    if (ending_block-g_CISO_cur_idx < CISO_IDX_MAX_ENTRIES-1){ //(ending_block-starting_block+1 < g_cso_idx_cache_num-1){
        o_end = g_CISO_idx_cache[ending_block-g_CISO_cur_idx];
    }
    else read_raw_data(&o_end, sizeof(u32), ending_block*sizeof(u32)+header_size); // read last two offsets
    o_end = (o_end&0x7FFFFFFF)<<align;
    u32 compressed_size = o_end-o_start;

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
        if (cur_block-g_CISO_cur_idx >= CISO_IDX_MAX_ENTRIES-1){
            read_raw_data(g_CISO_idx_cache, CISO_IDX_MAX_ENTRIES*sizeof(u32), cur_block * 4 + header_size);
            g_CISO_cur_idx = cur_block;
        }
        
        // read compressed block offset and size
        u32 b_offset = g_CISO_idx_cache[cur_block-g_CISO_cur_idx];
        u32 b_size = g_CISO_idx_cache[cur_block-g_CISO_cur_idx+1];
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
    
    return res;
}

static int readSector(u32 sector, u8* buf){
    return read_data(buf, SECTOR_SIZE, sector*SECTOR_SIZE);
}

static void normalizeName(char *filename)
{
    char *p;
   
    p = strstr(filename, ";1");

    if (p) {
        *p = '\0';
    }
}

static int findFile(const char * file, u32 lba, u32 dir_size, u32 is_dir, Iso9660DirectoryRecord *result_record)
{
    u32 pos;
    int ret;
    Iso9660DirectoryRecord *rec;
    char name[32];
    int re;

    pos = isoLBA2Pos(lba, 0);
    re = lba = 0;

    while ( re < dir_size ) {
        if (isoPos2LBA(pos) != lba) {
            lba = isoPos2LBA(pos);
            ret = readSector(lba, g_sector_buffer);

            if (ret != SECTOR_SIZE) {
                return -23;
            }
        }

        rec = (Iso9660DirectoryRecord*)&g_sector_buffer[isoPos2OffsetInSector(pos)];

        if(rec->len_dr == 0) {
            u32 remaining;

            remaining = isoPos2RestSize(pos);
            pos += remaining;
            re += remaining;
            continue;
        }
        
        #ifdef DEBUG
        if(rec->len_dr < rec->len_fi + sizeof(*rec)) {
            printk("%s: Corrupted directory record found in %s, LBA %d\n", __func__, g_filename, (int)lba);
        }
        #endif

        if(rec->len_fi > 32) {
            return -11;
        }

        if(rec->len_fi == 1 && rec->fi == 0) {
            if (0 == strcmp(file, ".")) {
                memcpy(result_record, rec, sizeof(*result_record));

                return 0;
            }
        } else if(rec->len_fi == 1 && rec->fi == 1) {
            if (0 == strcmp(file, "..")) {
                // didn't support ..
                return -19;
            }
        } else {
            memset(name, 0, sizeof(name));
            memcpy(name, &rec->fi, rec->len_fi);
            normalizeName(name);

            if (0 == strcmp(name, file)) {
                if (is_dir) {
                    if(!(rec->fileFlags & ISO9660_FILEFLAGS_DIR)) {
                        return -14;
                    }
                }

                memcpy(result_record, rec, sizeof(*result_record));

                return 0;
            }
        }

        pos += rec->len_dr;
        re += rec->len_dr;
    }

    return -18;
}

static int findPath(const char *path, Iso9660DirectoryRecord *result_record)
{
    int level = 0, ret;
    const char *cur_path, *next;
    u32 lba, dir_size;
    char cur_dir[32];

    if (result_record == NULL) {
        return -17;
    }

    memset(result_record, 0, sizeof(*result_record));
    lba = g_root_record.lsbStart;
    dir_size = g_root_record.lsbDataLength;

    cur_path = path;

    while(*cur_path == '/') {
        cur_path++;
    }

    next = strchr(cur_path, '/');

    while (next != NULL) {
        if (next-cur_path >= sizeof(cur_dir)) {
            return -15;
        }

        memset(cur_dir, 0, sizeof(cur_dir));
        strncpy(cur_dir, cur_path, next-cur_path);
        cur_dir[next-cur_path] = '\0';

        if (0 == strcmp(cur_dir, ".")) {
        } else if (0 == strcmp(cur_dir, "..")) {
            level--;
        } else {
            level++;
        }

        if(level > MAX_DIR_LEVEL) {
            return -16;
        }

        ret = findFile(cur_dir, lba, dir_size, 1, result_record);

        if (ret < 0) {
            return ret;
        }

        lba = result_record->lsbStart;
        dir_size = result_record->lsbDataLength;

        cur_path=next+1;

        // skip unwant path separator
        while(*cur_path == '/') {
            cur_path++;
        }
        
        next = strchr(cur_path, '/');
    }

    ret = findFile(cur_path, lba, dir_size, 0, result_record);

    return ret;
}

int isoOpen(const char *path)
{
    int ret;

    int k1 = pspSdkSetK1(0);

    if (g_isofd >= 0) {
        isoClose();
    }

    g_filename = path;

    if (reOpen() < 0) {
        #ifdef DEBUG
        printk("%s: open failed %s -> 0x%08X\n", __func__, g_filename, g_isofd);
        #endif
        ret = -2;
        goto error;
    }

    CISOHeader g_ciso_h;

    sceIoLseek(g_isofd, 0, PSP_SEEK_SET);
    memset(&g_ciso_h, 0, sizeof(g_ciso_h));
    ret = sceIoRead(g_isofd, &g_ciso_h, sizeof(g_ciso_h));

    if (ret != sizeof(g_ciso_h)) {
        ret = -9;
        goto error;
    }

    u32 magic = g_ciso_h.magic;
    
    read_data = &read_compressed_data;
    if (magic == CSO_MAGIC || magic == ZSO_MAGIC) {
        header_size = sizeof(CISOHeader);
        uncompressed_size = g_ciso_h.total_bytes;
        block_size = g_ciso_h.block_size;
        block_header = 0;
        align = g_ciso_h.align;
        g_total_sectors = g_ciso_h.total_bytes / SECTOR_SIZE;
        if (g_ciso_h.ver < 2){
            ciso_decompressor = (g_ciso_h.magic == ZSO_MAGIC)? &decompress_ziso : &decompress_ciso;
        }
        else{
            ciso_decompressor = &decompress_cso2;
        }
    }
    else if (magic == DAX_MAGIC){
        DAXHeader* dax_header = (DAXHeader*)&g_ciso_h;
        header_size = sizeof(DAXHeader);
        uncompressed_size = dax_header->uncompressed_size;
        block_size = DAX_BLOCK_SIZE;
        block_header = 2;
        align = 0;
        g_total_sectors = dax_header->uncompressed_size / SECTOR_SIZE;
        ciso_decompressor = (dax_header->version >= 1)? &decompress_dax1 : &decompress_zlib;
    }
    else if (magic == JSO_MAGIC){
        JisoHeader* jiso_header = (JisoHeader*)&g_ciso_h;
        header_size = sizeof(JisoHeader);
        uncompressed_size = jiso_header->uncompressed_size;
        block_size = jiso_header->block_size;
        block_header = 4*jiso_header->block_headers;
        align = 0;
        g_total_sectors = jiso_header->uncompressed_size / SECTOR_SIZE;
        ciso_decompressor = (jiso_header->method)? &decompress_dax1 : &decompress_jiso;
    }
    else {
        read_data = &read_raw_data;
        SceOff size, orig;
        orig = sceIoLseek(g_isofd, 0, PSP_SEEK_CUR);
        size = sceIoLseek(g_isofd, 0, PSP_SEEK_END);
        sceIoLseek(g_isofd, orig, PSP_SEEK_SET);
        g_total_sectors = isoPos2LBA((u32)size);
    }

    isoAlloc();

    ret = readSector(16, g_sector_buffer);

    if (ret != SECTOR_SIZE) {
        ret = -7;
        goto error;
    }

    if (memcmp(&g_sector_buffer[1], ISO_STANDARD_ID, sizeof(ISO_STANDARD_ID)-1)) {
        #ifdef DEBUG
        printk("%s: vol descriptor not found\n", __func__);
        #endif
        ret = -10;

        goto error;
    }

    memcpy(&g_root_record, &g_sector_buffer[0x9C], sizeof(g_root_record));

    return 0;

error:
    if (g_isofd >= 0) {
        isoClose();
    }
    pspSdkSetK1(k1);
    return ret;
}

int isoGetTotalSectorSize(void)
{
    return g_total_sectors;
}

void isoClose(void)
{
    int k1 = pspSdkSetK1(0);
    
    sceIoClose(g_isofd);
    g_isofd = -1;
    g_filename = NULL;

    isoFree();
    g_total_sectors = 0;
    ciso_cur_block = -1;
    g_CISO_cur_idx = -1;
    
    pspSdkSetK1(k1);
}

int isoGetFileInfo(char * path, u32 *filesize, u32 *lba)
{
    int ret = 0;
    Iso9660DirectoryRecord rec;
    int k1 = pspSdkSetK1(0);

    ret = findPath(path, &rec);

    if (ret >= 0) {
        if (lba){
           *lba = rec.lsbStart;
        }
        if (filesize) {
            *filesize = rec.lsbDataLength;
        }
    }
    
    pspSdkSetK1(k1);
    return ret;
}

int isoRead(void *buffer, u32 lba, int offset, u32 size)
{
    int k1 = pspSdkSetK1(0);
    u32 pos = isoLBA2Pos(lba, offset);
    int res = read_data(buffer, size, pos);
    pspSdkSetK1(k1);
    return res;
}
