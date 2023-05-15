#include "controller.h"
#include "common.h"

#define CONTROL_DELAY 10

Controller::Controller(){
    this->nowpad = this->newpad = this->oldpad = 0;
    this->n = 0;
}

Controller::~Controller(){
}
        
void Controller::update(int ignore){

    for (int i=0; i<ignore; i++)
        sceCtrlReadBufferPositive(&pad, 1);
    
    nowpad = pad.Buttons;
    newpad = nowpad & ~oldpad;
    
    if (oldpad == nowpad){
        n++;
        if (n >= CONTROL_DELAY){
            newpad = nowpad;
        }
    }
    else {
        n = 0;
        oldpad = nowpad;
    }
}

void Controller::flush(){
    while (pad.Buttons)
        sceCtrlReadBufferPositive(&pad, 1);
}

bool Controller::wait(void* busy_wait){
    bool ret;
    while (true){
        if (busy_wait != NULL){
            void (*exec)(void) = (void (*)())busy_wait;
            exec();
        }
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

bool Controller::any(){
    return (newpad!=0);
}

bool Controller::accept(){
    return (common::getConf()->swap_buttons)? this->circle() : this->cross();
}

bool Controller::decline(){
    return (common::getConf()->swap_buttons)? this->cross() : this->circle();
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

bool Controller::home(){
    return (newpad & PSP_CTRL_HOME);
}