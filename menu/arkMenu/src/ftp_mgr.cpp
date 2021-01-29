#include "ftp_mgr.h"
#include "ftpd.h"

#define MAX_LINES 10

static struct {
    char* msg[MAX_LINES];
    unsigned char cont;
} vla;

static void addMessage(char* msg){
    return;
    if (msg==NULL)
        return;
    if (vla.cont >= MAX_LINES){
        for (int i=1; i<MAX_LINES; i++)
            vla.msg[i-1] = vla.msg[i];
        vla.cont--;
    }
    vla.msg[vla.cont++] = msg;
}

void FTPManager::draw(){
    return;
    addMessage(mftpGetLastStatusMessage());
    
    int y = 50;
    
    for (int i=0; i<MAX_LINES; i++){
        common::printText(30, y, vla.msg[i]);
        y+=20;
    }
}

void FTPManager::control(Controller* pad){
}

void FTPManager::pause(){
    //ftpdTerm();
}

void FTPManager::resume(){
    //ftpdInit();
}

char* FTPManager::getInfo(){
    return (char*)"FTP Server (Under Construction)";
}

char* FTPManager::getName(){
    return "FTP";
}
    
Image* FTPManager::getIcon(){
    return common::getImage(IMAGE_FTP);
}
