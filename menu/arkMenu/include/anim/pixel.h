#ifndef PIXEL_H
#define PIXEL_H

#include <ctime>
#include <cstdlib>

#include "anim.h"

#define MAX_PIXELS 20

typedef struct{
	int x, y;
	int xspeed;
	int yspeed;
} t_pixel;

class PixelAnim : public Anim {

	private:
		t_pixel* pixels;
	
	public:
		PixelAnim();
		~PixelAnim();
		
		void draw();
};

#endif
