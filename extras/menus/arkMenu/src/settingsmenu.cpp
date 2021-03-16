#include "settingsmenu.h"
#include "gamemgr.h"
#include "system_mgr.h"


#define MENU_W 300
#define MENU_W_SPEED 50
#define MENU_H_SPEED 30
#define PAGE_SIZE 10

SettingsMenu::SettingsMenu(settings_entry** settings_entries, int max_options, void (*save_callback)()){
    this->animation = -1;
    this->index = 0;
    this->start = 0;
    this->w = 0;
    this->h = 0;
    this->x = 0;
    this->y = 0;
    this->customText = NULL;
    this->ntext = 0;
    this->changed = false;
    this->max_height = (max_options>0 && max_options<PAGE_SIZE)? 20*max_options : 20*PAGE_SIZE;
    this->max_options = max_options;
    this->settings_entries = settings_entries;
    this->info = "Menu Settings";
    this->name = "Settings";
    this->callback = save_callback;
}

SettingsMenu::~SettingsMenu(){
}

void SettingsMenu::setCustomText(string text[], int n){
    this->customText = text;
    this->ntext = n;
    animation = -1;
    while (animation != 0)
        sceKernelDelayThread(0);
}

void SettingsMenu::unsetCustomText(){
    animation = 1;
    while (animation != -2)
        sceKernelDelayThread(0);
    this->customText = NULL;
    this->ntext = 0;
}

void SettingsMenu::draw(){
    
    switch (animation){
    case -1:
        if (w < MENU_W || h < max_height){
            
            w += MENU_W_SPEED;
            if (w > MENU_W)
                w = MENU_W;
            
            h += MENU_H_SPEED;
            if (h > max_height)
                h = max_height;

            x = (480-w)/2;
            y = (272-h)/2;

            common::getImage(IMAGE_DIALOG)->draw_scale(x, y, w, h);
        }
        else {
            animation = 0;
            common::getImage(IMAGE_DIALOG)->draw_scale(x, y, w, h);
        }
        break;
    case 0:
        
        if (customText == NULL || ntext == 0){
            x = (480-MENU_W)/2;
            y = (272-max_height)/2;
            common::getImage(IMAGE_DIALOG)->draw_scale(x, y, MENU_W, max_height);
        
            int yoffset = y+30;
            int xoffset = x+10;
        
            for (int i=start; i<min(start+PAGE_SIZE, max_options); i++){
                unsigned char sel = settings_entries[i]->selection;
                if (i==index){
                    common::printText(xoffset, yoffset, settings_entries[i]->description, GRAY_COLOR, SIZE_LITTLE, 1, 1);
                    common::printText(xoffset+215, yoffset, settings_entries[i]->options[sel], GRAY_COLOR, SIZE_LITTLE, 1);
                }
                else{
                    string desc = settings_entries[i]->description;
                    size_t lastSlash = desc.rfind('/');
                    desc = desc.substr(lastSlash+1, -1);
                    if (desc.size() > 35) desc = desc.substr(0, 30) + "...";
                    common::printText(xoffset, yoffset, desc.c_str(), GRAY_COLOR, SIZE_LITTLE, 0, 0);
                    common::printText(xoffset+215, yoffset, settings_entries[i]->options[sel], GRAY_COLOR, SIZE_LITTLE, 0);
                }
                yoffset += 15;
            }
        }
        else {
            w = min(480, common::maxString(customText, ntext)*10);
            h = max_height;
            x = (480-w)/2;
            y = (272-h)/2;
            common::getImage(IMAGE_DIALOG)->draw_scale(x, y, w, h);
            
            int yoffset = y+30;
            int xoffset = x+10;
            for (int i = 0; i<ntext; i++){
                common::printText(xoffset, yoffset, customText[i].c_str(), GRAY_COLOR, SIZE_LITTLE, i==index);
                yoffset += 15;
            }
        }
        
        break;
    case 1:
        if (w > 0 || h > 0){
        
            w -= MENU_W_SPEED;
            if (w < 0)
                w = 0;
            
            h -= MENU_H_SPEED;
            if (h < 0)
                h = 0;
            
            x = (480-w)/2;
            y = (272-h)/2;
            
            common::getImage(IMAGE_DIALOG)->draw_scale(x, y, w, h);
        }
        else {
            animation = -2;
        }
        break;
    default: break;
    }
}
        
void SettingsMenu::control(Controller* pad){
    
    if (pad->down()){
        if (max_options > 0){
            if (this->index == (max_options-1)){
                this->index = 0;
                this->start = 0;
            }
            else if (this->index-this->start == PAGE_SIZE-1){
                if (this->index+1 < max_options)
                    this->index++;
                if (this->start+PAGE_SIZE < max_options)
                    this->start++;
            }
            else if (this->index+1 < max_options)
                this->index++;
            common::playMenuSound();
        }
    }
    else if (pad->up()){
        if (max_options > 0){
            if (this->index == 0){
                this->index = max_options-1;
                this->start = max_options - PAGE_SIZE;
                if (this->start < 0) this->start = 0;
            }
            else if (this->index == this->start){
                this->index--;
                if (this->start>0)
                    this->start--;
            }
            else
                this->index--;
            common::playMenuSound();
        }
    }
    else if (pad->accept()){
        common::playMenuSound();
        applyConf();
    }
    else if (pad->right()){
        if (max_options > 0 && index < max_options){
            if (settings_entries[index]->selection < (unsigned char)(settings_entries[index]->max_options-1))
                settings_entries[index]->selection++;
            else
                settings_entries[index]->selection = (unsigned char)0;
            common::playMenuSound();
            changed = true;
        }
    }
    else if (pad->left()){
        if (max_options > 0 && index < max_options){
            if (settings_entries[index]->selection > (unsigned char)0)
                settings_entries[index]->selection--;
            else
                settings_entries[index]->selection = (unsigned char)(settings_entries[index]->max_options-1);
            common::playMenuSound();
            changed = true;
        }
    }
}

void SettingsMenu::applyConf(){
    if (changed){
        for (int i=0; i<max_options; i++)
            *(settings_entries[i]->config_ptr) = settings_entries[i]->selection;
        if (this->callback != NULL) this->callback();
        changed = false;
    }
}

void SettingsMenu::readConf(){
    for (int i=0; i<max_options; i++)
        settings_entries[i]->selection = *(settings_entries[i]->config_ptr);
    changed = false;
}

void SettingsMenu::pause(){
    applyConf();
    animation = 1;
    while (animation != -2)
        sceKernelDelayThread(0);
}

void SettingsMenu::resume(){
    animation = -1;
    readConf();
    while (animation != 0)
        sceKernelDelayThread(0);
}
