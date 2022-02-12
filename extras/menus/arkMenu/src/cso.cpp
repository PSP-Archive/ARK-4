#include <zlib.h>

#include "cso.h"

#include <cmath>
#include "systemctrl.h"

void zlib_decompress(uint8_t *data, uint8_t* block_out, int in_size, int type);

extern "C"{
    int LZ4_decompress_fast(uint8_t* source, uint8_t* dest, int outputSize);
};

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
    if (icon == NULL)
        sceKernelDelayThread(50000);
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


void zlib_decompress(uint8_t *input, uint8_t* output, int in_size, int type)

{
    switch (type){
    case TYPE_CSO:
        sctrlDeflateDecompress(output, input, SECTOR_SIZE);
        break;
    case TYPE_ZSO:
        LZ4_decompress_fast(input, output, SECTOR_SIZE);
        break;
    case TYPE_DAX:
        { // use zlib decompress
            z_stream z;
            memset(&z, 0, sizeof(z));
            inflateInit2(&z, 15);
            z.next_in = input;
	        z.avail_in = in_size;
	        z.next_out = output;
	        z.avail_out = SECTOR_SIZE;
	        inflate(&z, Z_FINISH);
	        inflateEnd(&z);
        }
        break;
    }
}


int getInitialBlock(FILE* fp, u8* block_out, int ciso_type){

    if (fp == NULL)
        return 0;

    uint8_t compressed[SECTOR_SIZE];

    int block_size = 0;

    if (ciso_type == TYPE_DAX){
        block_size = DAX_BLOCK_SIZE;
    }
    else{
        fseek(fp, 16, SEEK_SET);
        fread(&block_size, 4, 1, fp);
    }
    int start_read = (32768 / block_size) * 4 + 24;
    fseek(fp, start_read, SEEK_SET);

    unsigned fo, fs;
    fread(&fo, 4, 1, fp);
    fread(&fs, 4, 1, fp);
    fseek(fp, fo, SEEK_SET);

    fs = min((int)fs, 200);

    fread(compressed, 1, fs, fp);

    zlib_decompress(compressed, block_out, fs, ciso_type);
    unsigned start = (block_out[158] + block_out[159] + block_out[160] + block_out[161]) * 4;
    fseek(fp, start+28, SEEK_SET);

    unsigned offset;
    fread(&offset, 4, 1, fp);

    unsigned size;
    fread(&size, 4, 1, fp);
    size -= offset;

    fseek(fp, offset, SEEK_SET);

    fread(compressed, 1, size, fp);
    zlib_decompress(compressed, block_out, size, ciso_type);

    return block_size;
}


void* Cso::fastExtract(const char* path, char* file, unsigned* size_out){

    FILE* fp = fopen(path, "rb");
    if (fp == NULL)
        return NULL;

    u8* block_out = (u8*)memalign(64, SECTOR_SIZE);
    int block_size = getInitialBlock(fp, block_out, ciso_type);

    if (size_out != NULL)
        *size_out = 0;

    void* buffer = NULL;
    uint8_t compressed[SECTOR_SIZE];

    common::upperString(file);

    int pos = 0;

    while (true){

        if (pos > block_size){
            fclose(fp);
            free(block_out);
            return NULL;
        }

        char tmpText[128];
        memset(tmpText, 0, 128);
        strncpy(tmpText, (char*)&block_out[pos], strlen(file));
        common::upperString(tmpText);

        if (!strcmp(tmpText, file)){
            if (size_out == NULL) return (void*)-1;
            pos -= 31;
            unsigned b_offset, b_size;

            b_offset = ((block_out[pos]) + (block_out[pos+1]<<8) + (block_out[pos+2]<<16) + (block_out[pos+3]<<24))*4 + 24;
            b_size = ((block_out[pos+8]) + (block_out[pos+9]<<8) + (block_out[pos+10]<<16) + (block_out[pos+11]<<24));

            int b_iter = (int)ceil(b_size/2048.f) + 1;
            int trail_size = block_size - (((b_iter -1) * block_size) - b_size);

            fseek(fp, b_offset, SEEK_SET);

            int buf_size = SECTOR_SIZE*b_iter + trail_size;
            buffer = memalign(64, buf_size);
            int compressed_size = 0;
            u32 buf = (u32)buffer;
            for (int x = 1; x<b_iter; x++){

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
                        zlib_decompress(compressed, (uint8_t*)buf, size, ciso_type);
                        buf += SECTOR_SIZE;
                    }
                    else{
                        fread(compressed, 1, size, fp);
                        zlib_decompress(compressed, (uint8_t*)buf, size, ciso_type);
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
            free(block_out);
            return buffer;
        }
        pos++;
    }
    fclose(fp);
    free(block_out);
    return NULL;

}

