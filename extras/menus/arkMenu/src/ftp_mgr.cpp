#include "ftp_mgr.h"
#include "network.h"
#include "system_mgr.h"
#include <pspnet_apctl.h>

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

static SceUID ftp_thread = -1;
static char pspIpAddr[32];

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

    common::getImage(IMAGE_DIALOG)->draw_scale(20, 30, 450, 235);
    
    char buffer[128];
    
    if (ftp_thread>=0){
        snprintf(buffer, 128, "FTP Server is running @ %s. Press %s to stop.", pspIpAddr, (common::getConf()->swap_buttons)? "X" : "()");
    }
    else{
        snprintf(buffer, 128, "FTP Server is stopped. Press %s to start.", (common::getConf()->swap_buttons)? "()" : "X");
    }
    common::printText(30, 50, buffer);
    
    int y = 70;
    
    for (int i=0; i<MAX_LINES; i++){
        common::printText(30, y, vla.msg[i].c_str());
        y+=20;
    }
}

static void startFTP(){
    if (ftp_thread >= 0) return;

    SystemMgr::pauseDraw();
    
    char* err = NULL;

    int ret;
    if ((ret=initializeNetwork()) >= 0){
        if ((ret=connect_to_apctl()) >= 0){
            ret = sceNetApctlGetInfo(8, (SceNetApctlInfo*)pspIpAddr);
            if (pspIpAddr[0] != '\0'){
                setFtpMsgHandler((void*)&addMessage);
                ftp_thread = sceKernelCreateThread("ftpd_main_thread", ftpdLoop, 0x18, 0x10000, 0, 0);
                sceKernelStartThread(ftp_thread, 0, 0);
            }
            else{
                err = "Could not get IP address";
            }
        }
        else{
            err = "Could not connect to Access Point";
        }
    }
    else{
        err = "Could not initialize network";
    }
    
    if (err){
        char buf[128];
        snprintf(buf, 128, "%s: 0x%X", err, ret);
        addMessage(buf);
    }
    
    SystemMgr::resumeDraw();
}

static void stopFTP(){
    addMessage("Disconnecting FTP server");
    mftpExitHandler(0, NULL);
    sceKernelWaitThreadEnd(ftp_thread, NULL);
    sceKernelTerminateDeleteThread(ftp_thread);
    shutdownNetwork();
    addMessage("FTP Disconnected");
    ftp_thread = -1;
    memset(pspIpAddr, 0, sizeof(pspIpAddr));
}

void FTPManager::control(Controller* pad){

    if (pad->accept() && ftp_thread < 0){
        startFTP();
    }
    else if (pad->decline() && ftp_thread>=0){
        stopFTP();
    }

}

void FTPManager::pause(){

}

void FTPManager::resume(){

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

