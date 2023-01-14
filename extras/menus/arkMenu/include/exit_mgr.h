#ifndef EXIT_MGR_H
#define EXIT_MGR_H

#include "system_entry.h"

extern "C"{
    void sctrlKernelExitVSH(void*);
}

class ExitManager : public SystemEntry{

    public:
        void draw(){};
        void control(Controller* pad){};
        void pause(){};
        void resume(){
            sctrlSESetUmdFile("");
          	sctrlSESetBootConfFileIndex(MODE_UMD);
            sctrlKernelExitVSH(NULL);
        };
        std::string getInfo(){return "Exit";};
        void setInfo(std::string info){};
        Image* getIcon(){return common::getImage(IMAGE_EXIT);};
        void setName(std::string name){};
        std::string getName(){return "Exit";};
        bool isStillLoading(){ return false; };
};

#endif
