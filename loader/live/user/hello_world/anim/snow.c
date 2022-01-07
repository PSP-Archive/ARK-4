#include "snow.h"

#define rand() genrand_int32()

void initSnow(snoflake* snowflake){
  int i;
  for (i = 0; i<100; i++) {
       snowflake[i].x = rand()%480;
       snowflake[i].y = rand()%272;
       snowflake[i].flake = rand()%3;
  }
}


void updateSnow(snoflake* snowflake, unsigned long* colours, int color)
{

  int a=0, sway=0;
     for (a = 0;a<100;a++) {
          sway = rand()%4;
          if (sway == 1 || sway == 2) {
             snowflake[a].x -= 1;
          } 
          if (sway == 3 || sway == 4) {
             snowflake[a].x += 1;
          }
          snowflake[a].y += rand()%4;
          if (snowflake[a].y > 272) {
               snowflake[a].y = 0;
               snowflake[a].x = rand()%480;
          }
          if(snowflake[a].flake==0) {
                                 drawPixel(snowflake[a].x,snowflake[a].y,1,1,colours[color]);
          }
          if(snowflake[a].flake==1) {
                                    drawPixel(snowflake[a].x,snowflake[a].y,2,2,colours[color]);
          }
          if(snowflake[a].flake==2) {
                                    drawPixel(snowflake[a].x,snowflake[a].y,3,3,colours[color]);
          }
     }
}

void fadeIn()
{
  int i;

  for (i = 0; i < 16; i+=3)
  {
    Fillvram(0x00111111 * i);
    sceDisplayWaitVblankStart();
    changeBuffer();
  }
}

void fadeOut()
{
  int i;

  for (i = 15; i >= 0; i--)
  {
    Fillvram(0x00111111 * i);
    sceDisplayWaitVblankStart();
    changeBuffer();
  }

	Fillvram(0);
	changeBuffer();
	Fillvram(0);
}
