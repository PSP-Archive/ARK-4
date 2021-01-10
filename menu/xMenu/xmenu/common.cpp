#include "common.h"
#include "entry.h"

using namespace common;

void common::setArgs(int c, char** v){
	argc = c;
	argv = v;
}

bool common::fileExists(const std::string &path){
	FILE* fp = fopen(path.c_str(), "rb");
	if (fp == NULL)
		return false;
	fclose(fp);
	return true;
}

void common::loadData(){
	//background = loadImage("defbg.png", 0);
	//noicon = loadImage("noicon.png", 0);
	
	PBPHeader header;
	
	FILE* fp = fopen(argv[0], "rb");
	fread(&header, 1, sizeof(PBPHeader), fp);
	fclose(fp);
	
	background = loadImage(argv[0], header.pic1_offset);
	noicon = loadImage(argv[0], header.icon0_offset);
	font = intraFontLoad("flash0:/font/ltn0.pgf", 0);
}

void common::deleteData(){
	freeImage(background);
	freeImage(noicon);
	intraFontUnload(font);
}

Image* common::getBG(){
	return background;
}

Image* common::getNoIcon(){
	return noicon;
}

intraFont* common::getFont(){
	return font;
}


void common::printText(float x, float y, const char *text){
	intraFontSetStyle(font, 0.51f, GRAY_COLOR, BLACK_COLOR, 0);
	intraFontPrint(font, x, y, text);
}

void common::flip(){
	sceGuFinish();
	sceGuSync(0,0);
	guStart();
	sceGuTexMode(GU_PSM_8888, 0, 0, 0);
	sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
	sceGuTexFilter(GU_NEAREST, GU_NEAREST);
	sceGuFinish();
	sceGuSync(0,0); 

	sceDisplayWaitVblankStart(); 
	flipScreen();
	sceKernelDcacheWritebackInvalidateAll();
	sceKernelDelayThread(THREAD_DELAY);
};
