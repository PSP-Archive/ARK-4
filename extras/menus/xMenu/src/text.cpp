#include "text.h"

TextAnim::TextAnim(string title, string subtitle){
	this->title = title;
	this->subtitle = subtitle;
	this->scroll = 200.f;
}

TextAnim::~TextAnim(){
}
		
void TextAnim::draw(float y){
	
	printTextScreen(200, y+30, this->title.c_str(), WHITE_COLOR);
    printTextScreen(200, y+60, this->subtitle.c_str(), WHITE_COLOR);

}
