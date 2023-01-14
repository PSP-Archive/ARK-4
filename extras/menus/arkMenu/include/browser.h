#ifndef BROWSER_H
#define BROWSER_H

#include <vector>
#include "entry.h"
#include "common.h"
#include "gfx.h"
#include "optionsMenu.h"
#include "system_entry.h"

using namespace std;

enum{
    NO_MODE,
    COPY,
    CUT,
    PASTE,
    DELETE,
    RENAME,
    MKDIR,
    MS0_DIR,
    EF0_DIR,
    FTP_DIR,
};

class BrowserDriver{
    public:
        virtual bool connect() = 0;
        virtual void disconnect() = 0;
        virtual bool isDevicePath(string path) = 0;
        virtual string getDevicePath() = 0;
        virtual vector<Entry*> listDirectory(string path) = 0;
        virtual void deleteFile(string path) = 0;
        virtual void deleteFolder(string path) = 0;
        virtual void createFolder(string path) = 0;
        virtual void copyFileTo(string orig, string dest, int* progress) = 0;
        virtual void copyFileFrom(string orig, string dest, int* progress) = 0;
};

class Browser : public SystemEntry{

    public:
        Browser();
        ~Browser();
        
        void draw();
        
        void control(Controller* pad);
        
        void pause(){
            animation = 1;
            while (animation != -2)
                sceKernelDelayThread(0);
        }
        void resume(){
            animation = -1;
            while (animation != 0)
                sceKernelDelayThread(0);
        }
        
        void setInfo(string info){};
        void setName(string name){};
        
        string getInfo(){
            return this->cwd;
        }
        
        string getName(){
            return "Files";
        }
        
        Image* getIcon(){
            return common::getImage(IMAGE_BROWSER);
        }

        bool isStillLoading(){ return false; }
        
        static void recursiveFolderDelete(string path);
        
        static BrowserDriver* ftp_driver;
        
        static const char* getCWD();
        
    private:
    
        string cwd; // Current Working Directory
        
        vector<Entry*>* entries; // entries in the current directory
        
        vector<string>* clipboard; // currently selected items
        
        int pasteMode; // COPY or CUT
        
        /* menu control variables */
        int index;  // index of currently selected item
        int start; // where to start drawing the menu
        bool animating; // animate the menu transition?
        unsigned int moving;
        int animation;
        
        /* Screen drawing thread data */
        bool draw_progress;
        int progress;
        int max_progress;
        string progress_desc[5]; // the fifth one is left for the actual progress
        
        /* Options Menu instance, will be drawn by the draw thread if it's different from null */
        OptionsMenu* optionsmenu;
        
        
        /* Options menu variables */
        int optionsDrawState;
        int optionsAnimX; // state of the animation
        int optionsAnimY;
        // options menu entries possition of the entries
        int pEntryIndex;
        
        /* Common browser images */
        Image* checkBox;
        Image* uncheckBox;
        Image* folderIcon;
        Image* fileIcon;
        
        /* Highlight the currently selected item? */
        bool enableSelection;
    
        void clearEntries();
    
        void moveDirUp();
        
        void update();
        
        void refreshDirs();
        
        void drawScreen();
        
        void drawOptionsMenu();
        
        void drawProgress();
        
        string formatText(string text);
        
        void select();
        
        void fillClipboard();
        
        Entry* get();
        
        void moveDown();
        void moveUp();
        
        void down();
        void up();
        
        void deleteFolder(string path);
        void deleteFile(string path);
        void copyFolder(string path);
        int copy_folder_recursive(const char * source, const char * destination);
        void copyFile(string path);
        void copyFile(string path, string destination);
        int pspIoMove(string src, string dest);
        
        string checkDestExists(string src, string destination, string name);
        
        void extractArchive(int type);
        
        void copy();
        void cut();
        void paste();
        
        void makedir();
        
        void rename();
        
        void removeSelection();
        
        void optionsMenu();
        void options();
        
};

const char* getBrowserCWD();

#endif
