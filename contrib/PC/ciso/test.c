#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include <zlib.h>

enum{
    TYPE_ISO,
    TYPE_CSO,
    TYPE_ZSO,
    TYPE_DAX,
    TYPE_JSO
};

#define SECTOR_SIZE 0x800
#define DAX_BLOCK_SIZE 0x2000
#define DAX_FRAME_SHIFT 13
#define DAX_COMP_BUF 9216

#define ISO_MAGIC 0x30444301
#define CSO_MAGIC 0x4F534943 // CISO
#define ZSO_MAGIC 0x4F53495A // ZISO
#define DAX_MAGIC 0x00584144 // DAX
#define JSO_MAGIC 0x4F53494A // JISO

#define ISO_MAGIC_OFFSET 0x8000

#define FILE_TO_EXTRACT "ICON0.PNG"

#define min(x,y) ((x<y)?x:y)
#define MIN min

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

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

static DAXHeader g_CISO_hdr __attribute__((aligned(64)));
static DAXHeader* dax_header = (DAXHeader*)&g_CISO_hdr;
static JisoHeader* jiso_header = (JisoHeader*)&g_CISO_hdr;

static int g_cso_idx_start_block = -1;
static u8 g_ciso_block_buf[DAX_COMP_BUF] __attribute__((aligned(64)));
static u8 g_ciso_dec_buf[DAX_BLOCK_SIZE] __attribute__((aligned(64)));
static char* isopath = NULL;
static int ciso_type = -1;

void upperString(char* str){
    while (*str){
        if (*str >= 'a' && *str <= 'z')
            *str -= 0x20;
        str++;
    }
}

u32 getMagic(const char* filename, unsigned int offset){
    FILE* fp = fopen(filename, "rb");
    if (fp == NULL)
        return 0;
    u32 magic;
    fseek(fp, offset, SEEK_SET);
    fread(&magic, 4, 1, fp);
    fclose(fp);
    return magic;
}

static int read_raw_data(u8* addr, u32 size, u32 offset){
    FILE* fp = fopen(isopath, "rb");
    if (fp == NULL)
        return 0;
    fseek(fp, offset, SEEK_SET);
    int res = fread(addr, 1, size, fp);
    fclose(fp);
    return res;
}

void zlib_decompress(uint8_t *input, uint8_t* output, int type, int block_size)
{
    switch (type){
    case TYPE_CSO:
        //sctrlDeflateDecompress(output, input, SECTOR_SIZE);
        { // use zlib decompress
            z_stream z;
            memset(&z, 0, sizeof(z));
            inflateInit2(&z, -15);
            z.next_in = input;
	        z.avail_in = block_size;
	        z.next_out = output;
	        z.avail_out = block_size;
	        inflate(&z, Z_FINISH);
	        printf("total out: %lu\n", z.total_out);
	        inflateEnd(&z);
        }
        break;
    case TYPE_ZSO:
        LZ4_decompress_fast(input, output, block_size);
        break;
    case TYPE_DAX:
        { // use zlib decompress
            z_stream z;
            memset(&z, 0, sizeof(z));
            inflateInit2(&z, 15);
            z.next_in = input;
	        z.avail_in = DAX_COMP_BUF;
	        z.next_out = output;
	        z.avail_out = DAX_BLOCK_SIZE;
	        int res = 0;
	        res = inflate(&z, Z_FINISH);
	        printf("total out: %lu\n", z.total_out);
	        inflateEnd(&z);
	        /*
	        unsigned long dsize = out_size;
	        int res = uncompress(output, &dsize, input, in_size);
            ret = (int)dsize;
            */
            printf("res: %d\n", res);
        }
        break;
    case TYPE_JSO:
        {
        u32 d_size = block_size;
        lzo1x_decompress(input, block_size, output, &d_size, 0);
        }
        break;
    }
}


int getInitialBlock(FILE* fp, u8* block_out, u8* compressed, int ciso_type){

    if (fp == NULL)
        return 0;

    int block_size = 0;

    if (ciso_type == TYPE_DAX){
        block_size = DAX_BLOCK_SIZE;
    }
    else if (ciso_type == TYPE_JSO){
        u16 bs = 0;
        fseek(fp, 0x006, SEEK_SET);
        fread(&bs, 2, 1, fp);
        block_size = bs;
    }
    else{
        fseek(fp, 16, SEEK_SET);
        fread(&block_size, 4, 1, fp);
    }
    
    printf("block size: %d\n", block_size);
    
    int start_read = (32768 / block_size) * 4 + 24;
    if (ciso_type == TYPE_DAX)
        start_read += 8;
    else if (ciso_type == TYPE_JSO)
        start_read += 24;
    
    printf("start read: %d\n", start_read);
    
    unsigned fo, fs;
    fseek(fp, start_read, SEEK_SET);
    fread(&fo, 4, 1, fp);
    fread(&fs, 4, 1, fp);
    fs -= fo;
    
    printf("fo: %d\n", fo);
    printf("fs: %d\n", fs);
    
    /*
    if (ciso_type == TYPE_DAX){
        fs = DAX_BLOCK_SIZE;
    }
    else{
        fs = min((int)fs, SECTOR_SIZE);
    }
    */
    
    fseek(fp, fo, SEEK_SET);
    fread(compressed, 1, fs, fp);
    zlib_decompress(compressed, block_out, ciso_type, block_size);
    
    {
        FILE* fp = fopen("block_out1.bin", "wb");
        fwrite(block_out, 1, block_size, fp);
        fclose(fp);
    }
    
    unsigned idx = (block_out[158] + (block_out[159]<<8) + (block_out[160]<<16) + (block_out[161]<<24));
    
    printf("idx: %d\n", idx);
    
    unsigned start = idx * (4/(block_size/SECTOR_SIZE));
    if (ciso_type == TYPE_DAX){
        start += 30;
    }
    else if (ciso_type == TYPE_JSO){
        start += 52;
    }
    else{
        start += 28;
    }
    
    printf("start: %d\n", start);

    unsigned offset, size;    
    fseek(fp, start, SEEK_SET);
    fread(&offset, 4, 1, fp);
    fread(&size, 4, 1, fp);
    
    size -= offset;
    
    /*
    if (ciso_type == TYPE_DAX){
        size = DAX_BLOCK_SIZE;
    }
    else{
        
        size = min((int)size, SECTOR_SIZE);
    }
    */
    
    fseek(fp, offset, SEEK_SET);

    printf("offset: %d\n", offset);
    printf("size: %d\n", size);

    fread(compressed, 1, size, fp);
    zlib_decompress(compressed, block_out, ciso_type, block_size);
    
    if (block_size == SECTOR_SIZE){
        fseek(fp, start+4, SEEK_SET);
        fread(&offset, 4, 1, fp);
        fread(&size, 4, 1, fp);
        fseek(fp, offset, SEEK_SET);
        fread(compressed, 1, size, fp);
        zlib_decompress(compressed, block_out+SECTOR_SIZE, ciso_type, block_size);
    }
    
    {
        FILE* fp = fopen("block_out2.bin", "wb");
        fwrite(block_out, 1, DAX_BLOCK_SIZE, fp);
        fclose(fp);
    }

    return block_size;
}

void* fastExtract(const char* path, char* file, unsigned* size_out, int ciso_type){

    FILE* fp = fopen(path, "rb");
    if (fp == NULL)
        return NULL;
        
    static u8 block_out[DAX_BLOCK_SIZE] __attribute__((aligned(64)));
    static u8 compressed[DAX_COMP_BUF] __attribute__((aligned(64)));
    int block_size = getInitialBlock(fp, block_out, compressed, ciso_type);

    if (size_out != NULL)
        *size_out = 0;

    void* buffer = NULL;

    upperString(file);

    int pos = 0;

    while (true){

        if (pos >= DAX_BLOCK_SIZE){
            fclose(fp);
            return NULL;
        }

        char tmpText[128];
        memset(tmpText, 0, 128);
        strncpy(tmpText, (char*)&block_out[pos], strlen(file));
        upperString(tmpText);

        if (!strcmp(tmpText, file)){
            if (size_out == NULL){
                fclose(fp);
                return (void*)-1;
            }
            pos -= 31;
            
            unsigned b_offset, b_size, b_iter;
            b_offset = ((block_out[pos]) + (block_out[pos+1]<<8) + (block_out[pos+2]<<16) + (block_out[pos+3]<<24)) * (4/(block_size/SECTOR_SIZE));
            b_size = ((block_out[pos+8]) + (block_out[pos+9]<<8) + (block_out[pos+10]<<16) + (block_out[pos+11]<<24));
            b_iter = (int)ceil(b_size/(float)block_size) + 1;
            int trail_size = block_size - (((b_iter -1) * block_size) - b_size);
            if (ciso_type == TYPE_DAX){
                b_offset += 32;
            }
            else if (ciso_type == TYPE_JSO){
                b_offset += 48;
            }
            else{
                b_offset += 24;
            }

            printf("b_offset: %d\n", b_offset);
            printf("b_size: %d\n", b_size);
            printf("b_iter: %d\n", b_iter);

            fseek(fp, b_offset, SEEK_SET);

            unsigned buf_size;
            /*
            if (ciso_type == TYPE_DAX) buf_size = block_size*b_iter + trail_size;
            else if (ciso_type == TYPE_JSO) buf_size = block_size*b_iter + trail_size;
            else buf_size = SECTOR_SIZE*b_iter + trail_size;
            */
            buf_size = block_size*b_iter + trail_size;
            
            buffer = memalign(64, buf_size);
            u8* buf = (u8*)buffer;
            
            for (unsigned x = 1; x<b_iter; x++){

                unsigned cur_pos = ftell(fp);
                bool is_compressed = true;

                unsigned offset, size;

                fread(&offset, 4, 1, fp);

                if (offset >= 0x80000000){
                    is_compressed = false;
                    offset -= 0x80000000;
                }
                fread(&size, 4, 1, fp);
                if (size >= 0x80000000)
                    size -= 0x80000000;
                size -= offset;                
                    
                fseek(fp, offset, SEEK_SET);
                
                if (size == block_size) is_compressed = false;

                if (is_compressed){
                    if (x < b_iter - 1){
                        fread(compressed, 1, size, fp);
                        zlib_decompress(compressed, (uint8_t*)buf, ciso_type, block_size);
                        buf += block_size;
                    }
                    else{
                        fread(compressed, 1, size, fp);
                        zlib_decompress(compressed, (uint8_t*)buf, ciso_type, block_size);
                        buf += trail_size;
                    }
                }
                else{
                    fread((uint8_t*)buf, 1, size, fp);
                    buf += size;
                }
                fseek(fp, cur_pos+4, SEEK_SET);
            }

            if (size_out != NULL)
                *size_out = buf_size;

            fclose(fp);
            return buffer;
        }
        pos++;
    }
    fclose(fp);
    return NULL;
}

static int get_nsector(){
    return dax_header->uncompressed_size / DAX_BLOCK_SIZE;
}

int read_dax_data(u8* addr, u32 size, u32 offset)
{
	u32 cur_block;
	int pos, ret, read_bytes;
	u32 o_offset = offset;

	while(size > 0) {
		cur_block = offset / DAX_BLOCK_SIZE;
		pos = offset & (DAX_BLOCK_SIZE - 1);

		if(cur_block >= get_nsector()) {
			// EOF reached
			break;
		}

        u32 b_offset = 0; read_raw_data(&b_offset, sizeof(u32), 32 + (4*cur_block));
        
        printf("Reading at offset %u\n", b_offset);
        
        int ret = read_raw_data(g_ciso_block_buf, DAX_COMP_BUF, b_offset);
        
        printf("Ret: %d\n", ret);
        
        {
        FILE* fp = fopen("com.bin", "wb");
        fwrite(g_ciso_block_buf, 1, DAX_COMP_BUF, fp);
        fclose(fp);
        }
	    
        zlib_decompress(g_ciso_block_buf, g_ciso_dec_buf, ciso_type, DAX_BLOCK_SIZE);
        
        {
        FILE* fp = fopen("dec.bin", "wb");
        fwrite(g_ciso_dec_buf, 1, DAX_BLOCK_SIZE, fp);
        fclose(fp);
        }

		read_bytes = MIN(size, (DAX_BLOCK_SIZE - pos));
		memcpy(addr, g_ciso_dec_buf + pos, read_bytes);
		size -= read_bytes;
		addr += read_bytes;
		offset += read_bytes;
	}

	return offset - o_offset;
}

static int read_jiso_data(u8* addr, u32 size, u32 offset)
{
    u32 cur_block;
    u32 pos, ret, read_bytes;
    u32 o_offset = offset;
    
    u8* com_buf = g_ciso_block_buf;
    u8* dec_buf = g_ciso_dec_buf;
    
    if(offset > jiso_header->uncompressed_size) {
        // return if the offset goes beyond the iso size
        return 0;
    }
    else if(offset + size > jiso_header->uncompressed_size) {
        // adjust size if it tries to read beyond the game data
        size = jiso_header->uncompressed_size - offset;
    }
    
    while(size > 0) {
        // calculate block number and offset within block
        cur_block = offset / jiso_header->block_size;
        pos = offset & (jiso_header->block_size - 1);

        if(cur_block >= jiso_header->uncompressed_size/jiso_header->block_size) {
            // EOF reached
            break;
        }
        
        // read compressed block offset
        u32 b_offset; read_raw_data(&b_offset, sizeof(u32), sizeof(JisoHeader) + (4*cur_block));
        u32 b_size; read_raw_data(&b_size, sizeof(u32), sizeof(JisoHeader) + (4*cur_block) + 4);
        u32 d_size = jiso_header->block_size;
        b_size -= b_offset;

        // read block, skipping header if needed
        b_size = read_raw_data(com_buf, b_size, b_offset + (4*jiso_header->block_headers));

        // decompress block
        if (b_size == jiso_header->block_size) memcpy(dec_buf, com_buf, b_size);
        else{
            switch (jiso_header->method){
            case JISO_METHOD_LZO: lzo1x_decompress(com_buf, b_size, dec_buf, &d_size, 0); break;
            //case JISO_METHOD_ZLIB: sceKernelDeflateDecompress(dec_buf, jiso_header->block_size, com_buf, 0); break;
            }
        }        

        // read data from block into buffer
        read_bytes = MIN(size, (jiso_header->block_size - pos));
        memcpy(addr, dec_buf + pos, read_bytes);
        size -= read_bytes;
        addr += read_bytes;
        offset += read_bytes;
    }

    u32 res = offset - o_offset;
    
    return res;
}

void testIconExtract(int argc, char** argv){

    if (ciso_type == TYPE_ISO) return;

    printf("Ciso Type: %d\n", ciso_type);
    printf("Filename: %s\n", isopath);
    
    unsigned size_out;
    void* data = fastExtract(isopath, FILE_TO_EXTRACT, &size_out, ciso_type);
    
    if (data){
        FILE* fp = fopen(FILE_TO_EXTRACT, "wb");
        fwrite(data, 1, size_out, fp);
        fclose(fp);
    }
    else{
        printf("ERROR: could not read "FILE_TO_EXTRACT"\n");
    }
    
    printf("Is prome patched: %d\n", fastExtract(isopath, "EBOOT.OLD", NULL, ciso_type));
}

void testIsoExtract(int argc, char** argv){
    int size = 8 * 1024 * 1024; // read 8MiB
    unsigned char* data = malloc(size);
    int offset = ISO_MAGIC_OFFSET; // starting from ISO magic

    int res = 0;

    switch(ciso_type){
    case TYPE_ISO: res = read_raw_data(data, size, offset); break;
    case TYPE_DAX: res = read_dax_data(data, size, offset); break;
    case TYPE_JSO: res = read_jiso_data(data, size, offset); break;
    default: break;
    }
    
    printf("Read %d bytes\n", res);
    
    FILE* fp = fopen("dump.bin", "wb");
    fwrite(data, 1, size, fp);
    fclose(fp);
}

int main(int argc, char** argv){

    if (argc < 2){
        printf("Usage: %s file.dax\n", argv[0]);
        return;
    }

    isopath = argv[1];
    
    read_raw_data(dax_header, sizeof(DAXHeader), 0);
    
    u32 magic = dax_header->magic;
    if (magic == CSO_MAGIC) ciso_type = TYPE_CSO;
    else if (magic == ZSO_MAGIC) ciso_type = TYPE_ZSO;
    else if (magic == DAX_MAGIC) ciso_type = TYPE_DAX;
    else if (magic == JSO_MAGIC) ciso_type = TYPE_JSO;
    else{
        magic = getMagic(isopath, ISO_MAGIC_OFFSET);
        if (magic == ISO_MAGIC){
            ciso_type = TYPE_ISO;
        }
        else{
            printf("Unsuported format\n");
            return -1;
        }
    }
    
    testIconExtract(argc, argv);
    
    testIsoExtract(argc, argv);
    
    return 0;
}
