#ifndef MENU_H
#define MENU_H

#include <vector>
#include <cstdlib>
#include "iso.h"
#include "eboot.h"
#include "gfx.h"

#define MAX_DRAW_ENTRIES 5
#define MAX_LOADED_ICONS 11

typedef enum {
    GAME = 0,
    HOMEBREW = 1,
    POPS = 2
}EntryType;

using namespace std;

class Menu{

    private:
        EntryType type;
        int index;
        int threadIndex;
        int animating;
        float animState;
        bool fastScrolling;
        bool animDelay;
        bool initLoad;
        bool stopLoading;
        vector<Entry*>* entries;
        
        void freeIcons();
        bool checkIconsNeeded(bool isSelected);
        void checkIndex();
        
    public:
        Menu(EntryType t);
        ~Menu();
        
        void draw(bool selected);
        
        void loadIconsDynamic(bool isSelected);
        
        bool waitIconsLoad(bool isSelected, bool forceQuit=false);
        
        void resumeIconLoading();
        
        void addEntry(Entry* e);
        Entry* getEntry();
        Entry* getEntry(int index);
        void clearEntries();
        size_t getVectorSize();
        vector<Entry*>* getVector();
        int getIndex(){ return index; };
        
        bool isAnimating();
        bool empty();
        
        void animStart(int direction);
        void moveUp();
        void moveDown();
        void stopFastScroll();
};
        
#endif
