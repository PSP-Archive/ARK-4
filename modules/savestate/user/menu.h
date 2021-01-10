#ifndef MENU_H
#define MENU_H

#include <vector>
#include <cstdlib>
#include "entry.h"
#include "gfx.h"

#define MAX_DRAW_ENTRIES 2
#define MAX_LOADED_ICONS 11

using namespace std;

class Menu{

	private:
		int index;
		int threadIndex;
		int animating;
		float animState;
		int fastScroll;
		bool fastScrolling;
		bool animDelay;
		bool initLoad;
		bool stopLoading;
		vector<Entry*>* entries;
		
		void freeScreenshots();
		bool checkScreenshotsNeeded(bool isSelected);
		
	public:
		Menu();
		~Menu();
		
		void draw(bool selected);
		
		void loadScreenshotsDynamic(bool isSelected);
		
		bool waitScreenshotsLoad(bool isSelected, bool forceQuit=false);
		
		void resumeScreenshotsLoading();
		
		void addEntry(Entry* e);
		Entry* getEntry();
		Entry* getEntry(int index);
		void clearEntries();
		size_t getVectorSize();
		vector<Entry*>* getVector();
		
		bool empty();
		
		void animStart(int direction);
		void moveUp();
		void moveDown();
		void stopFastScroll();
};
		
#endif
