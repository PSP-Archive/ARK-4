#ifndef SYSTEM_ENTRY_H
#define SYSTEM_ENTRY_H

#include <string>
#include "controller.h"
#include "gfx.h"
#include "common.h"

class SystemEntry{
    public:
        virtual void draw()=0;
        virtual void control(Controller* pad)=0;
        virtual void pause()=0;
        virtual void resume()=0;
        virtual std::string getInfo()=0;
        virtual void setInfo(std::string info)=0;
        virtual Image* getIcon()=0;
        virtual void setName(std::string name)=0;
        virtual std::string getName()=0;
        virtual bool isStillLoading()=0;
        virtual void drawInfo(){
            common::printText(5, 13, this->getInfo().c_str(), LITEGRAY, SIZE_MEDIUM, 0, 0);
        }
};

#endif
