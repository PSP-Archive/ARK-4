#ifndef SNOW_H
#define SNOW_H

#include <ctime>
#include <cstdlib>

#include "anim.h"

#define MAX_SNOWFLAKES 100

typedef struct{
    int x, y;
    int flake;
} Snowflake;

class SnowAnim : public Anim {

    private:
        Snowflake snowflakes[MAX_SNOWFLAKES];
    
    public:
        SnowAnim();
        ~SnowAnim();
        
        void draw();
};

#endif
