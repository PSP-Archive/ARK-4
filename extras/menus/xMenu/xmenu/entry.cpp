#include "entry.h"
#include "common.h"

Entry::Entry(string path){
	
	size_t lastSlash = path.rfind("/", string::npos);
	size_t substrPos = path.rfind("/", lastSlash-1)+1;

	this->path = path;
	this->ebootName = path.substr(lastSlash+1, string::npos);
	this->name = path.substr(substrPos, lastSlash-substrPos);
	readHeader();
	this->icon0 = loadIcon();
}

void Entry::readHeader(){
	void* data = malloc(sizeof(PBPHeader));
	
	FILE* fp = fopen(this->path.c_str(), "rb");
	fread(data, 1, sizeof(PBPHeader), fp);
	fclose(fp);
	
	this->header = (PBPHeader*)data;
}

Image* Entry::loadIcon(){
	Image* icon = loadImage(this->path.c_str(), this->header->icon0_offset);
	if (icon != NULL)
		return icon;
	return common::getNoIcon();
}

void Entry::animAppear(){
	for (int i=480; i>=0; i-=10){
		clearScreen(CLEAR_COLOR);
		blitAlphaImageToScreen(0, 0, 480-i, 272, this->pic1, i, 0);
		flipScreen();
	}
}

void Entry::animDisappear(){
	for (int i=0; i<=480; i+=10){
		clearScreen(CLEAR_COLOR);
		blitAlphaImageToScreen(0, 0, 480-i, 272, this->pic1, i, 0);
		flipScreen();
	}
}

string Entry::getName(){
	return this->name;
}

string Entry::getPath(){
	return this->path;
}
		
string Entry::getEbootName(){
	return this->ebootName;
}
		
Image* Entry::getIcon(){
	return this->icon0;
}

Image* Entry::getPic0(){
	return loadImage(this->path.c_str(), this->header->pic0_offset);
}

Image* Entry::getPic1(){
	return loadImage(this->path.c_str(), this->header->pic1_offset);
}

bool Entry::run(){
	this->pic0 = getPic0();
	this->pic1 = getPic1();
	if (this->pic1 == NULL)
		this->pic1 = common::getBG();
	
	animAppear();
	blitAlphaImageToScreen(0, 0, 480, 272, this->pic1, 0, 0);
	blitAlphaImageToScreen(0, 0, this->icon0->imageWidth, this->icon0->imageHeight, this->icon0, 10, 98);
	if (this->pic0 != NULL)
		blitAlphaImageToScreen(0, 0, this->pic0->imageWidth, this->pic0->imageHeight, this->pic0, 160, 85);
	guStart();
	common::printText(0, 0, "You have selected:");
	common::printText(10, 10, this->name.c_str());
	common::printText(10, 20, this->path.c_str());
	common::printText(0, 40, "Press Cross to continue");
	common::printText(0, 50, "Press Circle to cancel");
	common::flip();
	
	Controller control;
	bool ret;
	
	while (true){
		control.update();
		if (control.cross()){
			ret = true;
			break;
		}
		else if (control.circle()){
			ret = false;
			break;
		}
	}
	animDisappear();
	delete this->pic0;
	delete this->pic1;
	return ret;
}
		
Entry::~Entry(){
	delete this->icon0;
}
