#include "entry.h"

#include "eboot.h"
#include "iso.h"
#include "cso.h"
#include "pmf.h"
#include "sprites.h"
#include "system_mgr.h"

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

Image* Entry::getIcon(){
    return this->icon0;
}

Image* Entry::getPic0(){
    return (this->pic0 == NULL)? NULL : this->pic0;
}

Image* Entry::getPic1(){
    return (this->pic1 == NULL)? common::getImage(IMAGE_BG) : this->pic1;
}

void Entry::freeIcon(){
    Image* aux = this->icon0;
    this->icon0 = common::getImage(IMAGE_WAITICON);
    if (!common::isSharedImage(aux))
        delete aux;
}

void Entry::executeHomebrew(){
    struct SceKernelLoadExecVSHParam param;
    
    memset(&param, 0, sizeof(param));
    
    int runlevel = (
        this->path[0]=='e' && this->path[1]=='f'
        && this->name != "Recovery Menu"
    )? HOMEBREW_RUNLEVEL_GO : HOMEBREW_RUNLEVEL;
    
    param.args = strlen(this->path.c_str()) + 1;
    param.argp = (char*)this->path.c_str();
    param.key = "game";
    sctrlKernelLoadExecVSHWithApitype(runlevel, this->path.c_str(), &param);
}

void Entry::executePSN(){
    struct SceKernelLoadExecVSHParam param;
    
    memset(&param, 0, sizeof(param));
    
    int runlevel = (this->path[0]=='e' && this->path[1]=='f')? ISO_RUNLEVEL_GO : ISO_RUNLEVEL;

    param.args = 33;  // lenght of "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN" + 1
    param.argp = (char*)"disc0:/PSP_GAME/SYSDIR/EBOOT.BIN";
    param.key = "umdemu";
    sctrlSESetBootConfFileIndex(PSN_DRIVER);
    sctrlSESetUmdFile("");
    sctrlKernelLoadExecVSHWithApitype(runlevel, this->path.c_str(), &param);
}

void Entry::executePOPS(){
    struct SceKernelLoadExecVSHParam param;
    
    memset(&param, 0, sizeof(param));
    
    int runlevel = (this->path[0]=='e' && this->path[1]=='f')? POPS_RUNLEVEL_GO : POPS_RUNLEVEL;
    
    param.args = strlen(this->path.c_str()) + 1;
    param.argp = (char*)this->path.c_str();
    param.key = "pops";
    sctrlKernelLoadExecVSHWithApitype(runlevel, this->path.c_str(), &param);
}

void Entry::executeEboot(){
    this->gameBoot();
    switch (Eboot::getEbootType(this->path.c_str())){
    case TYPE_HOMEBREW:    this->executeHomebrew();    break;
    case TYPE_PSN:        this->executePSN();            break;
    case TYPE_POPS:        this->executePOPS();        break;
    }
}

void Entry::executeISO(){

    this->gameBoot();

    struct SceKernelLoadExecVSHParam param;
    
    memset(&param, 0, sizeof(param));

    if (Iso::isPatched(this->path) || Cso::isPatched(this->path))
        param.argp = (char*)"disc0:/PSP_GAME/SYSDIR/EBOOT.OLD";
    else
        param.argp = (char*)"disc0:/PSP_GAME/SYSDIR/EBOOT.BIN";

    int runlevel = (this->path[0]=='e' && this->path[1]=='f')? ISO_RUNLEVEL_GO : ISO_RUNLEVEL;

    param.key = "umdemu";
    param.args = 33;  // lenght of "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN" + 1
    sctrlSESetBootConfFileIndex(ISO_DRIVER);
    sctrlSESetUmdFile((char*)this->path.c_str());
    sctrlKernelLoadExecVSHWithApitype(runlevel, this->path.c_str(), &param);
}

void Entry::gameBoot(){

    if (common::getConf()->fast_gameboot)
        return;

    SystemMgr::pauseDraw();

    unsigned mp3_size;
    void* mp3_buffer = common::readFromPKG("BOOT.MP3", &mp3_size);
    
    SceUID boot_thread = sceKernelCreateThread("boot_thread", gameBootThread, 0x10, 0x8000, PSP_THREAD_ATTR_USER, NULL);
    
    sceKernelStartThread(boot_thread, 0, NULL);
    
    playMP3File(NULL, mp3_buffer, mp3_size);
    
    free(mp3_buffer);
    
    sceKernelWaitThreadEnd(boot_thread, NULL);
    
}

void Entry::drawBG(){
    this->getPic1()->draw(0, 0);
    if (this->getPic0() != NULL)
        this->getPic0()->draw(160, 85);
}

void Entry::freeTempData(){
    if (this->pic0 != NULL)
        delete this->pic0;
    if (this->pic1 != common::getImage(IMAGE_BG))
        delete this->pic1;
    if (this->icon1 != NULL)
        free(this->icon1);
    if (this->snd0 != NULL)
        free(this->snd0);
}

bool Entry::run(){

    bool ret;

    if (this->pic1 == NULL)
        this->pic1 = common::getImage(IMAGE_BG);
    
    common::clearScreen(CLEAR_COLOR);
    common::getImage(IMAGE_BG)->draw(0, 0);
    this->drawBG();
    this->getIcon()->draw(10, 98);
    Image* img = common::getImage(IMAGE_WAITICON);
    img->draw((480-img->getTexture()->width)/2, (272-img->getTexture()->height)/2);
    common::flipScreen();
    
    getTempData2();
    
    bool pmfPlayback = this->icon1 != NULL || this->snd0 != NULL;
        
    if (pmfPlayback){
        ret = pmfStart(this, this->icon1, this->icon1_size, this->snd0, this->at3_size, 10, 98);
    }
    else{
        common::clearScreen(CLEAR_COLOR);
        common::getImage(IMAGE_BG)->draw(0, 0);
        this->drawBG();
        this->getIcon()->draw(10, 98);
        common::flipScreen();

        sceKernelDelayThread(100000);

        Controller control;
    
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
    }
    sceKernelDelayThread(100000);
    return ret;
}

Entry::~Entry(){
}
