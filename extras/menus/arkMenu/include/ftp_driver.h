#ifndef FTP_DRIVER_H
#define FTP_DRIVER_H

#include "browser.h"

class FTPDriver : public BrowserDriver{
    private:
        bool connected;
        string root;
    public:
        FTPDriver();
        ~FTPDriver();
        virtual bool connect();
        virtual void disconnect();
        virtual bool isDevicePath(string path);
        virtual string getDevicePath();
        virtual vector<Entry*> listDirectory(string path);
        virtual void deleteFile(string path);
        virtual void deleteFolder(string path);
        virtual void createFolder(string path);
        virtual void createFile(string path);
        virtual void copyFileTo(string orig, string dest, int* progress);
        virtual void copyFileFrom(string orig, string dest, int* progress);
};

#endif