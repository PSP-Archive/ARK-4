/* Game Manager class */

#include <sstream>
#include <unistd.h>
#include "system_mgr.h"
#include "gamemgr.h"
#include "zip.h"
#include "osk.h"

static GameManager* self = NULL;

static bool loadingData = false;

GameManager::GameManager(){

    // set the global self variable as this instance for the threads to use it
    self = this;
    
    this->use_categories = true;

    // initialize the categories
    this->selectedCategory = -1;
    for (int i=0; i<MAX_CATEGORIES; i++){
        this->categories[i] = new Menu((EntryType)i);
    }
    
    // start the multithreaded icon loading
    this->dynamicIconRunning = ICONS_LOADING;
    this->iconSema = sceKernelCreateSema("icon0_sema",  0, 1, 1, NULL);
    this->iconThread = sceKernelCreateThread("icon0_thread", GameManager::loadIcons, 0x10, 0x10000, PSP_THREAD_ATTR_USER, NULL);
    sceKernelStartThread(this->iconThread,  0, NULL);
}

GameManager::~GameManager(){
    for (int i=0; i<MAX_CATEGORIES; i++){
        delete this->categories[i];
    }
}

int GameManager::loadIcons(SceSize _args, void *_argp){

    sceKernelDelayThread(0);

    while (self->dynamicIconRunning != ICONS_STOPPED){
        if (self->selectedCategory < 0){
            if (self->selectedCategory == -1) self->findEntries();
            sceKernelDelayThread(0);
            continue;
        }
        for (int i=0; i<MAX_CATEGORIES; i++){
            sceKernelWaitSema(self->iconSema, 1, NULL);
            self->categories[i]->loadIconsDynamic(i == self->selectedCategory);
            sceKernelSignalSema(self->iconSema, 1);
            sceKernelDelayThread(0);
        }
    }

    sceKernelExitDeleteThread(0);
    
    return 0;
}

void GameManager::pauseIcons(){
    if (self->dynamicIconRunning == ICONS_PAUSED) return; // already paused
    for (int i=0; i<MAX_CATEGORIES; i++)
        categories[i]->waitIconsLoad(i == selectedCategory, true);
    sceKernelWaitSema(iconSema, 1, NULL);
    self->dynamicIconRunning = ICONS_PAUSED;
}

void GameManager::resumeIcons(){
    if (self->dynamicIconRunning == ICONS_LOADING) return; // already running
    for (int i=0; i<MAX_CATEGORIES; i++)
        categories[i]->resumeIconLoading();
    sceKernelSignalSema(iconSema, 1);
    sceKernelDelayThread(0);
    self->dynamicIconRunning = ICONS_LOADING;
}

bool GameManager::waitIconsLoad(bool forceQuit){
    for (int i = 0; i<MAX_CATEGORIES; i++){
        if (!this->categories[i]->waitIconsLoad(i == selectedCategory, forceQuit))
            return false;
    }
    return true;
}

Menu* GameManager::getMenu(EntryType t){
    return this->categories[(int)t];
}

void GameManager::findEntries(){
    // clear entries
    this->categories[0]->clearEntries();
    this->categories[1]->clearEntries();
    this->categories[2]->clearEntries();
    
    // add recovery menu
    char cwd[128];
    getcwd((char*)cwd, sizeof(cwd));
    string recovery_path = string(cwd) + "/" + "RECOVERY.PBP";
    if (common::fileExists(recovery_path)){
        Eboot* recovery_menu = new Eboot(recovery_path);
        recovery_menu->setName("Recovery Menu");
        this->categories[HOMEBREW]->addEntry(recovery_menu);
    }
    
    // scan eboots
    this->findEboots("ms0:/PSP/VHBL/");
    this->findEboots("ms0:/PSP/APPS/");
    this->findEboots("ms0:/PSP/GAME/");
    this->findEboots("ef0:/PSP/GAME/");
    // scan ISOs
    this->findISOs("ms0:/ISO/");
    this->findISOs("ef0:/ISO/");
    // scan saves
    if (common::getConf()->scan_save){
        this->findSaveEntries("ms0:/PSP/SAVEDATA/");
        this->findSaveEntries("ef0:/PSP/SAVEDATA/");
    }
    this->selectedCategory = -2;
    // find the first category with entries
    for (int i=0; i<MAX_CATEGORIES && selectedCategory < 0; i++){
        if (!this->categories[i]->empty())
            this->selectedCategory = i;
    }
}

void GameManager::findEboots(const char* path){ 

    struct dirent* dit;
    DIR* dir = opendir(path);
    
    if (dir == NULL)
        return;
        
    while ((dit = readdir(dir))){

        string fullpath = Eboot::fullEbootPath(path, dit->d_name);
        if (fullpath == "") continue;
        if (strcmp(dit->d_name, ".") == 0) continue;
        if (strcmp(dit->d_name, "..") == 0) continue;
        if (common::fileExists(string(path)+string(dit->d_name))) continue;
        
        Eboot* e = new Eboot(fullpath);
        switch (Eboot::getEbootType(fullpath.c_str())){
        case TYPE_HOMEBREW:    this->categories[HOMEBREW]->addEntry(e);    break;
        case TYPE_PSN:         this->categories[GAME]->addEntry(e);        break;
        case TYPE_POPS:        this->categories[POPS]->addEntry(e);        break;
        }
    }
    closedir(dir);
}

void GameManager::findISOs(const char* path){

    struct dirent* dit;
    DIR* dir = opendir(path);
    
    if (dir == NULL)
        return;
        
    while ((dit = readdir(dir))){

        string fullpath = string(path)+string(dit->d_name);

        if (strcmp(dit->d_name, ".") == 0) continue;
        if (strcmp(dit->d_name, "..") == 0) continue;
        if (!common::fileExists(fullpath)) continue;
        if (Iso::isISO(fullpath.c_str())) this->categories[GAME]->addEntry(new Iso(fullpath));
        else if (Cso::isCSO(fullpath.c_str())) this->categories[GAME]->addEntry(new Cso(fullpath));
    }
    closedir(dir);
}

void GameManager::findSaveEntries(const char* path){
    struct dirent* dit;
    DIR* dir = opendir(path);
    
    if (dir == NULL)
        return;
        
    while ((dit = readdir(dir))){

        string fullpath = string(path)+string(dit->d_name);

        if (strcmp(dit->d_name, ".") == 0) continue;
        if (strcmp(dit->d_name, "..") == 0) continue;
        if (common::folderExists(fullpath)){
            struct dirent* savedit;
            DIR* savedir = opendir(fullpath.c_str());
            if (savedir == NULL)
                continue;
            while ((savedit = readdir(savedir))){
                if (strcmp(savedit->d_name, ".") == 0) continue;
                if (strcmp(savedit->d_name, "..") == 0) continue;
                string fullentrypath = fullpath + "/" + string(savedit->d_name);
                if ((common::getExtension(fullentrypath) == string("iso"))){
                    if (Iso::isISO(fullentrypath.c_str()))
                        this->categories[GAME]->addEntry(new Iso(fullentrypath));
                }
                else if ((common::getExtension(fullentrypath) == string("cso"))){
                    if (Cso::isCSO(fullentrypath.c_str()))
                        this->categories[GAME]->addEntry(new Cso(fullentrypath));
                }
                else if ((common::getExtension(fullentrypath) == string("zip"))){
                    if (Zip::isZip(fullentrypath.c_str()))
                        this->categories[HOMEBREW]->addEntry(new Zip(fullentrypath));
                }
                else if ((common::getExtension(fullentrypath) == string("pbp"))){
                    if (Eboot::isEboot(fullentrypath.c_str())){
                        Eboot* e = new Eboot(fullentrypath);
                        switch (Eboot::getEbootType(fullentrypath.c_str())){
                        case TYPE_HOMEBREW:    this->categories[HOMEBREW]->addEntry(e);    break;
                        case TYPE_PSN:        this->categories[GAME]->addEntry(e);        break;
                        case TYPE_POPS:        this->categories[POPS]->addEntry(e);        break;
                        }
                    }
                }
            }
            closedir(savedir);
        }
    }
    closedir(dir);
}

Entry* GameManager::getEntry(){
    if (selectedCategory < 0)
        return NULL;
    return this->categories[this->selectedCategory]->getEntry();
}

int GameManager::getPreviousCategory(int current){
    if (current > 0){
        current--;
    }
    else{
        current = MAX_CATEGORIES-1;
    }
    if (this->categories[current]->empty())
        return getPreviousCategory(current);
    return current;
}

int GameManager::getNextCategory(int current){
    if (current < MAX_CATEGORIES-1){
        current++;
    }
    else{
        current = 0;
    }
    if (this->categories[current]->empty())
        return getNextCategory(current);
    return current;
}

void GameManager::moveLeft(){
    if (selectedCategory < 0)
        return;

    int auxCategory = getPreviousCategory(selectedCategory);

    if (auxCategory != selectedCategory){
        this->categories[selectedCategory]->animStart(2);
        this->categories[auxCategory]->animStart(2);
        this->selectedCategory = auxCategory;
        common::playMenuSound();
    }
    sceKernelDelayThread(100000);
}

void GameManager::moveRight(){
    if (selectedCategory < 0)
        return;
    
    int auxCategory = getNextCategory(selectedCategory);
    
    if (auxCategory != selectedCategory){
        this->categories[selectedCategory]->animStart(2);
        this->categories[auxCategory]->animStart(2);
        this->selectedCategory = auxCategory;
        common::playMenuSound();
    }
    sceKernelDelayThread(100000);
}

void GameManager::moveUp(){
    if (selectedCategory < 0)
        return;
    this->categories[this->selectedCategory]->moveUp();
}

void GameManager::moveDown(){
    if (selectedCategory < 0)
        return;
    this->categories[this->selectedCategory]->moveDown();
}

void GameManager::stopFastScroll(){
    if (selectedCategory < 0)
        return;
    this->categories[this->selectedCategory]->stopFastScroll();
}

string GameManager::getInfo(){
    if (selectedCategory >= 0) return getEntry()->getName();
    else if (selectedCategory == -1) return "Loading games...";
    else if (selectedCategory == -2) return "No games available";
}

void GameManager::draw(){
    if (this->selectedCategory >= 0){
        for (int i=0; i<MAX_CATEGORIES; i++){
            if (i == (int)this->selectedCategory)
                continue;
            this->categories[i]->draw(false);
            sceKernelDelayThread(0);
        }
        this->categories[this->selectedCategory]->draw(true);
    }
    if (loadingData){
        Image* img = common::getImage(IMAGE_WAITICON);
        img->draw((480-img->getTexture()->width)/2, (272-img->getTexture()->height)/2);
    }
}

void GameManager::animAppear(){
    for (int i=480; i>=0; i-=40){
        common::clearScreen(CLEAR_COLOR);
        common::drawScreen();
        this->draw();
        Image* pic1 = this->getEntry()->getPic1();
        if (pic1 != NULL) pic1->draw(i, 0);
        Image* pic0 = this->getEntry()->getPic0();
        if (pic0 != NULL) pic0->draw(i+160, 85);
        this->getEntry()->getIcon()->draw(i+10, 98);
        common::flipScreen();
    }
}

void GameManager::animDisappear(){
    for (int i=0; i<=480; i+=40){
        common::clearScreen(CLEAR_COLOR);
        common::drawScreen();
        this->draw();
        Image* pic1 = this->getEntry()->getPic1();
        if (pic1 != NULL) pic1->draw(i, 0);
        Image* pic0 = this->getEntry()->getPic0();
        if (pic0 != NULL) pic0->draw(i+160, 85);
        this->getEntry()->getIcon()->draw(i+10, 98);
        common::flipScreen();
    }
}

void GameManager::endAllThreads(){
    dynamicIconRunning = ICONS_STOPPED;
    sceKernelWaitThreadEnd(iconThread, 0);
}

void GameManager::control(Controller* pad){

    if (pad->down()){
        this->moveDown();
    }
    else if (pad->up()){
        this->moveUp();
    }
    else {
        this->stopFastScroll();
    }
    
    if (pad->left())
        this->moveLeft();
    else if (pad->right())
        this->moveRight();
    else if (pad->accept()){
        if (selectedCategory >= 0 && !categories[selectedCategory]->isAnimating()){
            if (string(this->getEntry()->getType()) == string("ZIP"))
                this->extractHomebrew();
            else
                this->execApp();
        }
    }
    else if (pad->start()){
        if (selectedCategory >= 0 && !categories[selectedCategory]->isAnimating()){
            this->endAllThreads();
            this->getEntry()->execute();
        }
    }
}

void GameManager::updateGameList(const char* path){
    if (path == NULL 
          || strncmp(path, "ms0:/PSP/GAME/", 14) == 0 || !strncmp(path, "ms0:/ISO/", 9) == 0
          || strncmp(path, "ef0:/PSP/GAME/", 14) == 0 || !strncmp(path, "ef0:/ISO/", 9) == 0
          || strncmp(path, "ms0:/PSP/VHBL/", 14) == 0 || !strncmp(path, "ms0:/PSP/APPS/", 9) == 0
      ){
        self->selectedCategory = -1;
    }
}

void GameManager::execApp(){
    //this->waitIconsLoad();            
    if (common::getConf()->fast_gameboot){
        this->endAllThreads();
        this->getEntry()->execute();
    }

    this->pauseIcons();
    loadingData = true;
    SystemMgr::pauseDraw();
    this->getEntry()->getTempData1();
    loadingData = false;
    animAppear();
    if (this->getEntry()->run()){
        this->resumeIcons();
        SystemMgr::resumeDraw();
        //this->waitIconsLoad(true);
        this->endAllThreads();
        this->getEntry()->execute();
    }
    animDisappear();
    this->getEntry()->freeTempData();
    SystemMgr::resumeDraw();
    this->resumeIcons();
    sceKernelDelayThread(0);
}

void GameManager::extractHomebrew(){
    string text[6] = {
        "Extracting",
        "    "+this->getEntry()->getPath(),
        "into",
        "    /PSP/GAME/",
        "    ",
        "please wait..."
    };
    // get src and dest
    const char* src_path = this->getEntry()->getPath().c_str();
    char* dest_path = "ms0:/PSP/GAME/";
    // if extracting to same device as source file
    dest_path[0] = src_path[0];
    dest_path[1] = src_path[1];
    // do extraction
    unzipToDir(src_path, dest_path, NULL);
    // refresh game list
    GameManager::updateGameList(NULL);
}
