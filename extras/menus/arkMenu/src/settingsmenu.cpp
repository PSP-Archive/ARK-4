#include "settingsmenu.h"
#include "gamemgr.h"
#include "system_mgr.h"


#define MENU_W 400
#define MENU_W_SPEED 50
#define MENU_H_SPEED 30
#define PAGE_SIZE 10

extern string ark_version;

SettingsMenu::SettingsMenu(SettingsTable* table, bool shorten_paths, bool show_all_opts, bool show_info){
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
    this->max_height = (table->max_options>0 && table->max_options<PAGE_SIZE)? 20*(table->max_options+2) : 20*PAGE_SIZE;
    this->table = table;
    this->info = "Menu Settings";
    this->name = "Settings";
    this->save_callback = NULL;
    this->open_callback = NULL;
    this->close_callback = NULL;
    this->reset_callback = NULL;
    this->icon = IMAGE_SETTINGS;
    this->shorten_paths = shorten_paths;
    this->show_all_opts = show_all_opts;
    this->show_info = show_info;
    this->scroll.w = 200;
    this->scroll2.w = 100;
}

SettingsMenu::~SettingsMenu(){
}

void SettingsMenu::setCallbacks(void (*save_callback)(), void (*open_callback)(), void (*close_callback)(), void (*reset_callback)()){
    this->save_callback = save_callback;
    this->open_callback = open_callback;
    this->close_callback = close_callback;
    this->reset_callback = reset_callback;

    if (reset_callback)
        this->setFooter("[_] to Reset Settings");
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

    if (this->index >= table->max_options)
        this->index = table->max_options-1;
    
    switch (animation){
    case -1:
        // opening animation
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
        // no animation (fully open)
        if (table->changed){
            // recalculate max_height
            this->max_height = (table->max_options>0 && table->max_options<PAGE_SIZE)? 20*(table->max_options+2) : 20*PAGE_SIZE;
            table->changed = 0;
        }
        if (customText == NULL || ntext == 0){
            x = (480-MENU_W)/2;
            y = (272-max_height)/2;

            // draw main window
            common::getImage(IMAGE_DIALOG)->draw_scale(x, y, MENU_W, max_height);
        
            // draw scrollbar if more than one page
            if (table->max_options > PAGE_SIZE){
                int height = max_height / table->max_options;
                ya2d_draw_rect(x-8, y, 3, height*table->max_options, DARKGRAY, 1);
                ya2d_draw_rect(x-9, y + index*height, 5, height, DARKGRAY, 1);
                ya2d_draw_rect(x-7, y, 1, height*table->max_options, LITEGRAY, 1);
                ya2d_draw_rect(x-8, y + index*height, 3, height, LITEGRAY, 1);
            }
        
            // show information if needed
            if (show_info){
                if (today.tm_mday == 3 && today.tm_mon == 6)
                    common::printText(x+10, y+15, "In Loving Memory of Gregory Pitka (qwikrazor87). R.I.P.", GRAY_COLOR, SIZE_LITTLE, 0, 0, 0);
                else if (today.tm_mday == 25 && today.tm_mon == 11)
                    common::printText(x+10, y+15, "Merry Christmas!", GRAY_COLOR, SIZE_LITTLE, 0, 0, 0);
                else if (today.tm_mday == 20 && today.tm_mon == 3)
                    common::printText(x+10, y+15, "Amplified Robotic Ketamine", GRAY_COLOR, SIZE_LITTLE, 0, 0, 0);
                else
                    common::printText(x+40, y+15, ark_version.c_str(), GRAY_COLOR, SIZE_LITTLE, 0, 0, 0);
            }
            
            int yoffset = y+40;
            int xoffset = x+10;
        
            // draw each entry
            for (int i=start; i<min(start+PAGE_SIZE, table->max_options); i++){
                unsigned char sel = table->settings_entries[i]->selection;
                // draw highlighted entry
                if (i==index){
                    common::printText(xoffset, yoffset, table->settings_entries[i]->description, GRAY_COLOR, SIZE_MEDIUM, 1, &scroll, !shorten_paths);
                    common::printText(xoffset+255, yoffset, table->settings_entries[i]->options[sel], GRAY_COLOR, SIZE_MEDIUM, 1, &scroll2);
                }
                // non-highlighted entries
                else{
                    string desc = table->settings_entries[i]->description;
                    // shorten them when asked
                    if (shorten_paths){
                        size_t split = desc.rfind('/');
                        if (split == string::npos) split = desc.find(',');
                        desc = desc.substr(split+1, string::npos);
                        desc = desc.erase(0, desc.find_first_not_of(" \t\n\r\f\v"));
                    }
                    else {
                        // treat as text: translate
                        desc = TR(desc);
                    }
                    int tw = common::calcTextWidth(desc.c_str(), SIZE_LITTLE, !shorten_paths);
                    if (tw > scroll.w){
                        int charw = (tw/desc.size());
                        int nchars = scroll.w/charw;
                        desc = (nchars<desc.size())? desc.substr(0, nchars-3) + "..." : desc;
                    }
                    common::printText(xoffset, yoffset, desc.c_str(), GRAY_COLOR, SIZE_LITTLE, 0, 0, !shorten_paths);
                    // show option for entry? only if told so
                    if (show_all_opts)
                        common::printText(xoffset+255, yoffset, table->settings_entries[i]->options[sel], GRAY_COLOR, SIZE_LITTLE, 0);
                }
                yoffset += 15;
            }
        }
        else {
            // draw custom text
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
        // closing animation
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
        if (table->max_options > 0){
            if (this->index == table->max_options-1){
                this->index = 0;
                this->start = 0;
            }
            else if (this->index-this->start == PAGE_SIZE-1){
                if (this->index+1 < table->max_options)
                    this->index++;
                if (this->start+PAGE_SIZE < table->max_options)
                    this->start++;
            }
            else if (this->index+1 < table->max_options)
                this->index++;
            common::playMenuSound();
        }
    }
    else if (pad->up()){
        if (table->max_options > 0){
            if (this->index == 0){
                this->index = table->max_options-1;
                this->start = table->max_options - PAGE_SIZE;
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
        if (table->max_options > 0 && index < table->max_options){
            if (table->settings_entries[index]->selection < (unsigned char)(table->settings_entries[index]->max_options-1))
                table->settings_entries[index]->selection++;
            else
                table->settings_entries[index]->selection = (unsigned char)0;
            common::playMenuSound();
            changed = true;
        }
    }
    else if (pad->left()){
        if (table->max_options > 0 && index < table->max_options){
            if (table->settings_entries[index]->selection > (unsigned char)0)
                table->settings_entries[index]->selection--;
            else
                table->settings_entries[index]->selection = (unsigned char)(table->settings_entries[index]->max_options-1);
            common::playMenuSound();
            changed = true;
        }
    }
    else if (pad->square() && this->reset_callback) {
        common::playMenuSound();
        pause();
        this->reset_callback();
        resume();
        if (this->save_callback) this->save_callback();
    }
}

void SettingsMenu::applyConf(){
    if (changed){
        for (int i=0; i<table->max_options; i++)
            *(table->settings_entries[i]->config_ptr) = table->settings_entries[i]->selection;
        if (this->save_callback) this->save_callback();
        readConf(); // update in case callback has changed it
    }
}

void SettingsMenu::readConf(){
    for (int i=0; i<table->max_options; i++)
        table->settings_entries[i]->selection = *(table->settings_entries[i]->config_ptr);
    changed = false;
}

void SettingsMenu::pause(){
    applyConf();
    if (this->close_callback){
        SystemMgr::pauseDraw();
        this->close_callback();
        SystemMgr::resumeDraw();
    }
    animation = 1;
    while (animation != -2)
        sceKernelDelayThread(0);
}

void SettingsMenu::resume(){
    animation = -1;
    if (this->open_callback){
        SystemMgr::pauseDraw();
        this->index = 0;
        this->start = 0;
        this->open_callback();
        SystemMgr::resumeDraw();
    }
    readConf();
    while (animation != 0)
        sceKernelDelayThread(0);
}

int SettingsMenu::getIndex(){
    return index;
}
