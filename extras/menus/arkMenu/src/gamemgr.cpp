/* Game Manager class */

#include <sstream>
#include <fstream>
#include <unistd.h>
#include <algorithm>
#include <stdlib.h>
#include "system_mgr.h"
#include "gamemgr.h"
#include "music_player.h"
#include "osk.h"
#include "pmf.h"

extern int sctrlKernelMsIsEf();

static GameManager* self = NULL;

static bool loadingData = false;

ARKConfig* ark_config;

GameManager::GameManager(){

    // set the global self variable as this instance for the threads to use it
    self = this;
    
    this->use_categories = true;
    this->scanning = true;
    this->optionsmenu = NULL;

    // initialize the categories
    this->selectedCategory = -2;
    for (int i=0; i<MAX_CATEGORIES; i++){
        this->categories[i] = new Menu((EntryType)i);
    }
    
    // start the multithreaded icon loading
    this->maxDraw = MAX_CATEGORIES;
    this->dynamicIconRunning = ICONS_LOADING;
    this->iconSema = sceKernelCreateSema("icon0_sema",  0, 1, 1, NULL);
    this->iconThread = sceKernelCreateThread("icon0_thread", GameManager::loadIcons, 0x10, 0x20000, PSP_THREAD_ATTR_USER|PSP_THREAD_ATTR_VFPU, NULL);
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
        sceKernelDelayThread(0);
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
        string recovery_path = string(common::getArkConfig()->arkpath) + "RECOVERY.PBP";
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
        if (optionsmenu) optionsmenu->draw();
    }
    if (loadingData || selectedCategory == -1){
        static float angle = 1.0;
        Image* img = common::getImage(IMAGE_WAITICON);
        img->draw_rotate(
            (480-img->getTexture()->width)/2,
            (272-img->getTexture()->height)/2,
            angle
        );
        angle+=0.2;
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
			this->startBoot();
        }
    }
    else if (pad->select()){
        if (selectedCategory != -1){
            common::playMenuSound();
            this->waitIconsLoad();
            GameManager::updateGameList(NULL);
            this->waitIconsLoad();
        }
    }
    else if (pad->LT()){
        if (selectedCategory >= 0 && !categories[selectedCategory]->isAnimating()){
            common::playMenuSound();
            this->gameOptionsMenu();
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

static int loading_data;

int load_thread(int argc, void* argp){
    Entry* e = (Entry*)(*(void**)argp);
    e->getTempData2();
    loading_data = false;
    sceKernelExitDeleteThread(0);
    return 0;
}

bool GameManager::pmfPrompt(){

    bool ret;
    
    SystemMgr::pauseDraw();
    
    animAppear();
    
    Entry* entry = this->getEntry();

    loading_data = true;

    int thd = sceKernelCreateThread("gamedata_thread", (SceKernelThreadEntry)&load_thread, 0x10, 0x10000, PSP_THREAD_ATTR_USER|PSP_THREAD_ATTR_VFPU, NULL);
    sceKernelStartThread(thd, sizeof(entry), &entry);

    float angle = 1.0;
    Image* img = common::getImage(IMAGE_WAITICON);
    while (loading_data){
        common::clearScreen(CLEAR_COLOR);
        entry->drawBG();
        entry->getIcon()->draw(10, 98);
        img->draw_rotate((480-img->getWidth())/2, (272-img->getHeight())/2, angle);
        angle+=0.2;
        common::flipScreen();
    }
    
    bool pmfPlayback = entry->getIcon1() != NULL || entry->getSnd() != NULL;
        
    if (pmfPlayback && !MusicPlayer::isPlaying()){
        ret = pmfStart(entry, 10, 98);
    }
    else{
        Controller control;
    
        while (true){
            common::clearScreen(CLEAR_COLOR);
            entry->drawBG();
            entry->getIcon()->draw(10, 98);
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

void GameManager::gameOptionsMenu(){
    t_options_entry options_entries[] = {
        {OPTIONS_CANCELLED, "Cancel"},
        {0, "View Info"},
        {1, "Rename"},
        {2, "Delete"},
    };
    optionsmenu = new OptionsMenu("Game Options", sizeof(options_entries)/sizeof(t_options_entry), options_entries);
    int ret = optionsmenu->control();
    OptionsMenu* aux = optionsmenu;
    optionsmenu = NULL;
    delete aux;

    switch (ret){
    case 0:{
        // create a new options menu but each entry is some info about the game
        Entry* e = this->getEntry();
        string path = e->getPath();
        if (e->getType() == "EBOOT"){
            path = path.substr(0, path.rfind('/')+1);
        }
        SfoInfo info = e->getSfoInfo();
        string fullname = string("Name - ") + string(info.title);
        string gameid = string("Game ID - ") + string(info.gameid);
        string fullpath = "Path - " + e->getPath();
        string size = "Size - " + common::beautifySize(Browser::recursiveSize(path));
        t_options_entry gameinfo_entries[] = {
            {0, (char*)fullname.c_str()},
            {1, (char*)gameid.c_str()},
            {2, (char*)size.c_str()},
            {3, (char*)fullpath.c_str()}
        };
        optionsmenu = new OptionsMenu("Game Info", sizeof(gameinfo_entries)/sizeof(t_options_entry), gameinfo_entries);
        optionsmenu->control();
        OptionsMenu* aux = optionsmenu;
        optionsmenu = NULL;
        delete aux;
    } break;
    case 1:{
        // rename the ISO or Eboot folder name
        SystemMgr::pauseDraw();
        Entry* e = this->getEntry();
        string name = e->getName();
        OSK osk;

        char parent[128];
        strcpy(parent, e->getPath().c_str());
        char* pname = strstr(parent, name.c_str());
        *pname = 0;
        
        osk.init("New name for Game", name.c_str(), 50);
        osk.loop();
        if(osk.getResult() != OSK_CANCEL)
        {
            char tmpText[51];
            osk.getText((char*)tmpText);
            string newpath = (string(parent)+string(tmpText));
            string oldpath = (string(parent)+name);
            
            sceIoRename(oldpath.c_str(), newpath.c_str());
            e->setName(tmpText);
            e->setPath(newpath);
        }
        osk.end();
        SystemMgr::resumeDraw();
    } break;
    case 2:{
        // remove current entry from list (adjusting index and selectedCategory accordingly), delete file or folder depending on ISO/EBOOT
        // make checks to prevent deleting stuff like "UMD Drive" and "Recovery" entries
        Entry* e = this->getEntry();
        string name = e->getName();
        if (name != "UMD Drive" && name != "Recovery Menu"){
            // pause drawing so we don't crash due to race condition
            SystemMgr::pauseDraw();
            // get current menu and it's game list
            Menu* category = categories[selectedCategory];
            std::vector<Entry*>* entries = category->getVector();
            // remove entry from list
            entries->erase(entries->begin()+category->getIndex());
            // adjust selectedCategory
            int retries = 0;
            while (categories[selectedCategory]->getVectorSize() == 0){
                if (selectedCategory < POPS) selectedCategory++;
                else selectedCategory = GAME;
                if (++retries >= 4){ // can't find a valid menu
                    selectedCategory = -2; // no games available
                    break;
                }
            }
            // resume drawing
            SystemMgr::resumeDraw();

            // delete file/folder
            if (e->getType() == "ISO"){
                sceIoRemove(e->getPath().c_str());
            }
            else if (e->getType() == "EBOOT"){
                string path = e->getPath();
                if (strstr(path.c_str(), "/PSP/SAVEDATA/") != NULL){
                    // remove eboot only
                    sceIoRemove(e->getPath().c_str());
                }
                else{
                    string path = e->getPath();
                    // remove entire folder
                    string folder = path.substr(0, path.rfind("/")+1);
                    Browser::recursiveFolderDelete(folder);
                    if(strstr(path.c_str(), "%/") != NULL) {
                        // remove 1.50 kxploit folder
                        path.erase(path.find("%/"));
                        path += '/';
                        folder = path;
                        Browser::recursiveFolderDelete(folder);
                    }
                }
            }
            // free resources
            delete e;
        }
    }break;
    default: break;
    }
}

void GameManager::startBoot(){
    switch (common::getConf()->startbtn){
    case 0: { // Default Start Button (Current)
        this->endAllThreads();
        this->getEntry()->execute();
    } break;
    case 1: { // Last game
        const char* last_game = common::getConf()->last_game;
        if (Eboot::isEboot(last_game)){
            this->endAllThreads();
            Eboot* eboot = new Eboot(last_game);
            eboot->execute();
        }
        else if (Iso::isISO(last_game)){
            this->endAllThreads();
            Iso* iso = new Iso(last_game);
            iso->execute();
        }
    } break;
    case 2: { // Random ISO
        if (this->categories[GAME]->getVectorSize() > 0){
            this->endAllThreads();
            srand(time(NULL));
            int rand_idx = rand() % this->categories[GAME]->getVectorSize();
            Entry* e = this->categories[GAME]->getEntry(rand_idx);
            e->execute();
        }
    } break;
    default: break;
    }
}