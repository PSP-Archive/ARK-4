#include "vshmenu.h"
#include "gamemgr.h"
#include "system_mgr.h"


#define MENU_W 300
#define MENU_W_SPEED 50
#define MENU_H_SPEED 30

VSHMenu::VSHMenu(vsh_entry** vsh_entries, int max_options, void (*save_callback)()){
    this->animation = -1;
    this->index = 0;
    this->w = 0;
    this->h = 0;
    this->x = 0;
    this->y = 0;
    this->customText = NULL;
    this->ntext = 0;
    this->changed = false;
    this->max_height = (max_options>0)? 20*max_options : 20;
    this->max_options = max_options;
    this->vsh_entries = vsh_entries;
    this->info = "Menu Settings";
    this->name = "Settings";
    this->callback = save_callback;
}

VSHMenu::~VSHMenu(){
}

void VSHMenu::setCustomText(string text[], int n){
    this->customText = text;
    this->ntext = n;
    animation = -1;
    while (animation != 0)
        sceKernelDelayThread(0);
}

void VSHMenu::unsetCustomText(){
    animation = 1;
    while (animation != -2)
        sceKernelDelayThread(0);
    this->customText = NULL;
    this->ntext = 0;
}

void VSHMenu::draw(){
    
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
            w = MENU_W;
            h = (index <= max_options)? max_height : max_height + 50;
            x = (480-MENU_W)/2;
            y = (272-max_height)/2;
            common::getImage(IMAGE_DIALOG)->draw_scale(x, y, w, h);
        
            common::printText(x+10, y+15, "OPTIONS MENU", GRAY_COLOR, SIZE_MEDIUM);
        
            int yoffset = y+30;
            int xoffset = x+10;
        
            for (int i = 0; i<max_options; i++){
                unsigned char sel = vsh_entries[i]->selection;
                if (i==index){
                    common::printText(xoffset, yoffset, vsh_entries[i]->description, GRAY_COLOR, SIZE_LITTLE, 1, 1);
                    common::printText(xoffset+215, yoffset, vsh_entries[i]->options[sel], GRAY_COLOR, SIZE_LITTLE, 1);
                }
                else{
                    string desc(vsh_entries[i]->description);
                    if (desc.size() > 35) desc = desc.substr(0, 30) + "...";
                    common::printText(xoffset, yoffset, desc.c_str(), GRAY_COLOR, SIZE_LITTLE, 0, 0);
                    common::printText(xoffset+215, yoffset, vsh_entries[i]->options[sel], GRAY_COLOR, SIZE_LITTLE, 0);
                }
                yoffset += 15;
            }
        }
        else {
            w = min(480, common::maxString(customText, ntext)*10);
            h = (index <= max_options)? max_height : max_height + 50;
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
        
void VSHMenu::control(Controller* pad){
    
    if (pad->down()){
        if (max_options > 0){
            if (index < max_options-1){
                index++;
                common::playMenuSound();
            }
            else {
                index = 0;
                common::playMenuSound();
            }
        }
    }
    else if (pad->up()){
        if (max_options > 0){
            if (index > 0){
                index--;
                common::playMenuSound();
            }
            else {
                index = max_options-1;
                common::playMenuSound();
            }
        }
    }
    else if (pad->accept()){
        common::playMenuSound();
        applyConf();
    }
    else if (pad->right()){
        if (max_options > 0 && index < max_options){
            if (vsh_entries[index]->selection < (unsigned char)(vsh_entries[index]->max_options-1))
                vsh_entries[index]->selection++;
            else
                vsh_entries[index]->selection = (unsigned char)0;
            common::playMenuSound();
            changed = true;
        }
    }
    else if (pad->left()){
        if (max_options > 0 && index < max_options){
            if (vsh_entries[index]->selection > (unsigned char)0)
                vsh_entries[index]->selection--;
            else
                vsh_entries[index]->selection = (unsigned char)(vsh_entries[index]->max_options-1);
            common::playMenuSound();
            changed = true;
        }
    }
}

void VSHMenu::applyConf(){
    if (changed){
        for (int i=0; i<max_options; i++)
            *(vsh_entries[i]->config_ptr) = vsh_entries[i]->selection;
        if (this->callback != NULL) this->callback();
    }
}

void VSHMenu::readConf(){
    for (int i=0; i<max_options; i++)
        vsh_entries[i]->selection = *(vsh_entries[i]->config_ptr);
}

void VSHMenu::pause(){
    applyConf();
    animation = 1;
    while (animation != -2)
        sceKernelDelayThread(0);
}

void VSHMenu::resume(){
    changed = false;
    animation = -1;
    
    readConf();

    while (animation != 0)
        sceKernelDelayThread(0);
}
