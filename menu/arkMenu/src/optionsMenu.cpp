#include "optionsMenu.h"
#include "controller.h"

OptionsMenu::OptionsMenu(char* description, int n_options, t_options_entry* entries){
	this->description = description;
	this->n_options = n_options;
	this->entries = entries;
	this->index = 0;
	this->w = 10*this->maxString();
	this->h = 30 + 15*n_options;
	this->x = (480-w)/2;
	this->y = (272-h)/2;
}

OptionsMenu::~OptionsMenu(){
}

int OptionsMenu::maxString(){
	int max = strlen(description);
	for (int i=0; i<n_options; i++){
		int len = strlen(entries[i].name);
		if (len > max)
			max = len;
	}
	return max;
}

void OptionsMenu::draw(){
	common::getImage(IMAGE_DIALOG)->draw_scale(x, y, w, h);
	
	int xoffset = x+10;
	int yoffset = y+15;
	
	common::printText(xoffset+10, yoffset, description);
	
	for (int i=0; i<n_options; i++){
		yoffset+=15;
		common::printText(xoffset, yoffset, entries[i].name, LITEGRAY, SIZE_LITTLE, i==index);
	}
}
		
int OptionsMenu::control(){
	Controller pad;
	
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
		else if (pad.cross()){
			common::playMenuSound();
			ret = true;
			break;
		}
		else if (pad.circle()){
			common::playMenuSound();
			ret = false;
			break;
		}
	}
	return (ret)? entries[index].value : OPTIONS_CANCELLED;
}
