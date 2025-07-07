#ifndef GAMEMGR_H
#define GAMEMGR_H

#include <cstdlib>
#include <dirent.h>
#include <cstdio>
#include <string>
#include <cstring>
#include "system_entry.h"
#include "menu.h"
#include "controller.h"
#include "gfx.h"
#include "animations.h"
#include "browser.h"
#include "umd.h"

#define MAX_CATEGORIES 3

using namespace std;

enum{
    ICONS_STOPPED,
    ICONS_LOADING,
    ICONS_PAUSED,
};

class GameManager : public SystemEntry{

    private:

        GameManager();

        TextScroll scroll;
    
        /* Array of game menus */
        Menu* categories[MAX_CATEGORIES];
        
        /* Selected game menu */
        int selectedCategory;
        int maxDraw;
        
        bool use_categories;
        
        /* Multithreading variables */
        SceUID iconThread; // UID's of the icon thread
        SceUID iconSema; // semaphore to lock the thread when sleeping
        int dynamicIconRunning;
        bool scanning;
        
        /* Screen drawing thread data */
        bool hasLoaded; // whether the main thread has finished loading or not, if not then only draw the background and animation
        
        /* Options Menu instance, will be drawn by the draw thread if it's different from null */
        OptionsMenu* optionsmenu;

        void endAllThreads();
        
        // Entry animation
        void animAppear();
        void animDisappear();
        
        /* find all available menu entries
            ms0:/PSP/GAME for eboots
            ms0:/ISO for ISOs
            ms0:/PSP/SAVEDATA for both
         */
        void findEntries();
        void findEboots(const char* path);
        void findISOs(const char* path);
        void findSaveEntries(const char* path);

        /* move the menu in the specified direction */
        void moveLeft();
        void moveRight();
        void moveUp();
        void moveDown();
        void stopFastScroll();
        
        int getNextCategory(int current);
        int getPreviousCategory(int current);
        
        void execApp();
        void extractHomebrew();
        void gameOptionsMenu();
        void startBoot();
        
    public:
    
        ~GameManager();

        static GameManager* getInstance();
        
        /* thread to load icon0 in the background */
        static int loadIcons(SceSize _args, void *_argp);
        
        // update game list if specific path has changed
        static void updateGameList(const char* path);
        
        /* obtain the currently selected entry */
        Entry* getEntry();
        
        /* draw all three menus */
        void draw();
        
        /* control the menus */
        void control(Controller* pad);
        
        void pause(){
            while (selectedCategory == -1) sceKernelDelayThread(10000);
            for (int i=MAX_CATEGORIES; i>=0; i--){
                this->maxDraw = i;
                sceKernelDelayThread(60000);
            }
            pauseIcons();
        };
        
        void resume(){
            for (int i=0; i<=MAX_CATEGORIES; i++){
                this->maxDraw = i;
                sceKernelDelayThread(60000);
            }
            if (selectedCategory == -2) selectedCategory = -1;
            resumeIcons();
        }
        
        string getInfo();
        string getFooter();
        
        string getName(){
            return "Games";
        }

        void drawInfo(){
            string info = getInfo();
            bool is_entry_info = (getEntry() != NULL && info == getEntry()->getName());
            bool translate = ( !is_entry_info || info == "Recovery Menu" || info == "UMD Drive");
            common::printText(5, 13, info.c_str(), LITEGRAY, SIZE_MEDIUM, 0, &scroll, translate);
        }
        
        /* Control the icon threads */
        void pauseIcons();
        void resumeIcons();
        bool waitIconsLoad(bool forceQuit=false);
        
        void setInfo(string info){};
        void setFooter(string info){};
        void setName(string name){};
        
        Image* getIcon(){
            return common::getImage(IMAGE_GAME);
        }

        bool isStillLoading(){
            return (this->scanning);
        }

        /* Popup Menu */
        void MenuPopup();
        
        /* get a specific category menu */
        Menu* getMenu(EntryType t);
        
        static bool update_game_list;
        
};

#endif
