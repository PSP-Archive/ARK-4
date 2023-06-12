#ifndef WAVE_H
#define WAVE_H

#include <stdio.h>
#include <pspgu.h>
#include <pspgum.h>
#include <math.h>

#include "anim.h"

#define WAVES_FRAMESKIP 0 // anything higher and the waves will not move smoothly, lower and the fps drop
#define WAVE_AMPLITUDE 30 // how much the wave moves on the screen
// these colors allow the background's color to standout, and they look great
#define WAVE_COLOR 0xa0808000
#define WAVE_COLOR2 0xa0cccc00
#define WAVE_COLOR_OUTLINE 0x50cccc00

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
    
        int step;
        
        int frameskip;
        
        vertex* vertices; // the first wave
        vertex* vertices2; // the second wave
        vertex bubbles[10]; // bubbles
        
        void update();
};

#endif
