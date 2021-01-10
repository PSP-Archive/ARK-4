#include <sstream>

#include "pixel.h"
#include "common.h"

PixelAnim::PixelAnim(){
	pixels = new t_pixel[MAX_PIXELS];
	srand(time(NULL));
	for (int i = 0; i<MAX_PIXELS; i++){
		int xdirection = rand() % 2;
		int ydirection = rand() % 2;
		int xspeed = (rand() % 5) + 1;
		int yspeed = (rand() % 5) + 1;
		this->pixels[i].x = rand() % 480 + 1;
		this->pixels[i].y = rand() % 100 + 100;
		
		this->pixels[i].xspeed = (xdirection == 0)? -xspeed : xspeed;
		this->pixels[i].yspeed = (ydirection == 0)? -yspeed : yspeed;
	}
}

PixelAnim::~PixelAnim(){
	delete [] this->pixels;
}

void PixelAnim::draw(){
	for (int i=0; i<MAX_PIXELS; i++){
		this->pixels[i].x += this->pixels[i].xspeed;
		this->pixels[i].y += this->pixels[i].yspeed;
	
		if (this->pixels[i].y >= 250 || this->pixels[i].y <= 100)
			this->pixels[i].yspeed = -this->pixels[i].yspeed;
		
		if (this->pixels[i].x < 0)
			this->pixels[i].x = 480;
		
		if (this->pixels[i].x > 480)
			this->pixels[i].x = 0;
			
		common::printText((float)(pixels[i].x), (float)(pixels[i].y), ".", LITEGRAY, SIZE_LITTLE, true);
	}
}
