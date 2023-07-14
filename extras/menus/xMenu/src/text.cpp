#include "text.h"

TextAnim::TextAnim(string title, string subtitle){
    this->title = title;
    this->subtitle = subtitle;
    this->scroll = 200;
}

TextAnim::~TextAnim(){
}
        
void TextAnim::draw(float y){
    printTextScreen(200, y+30, this->title.c_str(), WHITE_COLOR);
    printTextScreen(scroll, y+60, this->subtitle.c_str(), WHITE_COLOR);
}
