#ifndef ANIM_H
#define ANIM_H

class Anim {

	public:
	
		Anim();
		~Anim();
		
		virtual void draw() = 0;
		
		/* Returns false if the animation overwrites the menu's background */
		virtual bool drawBackground();
};

#endif
