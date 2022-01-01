#include "exitmgr.h"
#include "system_mgr.h"

void ExitManager::draw(){

    common::getImage(IMAGE_DIALOG)->draw_scale(20, 30, 450, 235);
    
    char buffer[128];
    snprintf(buffer, 128, "Press %s to resume game.", (common::getConf()->swap_buttons)? "()" : "X");
    common::printText(30, 50, buffer, GRAY_COLOR, SIZE_BIG);

    snprintf(buffer, 128, "Press %s to exit game.", (common::getConf()->swap_buttons)? "X" : "()");
    common::printText(30, 70, buffer, GRAY_COLOR, SIZE_BIG);
    
}

void ExitManager::control(Controller* pad){

    if (pad->accept()){
    }
    else if (pad->decline()){
        sceKernelExitGame();
    }

}

void ExitManager::pause(){

}

void ExitManager::resume(){

}

std::string ExitManager::getInfo(){
    return "Current Game";
}

std::string ExitManager::getName(){
    return "EXIT";
}

void ExitManager::setInfo(std::string info){
    
}

void ExitManager::setName(std::string name){

}

Image* ExitManager::getIcon(){
    return common::getImage(IMAGE_SETTINGS);
}

