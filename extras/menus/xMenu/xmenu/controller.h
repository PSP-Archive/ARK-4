#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <pspsdk.h>
#include <pspctrl.h>

class Controller{

	private:
		SceCtrlData* pad;
	
	public:
	
		Controller();
		~Controller();
		
		void update();
		
		bool up();
		bool down();
		bool left();
		bool right();
		bool cross();
		bool circle();
		bool square();
		bool triangle();
		bool RT();
		bool LT();
		bool start();
		bool select();
};
		

#endif
