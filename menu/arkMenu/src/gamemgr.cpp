/* Game Manager class */

#include <sstream>
#include "system_mgr.h"
#include "gamemgr.h"
#include "zip.h"
#include "osk.h"

bool GameManager::update_game_list = false;

static GameManager* self = NULL;

static bool loadingData = false;

GameManager::GameManager(){

	// set the global self variable as this instance for the threads to use it
	self = this;
	
	this->use_categories = true;

	// start the drawing thread
	this->hasLoaded = false;
	
	GameManager::update_game_list = false;
	
	// initialize the categories
	this->selectedCategory = -1;
	for (int i=0; i<MAX_CATEGORIES; i++){
		this->categories[i] = new Menu((EntryType)i);
	}
	
	// find all available entries
	this->findEntries();
	
	// start the multithreaded icon loading
	this->dynamicIconRunning = true;
	this->iconSema = sceKernelCreateSema("icon0_sema",  0, 1, 1, NULL);
	this->iconThread = sceKernelCreateThread("icon0_thread", GameManager::loadIcons, 0x10, 0x10000, PSP_THREAD_ATTR_USER, NULL);
	sceKernelStartThread(this->iconThread,  0, NULL);
	
}

GameManager::~GameManager(){
	for (int i=0; i<MAX_CATEGORIES; i++){
		delete this->categories[i];
	}
}

void GameManager::installPlugin(string path){
	//self->vshmenu->getPluginsManager()->addNewPlugin(path);
}

int GameManager::loadIcons(SceSize _args, void *_argp){

	sceKernelDelayThread(0);

	while (self->dynamicIconRunning){
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
	for (int i=0; i<MAX_CATEGORIES; i++)
		categories[i]->waitIconsLoad(i == selectedCategory, true);
	sceKernelWaitSema(iconSema, 1, NULL);
}

void GameManager::resumeIcons(){
	for (int i=0; i<MAX_CATEGORIES; i++)
		categories[i]->resumeIconLoading();
	sceKernelSignalSema(iconSema, 1);
	sceKernelDelayThread(0);
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
	this->categories[0]->clearEntries();
	this->categories[1]->clearEntries();
	this->categories[2]->clearEntries();
	this->findEboots();
	this->findISOs();
	if (common::getConf()->scan_save)
		this->findSaveEntries();
	this->selectedCategory = -1;
	// find the first category with entries
	for (int i=0; i<MAX_CATEGORIES && selectedCategory < 0; i++){
		if (!this->categories[i]->empty())
			this->selectedCategory = i;
	}
}

void GameManager::findEboots(){
	struct dirent* dit;
	DIR* dir = opendir("ms0:/PSP/GAME/");
	
	if (dir == NULL)
		return;
		
	while ((dit = readdir(dir))){

		string fullpath = Eboot::fullEbootPath(dit->d_name);
		if (fullpath == "") continue;
		if (strcmp(dit->d_name, ".") == 0) continue;
		if (strcmp(dit->d_name, "..") == 0) continue;
		if (common::fileExists(string("ms0:/PSP/GAME/")+string(dit->d_name))) continue;
		
		Eboot* e = new Eboot(fullpath);
		switch (Eboot::getEbootType(fullpath.c_str())){
		case TYPE_HOMEBREW:	this->categories[HOMEBREW]->addEntry(e);	break;
		case TYPE_PSN:		this->categories[GAME]->addEntry(e);		break;
		case TYPE_POPS:		this->categories[POPS]->addEntry(e);		break;
		}
	}
	closedir(dir);
}

void GameManager::findISOs(){

	struct dirent* dit;
	DIR* dir = opendir("ms0:/ISO/");
	
	if (dir == NULL)
		return;
		
	while ((dit = readdir(dir))){

		string fullpath = string("ms0:/ISO/")+string(dit->d_name);

		if (strcmp(dit->d_name, ".") == 0) continue;
		if (strcmp(dit->d_name, "..") == 0) continue;
		if (!common::fileExists(fullpath)) continue;
		if (Iso::isISO(fullpath.c_str())) this->categories[GAME]->addEntry(new Iso(fullpath));
		else if (Cso::isCSO(fullpath.c_str())) this->categories[GAME]->addEntry(new Cso(fullpath));
	}
	closedir(dir);
}

void GameManager::findSaveEntries(){
	struct dirent* dit;
	DIR* dir = opendir("ms0:/PSP/SAVEDATA/");
	
	if (dir == NULL)
		return;
		
	while ((dit = readdir(dir))){

		string fullpath = string("ms0:/PSP/SAVEDATA/")+string(dit->d_name);

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
						case TYPE_HOMEBREW:	this->categories[HOMEBREW]->addEntry(e);	break;
						case TYPE_PSN:		this->categories[GAME]->addEntry(e);		break;
						case TYPE_POPS:		this->categories[POPS]->addEntry(e);		break;
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

char* GameManager::getInfo(){
	return (selectedCategory >= 0)? (char*)getEntry()->getName().c_str() : (char*)"No games available";
}

void GameManager::draw(){

	ostringstream fps;
	ya2d_calc_fps();
	fps<<ya2d_get_fps();
	
	common::printText(460, 260, fps.str().c_str());

	if (this->hasLoaded){

		char* entryName;

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
}

void GameManager::animAppear(){
	for (int i=480; i>=0; i-=40){
		common::clearScreen(CLEAR_COLOR);
		common::drawScreen();
		this->draw();
		Image* pic1 = this->getEntry()->getPic1();
		bool canDrawBg = !common::canDrawBackground();
		if (pic1 != common::getImage(IMAGE_BG) || canDrawBg)
			pic1->draw(i, 0);
		Image* pic0 = this->getEntry()->getPic0();
		if (pic0 != NULL)
			pic1->draw(i+160, 85);
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
		bool canDrawBg = !common::canDrawBackground();
		if (pic1 != common::getImage(IMAGE_BG) || canDrawBg)
			pic1->draw(i, 0);
		Image* pic0 = this->getEntry()->getPic0();
		if (pic0 != NULL)
			pic1->draw(i+160, 85);
		this->getEntry()->getIcon()->draw(i+10, 98);
		common::flipScreen();
	}
}

void GameManager::endAllThreads(){
	dynamicIconRunning = false;
	sceKernelWaitThreadEnd(iconThread, 0);
}

void GameManager::control(Controller* pad){

	this->hasLoaded = true;

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
		if (selectedCategory >= 0){
			if (string(this->getEntry()->getType()) == string("ZIP"))
				this->extractHomebrew();
			else
				this->execApp();
		}
	}
	else if (pad->start()){
		//vshmenu->getPluginsManager()->writeFiles(this->getEntry());
		this->waitIconsLoad(true);
		this->endAllThreads();
		if (selectedCategory >= 0)
			this->getEntry()->execute();
	}
	else if (pad->square()){
		this->switchCategoryMode();
	}
	if (GameManager::update_game_list){
		//SystemMgr::pauseDraw();
		this->hasLoaded = false;
		this->pauseIcons();
		this->findEntries();
		GameManager::update_game_list = false;
		this->hasLoaded = true;
		//SystemMgr::resumeDraw();
		this->resumeIcons();
	}
}

void GameManager::execApp(){
	this->waitIconsLoad();			
	if (common::getConf()->fast_gameboot){
		this->endAllThreads();
		this->getEntry()->execute();
	}
					
	this->pauseIcons();
	loadingData = true;
	this->getEntry()->getTempData1();
	SystemMgr::pauseDraw();
	loadingData = false;
	animAppear();
	if (this->getEntry()->run()){
		this->resumeIcons();
		SystemMgr::resumeDraw();
		//vshmenu->getPluginsManager()->writeFiles(this->getEntry());
		this->waitIconsLoad(true);
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
		"    ms0:/PSP/GAME/",
		"    ",
		"please wait..."
	};
	//this->drawVSH = true;
	//this->vshmenu->setCustomText(text, 6);
	unzipToDir(this->getEntry()->getPath().c_str(), (char*)"ms0:/PSP/GAME/", NULL);
	//this->vshmenu->unsetCustomText();
	//this->drawVSH = false;
	GameManager::update_game_list = true;
}

void GameManager::switchCategoryMode(){
	SystemMgr::pauseDraw();
	printf("pausing icons\n");
	pauseIcons();
	selectedCategory = -1;
	if (use_categories){
		printf("grouping\n");
		// group all categories into one
		vector<Entry*>* v = categories[1]->getVector();
		for (int i=0; i<v->size(); i++){
			categories[0]->addEntry(v->at(i));
		}
		v = categories[2]->getVector();
		for (int i=0; i<v->size(); i++){
			categories[0]->addEntry(v->at(i));
		}
		categories[1]->clearEntries();
		categories[2]->clearEntries();
		use_categories = false;
	}
	else{
		printf("rearrange\n");
		// rearrange all categories
		Menu* m = categories[0];
		vector<Entry*>* v = m->getVector();
		
		categories[0] = new Menu(GAME);
		for (int i=0; i<v->size(); i++){
			Entry* e = v->at(i);
			string type = e->getType();
			if (type == "EBOOT"){
				string subtype = e->getSubtype();
				if (subtype == "HOMEBREW"){
					categories[1]->addEntry(e);
				}
				else if (subtype == "PSN"){
					categories[0]->addEntry(e);
				}
				else if (subtype == "PSX"){
					categories[2]->addEntry(e);
				}
			}
			else if (type == "ISO"){
				categories[0]->addEntry(e);
			}
		}
		delete m;
		use_categories = true;
	}
	for (int i=0; i<MAX_CATEGORIES && selectedCategory < 0; i++){
		if (!this->categories[i]->empty())
			this->selectedCategory = i;
	}
	printf("resuming icons\n");
	resumeIcons();
	SystemMgr::resumeDraw();
}
