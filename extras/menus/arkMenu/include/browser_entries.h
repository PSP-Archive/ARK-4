#ifndef BROWSER_ENTRIES_H
#define BROWSER_ENTRIES_H

#include "browser.h"

class BrowserFile : public Entry{

    protected:
        bool selected;
        string fileSize;
        int filetype;
        string parent;

        virtual unsigned getFileSize();

    public:
    
        BrowserFile();
    
        BrowserFile(string path);
        BrowserFile(string parent, string name);
        
        ~BrowserFile();
        
        void calcSize();
        
        bool isSelected();
        
        void changeSelection();
        
        string getPath();
        
        string getName();
        
        string getSize();
        
        char* getType();
        
        char* getSubtype();
        
        void loadIcon();

        void freeIcon();
        
        void loadPics();
        
        void loadAVMedia();
        
        void doExecute();

        int getFileType() { return filetype; }

        string getFullPath() { return path; }
};

class BrowserFolder : public BrowserFile{
    public:
        BrowserFolder(string path);
        BrowserFolder(string parent, string name);
        
        ~BrowserFolder();

        string getName();
        
        char* getType();
        
        char* getSubtype();

        void loadIcon();
};

#endif
