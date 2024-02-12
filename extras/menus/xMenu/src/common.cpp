#include <sys/stat.h>
#include "common.h"
#include "entry.h"

using namespace common;

static t_conf config;

struct tm today;

void common::resetConf(){
    memset(&config, 0, sizeof(config));
    config.fast_gameboot = 0;
    config.language = 0;
    config.font = 1;
    config.plugins = 1;
    config.scan_save = 0;
    config.scan_cat = 0;
    config.scan_dlc = 0;
    config.swap_buttons = 0;
    config.animation = 0;
    config.main_menu = 0;
    config.sort_entries = 1;
    config.show_recovery = 1;
    config.show_fps = 0;
    config.text_glow = 3;
    config.screensaver = 2;
    config.redirect_ms0 = 0;
    config.startbtn = 0;
    config.menusize = 0;
    config.show_path = 0;
    config.browser_icon0 = 1;
}


void loadConfig(){
    FILE* fp = fopen(CONFIG_PATH, "rb");
    if (fp == NULL){
        resetConf();
        return;
    }   
    memset(&config, 0, sizeof(t_conf));
    fseek(fp, 0, SEEK_SET);
    fread(&config, 1, sizeof(t_conf), fp);
    fclose(fp);
    if (today.tm_mday == 1 && today.tm_mon == 3)
        config.language = 10; 
}


void common::loadConf() {
	loadConfig();
}



void common::setArgs(int c, char** v){
    argc = c;
    argv = v;
}

bool common::fileExists(const std::string &path){
    struct stat sb;
    return (stat(path.c_str(), &sb) == 0 && S_ISREG(sb.st_mode));
}

void common::saveConf() {
	FILE* fp = fopen(CONFIG_PATH, "wb");
	fwrite(&config, 1, sizeof(t_conf), fp);
	fclose(fp);
}

void common::loadData(){
	loadConfig();
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

t_conf* common::getConf() {
	return &config;
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
