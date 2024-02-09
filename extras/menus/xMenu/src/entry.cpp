#include "entry.h"
#include "common.h"

Entry::Entry(string path){
    
    size_t lastSlash = path.rfind("/", string::npos);
    size_t substrPos = path.rfind("/", lastSlash-1)+1;

    this->path = path;
    this->ebootName = path.substr(lastSlash+1, string::npos);
    this->name = path.substr(substrPos, lastSlash-substrPos);
    readHeader();
    this->icon0 = loadIcon();
}

void Entry::readHeader(){
    FILE* fp = fopen(path.c_str(), "rb");
    fread(&header, 1, sizeof(PBPHeader), fp);
    fclose(fp);
}

Image* Entry::loadIcon(){
    int size = (this->header.icon1_offset-this->header.icon0_offset);
    if (size){
        Image* icon = loadImage(this->path.c_str(), this->header.icon0_offset);
        if (icon != NULL)
            return icon;
    }
    return NULL;
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

Image* Entry::getPic0(){
    int size = this->header.pic1_offset-this->header.pic0_offset;
    if (size==0) return NULL;
    return loadImage(this->path.c_str(), this->header.pic0_offset);
}

Image* Entry::getPic1(){
    int size = this->header.snd0_offset-this->header.pic1_offset;
    if (size == 0) return NULL;
    return loadImage(this->path.c_str(), this->header.pic1_offset);
}

bool Entry::run(){
    this->pic0 = getPic0();
    this->pic1 = getPic1();
    
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
        if (control.cross()){
            ret = true;
            break;
        }
        else if (control.circle()){
            ret = false;
            break;
        }
    }
    
    animDisappear();

    if (this->pic0 != NULL)
        delete this->pic0;
    if (this->pic1 != NULL)
        delete this->pic1;
    this->pic0 = NULL;
    this->pic1 = NULL;

    return ret;
}
        
Entry::~Entry(){
    delete this->icon0;
}
