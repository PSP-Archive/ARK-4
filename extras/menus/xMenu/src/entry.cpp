#include "entry.h"
#include "common.h"

Entry::Entry(string path){
    
    this->path = path;
    this->sfo_buffer = NULL;
    this->icon0 = NULL;

    size_t lastSlash = path.rfind("/", string::npos);
    size_t substrPos = path.rfind("/", lastSlash-1)+1;

    this->ebootName = path.substr(lastSlash+1, string::npos);
    this->name = path.substr(substrPos, lastSlash-substrPos);
}

Entry::~Entry(){
    if (this->icon0) freeImage(this->icon0);
}

Entry* Entry::createIfPops(string path){
    Entry* ent = new Entry(path);
    ent->readHeader();
    
    if (ent->isPops()){
        ent->findNameInParam();
        free(ent->sfo_buffer);
        ent->sfo_buffer = NULL;
        return ent;
    }

    delete ent;
    return NULL;
}

void Entry::readHeader(){

    FILE* fp = fopen(path.c_str(), "rb");    
    fread(&header, 1, sizeof(PBPHeader), fp);

    u32 size = header.icon0_offset - header.param_offset;
    if (size){
        this->sfo_buffer = (unsigned char*)malloc(size);
        fseek(fp, this->header.param_offset, SEEK_SET);
        fread(sfo_buffer, 1, size, fp);
    }

    fclose(fp);
}

void Entry::loadIcon(){
    if (this->icon0 == NULL){
        int size = header.icon1_offset - header.icon0_offset;
        if (size){
            this->icon0 = loadImage(this->path.c_str(), this->header.icon0_offset);
        }
    }
}

void Entry::unloadIcon(){
    if (this->icon0){
        freeImage(this->icon0);
        this->icon0 = NULL;
    }
}

bool Entry::isPops(){
    if (this->sfo_buffer){
        u16 category = 0; int cat_size = sizeof(category);
        int size = (this->header.icon1_offset-this->header.icon0_offset);
        bool res = Entry::getSfoParam(sfo_buffer, size, "CATEGORY", (unsigned char*)&category, &cat_size);
        if (res){
            return (category == PS1_CAT);
        }
    }
    return false;
}

void Entry::findNameInParam(){
    if (this->sfo_buffer){
        char title[128];
        int title_size = sizeof(title);

        int size = header.icon1_offset - header.icon0_offset;
        bool res = Entry::getSfoParam(sfo_buffer, size, "TITLE", (unsigned char*)(title), &title_size);

        if (res){
            // remove any non-ASCII character
            for (int i=0; i<title_size && title[i]; i++){
                if (title[i] < 0 || title[i] > 128){
                    for (int j=i+1; j<title_size; j++){
                        title[j-1] = title[j];
                    }
                    title_size--;
                    i--;
                }
            }
            this->name = string(title);
        }
    }
}

void Entry::animAppear(){
    for (int i=480; i>=0; i-=10){
        clearScreen(CLEAR_COLOR);
        blitAlphaImageToScreen(0, 0, 480, 272, common::getBG(), 0, 0);
        if (this->pic1) blitAlphaImageToScreen(0, 0, 480-i, 272, this->pic1, i, 0);
        blitAlphaImageToScreen(0, 0, this->icon0->imageWidth, this->icon0->imageHeight, this->icon0, 20+i, 92);
        flipScreen();
    }
}

void Entry::animDisappear(){
    for (int i=0; i<=480; i+=10){
        clearScreen(CLEAR_COLOR);
        blitAlphaImageToScreen(0, 0, 480, 272, common::getBG(), 0, 0);
        if (this->pic1) blitAlphaImageToScreen(0, 0, 480-i, 272, this->pic1, i, 0);
        blitAlphaImageToScreen(0, 0, this->icon0->imageWidth, this->icon0->imageHeight, this->icon0, 20+i, 92);
        flipScreen();
    }
}

string Entry::getName(){
    return this->name;
}

string Entry::getPath(){
    return this->path;
}
        
string Entry::getEbootName(){
    return this->ebootName;
}
        
Image* Entry::getIcon(){
    return (icon0)? icon0 : common::getNoIcon();
}

Image* Entry::loadPic0(){
    int size = this->header.pic1_offset-this->header.pic0_offset;
    if (size==0) return NULL;
    return loadImage(this->path.c_str(), this->header.pic0_offset);
}

Image* Entry::loadPic1(){
    int size = this->header.snd0_offset-this->header.pic1_offset;
    if (size == 0) return NULL;
    return loadImage(this->path.c_str(), this->header.pic1_offset);
}

bool Entry::run(){

    if (common::getConf()->fast_gameboot) return true;

    this->pic0 = loadPic0();
    this->pic1 = loadPic1();
    
    animAppear();

    clearScreen(CLEAR_COLOR);
    blitAlphaImageToScreen(0, 0, 480, 272, common::getBG(), 0, 0);
    if (this->pic1) blitAlphaImageToScreen(0, 0, 480, 272, this->pic1, 0, 0);
    blitAlphaImageToScreen(0, 0, this->icon0->imageWidth, this->icon0->imageHeight, this->icon0, 20, 92);
    if (this->pic0 != NULL)
        blitAlphaImageToScreen(0, 0, this->pic0->imageWidth, this->pic0->imageHeight, this->pic0, 160, 25);
    common::flip();
    
    Controller control;
    bool ret;
    while (true){
        control.update();
        if (control.accept()){
            ret = true;
            break;
        }
        else if (control.decline()){
            ret = false;
            break;
        }
    }
    
    animDisappear();

    if (this->pic0 != NULL)
        freeImage(this->pic0);
    if (this->pic1 != NULL)
        freeImage(this->pic1);
    this->pic0 = NULL;
    this->pic1 = NULL;

    return ret;
}

bool Entry::getSfoParam(unsigned char* sfo_buffer, int buf_size, char* param_name, unsigned char* var, int* var_size){
    SFOHeader *header = (SFOHeader *)sfo_buffer;
    SFODir *entries = (SFODir *)(sfo_buffer + sizeof(SFOHeader));
    bool res = false;
    int i;
    for (i = 0; i < header->nitems; i++) {
        if (strcmp((char*)sfo_buffer + header->fields_table_offs + entries[i].field_offs, param_name) == 0) {
            memcpy(var, sfo_buffer + header->values_table_offs + entries[i].val_offs, *var_size);
            res = true;
            break;
        }
    }
    return res;
}

bool Entry::cmpEntriesForSort (Entry* i, Entry* j) {
    return (strcasecmp(i->getName().c_str(), j->getName().c_str())<0);
}