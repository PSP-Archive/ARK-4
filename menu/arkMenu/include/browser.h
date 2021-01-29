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
    SEND,
    RECEIVE
};

class Browser : public SystemEntry{

    public:
        Browser();
        ~Browser();
        
        void draw();
        
        void control(Controller* pad);
        
        void pause(){}
        void resume(){}
        
        char* getInfo(){
            return (char*)(this->cwd.c_str());
        }
        
        char* getName(){
            return "Files";
        }
        
        Image* getIcon(){
            return common::getImage(IMAGE_BROWSER);
        }
    
    private:
    
        string cwd; // Current Working Directory
        
        vector<Entry*>* entries; // entries in the current directory
        
        vector<string>* selectedBuffer; // currently selected items
        
        int pasteMode; // COPY or CUT
        
        /* menu control variables */
        int index;  // index of currently selected item
        int start; // where to start drawing the menu
        bool animating; // animate the menu transition?
        
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
    
        void moveDirUp();
        
        void update();
        
        void refreshDirs();
        
        void drawScreen();
        
        void drawOptionsMenu();
        
        void drawProgress();
        
        string formatText(string text);
        
        void select();
        
        void fillSelectedBuffer();
        
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
        
        void extractArchive(int type);
        
        void send();
        void recv();
        
        void copy();
        void cut();
        void paste();
        
        void makedir();
        
        void rename();
        
        void removeSelection();
        
        void optionsMenu();
        void options();
        
};

#endif
