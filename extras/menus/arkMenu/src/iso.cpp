#include "iso.h"
#include "cso.h"

using namespace std;

Iso :: Iso()
{
};

Iso :: Iso(string path)
{
    this->path = path;
    size_t lastSlash = path.rfind("/", string::npos);
    this->name = path.substr(lastSlash+1, string::npos);
    this->icon0 = common::getImage(IMAGE_WAITICON);
};

Iso :: ~Iso()
{
    if (this->icon0 != common::getImage(IMAGE_NOICON) && this->icon0 != common::getImage(IMAGE_WAITICON))
        delete this->icon0;
};

void Iso::loadIcon(){
    Image* icon = NULL;
    unsigned size;
    void* buffer = Iso::fastExtract(this->path.c_str(), "ICON0.PNG", &size);
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
    buffer = Iso::fastExtract(this->path.c_str(), "PIC0.PNG", &size);
    if (buffer != NULL){
        this->pic0 = new Image(buffer, YA2D_PLACE_RAM);
        free(buffer);
        buffer = NULL;
    }

    // grab pic1.png
    buffer = Iso::fastExtract(this->path.c_str(), "PIC1.PNG", &size);
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
    buffer = Iso::fastExtract(this->path.c_str(), "SND0.AT3", &size);
    if (buffer != NULL){
        this->snd0 = buffer;
        this->at3_size = size;
        buffer = NULL;
        size = 0;
    }
    
    // grab icon1.pmf
    buffer = Iso::fastExtract(this->path.c_str(), "ICON1.PMF", &size);
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
    return (this->fastExtract(path.c_str(), "PSP_GAME/SYSDIR/EBOOT.OLD") != NULL);
}

bool Iso::isISO(const char* filename){
    return (common::getMagic(filename, 0x8000) == ISO_MAGIC);
}

void* Iso::fastExtract(const char* path, char* file, unsigned* size){
    FILE* fp = fopen(path, "rb");
    if (fp == NULL)
        return NULL;
    
    void* buffer = NULL;
    if (size != NULL)
        *size = 0;
    
    common::upperString(file);
    
    fseek(fp, 32926, SEEK_SET);
    unsigned dir_lba;
    fread(&dir_lba, 4, 1, fp);
    fseek(fp, 4, SEEK_CUR);
    
    unsigned block_size;
    fread(&block_size, 4, 1, fp);
    
    unsigned dir_start = dir_lba*block_size + block_size;
    
    fseek(fp, dir_start, SEEK_SET);

    unsigned search_end = ftell(fp) + 2048;
    
    while (true){
        
        unsigned cur_pos = ftell(fp);
        
        unsigned char entry_size;
        fread(&entry_size, 1, 1, fp);
        
        if (entry_size == 0 || ftell(fp) >= search_end){
            fclose(fp);
            return NULL;
        }

        fseek(fp, cur_pos+32, SEEK_SET);
        
        unsigned char name_size;
        fread(&name_size, 1, 1, fp);
        
        char entry_name[256];
        memset(entry_name, 0, sizeof(entry_name));
        fread(entry_name, 1, name_size, fp);
        common::upperString(entry_name);
        
        if (!strcmp(entry_name, file)){
        
            if (size == NULL){
                fclose(fp);
                return (void*)-1;
            }
        
            fseek(fp, cur_pos + 2, SEEK_SET);
            unsigned icon0_offset;
            fread(&icon0_offset, 4, 1, fp);
            icon0_offset *= block_size;
            
            fseek(fp, ftell(fp) + 4, SEEK_SET);
            unsigned icon0_size;
            fread(&icon0_size, 4, 1, fp);
            
            if (icon0_size == 0){
                fclose(fp);
                return NULL;
            }
            
            fseek(fp, icon0_offset, SEEK_SET);
            
            buffer = malloc(icon0_size);
            memset(buffer, 0, icon0_size);
            if (size != NULL)
                *size = icon0_size;
            fread(buffer, icon0_size, 1, fp);
            
            fclose(fp);
            
            return buffer;
        }
        
        fseek(fp, cur_pos + entry_size, SEEK_SET);
    }
}
