#include "peops_menu.h"
#include "common.h"
#include "controller.h"

#define PEOPS_ENTRIES 10
#define MENU_W 300
#define MENU_H 20*PEOPS_ENTRIES

struct PeopsEntry{
	char* name;
	unsigned char n_opts;
	unsigned char index;
	char* ops[];
};

static struct{
	char* name;
	unsigned char n_opts;
	unsigned char index;
	char* ops[2];
} enable_custom_config = {
	"PEOPS Configuration",
	2,
	0,
	{"Automatic", "Manual"}
};

static struct{
	char* name;
	unsigned char n_opts;
	unsigned char index;
	char* ops[2];
} enable_peops = {
	"Enable PEOPS SPU",
	2,
	1,
	{"Disable", "Enable"}
};

static struct{
	char* name;
	unsigned char n_opts;
	unsigned char index;
	char* ops[11];
} volume = {
	"Volume",
	11,
	3,
	{"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10"}
};

static struct{
	char* name;
	unsigned char n_opts;
	unsigned char index;
	char* ops[9];
} reverb = {
	"Reverb",
	9,
	1,
	{"Off", "Room", "Small Studio", "Medium Studio", "Large Studio", "Hall", "Space Echo", "Echo/Delay", "Half Echo"}
};

static struct{
	char* name;
	unsigned char n_opts;
	unsigned char index;
	char* ops[3];
} interpolation = {
	"Interpolation",
	3,
	2,
	{"None", "Simple", "Gauss"}
};

static struct{
	char* name;
	unsigned char n_opts;
	unsigned char index;
	char* ops[2];
} enablexa = {
	"Enable XA Playback",
	2,
	1,
	{"Disable", "Enable"}
};

static struct{
	char* name;
	unsigned char n_opts;
	unsigned char index;
	char* ops[2];
} xapitch = {
	"Change XA Pitch",
	2,
	1,
	{"No", "Yes"}
};

static struct{
	char* name;
	unsigned char n_opts;
	unsigned char index;
	char* ops[2];
} irqwait = {
	"SPU IRQ Wait",
	2,
	1,
	{"No", "Yes"}
};

static struct{
	char* name;
	unsigned char n_opts;
	unsigned char index;
	char* ops[3];
} spuupdate = {
	"SPU Update Mode",
	3,
	0,
	{"Wait for Vblank", "Delay for 2ms", "Delay for 20ms"}
};

static struct{
	char* name;
	unsigned char n_opts;
	unsigned char index;
	char* ops[5];
} threadpriority = {
	"Thread Priority",
	5,
	0,
	{"17", "24", "32", "40", "48"}
};

PeopsEntry* peops_entries[PEOPS_ENTRIES] = {
	(PeopsEntry*)&enable_custom_config,
	(PeopsEntry*)&enable_peops,
	(PeopsEntry*)&volume,
	(PeopsEntry*)&reverb,
	(PeopsEntry*)&interpolation,
	(PeopsEntry*)&enablexa,
	(PeopsEntry*)&xapitch,
	(PeopsEntry*)&irqwait,
	(PeopsEntry*)&spuupdate,
	(PeopsEntry*)&threadpriority
};

PeopsMenu::PeopsMenu(){
	this->index = 0;
}

PeopsMenu::~PeopsMenu(){

}

void PeopsMenu::draw(){
	int x = (480-MENU_W)/2;
	int y = (272-MENU_H)/2;
	int yoffset = y+30;
	int xoffset = x+10;
	int iterations = (peops_entries[0]->index==1)? ((peops_entries[1]->index==1)? PEOPS_ENTRIES:2):1;
	common::getImage(IMAGE_DIALOG)->draw_scale(x, y, MENU_W, MENU_H);
	for (int i=0; i<iterations; i++){
		char* opt = peops_entries[i]->ops[peops_entries[i]->index];
		common::printText(xoffset, yoffset, peops_entries[i]->name, GRAY_COLOR, SIZE_LITTLE, i==index);
		common::printText(xoffset+150, yoffset, opt, GRAY_COLOR, SIZE_LITTLE, i==index);
		yoffset += 15;
	}
}
	
void PeopsMenu::control(){
	Controller pad;
	bool changed = false;
	bool controlling = true;
	while (controlling){
		pad.update();
		int iterations = (peops_entries[0]->index==1)? ((peops_entries[1]->index==1)? PEOPS_ENTRIES:2):1;
		if (pad.down()){
			if (index < iterations-1){
				index++;
				common::playMenuSound();
			}
			continue;
		}
		else if (pad.up()){
			if (index > 0){
				index--;
				common::playMenuSound();
			}
			continue;
		}
		
		else if (pad.right()){
			if (peops_entries[index]->index < (unsigned char)(peops_entries[index]->n_opts-1))
				peops_entries[index]->index++;
			else
				peops_entries[index]->index = (unsigned char)0;
			common::playMenuSound();
			changed = true;
		}
		else if (pad.left()){
			if (peops_entries[index]->index > (unsigned char)0)
				peops_entries[index]->index--;
			else
				peops_entries[index]->index = (unsigned char)(peops_entries[index]->n_opts-1);
			common::playMenuSound();
			changed = true;
		}
		else if (pad.circle()){
			common::playMenuSound();
			controlling = false;
		}
	}
	if (changed)
		processPeopsConf();
	
	sceKernelDelayThread(100000);
}

void PeopsMenu::processPeopsConf(){

}
