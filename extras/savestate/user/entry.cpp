#include "entry.h"
#include <sstream>

Entry::Entry(string name){
	this->name = name;
	this->index = 0;
	for (int i=0; i<MAX_SLOTS; i++){
		ostringstream str;
		str<<"ms0:/SAVESTATE/"<<name<<"/SLOT"<<i<<".BIN";
		this->slots[i].path = str.str();
		this->slots[i].screenshot = common::getImage(IMAGE_WAITICON);
	}

}

string Entry::getName(){
	return this->name;
}

string Entry::getPath(){
	return this->slots[this->index].path;
}

void Entry::loadScreenshot(){
	this->slots[this->index].screenshot = common::getImage(IMAGE_NOSCREEN);
}

Image* Entry::getScreenshot(){
	return this->slots[this->index].screenshot;
}

void Entry::freeScreenshot(){
	Image* aux = this->slots[this->index].screenshot;
	this->slots[this->index].screenshot = common::getImage(IMAGE_WAITICON);
	if (!common::isSharedImage(aux))
		delete aux;
}

void Entry::nextSlot(){
	freeScreenshot();
	
	if (this->index < MAX_SLOTS-1)
		this->index++;
	else
		this->index = 0;
}

void Entry::prevSlot(){
	freeScreenshot();
	
	if (this->index > 0)
		this->index--;
	else
		this->index = MAX_SLOTS-1;
}

bool Entry::run(){
	return false;
}

void Entry::draw(int x, int y){
	this->slots[this->index].screenshot->draw(x, y);
	
	int boxX = 20;
	int boxY = y;
	int boxH = 100;
	int boxW = x-40;
	
	common::getImage(IMAGE_BOX)->draw_scale(boxX, boxY, boxW, boxH);
	
	common::printText(boxX+5, boxY+15, "Game:");
	common::printText(boxX+5, boxY+30, "Path:");
	common::printText(boxX+5, boxY+45, "Slot:");
	
	common::printText(boxX+50, boxY+15, this->name.c_str());
	common::printText(boxX+50, boxY+30, this->slots[this->index].path.c_str(), GRAY_COLOR, SIZE_LITTLE, 0, 1);
	common::printText(boxX+50, boxY+45, (string("")+(char)(this->index+'0')).c_str());
}

Entry::~Entry(){
}
