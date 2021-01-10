#include "controller.h"
#include "menucommon.h"

Controller::Controller(){
	this->pad = new SceCtrlData;
	this->nowpad = this->newpad = this->oldpad = 0;
	this->n = 0;
}

Controller::~Controller(){
	delete this->pad;
}
		
void Controller::update(int delay){
	sceKernelDelayThread(delay);
	sceCtrlReadBufferPositive(this->pad, 1);
	
	nowpad = pad->Buttons;
	newpad = nowpad & ~oldpad;
	
	if (oldpad == nowpad){
		n++;
		if (n >= 14){
			newpad = nowpad;
		}
	}
	else {
		n = 0;
		oldpad = nowpad;
	}
}

void Controller::flush(){
	while (this->pad->Buttons)
		sceCtrlReadBufferPositive(this->pad, 1);
}

bool Controller::wait(){
	bool ret;
	while (true){
		this->update();
		if (this->cross()){
			ret = true;
			break;
		}
		else if (this->circle()){
			ret = false;
			break;
		}
	}
	return ret;
}

bool Controller::accept(){
	return this->cross(); //(common::getConf()->swap_buttons)? this->circle() : this->cross();
}

bool Controller::decline(){
	return this->circle(); //(common::getConf()->swap_buttons)? this->cross() : this->circle();
}
		
bool Controller::up(){
	return (newpad & PSP_CTRL_UP);
}

bool Controller::down(){
	return (newpad & PSP_CTRL_DOWN);
}

bool Controller::left(){
	return (newpad & PSP_CTRL_LEFT);
}

bool Controller::right(){
	return (newpad & PSP_CTRL_RIGHT);
}

bool Controller::cross(){
	return (newpad & PSP_CTRL_CROSS);
}

bool Controller::circle(){
	return (newpad & PSP_CTRL_CIRCLE);
}

bool Controller::square(){
	return (newpad & PSP_CTRL_SQUARE);
}

bool Controller::triangle(){
	return (newpad & PSP_CTRL_TRIANGLE);
}

bool Controller::RT(){
	return (newpad & PSP_CTRL_RTRIGGER);
}

bool Controller::LT(){
	return (newpad & PSP_CTRL_LTRIGGER);
}

bool Controller::start(){
	return (newpad & PSP_CTRL_START);
}

bool Controller::select(){
	return (newpad & PSP_CTRL_SELECT);
}
