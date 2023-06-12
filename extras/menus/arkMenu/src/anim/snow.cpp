#include "snow.h"
#include "common.h"

float flake_size[] = {0.5f, 1.0f, 2.0f};

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

void SnowAnim::printSnowFlake(int x, int y, float size){
    intraFontSetStyle(common::getFont(), size, LITEGRAY, 0, 0.f, INTRAFONT_WIDTH_VAR);
    intraFontPrint(common::getFont(), x, y, ".");
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
          printSnowFlake((float)(snowflakes[a].x), (float)(snowflakes[a].y), flake_size[snowflakes[a].flake]);
     }
}
