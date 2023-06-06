#include "iso.h"
#include <umd.h>

using namespace std;

#define CISO_IDX_MAX_ENTRIES 512

static int g_ciso_total_block;
static int g_cso_idx_start_block = -1;
static u8 g_ciso_block_buf[DAX_COMP_BUF] __attribute__((aligned(64)));
static u8 g_ciso_dec_buf[DAX_COMP_BUF] __attribute__((aligned(64)));
static u32 g_cso_idx_cache[CISO_IDX_MAX_ENTRIES];

extern "C"{
    int sctrlKernelExitVSH(void*);
    int sctrlDeflateDecompress(void*, void*, int);
    int lzo1x_decompress(void*, unsigned int, void*, unsigned int*, void*);
    int LZ4_decompress_fast(const char*, char*, int);
}

static void decompress_dax(void* src, int src_len, void* dst, int dst_len, u32 topbit){
    // use raw inflate with no NCarea check (DAX V0)
    sctrlDeflateDecompress(dst, src, DAX_COMP_BUF); // use raw inflate
}

static void decompress_dax1(void* src, int src_len, void* dst, int dst_len, u32 topbit){
    // for DAX Version 1 we can skip parsing NC-Areas and just use the block_size trick as in JSO and CSOv2
    if (src_len == dst_len) memcpy(dst, src, dst_len); // check for NC area
    else sctrlDeflateDecompress(dst, src, dst_len); // use raw inflate
}

static void decompress_jiso(void* src, int src_len, void* dst, int dst_len, u32 topbit){
    // while JISO allows for DAX-like NCarea, it by default uses compressed size check
    if (src_len == dst_len) memcpy(dst, src, dst_len); // check for NC area
    else lzo1x_decompress((void*)src, (unsigned int)src_len, (void*)dst, (unsigned int*)&dst_len, (void*)0); // use lzo
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
            ciso_decompressor = (dax_header->version >= 1)? &decompress_dax1 : &decompress_dax;
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
    static char pboot_path[256];
    if (has_update_file(pboot_path)){
        Iso::executeISOupdated(this->path.c_str(), pboot_path);
    }
    else{
        Iso::executeISO(this->path.c_str(), this->isPatched());
    }
}

void Iso::executeISOupdated(const char* path, const char* pboot_path){
    struct SceKernelLoadExecVSHParam param;
    memset(&param, 0, sizeof(param));

    sctrlSESetBootConfFileIndex(ISO_DRIVER);
    sctrlSESetUmdFile((char*)path);

    param.argp = (void*)pboot_path;
    param.args = strlen(pboot_path) + 1;
    param.key = "umdemu";

    int runlevel = (*(u32*)pboot_path == EF0_PATH && common::getConf()->redirect_ms0)? ISO_PBOOT_RUNLEVEL_GO : ISO_PBOOT_RUNLEVEL;

    sctrlKernelLoadExecVSHWithApitype(runlevel, pboot_path, &param);
}

void Iso::executeISO(const char* path, bool is_patched){
    struct SceKernelLoadExecVSHParam param;
    
    memset(&param, 0, sizeof(param));

    if (is_patched)
        param.argp = (char*)UMD_EBOOT_OLD;
    else
        param.argp = (char*)UMD_EBOOT_BIN;

    int runlevel = (*(u32*)path == EF0_PATH && common::getConf()->redirect_ms0)? ISO_RUNLEVEL_GO : ISO_RUNLEVEL;

    param.size = sizeof(param);
    param.key = "umdemu";
    param.args = 33;  // lenght of "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN" + 1
    sctrlSESetDiscType(PSP_UMD_TYPE_GAME);
    sctrlSESetBootConfFileIndex(ISO_DRIVER);
    sctrlSESetUmdFile((char*)path);
    sctrlKernelLoadExecVSHWithApitype(runlevel, path, &param);
}

int Iso::checkAudioVideo(){
    int type = 0;
    static u8 initial_block[SECTOR_SIZE*2];
    (this->*read_iso_data)(initial_block, SECTOR_SIZE*2, 32926);
    for (int i=0; i<SECTOR_SIZE*2; i++){
        if (strcmp((char*)&initial_block[i], "UMD_VIDEO") == 0){
            type |= PSP_UMD_TYPE_VIDEO;
        }
        else if (strcmp((char*)&initial_block[i], "UMD_AUDIO") == 0){
            type |= PSP_UMD_TYPE_AUDIO;
        }
    }
    return type;
}

void Iso::executeVideoISO(const char* path)
{
    
    Iso iso = Iso(path);

    int type = iso.checkAudioVideo();

    if (type == 0) return;

    if(iso.fastExtract("EBOOT.BIN", NULL)) {
        type |= PSP_UMD_TYPE_GAME;
    }

	sctrlSESetUmdFile((char*)path);
	sctrlSESetBootConfFileIndex(MODE_VSHUMD);
	sctrlSESetDiscType(type);
	sctrlKernelExitVSH(NULL);
}

int Iso::has_update_file(char* update_file){
    // game ID is always at offset 0x8373 within the ISO
    int lba = 16;
    int pos = 883;

    char game_id[10];

    (this->*read_iso_data)((u8*)game_id, 10, 0x8373);

    // remove the dash in the middle: ULUS-01234 -> ULUS01234
    game_id[4] = game_id[5];
    game_id[5] = game_id[6];
    game_id[6] = game_id[7];
    game_id[7] = game_id[8];
    game_id[8] = game_id[9];
    game_id[9] = 0;

    // try to find the update file
    char path[256];
    char* devs[] = {"ms0:", "ef0:"};

    for (int i=0; i<2; i++){
        sprintf(path, "%s/PSP/GAME/%s/PBOOT.PBP", devs[i], game_id);
        int fd = sceIoOpen(path, PSP_O_RDONLY, 0777);
        if (fd >= 0){
            // found
            sceIoClose(fd);
            if (update_file) strcpy(update_file, path);
            return 1;
        }
    }
    // not found
    return 0;
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

SfoInfo Iso::getSfoInfo(){
    SfoInfo info = this->Entry::getSfoInfo();
    unsigned int size = 0;
    unsigned char* sfo_buffer = (unsigned char*)fastExtract("PARAM.SFO", &size);
    if (sfo_buffer){
        int title_size = sizeof(info.title);
        Entry::getSfoParam(sfo_buffer, size, "TITLE", (unsigned char*)(info.title), &title_size);
        
        int id_size = sizeof(info.gameid);
        Entry::getSfoParam(sfo_buffer, size, "DISC_ID", (unsigned char*)(info.gameid), &id_size);

        free(sfo_buffer);
    }
    return info;
}

bool Iso::isISO(const char* filename){
    string ext = common::getExtension(filename);
    return (
        ext == "iso" ||
        ext == "cso" ||
        ext == "zso" ||
        ext == "jso" ||
        ext == "dax"
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
    u32 cur_block;
    u32 pos, ret, read_bytes;
    u32 o_offset = offset;
    u32 g_ciso_total_block = uncompressed_size/block_size;
    u8* com_buf = g_ciso_block_buf;
    u8* dec_buf = g_ciso_dec_buf;
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
    if (g_cso_idx_start_block < 0 || starting_block < g_cso_idx_start_block || starting_block-g_cso_idx_start_block+1 >= CISO_IDX_MAX_ENTRIES-1){
        read_raw_data((u8*)g_cso_idx_cache, CISO_IDX_MAX_ENTRIES*sizeof(u32), starting_block * sizeof(u32) + header_size);
        g_cso_idx_start_block = starting_block;
    }

    // Calculate total size of compressed data
    u32 o_start = (g_cso_idx_cache[starting_block-g_cso_idx_start_block]&0x7FFFFFFF)<<align;
    // last block index might be outside the block offset cache, better read it from disk
    u32 o_end; read_raw_data((u8*)&o_end, sizeof(u32), ending_block*sizeof(u32)+header_size);
    o_end = (o_end&0x7FFFFFFF)<<align;
    u32 compressed_size = o_end-o_start;

    // try to read at once as much compressed data as possible
    if (size > block_size*2){ // only if going to read more than two blocks
        if (size < compressed_size) compressed_size = size-block_size; // adjust chunk size if compressed data is bigger than uncompressed
        c_buf = top_addr - compressed_size; // read into the end of the user buffer
        read_raw_data(c_buf, compressed_size, o_start);
    }

    while(size > 0) {
        // calculate block number and offset within block
        cur_block = offset / block_size;
        pos = offset & (block_size - 1);

        // check if we need to refresh index table
        if (cur_block-g_cso_idx_start_block >= CISO_IDX_MAX_ENTRIES-1){
            read_raw_data((u8*)g_cso_idx_cache, CISO_IDX_MAX_ENTRIES*sizeof(u32), cur_block * 4 + header_size);
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
                if (size < compressed_size) compressed_size = size-block_size; // adjust if bigger than uncompressed
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
        read_bytes = min(size, (block_size - pos));
        memcpy(addr, dec_buf + pos, read_bytes);
        size -= read_bytes;
        addr += read_bytes;
        offset += read_bytes;
    }

    u32 res = offset - o_offset;
    
    return res;
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
