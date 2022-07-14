#include "iso.h"

using namespace std;

#define CISO_IDX_MAX_ENTRIES 512

static int g_ciso_total_block;
static int g_cso_idx_start_block = -1;
static u8 g_ciso_block_buf[DAX_COMP_BUF] __attribute__((aligned(64)));
static u8 g_ciso_dec_buf[DAX_BLOCK_SIZE] __attribute__((aligned(64)));
static u32 g_cso_idx_cache[CISO_IDX_MAX_ENTRIES];

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

Iso :: Iso()
{
};

Iso :: Iso(string path)
{
    this->path = path;
    size_t lastSlash = path.rfind("/", string::npos);
    this->name = path.substr(lastSlash+1, string::npos);
    this->icon0 = common::getImage(IMAGE_WAITICON);
    this->read_iso_data = &Iso::read_compressed_data;
    block_size = SECTOR_SIZE;
    
    CSOHeader header;
    DAXHeader* dax_header = (DAXHeader*)&header;
    JisoHeader* jiso_header = (JisoHeader*)&header;
    this->read_raw_data((u8*)&header, sizeof(CSOHeader), 0);
    u32 magic = common::getMagic(path.c_str(), 0);
    switch (magic){
        case CSO_MAGIC:
        case ZSO_MAGIC:
            header_size = sizeof(CSOHeader);
            block_size = header.block_size;
            uncompressed_size = header.file_size;
            block_header = 0;
            align = header.align;
            if (header.version == 2) ciso_decompressor = &decompress_cso2;
            else ciso_decompressor = (header.magic == ZSO_MAGIC)? &decompress_ziso : &decompress_ciso;
            break;
        case DAX_MAGIC:
            header_size = sizeof(DAXHeader);
            block_size = DAX_BLOCK_SIZE;
            uncompressed_size = dax_header->uncompressed_size;
            block_header = 2;
            align = 0;
            ciso_decompressor = (dax_header->version >= 1)? &decompress_dax1 : &decompress_zlib;
            break;
        case JSO_MAGIC:
            header_size = sizeof(JisoHeader);
            block_size = jiso_header->block_size;
            uncompressed_size = jiso_header->uncompressed_size;
            block_header = 4*jiso_header->block_headers;
            align = 0;
            ciso_decompressor = (jiso_header->method)? &decompress_dax1 : &decompress_jiso;
            break;
        default: // plain ISO
            this->read_iso_data = &Iso::read_raw_data;
            break;
    }
    g_ciso_total_block = uncompressed_size/block_size;
};

Iso :: ~Iso()
{
    if (this->icon0 != common::getImage(IMAGE_NOICON) && this->icon0 != common::getImage(IMAGE_WAITICON))
        delete this->icon0;
};

void Iso::loadIcon(){
    Image* icon = NULL;
    unsigned size;
    void* buffer = Iso::fastExtract("ICON0.PNG", &size);
    if (buffer != NULL){
        icon = new Image(buffer, YA2D_PLACE_RAM);
        free(buffer);
    }
    if (icon == NULL)
        sceKernelDelayThread(50000);
    icon = (icon == NULL)? common::getImage(IMAGE_NOICON) : icon;
    icon->swizzle();
    this->icon0 = icon;
}

void Iso::getTempData1(){
    this->pic0 = NULL;
    this->pic1 = NULL;

    void* buffer = NULL;
    unsigned size;

    // grab pic0.png
    buffer = Iso::fastExtract("PIC0.PNG", &size);
    if (buffer != NULL){
        this->pic0 = new Image(buffer, YA2D_PLACE_RAM);
        free(buffer);
        buffer = NULL;
    }

    // grab pic1.png
    buffer = Iso::fastExtract("PIC1.PNG", &size);
    if (buffer != NULL){
        this->pic1 = new Image(buffer, YA2D_PLACE_RAM);
        free(buffer);
        buffer = NULL;
    }
}

void Iso::getTempData2(){
    this->icon1 = NULL;
    this->snd0 = NULL;
    this->at3_size = 0;
    this->icon1_size = 0;

    void* buffer = NULL;
    unsigned size;
    
    // grab snd0.at3
    buffer = Iso::fastExtract("SND0.AT3", &size);
    if (buffer != NULL){
        this->snd0 = buffer;
        this->at3_size = size;
        buffer = NULL;
        size = 0;
    }
    
    // grab icon1.pmf
    buffer = Iso::fastExtract("ICON1.PMF", &size);
    if (buffer != NULL){
        this->icon1 = buffer;
        this->icon1_size = size;
        buffer = NULL;
        size = 0;
    }
}

void Iso::doExecute(){
    Iso::executeISO(this->path.c_str(), this->isPatched());
}

void Iso::executeISO(const char* path, bool is_patched){
    struct SceKernelLoadExecVSHParam param;
    
    memset(&param, 0, sizeof(param));

    if (is_patched)
        param.argp = (char*)"disc0:/PSP_GAME/SYSDIR/EBOOT.OLD";
    else
        param.argp = (char*)"disc0:/PSP_GAME/SYSDIR/EBOOT.BIN";

    int runlevel = (*(u32*)path == EF0_PATH)? ISO_RUNLEVEL_GO : ISO_RUNLEVEL;

    param.key = "umdemu";
    param.args = 33;  // lenght of "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN" + 1
    sctrlSESetBootConfFileIndex(ISO_DRIVER);
    sctrlSESetUmdFile((char*)path);
    sctrlKernelLoadExecVSHWithApitype(runlevel, path, &param);
}

char* Iso::getType(){
    return "ISO";
}

char* Iso::getSubtype(){
    return getType();
}

bool Iso::isPatched(){
    return (this->fastExtract("EBOOT.OLD") != NULL);
}

bool Iso::isISO(const char* filename){
    u32 magic = common::getMagic(filename, 0);
    return (
        magic == CSO_MAGIC ||
        magic == ZSO_MAGIC ||
        magic == DAX_MAGIC ||
        magic == JSO_MAGIC ||
        common::getMagic(filename, 0x8000) == ISO_MAGIC
    );
}

int Iso::read_raw_data(u8* addr, u32 size, u32 offset){
    FILE* fp = fopen(path.c_str(), "rb");
    if (fp == NULL)
        return 0;
    fseek(fp, offset, SEEK_SET);
    int res = fread(addr, 1, size, fp);
    fclose(fp);
    return res;
}

int Iso::read_compressed_data(u8 *addr, u32 size, u32 offset)
{

    if(offset > uncompressed_size) {
        // return if the offset goes beyond the iso size
        return 0;
    }
    else if(offset + size > uncompressed_size) {
        // adjust size if it tries to read beyond the game data
        size = uncompressed_size - offset;
    }

    u8* com_buf = g_ciso_block_buf;
    u8* dec_buf = g_ciso_dec_buf;
    u32 starting_block = offset / block_size;
    u32 ending_block = (offset+size)/block_size;
    u32 pos = offset & (block_size-1);
    u32 o_size = size;
    u32 cur_block = starting_block;
    if (ending_block%block_size) ending_block++;

    // refresh index table if needed
    if (g_cso_idx_start_block < 0 || starting_block < g_cso_idx_start_block || starting_block >= g_cso_idx_start_block + CISO_IDX_MAX_ENTRIES - 1){
        read_raw_data((u8*)g_cso_idx_cache, CISO_IDX_MAX_ENTRIES*sizeof(u32), starting_block * 4 + header_size);
        g_cso_idx_start_block = starting_block;
    }

    // read first block if not aligned to sector size
    if (pos) {
        
        int r = min(size, (block_size - pos));
        
        u32 b_offset = g_cso_idx_cache[cur_block-g_cso_idx_start_block];
        u32 b_size = g_cso_idx_cache[cur_block-g_cso_idx_start_block+1];
        u32 topbit = b_offset&0x80000000;
        b_offset = (b_offset&0x7FFFFFFF) << align;
        b_size = (b_size&0x7FFFFFFF) << align;
        b_size -= b_offset;

        if (cur_block == g_ciso_total_block-1 && header_size == sizeof(DAXHeader))
            b_size = DAX_COMP_BUF; // fix for last DAX block (you can't trust the value of b_size since there's no offset for last_block+1)

        // read block, skipping header if needed        
        b_size = read_raw_data(com_buf, b_size, b_offset + block_header);

        // decompress block
        ciso_decompressor(com_buf, b_size, dec_buf, block_size, topbit);
        
        memcpy(addr, dec_buf + pos, r);
        
        size -= r;
        cur_block++;
        addr += r;
    }

    // read intermediate blocks if more than one block is left
    u32 n_blocks = size / block_size;
    if (n_blocks) {
        
        u32 last_block;
        if (cur_block+n_blocks < g_cso_idx_start_block+CISO_IDX_MAX_ENTRIES)
            last_block = g_cso_idx_cache[cur_block+n_blocks-g_cso_idx_start_block];
        else
            read_raw_data((u8*)&last_block, sizeof(u32), (cur_block+n_blocks)*4 + header_size);
        
        u32 o_start = (g_cso_idx_cache[cur_block-g_cso_idx_start_block]&0x7FFFFFFF)<<align;
        u32 o_end = (last_block&0x7FFFFFFF)<<align;
        u32 compressed_size = o_end - o_start;
        u8* c_offset = addr + size - compressed_size;
        
        read_raw_data(c_offset, compressed_size, o_start);

        for (int i=0; i<n_blocks; i++){
            if (cur_block>=g_cso_idx_start_block+CISO_IDX_MAX_ENTRIES-1){
                read_raw_data((u8*)g_cso_idx_cache, CISO_IDX_MAX_ENTRIES*sizeof(u32), cur_block * 4 + header_size);
                g_cso_idx_start_block = cur_block;
            }
        
            u32 b_offset = g_cso_idx_cache[cur_block-g_cso_idx_start_block];
            u32 b_size = g_cso_idx_cache[cur_block-g_cso_idx_start_block+1];
            u32 topbit = b_offset&0x80000000;
            b_offset = (b_offset&0x7FFFFFFF) << align;
            b_size = (b_size&0x7FFFFFFF) << align;
            b_size -= b_offset;
            
            if (cur_block == g_ciso_total_block-1 && header_size == sizeof(DAXHeader))
                b_size = DAX_COMP_BUF; // fix for last DAX block (you can't trust the value of b_size since there's no offset for last_block+1)
            
            memcpy(com_buf, c_offset+block_header, b_size);
            c_offset += b_size;
            
            // decompress block
            ciso_decompressor(com_buf, b_size, dec_buf, block_size, topbit);
            
            memcpy(addr, dec_buf, block_size);
            addr += block_size;
            size -= block_size;
            cur_block++;
        }
    }

    // read remaining data
    if (size) {
        if (cur_block>=g_cso_idx_start_block+CISO_IDX_MAX_ENTRIES-1){
            read_raw_data((u8*)g_cso_idx_cache, CISO_IDX_MAX_ENTRIES*sizeof(u32), cur_block * 4 + header_size);
            g_cso_idx_start_block = cur_block;
        }
        
        u32 b_offset = g_cso_idx_cache[cur_block-g_cso_idx_start_block];
        u32 b_size = g_cso_idx_cache[cur_block-g_cso_idx_start_block+1];
        u32 topbit = b_offset&0x80000000;
        b_offset = (b_offset&0x7FFFFFFF) << align;
        b_size = (b_size&0x7FFFFFFF) << align;
        b_size -= b_offset;

        if (cur_block == g_ciso_total_block-1 && header_size == sizeof(DAXHeader))
            b_size = DAX_COMP_BUF; // fix for last DAX block (you can't trust the value of b_size since there's no offset for last_block+1)

        // read block, skipping header if needed        
        b_size = read_raw_data(com_buf, b_size, b_offset + block_header);

        // decompress block
        ciso_decompressor(com_buf, b_size, dec_buf, block_size, topbit);
    
        memcpy(addr, dec_buf, size);
        size = 0;
    }

    // return remaining size
    return o_size - size;
}

void* Iso::fastExtract(char* file, unsigned* size){
    
    void* buffer = NULL;
    if (size != NULL)
        *size = 0;
    
    g_cso_idx_start_block = -1;
    
    if (file_cache.find(file) != file_cache.end()){
        if (size == NULL) return (void*)-1;
        FileData file_data = file_cache[file];
        void* buffer = malloc(file_data.size);
        *size = file_data.size;
        (this->*read_iso_data)((u8*)buffer, file_data.size, file_data.offset);
        return buffer;
    }
    
    static u8 initial_block[SECTOR_SIZE*2];
    
    (this->*read_iso_data)(initial_block, 12, 32926);
    
    unsigned dir_lba = ((unsigned*)initial_block)[0];
    unsigned block_size = ((unsigned*)initial_block)[2];
    unsigned dir_start = dir_lba*block_size + block_size;
    
    (this->*read_iso_data)(initial_block, sizeof(initial_block), dir_start);
    
    for (int i=0; i<sizeof(initial_block); i++){
        if (strcasecmp((const char*)&initial_block[i], file) == 0){
            if (size == NULL){
                return (void*)-1;
            }
            u8* sfo = (u8*)&initial_block[i-31];
            FileData file_data;
            file_data.offset = (sfo[0] + (sfo[1]<<8) + (sfo[2]<<16) + (sfo[3]<<24))*block_size;
            file_data.size = (sfo[8] + (sfo[9]<<8) + (sfo[10]<<16) + (sfo[11]<<24));
            
            if (file_data.size == 0){
                return NULL;
            }
            
            void* buffer = malloc(file_data.size);
            *size = file_data.size;
            (this->*read_iso_data)((u8*)buffer, file_data.size, file_data.offset);
            file_cache[file] = file_data;
            return buffer;
        }
    }
    
    return NULL;
}
