#ifndef WAVE_H
#define WAVE_H

#include <stdio.h>
#include <pspgu.h>
#include <pspgum.h>
#include <math.h>

#include "anim.h"

#define WAVES_FRAMESKIP 5 // anything higher and the waves will not move smoothly, lower and the fps drop
#define WAVE_STEP 0.2  // the waves speed, I find this value to be the best
#define WAVE_AMPLITUDE 30.f // how much the wave moves on the screen
#define WAVE_ANGLE 0.5 // a simple initial value
// these colors allow the background's color to standout, and they look great
#define WAVE_COLOR 0x44E0E0E0
#define WAVE_COLOR2 0x44C0C0C0
#define WAVE_COLOR_OUTLINE 0x44A0A0A0

typedef struct
{
	unsigned int color;
	short x, y, z;
} vertex;

class Waves : public Anim
{
	public:
	
		Waves();
		~Waves();
		
		void draw();
	
	private:
	
		float amplitude;
		float step;
		float angle;
		
		int frameskip;
		
		vertex* vertices; // the first wave
		vertex* vertices2; // the second wave
		
		void update();
};

#endif
