/* Game Manager class */

#include <sstream>
#include <fstream>
#include <unistd.h>
#include <algorithm>
#include "system_mgr.h"
#include "gamemgr.h"
#include "osk.h"
#include "pmf.h"

extern int sctrlKernelMsIsEf();

static GameManager* self = NULL;

static bool loadingData = false;

GameManager::GameManager(){

    // set the global self variable as this instance for the threads to use it
    self = this;
    
    this->use_categories = true;
    this->scanning = true;

    // initialize the categories
    this->selectedCategory = -2;
    for (int i=0; i<MAX_CATEGORIES; i++){
        this->categories[i] = new Menu((EntryType)i);
    }
    
    // start the multithreaded icon loading
    this->maxDraw = MAX_CATEGORIES;
    this->dynamicIconRunning = ICONS_LOADING;
    this->iconSema = sceKernelCreateSema("icon0_sema",  0, 1, 1, NULL);
    this->iconThread = sceKernelCreateThread("icon0_thread", GameManager::loadIcons, 0x10, 0x20000, PSP_THREAD_ATTR_USER, NULL);
    sceKernelStartThread(this->iconThread,  0, NULL);
    
}

GameManager::~GameManager(){
    for (int i=0; i<MAX_CATEGORIES; i++){
        delete this->categories[i];
    }
}

int GameManager::loadIcons(SceSize _args, void *_argp){

    while (self->dynamicIconRunning != ICONS_STOPPED){
        // check UMD status
        sceKernelDelayThread(100000);
        std::vector<Entry*>* game_entries = self->categories[GAME]->getVector();
        bool has_umd = UMD::isUMD();
        bool umd_loaded = game_entries->size() > 0 && string("UMD") == game_entries->at(0)->getType();
        if (has_umd && !umd_loaded){ // UMD inserted but not loaded
            SystemMgr::pauseDraw();
            game_entries->insert(game_entries->begin(), new UMD());
            SystemMgr::resumeDraw();
            common::playMenuSound();
        }
        else if (umd_loaded && !has_umd){ // UMD loaded but not inserted
            SystemMgr::pauseDraw();
            UMD* umd = (UMD*)game_entries->at(0);
            game_entries->erase(game_entries->begin());
            delete umd;
            if (game_entries->size() == 0 && self->selectedCategory == GAME){
                self->selectedCategory = HOMEBREW;
            }
            SystemMgr::resumeDraw();
            common::playMenuSound();
        }
        // load icons
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

    this->scanning = true;

    int ms_is_ef = sctrlKernelMsIsEf();

    // clear entries
    this->categories[0]->clearEntries();
    this->categories[1]->clearEntries();
    this->categories[2]->clearEntries();
    
    // scan eboots
    this->findEboots("ms0:/PSP/VHBL/");
    this->findEboots("ms0:/PSP/APPS/");
    this->findEboots("ms0:/PSP/GAME/");
    if (!ms_is_ef) this->findEboots("ef0:/PSP/GAME/");
    // scan ISOs
    this->findISOs("ms0:/ISO/");
    if (!ms_is_ef) this->findISOs("ef0:/ISO/");
    // scan saves
    if (common::getConf()->scan_save){
        this->findSaveEntries("ms0:/PSP/SAVEDATA/");
        if (!ms_is_ef) this->findSaveEntries("ef0:/PSP/SAVEDATA/");
    }

    if (common::getConf()->sort_entries){
        std::sort(this->categories[0]->getVector()->begin(), this->categories[0]->getVector()->end(), Entry::cmpEntriesForSort);
        std::sort(this->categories[1]->getVector()->begin(), this->categories[1]->getVector()->end(), Entry::cmpEntriesForSort);
        std::sort(this->categories[2]->getVector()->begin(), this->categories[2]->getVector()->end(), Entry::cmpEntriesForSort);
    }

    // add recovery menu
    if (common::getConf()->show_recovery){
        char cwd[128];
        getcwd((char*)cwd, sizeof(cwd));
        string recovery_path = string(cwd) + "/" + "RECOVERY.PBP";
        if (common::fileExists(recovery_path)){
            Eboot* recovery_menu = new Eboot(recovery_path);
            recovery_menu->setName("Recovery Menu");
            this->categories[HOMEBREW]->getVector()->insert(this->categories[HOMEBREW]->getVector()->begin(), recovery_menu);
        }
    }

    // find the first category with entries
    this->selectedCategory = -2;
    for (int i=0; i<MAX_CATEGORIES && selectedCategory < 0; i++){
        if (!this->categories[i]->empty())
            this->selectedCategory = i;
    }

    this->scanning = false;
}

void GameManager::findEboots(const char* path){ 

    struct dirent* dit;
    DIR* dir = opendir(path);
    
    if (dir == NULL)
        return;
        
    while ((dit = readdir(dir))){
		if (strstr(dit->d_name, "%") != NULL) continue;
        if (strcmp(dit->d_name, ".") == 0) continue;
        if (strcmp(dit->d_name, "..") == 0) continue;
        if (!FIO_SO_ISDIR(dit->d_stat.st_attr)) continue;
        
        string fullpath = Eboot::fullEbootPath(path, dit->d_name);
        if (fullpath == ""){
            if (common::getConf()->scan_cat){
                findEboots((string(path) + dit->d_name + "/").c_str());
            }
            continue;
        }

        Eboot* e = new Eboot(fullpath);
        switch (Eboot::getEbootType(fullpath.c_str())){
        case TYPE_PSN:         this->categories[GAME]->addEntry(e);        break;
        case TYPE_POPS:        this->categories[POPS]->addEntry(e);        break;
        default:               this->categories[HOMEBREW]->addEntry(e);    break;
        }
    }
    closedir(dir);
}

void GameManager::findISOs(const char* path){

    int dir = sceIoDopen(path);
    
    SceIoDirent entry;
    SceIoDirent* dit = &entry;
    memset(&entry, 0, sizeof(SceIoDirent));

    pspMsPrivateDirent* pri_dirent = (pspMsPrivateDirent*)malloc(sizeof(pspMsPrivateDirent));
    pri_dirent->size = sizeof(pspMsPrivateDirent);
    entry.d_private = (void*)pri_dirent;

    if (dir == NULL)
        return;
        
    while (sceIoDread(dir, dit) > 0){

        if (strcmp(dit->d_name, ".") == 0) continue;
        if (strcmp(dit->d_name, "..") == 0) continue;

        string fullpath = string(path)+string(dit->d_name);

        if (FIO_SO_ISDIR(dit->d_stat.st_attr)){
            if (common::getConf()->scan_cat && string(dit->d_name) != string("VIDEO")){
                findISOs((string(path) + dit->d_name + "/").c_str());
            }
            continue;
        }
        else if (!common::fileExists(fullpath)){
            fullpath = string(path) + string(dit->d_name).substr(0, 4) + string(pri_dirent->s_name);
        }
        if (Iso::isISO(fullpath.c_str())) this->categories[GAME]->addEntry(new Iso(fullpath));
    }
    sceIoDclose(dir);
    free(pri_dirent);
}

void GameManager::findSaveEntries(const char* path){
    struct dirent* dit;
    DIR* dir = opendir(path);
    
    if (dir == NULL)
        return;
        
    while ((dit = readdir(dir))){

        if (strcmp(dit->d_name, ".") == 0) continue;
        if (strcmp(dit->d_name, "..") == 0) continue;
        if (FIO_SO_ISDIR(dit->d_stat.st_attr)){
            struct dirent* savedit;
            string fullpath = string(path)+string(dit->d_name);
            DIR* savedir = opendir(fullpath.c_str());
            if (savedir == NULL)
                continue;
            while ((savedit = readdir(savedir))){
                if (strcmp(savedit->d_name, ".") == 0) continue;
                if (strcmp(savedit->d_name, "..") == 0) continue;
                string fullentrypath = fullpath + "/" + string(savedit->d_name);
                if (Iso::isISO(fullentrypath.c_str())) this->categories[GAME]->addEntry(new Iso(fullentrypath));
                else if (Eboot::isEboot(fullentrypath.c_str())){
                    Eboot* e = new Eboot(fullentrypath);
                    switch (Eboot::getEbootType(fullentrypath.c_str())){
                    case TYPE_PSN:         this->categories[GAME]->addEntry(e);        break;
                    case TYPE_POPS:        this->categories[POPS]->addEntry(e);        break;
                    default:               this->categories[HOMEBREW]->addEntry(e);    break;
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
    do { 
        if (current > 0){
            current--;
        }
        else{
            current = MAX_CATEGORIES-1;
        }
    } while (this->categories[current]->empty());
    return current;
}

int GameManager::getNextCategory(int current){
    do {
        if (current < MAX_CATEGORIES-1){
            current++;
        }
        else{
            current = 0;
        }
    } while (this->categories[current]->empty());
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
    return "Unknown Menu State";
}

void GameManager::draw(){
    if (this->selectedCategory >= 0){
        for (int i=0; i<this->maxDraw; i++){
            if (i == (int)this->selectedCategory)
                continue;
            this->categories[i]->draw(false);
            sceKernelDelayThread(0);
        }
        if (this->selectedCategory < this->maxDraw)
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
        if (pic1 != NULL){
            if (pic1->getWidth() == 480 && pic1->getHeight() == 272)
                pic1->draw(i, 0);
            else
                pic1->draw_scale(i, 0, 480, 272);
        }
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
        if (pic1 != NULL){
            if (pic1->getWidth() == 480 && pic1->getHeight() == 272)
                pic1->draw(i, 0);
            else
                pic1->draw_scale(i, 0, 480, 272);
        }
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
            common::playMenuSound();
            this->execApp();
        }
    }
    else if (pad->start()){
        if (selectedCategory >= 0 && !categories[selectedCategory]->isAnimating()){
            this->endAllThreads();
            this->getEntry()->execute();
        }
    }
    else if (pad->select()){
        if (selectedCategory != -1){
            common::playMenuSound();
            self->waitIconsLoad();
            GameManager::updateGameList(NULL);
            self->waitIconsLoad();
        }
    }
}

void GameManager::updateGameList(const char* path){
    if (path == NULL 
          || strncmp(path, "ms0:/PSP/GAME/", 14) == 0 || !strncmp(path, "ms0:/ISO/", 9) == 0
          || strncmp(path, "ef0:/PSP/GAME/", 14) == 0 || !strncmp(path, "ef0:/ISO/", 9) == 0
          || strncmp(path, "ms0:/PSP/VHBL/", 14) == 0 || !strncmp(path, "ms0:/PSP/APPS/", 9) == 0
      ){
        int icon_status = self->dynamicIconRunning;
        if (icon_status == ICONS_LOADING){
            self->pauseIcons();
        }
        SystemMgr::pauseDraw();
        self->selectedCategory = -1;
        SystemMgr::resumeDraw();
        if (icon_status == ICONS_LOADING){
            self->resumeIcons();
        }
    }
}

void GameManager::execApp(){
    if (common::getConf()->fast_gameboot){
        this->endAllThreads();
        this->getEntry()->execute();
    }

    loadingData = true;
    this->waitIconsLoad();
    this->getEntry()->getTempData1();
    loadingData = false;
    if (this->pmfPrompt()){
        this->endAllThreads();
        this->getEntry()->execute();
    }
    this->getEntry()->freeTempData();
    sceKernelDelayThread(0);
}

bool GameManager::pmfPrompt(){


    bool ret;
    
    SystemMgr::pauseDraw();
    
    animAppear();
    
    Entry* entry = this->getEntry();

    common::clearScreen(CLEAR_COLOR);
    entry->drawBG();
    entry->getIcon()->draw(10, 98);
    Image* img = common::getImage(IMAGE_WAITICON);
    img->draw((480-img->getWidth())/2, (272-img->getHeight())/2);
    common::flipScreen();
    
    entry->getTempData2();
    
    bool pmfPlayback = entry->getIcon1() != NULL || entry->getSnd() != NULL;
        
    if (pmfPlayback){
        ret = pmfStart(entry, 10, 98);
    }
    else{
        Controller control;
    
        while (true){
            common::clearScreen(CLEAR_COLOR);
            entry->drawBG();
            entry->getIcon()->draw(10, 98);
            common::flipScreen();
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
    if (!ret){
        common::playMenuSound();
        animDisappear();
    }
    SystemMgr::resumeDraw();
    return ret;
}
