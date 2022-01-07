#include "snow.h"
#include "common.h"

SnowAnim::SnowAnim(){
  int i;
  for (i = 0; i<100; i++) {
       snowflakes[i].x = rand()%480;
       snowflakes[i].y = rand()%272;
       snowflakes[i].flake = rand()%3;
  }
}

SnowAnim::~SnowAnim(){
}


void SnowAnim::draw()
{

  int a=0, sway=0;
     for (a = 0;a<100;a++) {
          sway = rand()%4;
          if (sway == 1 || sway == 2) {
             snowflakes[a].x -= 1;
          } 
          if (sway == 3 || sway == 4) {
             snowflakes[a].x += 1;
          }
          snowflakes[a].y += rand()%4;
          if (snowflakes[a].y > 272) {
               snowflakes[a].y = 0;
               snowflakes[a].x = rand()%480;
          }
          switch(snowflakes[a].flake){
          case 0:
                common::printText((float)(snowflakes[a].x), (float)(snowflakes[a].y), ".", LITEGRAY, 0.5f);
                break;
          case 1:
                common::printText((float)(snowflakes[a].x), (float)(snowflakes[a].y), ".", LITEGRAY, 1.0f);
                break;
          case 2:
                common::printText((float)(snowflakes[a].x), (float)(snowflakes[a].y), ".", LITEGRAY, 2.f);
                break;
          }
     }
}
