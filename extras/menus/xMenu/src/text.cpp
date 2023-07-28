#include "text.h"

TextAnim::TextAnim(string title, string subtitle){
    this->title = title;
    this->subtitle = subtitle;
    this->scroll = 200;
    this->ci = 0;
    this->skip = 120;
}

TextAnim::~TextAnim(){
}
        
void TextAnim::draw(float y){
    if (skip) skip--;
    else{
        scroll--;
        if (scroll < 200){
            ci++;
            scroll = 208;
            if (ci >= subtitle.size()+15){
                ci = 0;
                skip = 120;
                scroll = 200;
            }
        }
    }
    printTextScreen(200, y+30, title.c_str(), WHITE_COLOR);
    if (ci < subtitle.size())
        printTextScreen(scroll, y+60, subtitle.substr(ci, string::npos).c_str(), WHITE_COLOR);
}
