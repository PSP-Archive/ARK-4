#ifndef BROWSER_H
#define BROWSER_H

#include <vector>
#include "entry.h"
#include "common.h"
#include "gfx.h"
#include "optionsmenu.h"
#include "system_entry.h"
#include "lang.h"

using namespace std;

enum{
    NO_MODE,
    COPY,
    CUT,
    PASTE,
    DELETE,
    RENAME,
    CREATE,
    USB_DEV,
    MS0_DIR,
    EF0_DIR,
    FTP_DIR,
    UMD_DIR,
};

#define ROOT_DIR "ms0:/" // Initial directory
#define GO_ROOT "ef0:/" // PSP Go initial directory
#define FTP_ROOT "ftp:/" // FTP directory
#define UMD_ROOT "disc0:/" // UMD directory
#define EH0_ROOT "eh0:/" // Go Hidden directory

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
        virtual void createFile(string path) = 0;
        virtual void copyFileTo(string orig, string dest, int* progress) = 0;
        virtual void copyFileFrom(string orig, string dest, int* progress) = 0;
};

class Browser : public SystemEntry{

    public:
        ~Browser();
        static Browser* getInstance();
        
        void draw();
        
        void control(Controller* pad);
        
        void pause(){
            animation = 1;
            while (animation != -2)
                sceKernelDelayThread(0);
        }
        void resume(){
            animation = -1;
            if (firstboot){
                this->refreshDirs();
                firstboot = false;
            }
            while (animation != 0)
                sceKernelDelayThread(0);
        }
        
        void setInfo(string info){};
        void setFooter(string footer){};
        void setName(string name){};
        
        string getFooter(){}
        string getInfo(){
            return this->cwd;
        }

        void drawInfo(){
            static TextScroll scroll;
            if (devsize.size() > 0){
                int w = common::calcTextWidth(cwd.c_str(), SIZE_MEDIUM, 0);
                scroll.w = 150;
                common::printText(5, 13, cwd.c_str(), LITEGRAY, SIZE_MEDIUM, 0, NULL, 0);
                common::printText(5+w, 13, string(" ("+TR("Free size")+": "+devsize+")").c_str(), LITEGRAY, SIZE_MEDIUM, 0, &scroll);
            }
            else{
                scroll.w = 200;
                common::printText(5, 13, cwd.c_str(), LITEGRAY, SIZE_MEDIUM, 0, &scroll, 0);
            }
        }
        
        string getName(){
            return "Files";
        }
        
        Image* getIcon(){
            return common::getImage(IMAGE_BROWSER);
        }

        bool isStillLoading(){ return false; }
        
        static void recursiveFolderDelete(string path);
        static long recursiveSize(string path);
        
        static BrowserDriver* ftp_driver;
        
        static const char* getCWD();
        
    private:

        Browser();
    
        string cwd; // Current Working Directory

        string devsize; // device size (only if in root)
        
        vector<Entry*>* entries; // entries in the current directory
        
        vector<string>* clipboard; // currently selected items
        
        int pasteMode; // COPY or CUT
        
        /* menu control variables */
        int index;  // index of currently selected item
        int start; // where to start drawing the menu
        bool animating; // animate the menu transition?
        unsigned int moving;
        int animation;
        bool firstboot;
        bool is_loading;
        
        /* Screen drawing thread data */
        bool hide_main_window;
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
        
        /* Highlight the currently selected item? */
        bool enableSelection;
    
        void clearEntries();

        bool isRootDir(string dir);
    
        void moveDirUp();
        
        void update(Entry* ent, bool skip_prompt);
        
        void refreshDirs(const char* retry=NULL);
        
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
        void left();
        void right();
        
        void deleteFolder(string path);
        void deleteFile(string path);
        void copyFolder(string path);
        int copy_folder_recursive(const char * source, const char * destination);
        void copyFile(string path);
        void copyFile(string path, string destination);
        int pspIoMove(string src, string dest);
        int loadStartModule(string modpath, bool wait_on_ok=true);
        
        string checkDestExists(string src, string destination, string name);
        
        void extractArchive();
        void installPlugin();
        void installTheme();
        
        void copy();
        void cut();
        void paste();
        
        void makedir();
        void makefile();
        void createNew();
        void toggleUSB();
        
        void rename();
        
        void removeSelection();
        
        void optionsMenu();
        void options();
        
        static void unarchiverLogger(const char* filepath, int cur, int max);
};

const char* getBrowserCWD();

#endif
