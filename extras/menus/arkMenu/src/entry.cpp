#include <cstring>
#include <pspdisplay.h>
#include "entry.h"
#include "eboot.h"
#include "iso.h"
#include "sprites.h"
#include "system_mgr.h"
#include "music_player.h"
#include "pspav.h"
#include "pspav_wrapper.h"

extern "C" int sceDisplaySetHoldMode(int);

int gameBootThread(SceSize _args, void *_argp){
    Sprites s;

    int w = 480/10;
    int h = 272/10;

    while (w < 480 || h < 272){
    
        common::clearScreen(CLEAR_COLOR);
        
        common::getImage(IMAGE_BG)->draw(0, 0);
    
        if (w+80<480) w+=80;
        else w=480;

        if (h+40<272) h+=40;
        else h=272;

        int x = (480-w)/2;
        int y = (272-h)/2;

        common::getImage(IMAGE_LOADING)->draw_scale(x, y, w, h);
        s.drawFadeout();
        common::flipScreen();
    }
    while (true){
        common::clearScreen(CLEAR_COLOR);
        common::getImage(IMAGE_LOADING)->draw(0, 0);
        bool ret = s.drawFadeout();
        common::flipScreen();
        if (!ret)
            break;
    }
    
    sceKernelExitDeleteThread(0);
    
    return 0;
}

Entry::Entry(){

}

Entry::Entry(string path){
}

void Entry::setName(string name){
    this->name = name;
}

string Entry::getName(){
    return this->name;
}

string Entry::getPath(){
    return this->path;
}

void Entry::setPath(string path){
    this->path = path;
}

Image* Entry::getIcon(){
    return this->icon0;
}

void* Entry::getIcon1(){
    return this->icon1;
}

int Entry::getIcon1Size(){
    return this->icon1_size;
}

Image* Entry::getPic0(){
    return this->pic0;
}

Image* Entry::getPic1(){
    return this->pic1;
}

void* Entry::getSnd(){
    return this->snd0;
}

int Entry::getSndSize(){
    return this->at3_size;
}

void Entry::freeIcon(){
    register Image* aux = this->icon0;
    this->icon0 = common::getImage(IMAGE_WAITICON);
    if (aux && !common::isSharedImage(aux))
        delete aux;
}

void Entry::execute(bool isAutoboot){
    if (!isAutoboot) {
        char* last_game = common::getConf()->last_game;
        if (strcmp(last_game, this->path.c_str()) != 0 && name != "UMD Drive" && name != "Recovery Menu"){
            strcpy(last_game, this->path.c_str());
        }
        common::saveConf();
        this->gameBoot();
    }
    this->doExecute();
}

void Entry::gameBoot(){

    //if (common::getConf()->fast_gameboot && name != "Recovery Menu")
    if (common::getConf()->fast_gameboot) {
        MusicPlayer::fullStop();
        SystemMgr::pauseDraw();
        sceDisplaySetHoldMode(1);
        return;
    }

    MusicPlayer::fullStop();

    SystemMgr::pauseDraw();

    unsigned mp3_size;
    void* mp3_buffer = common::readFromPKG("BOOT.MP3", &mp3_size);
    
    SceUID boot_thread = sceKernelCreateThread("boot_thread", gameBootThread, 0x10, 0x8000, PSP_THREAD_ATTR_USER, NULL);
    
    sceKernelStartThread(boot_thread, 0, NULL);
    
    playMP3File(NULL, mp3_buffer, mp3_size);
    
    free(mp3_buffer);
    
    sceKernelWaitThreadEnd(boot_thread, NULL);

    sceDisplaySetHoldMode(1);
    
}

void Entry::drawBG(){
    if (this->pic1 != NULL){
        if (this->pic1->getWidth() == 480 && this->pic1->getHeight() == 272)
            this->pic1->draw(0, 0);
        else
            this->pic1->draw_scale(0, 0, 480, 272);
    }
    else common::drawScreen();
    if (this->pic0 != NULL) this->pic0->draw(160, 85);
}

void Entry::freeTempData(){
    if (this->pic0 != NULL)
        delete this->pic0;
    if (this->pic1 != NULL)
        delete this->pic1;
    if (this->icon1 != NULL)
        free(this->icon1);
    if (this->snd0 != NULL)
        free(this->snd0);
}

bool Entry::isArchive(const char* path){
    return  common::getExtension(path) == "zip" ||
            common::getExtension(path) == "rar" ||
            common::getExtension(path) == "tar" ||
            common::getExtension(path) == "7z"   ;
}

bool Entry::isPRX(const char* path){
    return (common::getExtension(path) == "prx");
}

bool Entry::isARK(const char* path){
    return (common::getExtension(path) == "ark");
}

bool Entry::isTXT(const char* path){
    string ext = common::getExtension(path);
    return (ext == "txt" || ext == "cfg" || ext == "ini" || ext == "log" || ext == "py" || ext == "lua");
}

bool Entry::isIMG(const char* path){
    string ext = common::getExtension(path);
    return (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp");
}

bool Entry::isMusic(const char* path){
    string ext = common::getExtension(path);
    return (ext == "mp3");
}

bool Entry::isVideo(const char* path){
    string ext = common::getExtension(path);
    return (ext == "pmf" || ext == "mps");
}

Entry::~Entry(){
}

bool Entry::cmpEntriesForSort (Entry* i, Entry* j) {
    return (strcasecmp(i->getName().c_str(), j->getName().c_str())<0);
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

void Entry::animAppear(){
    for (int i=480; i>=0; i-=40){
        common::clearScreen(CLEAR_COLOR);
        SystemMgr::drawScreen();
        Image* pic1 = this->getPic1();
        if (pic1 != NULL){
            if (pic1->getWidth() == 480 && pic1->getHeight() == 272)
                pic1->draw(i, 0);
            else
                pic1->draw_scale(i, 0, 480, 272);
        }
        Image* pic0 = this->getPic0();
        if (pic0 != NULL) pic0->draw(i+160, 85);
        this->getIcon()->draw(i+20, 92);
        common::flipScreen();
    }
}

void Entry::animDisappear(){
    for (int i=0; i<=480; i+=40){
        common::clearScreen(CLEAR_COLOR);
        common::drawScreen();
        SystemMgr::drawScreen();
        Image* pic1 = this->getPic1();
        if (pic1 != NULL){
            if (pic1->getWidth() == 480 && pic1->getHeight() == 272)
                pic1->draw(i, 0);
            else
                pic1->draw_scale(i, 0, 480, 272);
        }
        Image* pic0 = this->getPic0();
        if (pic0 != NULL) pic0->draw(i+160, 85);
        this->getIcon()->draw(i+20, 92);
        common::flipScreen();
    }
}

static int loading_data;

int load_thread(int argc, void* argp){
    Entry* e = (Entry*)(*(void**)argp);
    e->loadAVMedia();
    loading_data = false;
    sceKernelExitDeleteThread(0);
    return 0;
}

bool Entry::pmfPrompt(){

    bool ret;
    
    SystemMgr::pauseDraw();
    
    animAppear();

    loading_data = true;

    Entry* entry = this;

    int thd = sceKernelCreateThread("gamedata_thread", (SceKernelThreadEntry)&load_thread, 0x10, 0x10000, PSP_THREAD_ATTR_USER|PSP_THREAD_ATTR_VFPU, NULL);
    sceKernelStartThread(thd, sizeof(entry), &entry);

    float angle = 1.0;
    Image* img = common::getImage(IMAGE_WAITICON);
    while (loading_data){
        common::clearScreen(CLEAR_COLOR);
        entry->drawBG();
        entry->getIcon()->draw(20, 92);
        img->draw_rotate((480-img->getWidth())/2, (272-img->getHeight())/2, angle);
        angle+=0.2;
        common::flipScreen();
    }
    
    bool pmfPlayback = entry->getIcon1() != NULL || entry->getSnd() != NULL;
        
    if (pmfPlayback && !MusicPlayer::isPlaying()){
        if (sceUtilityLoadModule(PSP_MODULE_AV_PLAYER)>=0){
            PSPAVEntry ave = convertEntry(entry);
            ret = pspavPlayGamePMF(&ave, &av_callbacks, 20, 92);
            sceUtilityUnloadModule(PSP_MODULE_AV_PLAYER);
        }
    }
    else{
        Controller control;
    
        while (true){
            common::clearScreen(CLEAR_COLOR);
            entry->drawBG();
            entry->getIcon()->draw(20, 92);
            common::flipScreen();
            control.update(1);
            if (control.accept()){
                ret = true;
                break;
            }
            else if (control.decline()){
                ret = false;
                break;
            }
        }
    }
    if (!ret){
        common::playMenuSound();
        animDisappear();
    }
    SystemMgr::resumeDraw();
    return ret;
}
