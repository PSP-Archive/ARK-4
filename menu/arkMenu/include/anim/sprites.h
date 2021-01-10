#ifndef SPRITES_H
#define SPRITES_H

#include "anim.h"

#define MAX_SPRITES 10

class Sprites : public Anim {

	private:
		struct {
			int xdir;
			int ydir;
			int x;
			int y;
			int xspeed;
			int yspeed;
			float size;
		} sprites[MAX_SPRITES];
		
		bool canDraw(int i);
		
		bool drawCommon(bool fadeout);
	
	public:
		Sprites();
		~Sprites();
		void draw();
		bool drawFadeout();
};


#endif
