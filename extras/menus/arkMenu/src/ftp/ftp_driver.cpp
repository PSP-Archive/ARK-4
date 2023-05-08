#include <cstring>
#include <algorithm>
#include "ftp_driver.h"
#include "osk.h"
#include "network.h"
#include "browser_entries.h"
#include "ftpclient.h"

static char* ANONYMOUS = "anonymous";

class FTPFile : public BrowserFile{
    protected:
        string st_size;
        unsigned getFileSize(){
            return (unsigned)atoi(this->st_size.c_str());
        }
    public:
        FTPFile(string path, string name, string size){
            this->path = path+name;
            this->name = name;
            this->st_size = size;
            this->selected = false;
            this->calcSize();
        }
        ~FTPFile(){}
};

FTPDriver::FTPDriver(){
    connected = false;
    root = "";
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
    osk.init("Enter address of FTP Server", "", 50);
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

        int port = 21;
        char* p;
        if ((p=strstr(tmpText, "/")) != NULL){
            this->root = string(p);
            p[0] = 0;
        }
        if ((p=strstr(tmpText, ":")) != NULL){
            port = atoi(p+1);
            p[0] = 0;
        }
        char* host = resolveHostAddress(tmpText);

        if (!ftpConnect(host, port)){
            ftpClean();
            shutdownNetwork();
            return false;
        }

        char* user = ANONYMOUS;
        char password[51];
        password[0] = 0;

        osk.init("Enter username (cancel for anonymous)", "", 50);
        osk.loop();
        if(osk.getResult() != OSK_CANCEL)
        {
            osk.getText((char*)tmpText);
            user = tmpText;
        }
        osk.end();

        if (user != ANONYMOUS){
            osk.init("Enter password", "", 50);
            osk.loop();
            if(osk.getResult() != OSK_CANCEL)
            {
                osk.getText((char*)password);
                user = tmpText;
            }
            osk.end();
        }

        printf("FTP log in\n");
        if (ftpLogin(user, password) < 0){
            if (user != ANONYMOUS){
                ftpClean();
                shutdownNetwork();
                return false;
            }
        }
        
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
    ftp_path = this->root + ftp_path;
    printf("FTP::CWD -> %s\n", ftp_path.c_str());
    ftpCWD((char*)ftp_path.c_str());
    printf("FTP::List\n");
    remoteDirent* dir = ftpLIST();
    vector<BrowserFile*> files;
    vector<BrowserFolder*> folders;
    vector<Entry*> ret;
    files.clear();
    folders.clear();
    ret.clear();
    
    if (dir != NULL){
        printf("FTP::Entries -> %d\n", dir->totalCount);    
        for (int i=0; i<dir->totalCount; i++){
            if (dir->files[i].d_name[0] == 0) break;
            printf("Processing entry: %s\n", dir->files[i].d_name);
            printf("Is Dir: %d\n", FIO_SO_ISDIR(dir->files[i].st_attr));
            int code;
            if (sscanf(dir->files[i].d_name, "%d ", &code) == 1){
                string file_name(dir->files[i].d_name);
                file_name = file_name.substr(file_name.find(' ')+1);
                if (FIO_SO_ISDIR(dir->files[i].st_attr)) {
                    BrowserFolder* folder = new BrowserFolder(path + file_name + "/");
                    printf("adding folder: %s -> %s\n", file_name.c_str(), folder->getName().c_str());
                    folders.push_back(folder);
                } else if (FIO_SO_ISREG(dir->files[i].st_attr)) {
                    BrowserFile* file = new FTPFile(path, file_name, dir->files[i].st_size);
                    printf("adding file: %s -> %s\n", file_name.c_str(), file->getName().c_str());
                    files.push_back(file);
                }
            }
        }
    }
    
    if (common::getConf()->sort_entries){

        BrowserFolder* dot = NULL;
        BrowserFolder* dotdot = NULL;
        if (folders[0]->getName() == "./"){
            dot = folders[0];
            folders.erase(folders.begin());
        }
        if (folders[0]->getName() == "../"){
            dotdot = folders[0];
            folders.erase(folders.begin());
        }

        std::sort(folders.begin(), folders.end(), Entry::cmpEntriesForSort);
        std::sort(files.begin(), files.end(), Entry::cmpEntriesForSort);

        if (dotdot) folders.insert(folders.begin(), dotdot);
        if (dot) folders.insert(folders.begin(), dot);
    }

    ret.push_back(new BrowserFolder("ftp:/<refresh>"));
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
    if (this->isDevicePath(path)){
        path = path.substr(this->getDevicePath().size()-1, path.size());
    }
    ftpDELE((char*)path.c_str());
}

void FTPDriver::deleteFolder(string path){
    if (this->isDevicePath(path)){
        path = path.substr(this->getDevicePath().size()-1, path.size());
    }
    ftpRMD((char*)path.c_str());
}

void FTPDriver::createFolder(string path){
    if (this->isDevicePath(path)){
        path = path.substr(this->getDevicePath().size()-1, path.size());
    }
    ftpMKD((char*)path.c_str());
}

void FTPDriver::createFile(string path){
    if (this->isDevicePath(path)){
        path = path.substr(this->getDevicePath().size()-1, path.size());
    }
    ftpAPPE((char*)path.c_str());
}

void FTPDriver::copyFileTo(string orig, string dest, int* progress){
    //string ftp_path = dest.substr(this->getDevicePath().size(), dest.size());
    size_t lastSlash = orig.rfind("/", string::npos);
    //int res = ftpSTOR((char*)orig.c_str(), (char*)ftp_path.c_str());    // uploads a file to FTP server
    string localdir = orig.substr(0, lastSlash+1);
    string filename = orig.substr(lastSlash+1);
    if (isDevicePath(dest)){
        string remotedir = dest.substr(this->getDevicePath().size()-1, dest.size());
        printf("CWD %s\n", remotedir.c_str());
        ftpCWD((char*)remotedir.c_str());
    }
    printf("localdir: %s\n", localdir.c_str());
    printf("filename: %s\n", filename.c_str());
    int res = ftpSTOR((char*)localdir.c_str(), (char*)filename.c_str());
}

void FTPDriver::copyFileFrom(string orig, string dest, int* progress){
    string ftp_path = orig;
    if (isDevicePath(orig)){
        size_t lastSlash = orig.rfind("/", string::npos);
        ftp_path = orig.substr(lastSlash+1); //orig.substr(this->getDevicePath().size(), orig.size());
        string remotedir = orig.substr(0, lastSlash);
        remotedir = remotedir.substr(this->getDevicePath().size()-1);
        printf("CWD %s\n", remotedir.c_str());
        ftpCWD((char*)remotedir.c_str());
    }
    int res = ftpRETR((char*)dest.c_str(), (char*)ftp_path.c_str()); // downloads a file from FTP server
}
