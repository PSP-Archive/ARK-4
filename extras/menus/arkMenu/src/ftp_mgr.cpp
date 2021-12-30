#include "ftp_mgr.h"
#include "network.h"
#include "system_mgr.h"

#define MAX_LINES 10

extern "C"{
#include "utils.h"
#include "ftpsp.h"
extern void setFtpMsgHandler(void*);
}

static struct {
    string msg[MAX_LINES];
    unsigned char cont;
} vla;

static void addMessage(char* msg){
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

    int y = 50;
    
    common::getImage(IMAGE_DIALOG)->draw_scale(50, 20, 400, 230);
    
    for (int i=0; i<MAX_LINES; i++){
        common::printText(30, y, vla.msg[i].c_str());
        y+=20;
    }
}

void FTPManager::control(Controller* pad){
}

void FTPManager::pause(){

    ftpsp_shutdown();
    
    deinit_net();
    unload_net_modules();

}

void FTPManager::resume(){

    //load_net_modules();
    //init_net();
    //connect_net(3);
    
    SystemMgr::pauseDraw();
    
    int ret;
    if ((ret=initializeNetwork()) < 0){
        char buf[128];
        snprintf(buf, 128, "Could not initialize network: 0x%X", ret);
        addMessage(buf);
        return;
    }
    
    if ((ret=connect_to_apctl()) < 0){
        char buf[128];
        snprintf(buf, 128, "Could not connect to Access Point: 0x%X", ret);
        addMessage(buf);
        return;
    }

    setFtpMsgHandler((void*)&addMessage);
    
    SystemMgr::resumeDraw();
    
    ftpsp_init();

}

std::string FTPManager::getInfo(){
    return "FTP Server";
}

std::string FTPManager::getName(){
    return "FTP";
}

void FTPManager::setInfo(std::string info){
    
}

void FTPManager::setName(std::string name){

}

Image* FTPManager::getIcon(){
    return common::getImage(IMAGE_FTP);
}

