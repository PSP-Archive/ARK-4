#ifndef MENUMGR_H
#define MENUMGR_H

#include <pspkernel.h>
#include "menu.h"

class MenuManager {

	private:
	
		Menu* menu;
	
		int selected;
	
		// drawing thread variables
		SceUID drawThreadID;
		bool drawRunning;
		int bootStatus1;
		int bootStatus2;
		bool exitAnim;
		
		// dynamic icon loading variables
		bool dynamicIconRunning;
		SceUID iconThread;
	
		void draw();
		void findEntries();
		
		static int drawThread(SceSize argc, void* argp);
		static int loadIcons(SceSize argc, void* argp);

	public:
	
		MenuManager();
		~MenuManager();
		
		void run();

};

#endif
