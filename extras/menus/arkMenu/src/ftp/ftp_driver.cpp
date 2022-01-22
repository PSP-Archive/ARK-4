#include <cstring>
#include "ftp_driver.h"
#include "osk.h"
#include "network.h"
#include "browser_entries.h"
#include "ftpclient.h"

class FTPFile : public BrowserFile{
    protected:
        string st_size;
        unsigned getFileSize(){
            return (unsigned)atoi(this->st_size.c_str());
        }
    public:
        FTPFile(string path, string name, string size){
            this->path = path;
            this->name = name;
            this->st_size = size;
            this->selected = false;
            this->calcSize();
        }
        ~FTPFile(){}
};

FTPDriver::FTPDriver(){
    connected = false;
}

FTPDriver::~FTPDriver(){

}

bool FTPDriver::isDevicePath(string path){
    string dev_path = this->getDevicePath();
    string sub_path = path.substr(0, dev_path.size());
    return (dev_path == sub_path);
}

string FTPDriver::getDevicePath(){
    return "ftp:/";
}

bool FTPDriver::connect(){
    if (connected) return true;
    printf("Initialize network\n");
    if (initializeNetwork() < 0) return false;
    printf("connect to access point\n");
    if (connect_to_apctl() < 0) return false;

    bool ret = false;
    char tmpText[51];
    OSK osk;
    osk.init("Hostname or IP address of FTP Server", "", 50);
    osk.loop();
    if(osk.getResult() != OSK_CANCEL)
    {
        osk.getText((char*)tmpText);
        ret = true;
    }
    osk.end();
    
    if (ret){
        ftpInit();
        printf("FTP Connecting to server\n");
        if (!ftpConnect(tmpText, 21)){
            shutdownNetwork();
            return false;
        }
        /*
        printf("FTP log in\n");
        if (ftpLogin("anonymous", "") < 0){
            shutdownNetwork();
            return false;
        }
        */
        connected = true;
        printf("FTP connected\n");
    }
    else{
        printf("shutdown network\n");
        shutdownNetwork();
    }
    return ret;
}

void FTPDriver::disconnect(){
    ftpDisconnect();
    shutdownNetwork();
    ftpClean();
    connected = false;
}

vector<Entry*> FTPDriver::listDirectory(string path){
    printf("FTP::Path -> %s\n", path.c_str());
    string ftp_path = path.substr(getDevicePath().size(), path.size());
    if (ftp_path.size() == 0) ftp_path = "/";
    if (ftp_path[0] != '/') ftp_path = string("/") + ftp_path;
    printf("FTP::CWD -> %s\n", ftp_path.c_str());
    ftpCWD((char*)ftp_path.c_str());
    printf("FTP::List\n");
    remoteDirent* dir = ftpLIST();
    vector<BrowserFile*> files;
    vector<BrowserFolder*> folders;
    vector<Entry*> ret;
    
    if (dir != NULL){
        printf("FTP::Entries -> %d\n", dir->totalCount);    
        for (int i=0; i<dir->totalCount; i++){
            if (dir->files[i].d_name[0] == 0) break;
            printf("Processing entry: %s\n", dir->files[i].d_name);
            printf("Is Dir: %d\n", FIO_SO_ISDIR(dir->files[i].st_attr));
            int code; char file_name[256];
            if (sscanf(dir->files[i].d_name, "%d%s", &code, file_name) == 2){
                if (FIO_SO_ISDIR(dir->files[i].st_attr)) {
			        folders.push_back(new BrowserFolder(path + file_name + "/"));
		        } else if (FIO_SO_ISREG(dir->files[i].st_attr)) {
		            files.push_back(new FTPFile(path, file_name, dir->files[i].st_size));
		        }
		    }
        }
    }
    
    ret.push_back(new BrowserFolder("ftp:/<disconnect>"));
    
    printf("FTP::Folders -> %d\n", folders.size());
    for (int i=0; i<folders.size(); i++){
        ret.push_back(folders[i]);
    }
    printf("FTP::Files -> %d\n", files.size());
    for (int i=0; i<files.size(); i++){
        ret.push_back(files[i]);
    }
    
    return ret;
}

void FTPDriver::deleteFile(string path){
    string ftp_path = path.substr(this->getDevicePath().size(), path.size());
    ftpDELE((char*)ftp_path.c_str());
}

void FTPDriver::deleteFolder(string path){
    string ftp_path = path.substr(this->getDevicePath().size(), path.size());
    ftpRMD((char*)ftp_path.c_str());
}

void FTPDriver::createFolder(string path){
    string ftp_path = path.substr(this->getDevicePath().size(), path.size());
    ftpMKD((char*)ftp_path.c_str());
}

void FTPDriver::copyFileTo(string orig, string dest, int* progress){
    string ftp_path = dest.substr(this->getDevicePath().size(), dest.size());
    int res = ftpSTOR((char*)orig.c_str(), (char*)ftp_path.c_str());	// uploads a file to FTP server
}

void FTPDriver::copyFileFrom(string orig, string dest, int* progress){
    string ftp_path = orig.substr(this->getDevicePath().size(), orig.size());
    int res = ftpRETR((char*)ftp_path.c_str(), (char*)dest.c_str()); // downloads a file from FTP server
}
