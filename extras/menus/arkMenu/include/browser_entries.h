#ifndef BROWSER_ENTRIES_H
#define BROWSER_ENTRIES_H

#include "browser.h"

class BrowserFile : public Entry{

    protected:
        bool selected;
        string fileSize;
        int filetype;
        string shortname;
        string parent;

        virtual unsigned getFileSize();

        void setShortName(string shortname);

    public:
    
        BrowserFile();
    
        BrowserFile(string path, string shortname);
        BrowserFile(string parent, string name, string shortname);
        
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
        BrowserFolder(string path, string shortname);
        BrowserFolder(string parent, string name, string shortname);
        
        ~BrowserFolder();
        
        char* getType();
        
        char* getSubtype();
};

#endif
