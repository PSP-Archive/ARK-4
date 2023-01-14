#ifndef EXIT_MGR_H
#define EXIT_MGR_H

#include "system_entry.h"

class ExitManager : public SystemEntry{

    public:
        void draw(){};
        void control(Controller* pad){};
        void pause(){};
        void resume(){sceKernelExitGame();};
        std::string getInfo(){return "Exit";};
        void setInfo(std::string info){};
        Image* getIcon(){return common::getImage(IMAGE_EXIT);};
        void setName(std::string name){};
        std::string getName(){return "Exit";};
        bool isStillLoading(){ return false; };
};

#endif
