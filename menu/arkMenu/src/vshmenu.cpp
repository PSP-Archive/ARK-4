#include "vshmenu.h"
#include "gamemgr.h"
#include "system_mgr.h"
#include "vshmenu_entries.h"


#define MENU_W 300
#define MENU_H 20*MAX_OPTIONS
#define MENU_W_SPEED 50
#define MENU_H_SPEED 30


VSHMenu::VSHMenu(){
	this->animation = -2;
	this->index = 0;
	this->w = 0;
	this->h = 0;
	this->x = 0;
	this->y = 0;
	this->customText = NULL;
	this->ntext = 0;
	this->state = MAIN_MENU;
	this->changed = false;
}

VSHMenu::~VSHMenu(){
}

void VSHMenu::setCustomText(string text[], int n){
	this->customText = text;
	this->ntext = n;
	animation = -1;
	while (animation != 0)
		sceKernelDelayThread(0);
}

void VSHMenu::unsetCustomText(){
	animation = 1;
	while (animation != -2)
		sceKernelDelayThread(0);
	this->customText = NULL;
	this->ntext = 0;
}

void VSHMenu::draw(){
	
	switch (this->state){
	case MAIN_MENU: break;
	case PLUGINS_MENU: plugins.draw(); return;
	case PEOPS_MENU: peops_menu.draw(); return;
	}

	switch (animation){
	case -1:
		if (w < MENU_W || h < MENU_H){
			
			w += MENU_W_SPEED;
			if (w > MENU_W)
				w = MENU_W;
			
			h += MENU_H_SPEED;
			if (h > MENU_H)
				h = MENU_H;

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
		
		if (customText == NULL || ntext == 0){
			w = MENU_W;
			h = (index <= MAX_OPTIONS)? MENU_H : MENU_H + 50;
			x = (480-MENU_W)/2;
			y = (272-MENU_H)/2;
			common::getImage(IMAGE_DIALOG)->draw_scale(x, y, w, h);
		
			common::printText(x+10, y+15, "OPTIONS MENU", GRAY_COLOR, SIZE_MEDIUM);
		
			{
			int yoffset = y+30;
			int xoffset = x+10;
		
			for (int i = 0; i<MAX_OPTIONS; i++){
				unsigned char sel = vsh_entries[i]->selection;
				common::printText(xoffset, yoffset, vsh_entries[i]->description, GRAY_COLOR, SIZE_LITTLE, i==index);
				common::printText(xoffset+150, yoffset, vsh_entries[i]->options[sel], GRAY_COLOR, SIZE_LITTLE, i==index);
				yoffset += 15;
			}
			common::printText(xoffset, yoffset, "Plugins Manager", GRAY_COLOR, SIZE_LITTLE, index==MAX_OPTIONS);
			if (index > MAX_OPTIONS){
				common::printText(xoffset+20, yoffset+15, "Manage Global Plugins", GRAY_COLOR, SIZE_LITTLE, index==MAX_OPTIONS+1);
				common::printText(xoffset+20, yoffset+30, "Manage Local Plugins", GRAY_COLOR, SIZE_LITTLE, index==MAX_OPTIONS+2);
			}
			}
		}
		else {
			w = min(480, common::maxString(customText, ntext)*10);
			h = (index <= MAX_OPTIONS)? MENU_H : MENU_H + 50;
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
		
void VSHMenu::control(Controller* pad){
	
	if (pad->down()){
		if (index > MAX_OPTIONS){
			if (index < MAX_OPTIONS+2){
				index++;
				common::playMenuSound();
			}
			else {
				index = MAX_OPTIONS+1;
				common::playMenuSound();
			}
		}
		else if (index < MAX_OPTIONS){
			index++;
			common::playMenuSound();
		}
		else {
			index = 0;
			common::playMenuSound();
		}
	}
	else if (pad->up()){
		if (index > MAX_OPTIONS){
			if (index > MAX_OPTIONS+1){
				index--;
				common::playMenuSound();
			}
			else {
				index = MAX_OPTIONS+2;
				common::playMenuSound();
			}
		}
		else if (index > 0){
			index--;
			common::playMenuSound();
		}
		else {
			index = MAX_OPTIONS;
			common::playMenuSound();
		}
	}
	
	else if (pad->accept()){
		GameManager* g = (GameManager*)(SystemMgr::getSystemEntry(0));
		Entry* e = g->getEntry();
		switch (index){
		case MAX_OPTIONS-1: common::playMenuSound(); state = PEOPS_MENU; peops_menu.control(); state = MAIN_MENU; break;
		case MAX_OPTIONS: index++; common::playMenuSound(); break;
		case MAX_OPTIONS+1: state = PLUGINS_MENU; plugins.control(NULL); state = MAIN_MENU; break;
		case MAX_OPTIONS+2: state = PLUGINS_MENU; plugins.control(e); state = MAIN_MENU; break;
		default: break;
		}
	}
	
	else if (pad->decline()){
		common::playMenuSound();
		if (index > MAX_OPTIONS){
			index = MAX_OPTIONS;
		}
	}
	else if (pad->right()){
		if (index < MAX_OPTIONS){
			if (vsh_entries[index]->selection < (unsigned char)(vsh_entries[index]->max_options-1))
				vsh_entries[index]->selection++;
			else
				vsh_entries[index]->selection = (unsigned char)0;
			common::playMenuSound();
			changed = true;
		}
	}
	else if (pad->left()){
		if (index < MAX_OPTIONS){
			if (vsh_entries[index]->selection > (unsigned char)0)
				vsh_entries[index]->selection--;
			else
				vsh_entries[index]->selection = (unsigned char)(vsh_entries[index]->max_options-1);
			common::playMenuSound();
			changed = true;
		}
	}
}

void VSHMenu::pause(){
	for (int i=0; i<MAX_OPTIONS; i++)
		*(vsh_entries[i]->config_ptr) = vsh_entries[i]->selection;
	
	if (changed)
		common::saveConf();
	
	animation = 1;
	
	while (animation != -2)
		sceKernelDelayThread(0);
}

void VSHMenu::resume(){
	changed = false;
	animation = -1;

	for (int i=0; i<MAX_OPTIONS; i++)
		vsh_entries[i]->selection = *(vsh_entries[i]->config_ptr);
	
	while (animation != 0)
		sceKernelDelayThread(0);
}

PluginsManager* VSHMenu::getPluginsManager(){
	return &(this->plugins);
}
