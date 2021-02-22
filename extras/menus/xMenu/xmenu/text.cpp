#include "text.h"

TextAnim::TextAnim(string glowText, string scrollText){
    this->glowText = glowText;
    this->scrollText = scrollText;
    this->scroll = 200.f;
}

TextAnim::~TextAnim(){
}
        
void TextAnim::draw(float y){

    float t = ((float)(clock() % CLOCKS_PER_SEC)) / ((float)CLOCKS_PER_SEC);
    int val = (t < 0.5f) ? t*511 : (1.0f-t)*511;
    intraFontSetStyle(common::getFont(), 0.7f, (0xFF<<24)+(val<<16)+(val<<8)+(val),0, 0.f, 0);
    intraFontPrint(common::getFont(), 200, y+30, this->glowText.c_str());

    intraFontSetStyle(common::getFont(), 0.5f, WHITE, 0, 0.f, INTRAFONT_SCROLL_LEFT);
    this->scroll = intraFontPrintColumn(common::getFont(), this->scroll, y+60, 250, this->scrollText.c_str());

}
