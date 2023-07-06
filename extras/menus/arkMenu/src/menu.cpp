/* Category Class that defines each of the individual menus */

#include "menu.h"
#include "mp3.h"

static SceUID iconSema = -1;

Menu::Menu(EntryType t){
    this->type = t;
    this->index = 0;
    this->threadIndex = 0;
    this->animating = 0;
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

void Menu::freeIcons(){
    for (int i = 0; i < this->threadIndex-5; i++)
        this->getEntry(i)->freeIcon();

    for (int i = this->threadIndex+6; i<this->getVectorSize(); i++)
        this->getEntry(i)->freeIcon();
}

bool Menu::checkIconsNeeded(bool isSelected){
    int threshold = (isSelected)? 2 : 5;
    for (int i = max(this->threadIndex-threshold, 0); i < min((int)this->getVectorSize(), this->threadIndex+threshold+1); i++){
        if (this->getEntry(i)->getIcon() == common::getImage(IMAGE_WAITICON))
            return true;
    }
    return false;
}

void Menu::checkIndex(){
    if (this->index>=this->getVectorSize()){
        this->index = this->getVectorSize()-1;
    }
    if (this->index<0){
        this->index = 0;
    }
}

void Menu::loadIconsDynamic(bool isSelected){

    if (this->fastScrolling || this->getVectorSize() == 0 || stopLoading)
        return; // we don't need to load any icons

    this->checkIndex();

    sceKernelWaitSema(iconSema, 1, NULL);

    this->stopLoading = false;
    this->threadIndex = this->index; // prevents our working index from changing
    freeIcons(); // delete any icon that won't be loaded
    if (!checkIconsNeeded(isSelected)){ // check if we need to load icons
        sceKernelSignalSema(iconSema, 1);
        return;
    }
    // start loading the needed icons
    for (int i = max(0, this->threadIndex-5); i < min((int)this->getVectorSize(), this->threadIndex+6); i++){
        if (this->index != this->threadIndex || this->stopLoading)
            break; // stop loading the icons if the state of the menu has changed
        if (this->getEntry(i)->getIcon() == common::getImage(IMAGE_WAITICON))
            this->getEntry(i)->loadIcon();
    }
    initLoad = !this->stopLoading;
    sceKernelSignalSema(iconSema, 1);
}

bool Menu::waitIconsLoad(bool isSelected, bool forceQuit){
    if (this->getVectorSize() == 0) return true;
    this->stopLoading = forceQuit;
    sceKernelWaitSema(iconSema, 1, NULL); // wait for the thread to release the semaphore
    if (this->getEntry()->getIcon() == common::getImage(IMAGE_WAITICON))
        this->getEntry()->loadIcon();
    sceKernelSignalSema(iconSema, 1);
    return true;
}

void Menu::resumeIconLoading(){
    this->stopLoading = false;
}

void Menu::draw(bool selected){

    if (this->getVectorSize() == 0)
        return;

    this->checkIndex();

    int xoffset = ((int)this->type)*144 + ((int)this->type+1)*10 + 5;
    int yoffset = 20;
    int curentry_yoffset = 0;

    // draw scrollbar
    if (selected && fastScrolling){
        int height = 240/entries->size();
        int x = xoffset-7;
        int y = yoffset+10;
        ya2d_draw_rect(x+2, y, 3, height*entries->size(), DARKGRAY, 1);
        ya2d_draw_rect(x+1, y + index*height, 5, height, DARKGRAY, 1);
        ya2d_draw_rect(x+3, y, 1, height*entries->size(), LITEGRAY, 1);
        ya2d_draw_rect(x+2, y + index*height, 3, height, LITEGRAY, 1);
    }
    
    // draw icon, calculating whatever for animation
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
            animState += (animating == 2)? 0.5f : 0.25f;
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

    for (int i=this->index-2; yoffset<272; i++){
        if (i < 0){
            yoffset+=40;
            continue;
        }

        if (i >= this->getVectorSize()){
            break;
        }
            
        if (selected && i == this->index){
            curentry_yoffset = yoffset;
            if (animating)
                yoffset += 70;
            else
                yoffset += 90;
            continue;
        }
        else{
            Image* e_icon = getEntry(i)->getIcon();
            if (animating == 1 && i == this->index-1 && !fastScrolling){
                e_icon->draw_scale(xoffset+2, yoffset+anim*40, 0.75f, 0.75f);
                yoffset += 60;
            }
            else if (animating == -1 && i == this->index+1 && !fastScrolling){
                e_icon->draw_scale(xoffset+2, yoffset+anim*40, 0.75f, 0.75f);
                yoffset += 60;
            }
            else{
                e_icon->draw_scale(xoffset, yoffset+anim*40, 0.5f, 0.5f);
                yoffset += 40;
            }
        }
    }

    if (selected){
        Image* e_icon = getEntry(this->index)->getIcon();
        int height = e_icon->getTexture()->height;
        if (height != 80)
            curentry_yoffset = (272-height)/2;
        
        if (animating){
            e_icon->draw_scale(xoffset+2, curentry_yoffset+5+anim*40, 0.75f, 0.75f);
        }
        else {
            int cw = common::calcTextWidth("...", 1.f, 0);
            float factor = float(e_icon->getWidth())/float(cw);
            common::printText(xoffset, curentry_yoffset+(height/2), "...", WHITE_COLOR, factor, 1, NULL, 0);
            e_icon->draw(xoffset+5, curentry_yoffset+5+anim*40);
        }   
    }
}

void Menu::animStart(int direction){
    animating = direction;
    animState = 0.f;
    animDelay = false;
}

bool Menu::isAnimating(){
    return animating != 0;
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
    for (int i=0; i<entries->size(); i++){
        Entry* entry = entries->at(i);
        entries->at(i) = NULL;
        delete entry;
    }
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
        if (this->index <= 0){
            animating = 0;
            this->index = 0;
            this->stopFastScroll();
        }
        else{
            this->index--;
            common::playMenuSound();
        }
    }
    else if (this->index > 0){
        common::playMenuSound();
        this->stopFastScroll();
        animStart(1);
    }
    else
        this->stopFastScroll();
}

void Menu::moveDown(){
    if (animating || fastScrolling){
        fastScrolling = true;
        if (this->index >= this->getVectorSize()-1){
            animating = 0;
            this->index = this->getVectorSize()-1;
            this->stopFastScroll();
        }
        else{
            common::playMenuSound();
            this->index++;
        }
    }
    else if (this->index < this->getVectorSize()-1){
        common::playMenuSound();
        this->stopFastScroll();
        animStart(-1);
    }
    else
        this->stopFastScroll();
}

void Menu::stopFastScroll(){
    fastScrolling = false;
    animDelay = false;
}
