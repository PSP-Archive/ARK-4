#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <malloc.h>
#include <zlib.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

#define MIN(x,y) ((x<y)?x:y)
#define SECTOR_SIZE 2048

#define CSO_MAGIC 0x4F534943 // CISO
#define ZSO_MAGIC 0x4F53495A // ZISO
#define DAX_MAGIC 0x00584144 // DAX
#define JSO_MAGIC 0x4F53494A // JISO

#define DAX_BLOCK_SIZE 0x2000
#define DAX_COMP_BUF 0x2400

#define CISO_IDX_MAX_ENTRIES 256

int g_cso_idx_cache_num = CISO_IDX_MAX_ENTRIES;

typedef struct 
{
    unsigned magic;
    unsigned header_size;
    unsigned long long file_size;
    unsigned block_size;
    unsigned char version;
    unsigned char align;
    char reserved[2];
} CSOHeader;

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
    JISO_METHOD_LZO     = 0,
    JISO_METHOD_ZLIB    = 1,
} JisoMethod;


// 0x000023D4
int g_total_sectors = -1;

// 0x000024C0
static u8 *g_ciso_block_buf = NULL;

// 0x000024C4, size CISO_DEC_BUFFER_SIZE + (1 << g_CISO_hdr.align), align 64
static u8 *g_ciso_dec_buf = NULL;

// 0x00002704
static int g_CISO_cur_idx = 0;

static u32 *g_cso_idx_cache = NULL;
static int g_cso_idx_start_block = -1;

// reader data
static u32 header_size;
static u32 block_size;
static u32 uncompressed_size;
static u32 block_header;
static u32 align;
static u32 g_ciso_total_block;

// reader functions
static int (*read_iso_data)(u8* addr, u32 size, u32 offset);
static void (*ciso_decompressor)(void* src, int src_len, void* dst, int dst_len, u32 topbit);

FILE* isofile = NULL;

int raw_calls = 0;
int read_raw_data(u8* addr, u32 size, u32 offset){
    raw_calls++;
    fseek(isofile, offset, SEEK_SET);
    int res = fread(addr, 1, size, isofile);
    return res;
}

int read_compressed_data(u8* addr, u32 size, u32 offset)
{

    raw_calls = 0;
    u32 o_size = size;

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
            read_raw_data(g_cso_idx_cache, g_cso_idx_cache_num*sizeof(u32), cur_block * 4 + header_size);
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
    
    printf("io calls: %d\n", raw_calls);
    if (raw_calls > 4){
        printf("Got high number of IO calls (%d) when reading %d bytes at %d\n", raw_calls, res, o_offset); 
    }
    
    if (res != o_size) printf("ERROR: got %d/%d\n", res, o_size);
    
    return res;
}

void sctrlDeflateDecompress(void* dst, void* src, u32 dst_len){
    z_stream zst;
    memset(&zst, 0, sizeof(zst));
    inflateInit2(&zst, -15);
    
    zst.next_in = src;
    zst.avail_in = -1;
    zst.next_out = dst;
    zst.avail_out = dst_len;
    int status = inflate(&zst, Z_FINISH);
}

static void decompress_zlib(void* src, int src_len, void* dst, int dst_len, u32 topbit){
    // use raw inflate with no NCarea check (DAX V0)
    sctrlDeflateDecompress(dst, src, dst_len); // use raw inflate
}

static void decompress_dax1(void* src, int src_len, void* dst, int dst_len, u32 topbit){
    // for DAX Version 1 we can skip parsing NC-Areas and just use the block_size trick as in JSO and CSOv2
    if (src_len == dst_len) memcpy(dst, src, dst_len); // check for NC area
    else sctrlDeflateDecompress(dst, src, dst_len); // use raw inflate
}

static void decompress_jiso(void* src, int src_len, void* dst, int dst_len, u32 topbit){
    // while JISO allows for DAX-like NCarea, it by default uses compressed size check
    if (src_len == dst_len) memcpy(dst, src, dst_len); // check for NC area
    else lzo1x_decompress((unsigned char*)src, src_len, (unsigned char*)dst, (unsigned int*)&dst_len, (void*)0); // use lzo
}

static void decompress_ciso(void* src, int src_len, void* dst, int dst_len, u32 topbit){
    if (topbit) memcpy(dst, src, dst_len); // check for NC area
    else sctrlDeflateDecompress(dst, src, dst_len); // use raw inflate
}

static void decompress_ziso(void* src, int src_len, void* dst, int dst_len, u32 topbit){
    if (topbit) memcpy(dst, src, dst_len); // check for NC area
    else LZ4_decompress_fast((const char*)src, (char*)dst, dst_len);
}

static void decompress_cso2(void* src, int src_len, void* dst, int dst_len, u32 topbit){
    // in CSOv2, top bit represents compression method instead of NCarea
    if (src_len == dst_len) memcpy(dst, src, dst_len); // check for NC area (JSO-like)
    else if (topbit) LZ4_decompress_fast((const char*)src, (char*)dst, dst_len);
    else sctrlDeflateDecompress(dst, src, dst_len); // use raw inflate
}

void* fastExtract(char* file, u32* size){
    
    void* buffer = NULL;
    if (size != NULL)
        *size = 0;
    
    g_cso_idx_start_block = -1;
    
    static u8 initial_block[SECTOR_SIZE*2];
    
    read_iso_data(initial_block, 12, 32926);
    
    unsigned dir_lba = ((unsigned*)initial_block)[0];
    unsigned block_size = ((unsigned*)initial_block)[2];
    unsigned dir_start = dir_lba*block_size + block_size;
    
    read_iso_data(initial_block, sizeof(initial_block), dir_start);
    
    FILE* fp = fopen("initial_block.bin", "wb");
    fwrite(initial_block, 1, sizeof(initial_block), fp);
    fclose(fp);
    
    for (int i=0; i<sizeof(initial_block); i++){
        if (strcasecmp((const char*)&initial_block[i], file) == 0){
            if (size == NULL){
                return (void*)-1;
            }
            u8* sfo = (u8*)&initial_block[i-31];
            u32 offset = (sfo[0] + (sfo[1]<<8) + (sfo[2]<<16) + (sfo[3]<<24))*block_size;
            *size = (sfo[8] + (sfo[9]<<8) + (sfo[10]<<16) + (sfo[11]<<24));
            
            if (*size == 0){
                return NULL;
            }
            
            void* buffer = malloc(*size);
            read_iso_data(buffer, *size, offset);
            return buffer;
        }
    }
    
    return NULL;
}

int main(int argc, char** argv){

    if (argc < 2){
        printf("Usage: inferno isopath\n");
        return -1;
    }
    
    g_ciso_block_buf = memalign(64, DAX_COMP_BUF);
    g_ciso_dec_buf = memalign(64, DAX_BLOCK_SIZE);
    g_cso_idx_cache = malloc(sizeof(u32)*CISO_IDX_MAX_ENTRIES);
    
    printf("ISO file: %s\n", argv[1]);
    
    isofile = fopen(argv[1], "rb");
    
    CSOHeader header;
    DAXHeader* dax_header = (DAXHeader*)&header;
    JisoHeader* jiso_header = (JisoHeader*)&header;
    read_raw_data(&header, sizeof(header), 0);
    
    read_iso_data = &read_compressed_data;
    
    switch (header.magic){
        case ZSO_MAGIC:
            printf("ZSO detected\n");
        case CSO_MAGIC:
            printf("CSO detected\n");
            header_size = sizeof(CSOHeader);
            block_size = header.block_size;
            uncompressed_size = header.file_size;
            block_header = 0;
            align = header.align;
            if (header.version == 2) ciso_decompressor = &decompress_cso2;
            else ciso_decompressor = (header.magic == ZSO_MAGIC)? &decompress_ziso : &decompress_ciso;
            break;
        case DAX_MAGIC:
            printf("DAX detected\n");
            header_size = sizeof(DAXHeader);
            block_size = DAX_BLOCK_SIZE;
            uncompressed_size = dax_header->uncompressed_size;
            block_header = 2;
            align = 0;
            ciso_decompressor = (dax_header->version >= 1)? &decompress_dax1 : &decompress_zlib;
            break;
        case JSO_MAGIC:
            printf("JSO detected\n");
            header_size = sizeof(JisoHeader);
            block_size = jiso_header->block_size;
            uncompressed_size = jiso_header->uncompressed_size;
            block_header = 4*jiso_header->block_headers;
            align = 0;
            ciso_decompressor = (jiso_header->method)? &decompress_dax1 : &decompress_jiso;
            break;
        default: // plain ISO
            printf("ISO detected\n");
            read_iso_data = &read_raw_data;
            break;
    }
    
    printf("Magic: %p\n", header.magic);
    
    g_ciso_total_block = uncompressed_size/block_size;
    
    printf("block size: %d\n", block_size);
    printf("ISO size: %d\n", uncompressed_size);
    printf("block header: %d\n", block_header);
    printf("align: %d\n", align);
    printf("total blocks: %d\n", g_ciso_total_block);
    
    static u8 buf[2048*1000];
    
    // do arbitrary reads
    read_compressed_data(buf, 14, 16);
    read_compressed_data(buf, sizeof(header), 16*2048+1);
    read_compressed_data(buf, sizeof(header)+2048, 16*2048+135);
    read_compressed_data(buf, sizeof(header)+2048*2, 16*2048+135);
    read_compressed_data(buf, sizeof(header)+2048*3, 16*2048+135);
    read_compressed_data(buf, sizeof(buf)-sizeof(header), 64*2048+10);
    read_compressed_data(buf, 6543, 5483);
    read_compressed_data(buf, 65536, 1040527360);
    
    read_compressed_data(buf, 65536, 1075742720); // 5 calls
    
    read_compressed_data(buf, 43008, 0x2fd23800);
    read_compressed_data(buf, 10464, 0x33c82000);
    read_compressed_data(buf, 10240, 0x2a9c5000);
    

    
    // do one big read
    FILE* fp;
    //read_compressed_data(buf, sizeof(buf), 16*2048);
    fp = fopen("output.bin", "wb");
    fwrite(buf, 1, 10240, fp);
    fclose(fp);
    
    fp = fopen("ICON0.PNG", "wb");
    u32 size; void* iconbuf = fastExtract("ICON0.PNG", &size);
    fwrite(iconbuf, 1, size, fp);
    fclose(fp);
    
    fclose(isofile);
    
    printf("finished\n");
    
    return 0;

}
