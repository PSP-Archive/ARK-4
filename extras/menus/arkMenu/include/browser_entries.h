#ifndef BROWSER_ENTRIES_H
#define BROWSER_ENTRIES_H

#include "browser.h"

class BrowserFile : public Entry{

    protected:
        bool selected;
        string fileSize;
        int filetype;

        virtual unsigned getFileSize();

    public:
    
        BrowserFile();
    
        BrowserFile(string path);
        
        BrowserFile(BrowserFile* orig);
        
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
        
        void getTempData1();
        
        void getTempData2();
        
        void doExecute();

        int getFileType() { return filetype; }
};

class BrowserFolder : public BrowserFile{
    public:
        BrowserFolder(string path);
        
        BrowserFolder(BrowserFolder* orig);
        
        BrowserFolder(string parent, string name);
        
        ~BrowserFolder();
        
        char* getType();
        
        char* getSubtype();
};

#endif
