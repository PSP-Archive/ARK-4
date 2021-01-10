/* Category Class that defines each of the individual menus */

#include "menu.h"

static SceUID iconSema = -1;

Menu::Menu(){
	this->index = 0;
	this->threadIndex = 0;
	this->animating = 0;
	this->fastScroll = 1;
	this->fastScrolling = false;
	this->animDelay = true;
	this->animState = 0.f;
	this->initLoad = false;
	this->stopLoading = false;
	this->entries = new vector<Entry*>();
	if (iconSema < 0)
		iconSema = sceKernelCreateSema("icon_sema",  0, 1, 1, NULL);
}


Menu::~Menu(){
	this->clearEntries();
	delete this->entries;
}

void Menu::freeScreenshots(){
	for (int i = 0; i < this->threadIndex-5; i++)
		this->getEntry(i)->freeScreenshot();

	for (int i = this->threadIndex+6; i<this->getVectorSize(); i++)
		this->getEntry(i)->freeScreenshot();
}

bool Menu::checkScreenshotsNeeded(bool isSelected){
	int umbral = (isSelected)? 2 : 5;
	for (int i = max(this->threadIndex-umbral, 0); i < min((int)this->getVectorSize(), this->threadIndex+umbral+1); i++){
		if (this->getEntry(i)->getScreenshot() == common::getImage(IMAGE_WAITICON))
			return true;
	}
	return false;
}

void Menu::loadScreenshotsDynamic(bool isSelected){

	if (this->fastScrolling || this->getVectorSize() == 0 || stopLoading)
		return; // we don't need to load any icons

	sceKernelWaitSema(iconSema, 1, NULL);

	this->stopLoading = false;
	this->threadIndex = this->index; // prevents our working index from changing
	freeScreenshots(); // delete any icon that won't be loaded
	if (!checkScreenshotsNeeded(isSelected)){ // check if we need to load icons
		sceKernelSignalSema(iconSema, 1);
		return;
	}
	// start loading the needed icons
	for (int i = max(0, this->threadIndex-5); i < min((int)this->getVectorSize(), this->threadIndex+6); i++){
		if (this->index != this->threadIndex || this->stopLoading)
			break; // stop loading the icons if the state of the menu has changed
		if (this->getEntry(i)->getScreenshot() == common::getImage(IMAGE_WAITICON))
			this->getEntry(i)->loadScreenshot();
	}
	initLoad = !this->stopLoading;
	sceKernelSignalSema(iconSema, 1);
}

bool Menu::waitScreenshotsLoad(bool isSelected, bool forceQuit){
	this->stopLoading = forceQuit;
	sceKernelWaitSema(iconSema, 1, NULL); // wait for the thread to release the semaphore
	if (this->getEntry()->getScreenshot() == common::getImage(IMAGE_WAITICON))
		this->getEntry()->loadScreenshot();
	sceKernelSignalSema(iconSema, 1);
	return true;
}

void Menu::resumeScreenshotsLoading(){
	this->stopLoading = false;
}

void Menu::draw(bool selected){

	if (this->getVectorSize() == 0)
		return;

	int xoffset = 300;
	int yoffset = -25;
	int curentry_yoffset = 0;
	
	float anim = 0.0f;
	if (animState >= 1.0f){
		if (animating == -1){
			if (this->index < this->getVectorSize()-1)
				this->index++;
			else
				animating = 0;
		}
		else if (animating == 1){
			if (this->index > 0)
				this->index--;
			else
				animating = 0;
		}
		if (fastScrolling){
			animState = 0.74f;
			animDelay = false;
		}
		else
			animating = 0;
	}
	
	if (animating){
		anim = (animating != 2)? animState : 0.f;
		if (!this->animDelay)
			animState += 0.25f;
		if (animating == -1)
			anim *= -1;
	}

	float scale = 1.f;
	if (animating){
		if (animState < 0.5f)
			scale = 0.5f;
		else if (animState < 1.f)
			scale = 0.75f;
	}

	for (int i=this->index-1; yoffset<272; i++){
		if (i < 0){
			yoffset+=40;
			continue;
		}
		if (i >= this->getVectorSize())
			break;
			
		if (i == this->index){
			curentry_yoffset = yoffset;
			if (animating)
				yoffset += 70;
			else
				yoffset += 90;
			continue;
		}
		else{
		
			if (animating == 1 && i == this->index-1 && !fastScrolling){
				getEntry(i)->getScreenshot()->draw_scale(xoffset+2, yoffset+anim*40, 0.75f, 0.75f);
				yoffset += 60;
			}
			else if (animating == -1 && i == this->index+1 && !fastScrolling){
				getEntry(i)->getScreenshot()->draw_scale(xoffset+2, yoffset+anim*40, 0.75f, 0.75f);
				yoffset += 60;
			}
			else{
				getEntry(i)->getScreenshot()->draw_scale(xoffset, yoffset+anim*40, 0.5f, 0.5f);
				yoffset += 40;
			}
		}
	}
	int height = getEntry(this->index)->getScreenshot()->getTexture()->height;
	if (animating){
		getEntry(this->index)->getScreenshot()->draw_scale(xoffset+2, curentry_yoffset+5+anim*40, 0.75f, 0.75f);
	}
	else {
		getEntry(this->index)->draw(xoffset+5, curentry_yoffset+5+anim*40);
	}
}

void Menu::animStart(int direction){
	animating = direction;
	animState = 0.f;
	animDelay = false;
}

bool Menu::empty(){
	bool ret = !this->entries->size();
	return ret;
}

void Menu::addEntry(Entry* e){
	this->entries->push_back(e);
}

Entry* Menu::getEntry(){
	return entries->at(this->index);
}

Entry* Menu::getEntry(int index){
	return entries->at(index);
}

void Menu::clearEntries(){
	this->entries->clear();
	this->index = 0;
	this->initLoad = false;
}

size_t Menu::getVectorSize(){
	size_t ret = this->entries->size();
	return ret;
}

vector<Entry*>* Menu::getVector(){
	return entries;
}

void Menu::moveUp(){
	if (animating || fastScrolling){
		fastScrolling = true;
		this->index -= fastScroll;
		fastScroll++;
		if (this->index <= 0){
			animating = 0;
			this->index = 0;
			this->stopFastScroll();
		}
	}
	else if (this->index > 0){
		this->stopFastScroll();
		animStart(1);
	}
	else
		this->stopFastScroll();
}

void Menu::moveDown(){
	if (animating || fastScrolling){
		fastScrolling = true;
		this->index += fastScroll;
		fastScroll++;
		if (this->index >= this->getVectorSize()-1){
			animating = 0;
			this->index = this->getVectorSize()-1;
			this->stopFastScroll();
		}
	}
	else if (this->index < this->getVectorSize()-1){
		this->stopFastScroll();
		animStart(-1);
	}
	else
		this->stopFastScroll();
}

void Menu::stopFastScroll(){
	fastScrolling = false;
	fastScroll = 1;
	animDelay = false;
}
