#ifndef ENTRY_H
#define ENTRY_H

#include <string>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <malloc.h>
#include "menucommon.h"
#include "gfx.h"

#define MAX_SLOTS 10

class Entry{

	private:

		string name;
		
		int index;
		
		struct {
			string path;
			Image* screenshot;
		} slots[10];
				
	public:
		Entry(string name);
		~Entry();
		
		string getName();
		
		string getPath();
		
		Image* getScreenshot();
		
		void freeScreenshot();
		
		void loadScreenshot();
		
		void nextSlot();
		void prevSlot();
		
		bool run();
		
		void draw(int x, int y);
};

#endif
