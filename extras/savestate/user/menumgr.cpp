#include "menumgr.h"
#include "menucommon.h"
#include "controller.h"

#include "entry.h"

#define MAX_OPTIONS 3

static struct {
	int x;
	int y;
	char* name;
	Image* img;
} pEntries[3] = {
	{20, 155, "Load", NULL},
	{105, 155, "Save", NULL},
	{180, 155, "Exit", NULL},
};

static MenuManager* self;

MenuManager::MenuManager(){

	self = this;

	this->selected = 0;
	
	pEntries[0].img = common::getImage(IMAGE_LOAD);
	pEntries[1].img = common::getImage(IMAGE_SAVE);
	pEntries[2].img = common::getImage(IMAGE_EXIT);
	
	menu = new Menu();
	
	this->findEntries();

	// start the drawing thread
	this->drawRunning = true;
	this->bootStatus1 = -240;
	this->bootStatus2 = 5;
	this->exitAnim = false;
	this->drawThreadID = sceKernelCreateThread("draw_thread", MenuManager::drawThread, 0x10, 0x8000, PSP_THREAD_ATTR_USER, NULL);
	
	// start the multithreaded icon loading
	this->dynamicIconRunning = true;
	//this->iconSema = sceKernelCreateSema("icon0_sema",  0, 1, 1, NULL);
	this->iconThread = sceKernelCreateThread("icon0_thread", MenuManager::loadIcons, 0x10, 0x10000, PSP_THREAD_ATTR_USER, NULL);
}

MenuManager::~MenuManager(){
	this->drawRunning = false;
	sceKernelWaitThreadEnd(this->drawThreadID, NULL);
	this->dynamicIconRunning = false;
	sceKernelWaitThreadEnd(this->iconThread, NULL);
}
		
void MenuManager::run(){

	sceKernelStartThread(this->drawThreadID, 0, NULL);
	sceKernelStartThread(this->iconThread,  0, NULL);

	while (bootStatus1 != 0 && bootStatus2 != 50)
		sceKernelDelayThread(100);
	
	Controller pad;
	
	while (true){
		pad.update();
		
		if (pad.down())
			menu->moveDown();
		else if (pad.up())
			menu->moveUp();
		else
			menu->stopFastScroll();
		
		if (pad.right()){
			if (selected < MAX_OPTIONS-1)
				selected++;
			else
				selected = 0;
		}
		else if (pad.left()){
			if (selected > 0)
				selected--;
			else
				selected = MAX_OPTIONS-1;
		}
		
		else if (pad.decline()){
			exitAnim = true;
			break;
		}
	}
	while (bootStatus1 > -240 || bootStatus2 > 5)
		sceKernelDelayThread(100);
}

void MenuManager::findEntries(){
	for (char i='0'; i<='9'; i++)
		menu->addEntry(new Entry(string("test")+i));
}

void MenuManager::draw(){

	int y = (272-bootStatus2)/2;
	y+=50;

	if (bootStatus1 == 0 && bootStatus2 == 50 && !exitAnim){
		common::getImage(IMAGE_BG)->draw(0, 0);
	
		switch (selected){
			case 0: menu->draw(true); break;
			case 1: menu->getEntry()->draw(305, 20); break;
			default: break;
		}
	
		common::getImage(IMAGE_BOX)->draw_scale(bootStatus1, y, 240, bootStatus2);
		
		for (int i=0; i<MAX_OPTIONS; i++){
			pEntries[i].img->draw(pEntries[i].x, pEntries[i].y);
			if (i == selected)
				common::printText(pEntries[i].x+15, pEntries[i].y+50, pEntries[i].name, GRAY_COLOR, SIZE_LITTLE, 1);
		}
		
	}
	else if (exitAnim){
		common::getImage(IMAGE_BG)->draw(0, 0);
		common::getImage(IMAGE_BOX)->draw_scale(bootStatus1, y, 240, bootStatus2);
		
		if (bootStatus2 > 5)
			bootStatus2 -= 5;
		else if (bootStatus1 > -240)
			bootStatus1 -= 10;
	}
	else {
		common::getImage(IMAGE_BG)->draw(0, 0);
		common::getImage(IMAGE_BOX)->draw_scale(bootStatus1, y, 240, bootStatus2);
		
		if (bootStatus1 < 0)
			bootStatus1 += 40;
		else
			bootStatus2 += 5;
	}

}
		
int MenuManager::drawThread(SceSize argc, void* argp){

	while (self->drawRunning){
		common::clearScreen(CLEAR_COLOR);
		self->draw();
		common::flipScreen();
		sceKernelDelayThread(0);
	}
	
	sceKernelExitDeleteThread(0);
	return 0;
}

int MenuManager::loadIcons(SceSize argc, void* argp){

	while (self->dynamicIconRunning){
		self->menu->loadScreenshotsDynamic(true);
		sceKernelDelayThread(0);
	}

	sceKernelExitDeleteThread(0);
	
	return 0;
}
