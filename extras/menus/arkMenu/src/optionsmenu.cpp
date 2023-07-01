#include "optionsmenu.h"
#include "controller.h"
#include "lang.h"

OptionsMenu::OptionsMenu(char* description, int n_options, t_options_entry* entries){
    this->description = description;
    this->n_options = n_options;
    this->entries = entries;
    this->index = 0;
    this->w = min(this->maxString(), 300);
    this->h = 30 + 15*n_options;
    this->x = (480-w)/2;
    this->y = (272-h)/2;
    this->scroll.w = 280;
}

OptionsMenu::~OptionsMenu(){
}

int OptionsMenu::maxString(){
    string cur_max = TR(description);
    for (int i=0; i<n_options; i++){
        string tmp = TR(entries[i].name);
        if (tmp.size() > cur_max.size()){
            cur_max = tmp;
        }
    }
    return common::calcTextWidth(cur_max.c_str(), SIZE_MEDIUM)+20;
}

void OptionsMenu::draw(){
    common::getImage(IMAGE_DIALOG)->draw_scale(x, y, w, h);
    
    int xoffset = x+10;
    int yoffset = y+15;
    
    common::printText(xoffset+10, yoffset, description);
    
    for (int i=0; i<n_options; i++){
        yoffset+=15;
        
        if (i == index)
            common::printText(xoffset, yoffset, entries[i].name, LITEGRAY, SIZE_MEDIUM, 1, &scroll);
        else {
            string s = TR(entries[i].name);
            int tw = common::calcTextWidth(s.c_str(), SIZE_LITTLE);
            if (tw > scroll.w){
                float cw = float(tw)/s.size();
                int nchars = scroll.w / cw;
                s = (s.substr(0, nchars-3)+"...");
            }
            common::printText(xoffset, yoffset, s.c_str(), LITEGRAY);
        }
    }
}
        
int OptionsMenu::control(){
    Controller pad;
    pad.flush();
    
    bool ret = true;
    
    while (true){
        pad.update();
        if (pad.up()){
            if (index > 0){
                index--;
                common::playMenuSound();
            }
        }
        else if (pad.down()){
            if (index < n_options-1){
                index++;
                common::playMenuSound();
            }
        }
        else if (pad.accept()){
            common::playMenuSound();
            ret = true;
            break;
        }
        else if (pad.decline()){
            common::playMenuSound();
            ret = false;
            break;
        }
    }
    pad.flush();
    return (ret)? entries[index].value : OPTIONS_CANCELLED;
}
