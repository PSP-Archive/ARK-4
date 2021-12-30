#include "ftp_mgr.h"
#include "network.h"
#include "system_mgr.h"

#define MAX_LINES 10

//extern "C"{
#include "ftpd.h"
extern void setFtpMsgHandler(void*);
extern int mftpExitHandler(SceSize argc, void *argv);
//}

static struct {
    string msg[MAX_LINES];
    unsigned char cont;
} vla;

SceUID ftp_thread = -1;

static void addMessage(const char* msg){
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
    
    common::getImage(IMAGE_DIALOG)->draw_scale(20, 30, 400, 230);
    
    for (int i=0; i<MAX_LINES; i++){
        common::printText(30, y, vla.msg[i].c_str());
        y+=20;
    }
}

void FTPManager::control(Controller* pad){

    if (pad->decline() && ftp_thread>=0){
        addMessage("Disconnecting FTP server");
        mftpExitHandler(0, NULL);
        sceKernelWaitThreadEnd(ftp_thread, NULL);
        sceKernelTerminateDeleteThread(ftp_thread);
        ftp_thread = -1;
        shutdownNetwork();
        addMessage("FTP Disconnected");
    }

}

void FTPManager::pause(){

}

void FTPManager::resume(){

    if (ftp_thread >= 0) return;

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
    
    if (ftp_thread < 0){
        addMessage((common::getConf()->swap_buttons)? "Press X to shutdown FTP !" : "Press () to shutdown FTP !");
        ftp_thread = sceKernelCreateThread("ftpd_main_thread", ftpdLoop, 0x18, 0x10000, 0, 0);
        sceKernelStartThread(ftp_thread, 0, 0);
    }
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

