#ifndef EXITMGR_H
#define EXITMGR_H

#include "system_entry.h"
#include "common.h"

class ExitManager : public SystemEntry{
    void draw();
    void control(Controller* pad);
    void pause();
    void resume();
    std::string getInfo();
    void setInfo(std::string info);
    std::string getName();
    void setName(std::string name);
    Image* getIcon();
};

#endif

