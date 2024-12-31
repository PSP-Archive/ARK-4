#ifndef FTP_H
#define FTP_H

#include "system_entry.h"
#include "common.h"

class NetworkManager : public SystemEntry{
    int animation;
    void draw();
    void control(Controller* pad);
    void pause();
    void resume();
    std::string getInfo();
    void setInfo(std::string info);
    void setFooter(std::string footer);
    std::string getFooter();
    std::string getName();
    void setName(std::string name);
    Image* getIcon();
    bool isStillLoading(){ return false; }
};

#endif

