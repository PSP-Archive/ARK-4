#include "sprites.h"

#include <ctime>

#include "common.h"

Sprites::Sprites(){
	srand(time(NULL));
	for (int i=0; i<MAX_SPRITES; i++){
		sprites[i].xdir = rand() % 2;
		sprites[i].ydir = rand() % 2;
		sprites[i].x = 240;
		sprites[i].y = 136;
		sprites[i].xspeed = rand() % 5 + 1;
		sprites[i].yspeed = rand() % 5 + 1;
		sprites[i].size = (float)(rand() % 5 + 5)/10.f;
	}
}

Sprites::~Sprites(){
}

bool Sprites::canDraw(int i){
	int w = common::getImage(IMAGE_SPRITE)->getTexture()->width;
	int h = common::getImage(IMAGE_SPRITE)->getTexture()->height;
	return (sprites[i].x < 480 && sprites[i].x+w >= 0 && sprites[i].y+h >= 0 && sprites[i].y < 272);
}

void Sprites::draw(){
	drawCommon(false);
}

bool Sprites::drawFadeout(){
	return drawCommon(true);
}

bool Sprites::drawCommon(bool fadeOut){
	int drawn = 0;
	for (int i=0; i<MAX_SPRITES; i++){
		bool can_draw = false;
		if (!fadeOut){
			int w = common::getImage(IMAGE_SPRITE)->getTexture()->width;
			int h = common::getImage(IMAGE_SPRITE)->getTexture()->height;
			if (sprites[i].x > 480 || sprites[i].x-w < 0)
				sprites[i].xdir = !sprites[i].xdir;
			if (sprites[i].y > 272 || sprites[i].y-h < 0)
				sprites[i].ydir = !sprites[i].ydir;
			can_draw = true;
		}
		else
			can_draw = canDraw(i);
			
		if (can_draw){
			if (sprites[i].xspeed == 1)
				sprites[i].xspeed = 3;
			if (sprites[i].yspeed == 1)
				sprites[i].yspeed = 3;
			common::getImage(IMAGE_SPRITE)->draw_scale(sprites[i].x, sprites[i].y, sprites[i].size, sprites[i].size);
			switch (sprites[i].xdir){
			case 0: sprites[i].x -= sprites[i].xspeed; break;
			default: sprites[i].x += sprites[i].xspeed; break;
			}
			switch (sprites[i].ydir){
			case 0: sprites[i].y -= sprites[i].yspeed; break;
			default: sprites[i].y += sprites[i].yspeed; break;
			}
			drawn++;
		}
	}
	return !(drawn == 0);
}
