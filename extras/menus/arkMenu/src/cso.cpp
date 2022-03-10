#include "cso.h"

#include <cmath>
#include "systemctrl.h"
#include "kubridge.h"

void zlib_decompress(uint8_t *data, uint8_t* block_out, int type);

Cso :: Cso(string path)
{
    this->path = path;
    size_t lastSlash = path.rfind("/", string::npos);
    this->name = path.substr(lastSlash+1, string::npos);
    this->icon0 = common::getImage(IMAGE_WAITICON);
    u32 magic = common::getMagic(path.c_str(), 0);
    switch (magic){
    case CSO_MAGIC: this->ciso_type = TYPE_CSO; break;
    case ZSO_MAGIC: this->ciso_type = TYPE_ZSO; break;
    case DAX_MAGIC: this->ciso_type = TYPE_DAX; break;
    }
};


Cso::~Cso(){
    if (this->icon0 != common::getImage(IMAGE_NOICON) && this->icon0 != common::getImage(IMAGE_WAITICON))
        delete this->icon0;
    this->clear();
}

void Cso :: clear()
{
};


void Cso::loadIcon(){
    Image* icon = NULL;

    unsigned buf_size;
    void* buffer = this->fastExtract(this->path.c_str(), "ICON0.PNG", &buf_size);

    if (buffer != NULL){
        icon = new Image(buffer, YA2D_PLACE_RAM);
        free(buffer);
    }
    icon = (icon == NULL)? common::getImage(IMAGE_NOICON) : icon;
    icon->swizzle();
    this->icon0 = icon;
}


void Cso::getTempData1(){
    this->pic0 = NULL;
    this->pic1 = NULL;

    void* buffer = NULL;
    unsigned buf_size;

    // grab pic0.png
    buffer = this->fastExtract(this->path.c_str(), "PIC0.PNG", &buf_size);
    if (buffer != NULL){
        this->pic0 = new Image(buffer, YA2D_PLACE_RAM);
        free(buffer);
        buffer = NULL;
    }

    // grab pic1.png
    buffer = this->fastExtract(this->path.c_str(), "PIC1.PNG", &buf_size);
    if (buffer != NULL){
        this->pic1 = new Image(buffer, YA2D_PLACE_RAM);
        free(buffer);
        buffer = NULL;
    }
}

void Cso::getTempData2(){
    this->icon1 = NULL;
    this->snd0 = NULL;
    this->at3_size = 0;
    this->icon1_size = 0;

    void* buffer = NULL;
    unsigned size;
    
    // grab snd0.at3
    buffer = this->fastExtract(this->path.c_str(), "SND0.AT3", &size);
    if (buffer != NULL){
        this->snd0 = buffer;
        this->at3_size = size;
        buffer = NULL;
        size = 0;
    }
    
    // grab icon1.pmf
    buffer = this->fastExtract(this->path.c_str(), "ICON1.PMF", &size);
    if (buffer != NULL){
        this->icon1 = buffer;
        this->icon1_size = size;
        buffer = NULL;
        size = 0;
    }
}


char* Cso::getType(){
    return "ISO";
}


char* Cso::getSubtype(){
    static char* type_s[] = {
        "CSO", "ZSO", "DAX"
    };
    return type_s[this->ciso_type];
}


bool Cso::isPatched(){
    return (fastExtract(path.c_str(), "EBOOT.OLD")!=NULL);
}

void Cso::doExecute(){
    Iso::executeISO(this->path.c_str(), this->isPatched());
}


bool Cso::isCSO(const char* filename){
    u32 magic = common::getMagic(filename, 0);
    return (magic == CSO_MAGIC || magic == ZSO_MAGIC || magic == DAX_MAGIC);
}


void zlib_decompress(uint8_t *input, uint8_t* output, int type)
{
    switch (type){
    case TYPE_CSO:
        sctrlDeflateDecompress(output, input, SECTOR_SIZE);
        break;
    case TYPE_ZSO:
        LZ4_decompress_fast((const char*)input, (char*)output, SECTOR_SIZE);
        break;
    case TYPE_DAX:
        memcpy(input, input+2, DAX_COMP_BUF-6);
        sctrlDeflateDecompress(output, input, DAX_BLOCK_SIZE);
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
    else{
        fseek(fp, 16, SEEK_SET);
        fread(&block_size, 4, 1, fp);
    }
    int start_read = (32768 / block_size) * 4 + 24;
    if (ciso_type == TYPE_DAX)
        start_read += 8;
    
    unsigned fo, fs;
    fseek(fp, start_read, SEEK_SET);
    fread(&fo, 4, 1, fp);
    
    if (ciso_type == TYPE_DAX){
        fs = DAX_BLOCK_SIZE;
    }
    else{
        fread(&fs, 4, 1, fp);
        fs = min((int)fs, SECTOR_SIZE);
    }
    
    fseek(fp, fo, SEEK_SET);
    fread(compressed, 1, fs, fp);
    zlib_decompress(compressed, block_out, ciso_type);
    
    unsigned idx = (block_out[158] + (block_out[159]<<8) + (block_out[160]<<16) + (block_out[161]<<24));
    
    unsigned start = 0;
    if (ciso_type == TYPE_DAX){
        start = idx + 30;
    }
    else{
        start = idx*4 + 28;
    }
    
    fseek(fp, start, SEEK_SET);

    unsigned offset;
    fread(&offset, 4, 1, fp);

    unsigned size;
    if (ciso_type == TYPE_DAX){
        size = DAX_BLOCK_SIZE;
    }
    else{
        fread(&size, 4, 1, fp);
        size -= offset;
        size = min((int)size, SECTOR_SIZE);
    }
    
    fseek(fp, offset, SEEK_SET);

    fread(compressed, 1, size, fp);
    zlib_decompress(compressed, block_out, ciso_type);

    return block_size;
}

void* Cso::fastExtract(const char* path, char* file, unsigned* size_out){

    FILE* fp = fopen(path, "rb");
    if (fp == NULL)
        return NULL;
        
    static u8 block_out[DAX_BLOCK_SIZE] __attribute__((aligned(64)));
    static u8 compressed[DAX_COMP_BUF] __attribute__((aligned(64)));
    int block_size = getInitialBlock(fp, block_out, compressed, ciso_type);

    if (size_out != NULL)
        *size_out = 0;

    void* buffer = NULL;

    common::upperString(file);

    int pos = 0;

    while (true){

        if (pos > block_size){
            fclose(fp);
            return NULL;
        }

        char tmpText[128];
        memset(tmpText, 0, 128);
        strncpy(tmpText, (char*)&block_out[pos], strlen(file));
        common::upperString(tmpText);

        if (!strcmp(tmpText, file)){
            if (size_out == NULL){
                fclose(fp);
                free(block_out);
                free(compressed);
                return (void*)-1;
            }
            pos -= 31;
            unsigned b_offset, b_size, b_iter;

            b_offset = ((block_out[pos]) + (block_out[pos+1]<<8) + (block_out[pos+2]<<16) + (block_out[pos+3]<<24));
            b_size = ((block_out[pos+8]) + (block_out[pos+9]<<8) + (block_out[pos+10]<<16) + (block_out[pos+11]<<24));
            if (ciso_type == TYPE_DAX){
                b_offset += 32;
                b_iter = (int)ceil(b_size/(float)DAX_BLOCK_SIZE) + 1;
            }
            else{
                b_offset = b_offset*4 + 24;
                b_iter = (int)ceil(b_size/2048.f) + 1;
            }
            
            int trail_size = block_size - (((b_iter -1) * block_size) - b_size);

            fseek(fp, b_offset, SEEK_SET);

            unsigned buf_size;
            if (ciso_type == TYPE_DAX) buf_size = block_size*b_iter + trail_size;
            else buf_size = SECTOR_SIZE*b_iter + trail_size;
            
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

                if (is_compressed){
                    if (x < b_iter - 1){
                        fread(compressed, 1, size, fp);
                        zlib_decompress(compressed, (uint8_t*)buf, ciso_type);
                        buf += block_size;
                    }
                    else{
                        fread(compressed, 1, size, fp);
                        zlib_decompress(compressed, (uint8_t*)buf, ciso_type);
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

