#include <sstream>
#include <fstream>

#include "net_mgr.h"
#include "network.h"
#include "system_mgr.h"
#include <pspnet_apctl.h>
#include <psphttp.h>
#include "eboot.h"

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

void NetworkManager::draw(){

    static int x, y, w, h;

    switch (animation){
    case -1:
        if (w < 400 || h < 200){
            
            w += 50;
            if (w > 400)
                w = 400;
            
            h += 30;
            if (h > 200)
                h = 200;

            x = (480-w)/2;
            y = (272-h)/2;

            common::getImage(IMAGE_DIALOG)->draw_scale(x, y, w, h);
        }
        else {
            animation = 0;
            common::getImage(IMAGE_DIALOG)->draw_scale(x, y, w, h);
        }
        break;
    case 0:
        common::getImage(IMAGE_DIALOG)->draw_scale(20, 30, 450, 235);
        
        char buffer[128];
        
        if (ftp_thread>=0){
            snprintf(buffer, 128, "FTP Server is running @ %s. Press %s to stop.", pspIpAddr, (common::getConf()->swap_buttons)? "X" : "()");
        }
        else{
            snprintf(buffer, 128, "FTP Server is stopped. Press %s to start.", (common::getConf()->swap_buttons)? "()" : "X");
        }
        common::printText(30, 50, buffer, GRAY_COLOR, SIZE_BIG, 1);
        common::printText(30, 70, "Press [] to check for Updates", GRAY_COLOR, SIZE_BIG);
        
        y = 90;
        
        for (int i=0; i<MAX_LINES; i++){
            common::printText(30, y, vla.msg[i].c_str());
            y+=20;
        }
        break;
    case 1:
        if (w > 0 || h > 0){
        
            w -= 50;
            if (w < 0)
                w = 0;
            
            h -= 30;
            if (h < 0)
                h = 0;
            
            x = (480-w)/2;
            y = (272-h)/2;
            
            common::getImage(IMAGE_DIALOG)->draw_scale(x, y, w, h);
        }
        else {
            animation = -2;
        }
        break;
    default: break;
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

static string parsePspUpdateList(u32* update_ver){

    std::ifstream input("psp-updatelist.txt");

    for( std::string line; getline( input, line ); ){
        // scan for all the tokens
        size_t image_version = line.find("ImageVersion=");
        size_t cdn = line.find("CDN=");
        size_t eboot_pbp = line.find("EBOOT.PBP");
        if (image_version != string::npos && cdn != string::npos && eboot_pbp != string::npos){
            // grab version string
            string ver = line.substr(image_version+13, 8);
            // convert from hex to u32
            *update_ver = strtol(ver.c_str(), NULL, 16);
            // grab eboot url
            string eboot_url = line.substr(cdn+4);
            // trim URL
            char* eboot_path = (char*)eboot_url.c_str();
            char* eboot_pbp = strstr(eboot_path, "EBOOT.PBP");
            eboot_pbp[9] = 0; // C++ strings are inmutable? not round here they ain't
            return string(eboot_path);
        }
    }

    *update_ver = 0;
    return "";
}

static string getServerUrl(){
    // read first line of server file
    std::ifstream input("UPDATER.TXT");
    string line = ""; getline(input, line);
    // trim text
    if (line.size() == 0) return "";
    if (line[line.size()-1] == '\n') line = line.substr(0, line.size()-1);
    if (line[line.size()-1] == '\r') line = line.substr(0, line.size()-1);
    // append psp-updatelist.txt
    if (line[line.size()-1] != '/') line += "/";
    line += "psp-updatelist.txt";
    return line;
}

static void checkUpdates(){
    char* err = NULL;
    int ret;
    string path = getServerUrl();
    char* update_folder = "ms0:/PSP/GAME/UPDATE";
    char* update_eboot = "ms0:/PSP/GAME/UPDATE/EBOOT.PBP";
    string updater_url;
    u32 update_ver;
    bool do_update = false;
    char buf[128];

    if (common::getPspModel() == PSP_GO){
        update_folder[0] = update_eboot[0] = 'e';
        update_folder[1] = update_eboot[1] = 'f';
    }
    else if (common::getArkConfig()->exec_mode == PS_VITA){
        // redirect to ms0:/PSP/APPS/UPDATE/VBOOT.PBP
        update_eboot[21] = 'V';
        update_folder[9] = update_eboot[9] = 'A';
        update_folder[10] = update_eboot[10] = 'P';
        update_folder[11] = update_eboot[11] = 'P';
        update_folder[12] = update_eboot[12] = 'S';
        update_folder[13] = 0;
        sceIoMkdir(update_folder, 0777);
        update_folder[13] = '/';
    }

    SystemMgr::pauseDraw();
    if ((ret=initializeNetwork()) >= 0){
        if ((ret=connect_to_apctl()) >= 0){
            SystemMgr::resumeDraw();
            if (ftp_thread>=0) stopFTP();

            addMessage("Checking for updates");

            sceUtilityLoadModule(PSP_MODULE_NET_PARSEURI);
            sceUtilityLoadModule(PSP_MODULE_NET_PARSEHTTP);
            sceUtilityLoadModule(PSP_MODULE_NET_HTTP);

            int templateid=-1, connectionid=-1, requestid=-1, fd=-1;

            if ((ret=sceHttpInit(20000)) < 0){
                err = "Could not initialize HTTP library";
                goto update_end;
            }

            addMessage("Downloading psp-updatelist.txt");

            wget((char*)path.c_str(), "psp-updatelist.txt");

            updater_url = parsePspUpdateList(&update_ver);

            snprintf(buf, 128, "Got version %p @ %s", update_ver, updater_url.c_str());
            addMessage(buf);

            sceIoRemove("psp-updatelist.txt");

            do_update = common::getConf()->force_update || sctrlHENGetMinorVersion() < update_ver;

            if (!do_update){
                addMessage("No need to update!");
            }
            else{
                addMessage("Downloading updater");
                sceIoMkdir(update_folder, 0777);
                wget((char*)updater_url.c_str(), update_eboot);
            }

            update_end:
            sceHttpEnd();
            sceUtilityUnloadModule(PSP_MODULE_NET_HTTP);
            sceUtilityUnloadModule(PSP_MODULE_NET_PARSEHTTP);
            sceUtilityUnloadModule(PSP_MODULE_NET_PARSEURI);
            shutdownNetwork();

            if (do_update){
                addMessage("Running Updater");
                Eboot* eboot = new Eboot(update_eboot);
                eboot->execute();
            }
        }
        else{
            err = "Could not connect to Access Point";
            SystemMgr::resumeDraw();
        }
    }
    else{
        err = "Could not initialize network";
        SystemMgr::resumeDraw();
    }

    if (err){
        snprintf(buf, 128, "%s: 0x%X", err, ret);
        addMessage(buf);
    }
}

void NetworkManager::control(Controller* pad){

    if (pad->accept() && ftp_thread < 0){
        startFTP();
    }
    else if (pad->decline() && ftp_thread>=0){
        stopFTP();
    }
    else if (pad->square()){
        checkUpdates();
    }

}

void NetworkManager::pause(){
    animation = 1;
    while (animation != -2)
        sceKernelDelayThread(0);
}

void NetworkManager::resume(){
    animation = -1;
    while (animation != 0)
        sceKernelDelayThread(0);
}

std::string NetworkManager::getInfo(){
    return "Network Tools";
}

std::string NetworkManager::getName(){
    return "Network";
}

void NetworkManager::setInfo(std::string info){
    
}

void NetworkManager::setName(std::string name){

}

Image* NetworkManager::getIcon(){
    return common::getImage(IMAGE_FTP);
}

