#ifndef SNOW_H
#define SNOW_H

#include "../graphics/graphics.h"
#include "../common/rand.h"

typedef struct snoflake {
        int x,y,flake;
} snoflake;


void initSnow(snoflake* snowflake);
void updateSnow(snoflake* snowflake, unsigned long* colours, int color);
void fadeIn();
void fadeOut();


#endif
