#include "controller.h"

Controller::Controller(){
	this->pad = new SceCtrlData;
}

Controller::~Controller(){
	delete this->pad;
}
		
void Controller::update(){
	sceKernelDelayThread(100000);
	sceCtrlReadBufferPositive(this->pad, 1);
}
		
bool Controller::up(){
	return (this->pad->Buttons & PSP_CTRL_UP);
}

bool Controller::down(){
	return (this->pad->Buttons & PSP_CTRL_DOWN);
}

bool Controller::left(){
	return (this->pad->Buttons & PSP_CTRL_LEFT);
}

bool Controller::right(){
	return (this->pad->Buttons & PSP_CTRL_RIGHT);
}

bool Controller::cross(){
	return (this->pad->Buttons & PSP_CTRL_CROSS);
}

bool Controller::circle(){
	return (this->pad->Buttons & PSP_CTRL_CIRCLE);
}

bool Controller::square(){
	return (this->pad->Buttons & PSP_CTRL_SQUARE);
}

bool Controller::triangle(){
	return (this->pad->Buttons & PSP_CTRL_TRIANGLE);
}

bool Controller::RT(){
	return (this->pad->Buttons & PSP_CTRL_RTRIGGER);
}

bool Controller::LT(){
	return (this->pad->Buttons & PSP_CTRL_LTRIGGER);
}

bool Controller::start(){
	return (this->pad->Buttons & PSP_CTRL_START);
}

bool Controller::select(){
	return (this->pad->Buttons & PSP_CTRL_SELECT);
}
