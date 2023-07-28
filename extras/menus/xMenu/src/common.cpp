#include <sys/stat.h>
#include "common.h"
#include "entry.h"

using namespace common;

void common::setArgs(int c, char** v){
    argc = c;
    argv = v;
}

bool common::fileExists(const std::string &path){
    struct stat sb;
    return (stat(path.c_str(), &sb) == 0 && S_ISREG(sb.st_mode));
}

void common::loadData(){
    PBPHeader header;
    
    FILE* fp = fopen(argv[0], "rb");
    fread(&header, 1, sizeof(PBPHeader), fp);
    fclose(fp);
    
    background = loadImage(argv[0], header.pic1_offset);
    noicon = loadImage(argv[0], header.icon0_offset);
}

void common::deleteData(){
    freeImage(background);
    freeImage(noicon);
}

Image* common::getBG(){
    return background;
}

Image* common::getNoIcon(){
    return noicon;
}

void common::printText(float x, float y, const char *text, u32 color){
    printTextScreen(x, y, text, color);
}

void common::flip(){
    sceGuFinish();
    sceGuSync(0,0);
    guStart();
    sceGuTexMode(GU_PSM_8888, 0, 0, 0);
    sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
    sceGuTexFilter(GU_NEAREST, GU_NEAREST);
    sceGuFinish();
    sceGuSync(0,0); 

    sceDisplayWaitVblankStart(); 
    flipScreen();
    sceKernelDcacheWritebackInvalidateAll();
    sceKernelDelayThread(THREAD_DELAY);
};
