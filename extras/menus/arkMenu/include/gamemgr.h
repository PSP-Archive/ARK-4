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

#define MAX_CATEGORIES 3

using namespace std;

class GameManager : public SystemEntry{

    private:
    
        /* Array of game menus */
        Menu* categories[MAX_CATEGORIES];
        
        /* Selected game menu */
        int selectedCategory;
        
        bool use_categories;
        
        /* Multithreading variables */
        SceUID iconThread; // UID's of the icon thread
        SceUID iconSema; // semaphore to lock the thread when sleeping
        bool dynamicIconRunning;
        /* Control the icon threads */
        void pauseIcons();
        void resumeIcons();
        bool waitIconsLoad(bool forceQuit=false);
        
        /* Screen drawing thread data */
        bool hasLoaded; // whether the main thread has finished loading or not, if not then only draw the background and animation
        
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
        
    public:
    
        GameManager();
        ~GameManager();
        
        /* thread to load icon0 in the background */
        static int loadIcons(SceSize _args, void *_argp);
        
        static void updateGameList();
        
        /* obtain the currently selected entry */
        Entry* getEntry();
        
        /* draw all three menus */
        void draw();
        
        /* control the menus */
        void control(Controller* pad);
        
        void pause(){
            pauseIcons();
        };
        
        void resume(){
            resumeIcons();
        }
        
        string getInfo();
        
        string getName(){
            return "Game";
        }
        
        void setInfo(string info){};
        void setName(string name){};
        
        Image* getIcon(){
            return common::getImage(IMAGE_GAME);
        }

        /* Popup Menu */
        void MenuPopup();
        
        /* get a specific category menu */
        Menu* getMenu(EntryType t);
        
        static bool update_game_list;
        
};

#endif
