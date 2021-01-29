#ifndef FTP_H
#define FTP_H

#include "system_entry.h"
#include "common.h"

class FTPManager : public SystemEntry{
    void draw();
    void control(Controller* pad);
    void pause();
    void resume();
    char* getInfo();
    char* getName();
    Image* getIcon();
};

#endif
