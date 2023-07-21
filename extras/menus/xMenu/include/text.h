#ifndef TEXT_H
#define TEXT_H

#include <string>
#include <time.h>
#include "common.h"
#include "graphics.h"

// Colors
enum colors {
    RED =    0x000000FF,
    GREEN =    0x0000FF00,
    BLUE =    0x00FF0000,
    WHITE =    0x00FFFFFF,
    LITEGRAY = 0x00BFBFBF,
    GRAY =  0x007F7F7F,
    DARKGRAY = 0x003F3F3F,
    BLACK = 0x00000000,
};

using namespace std;

class TextAnim{

    private:

        int scroll, ci, skip;
        string title;
        string subtitle;
        
    public:
    
        TextAnim(string title, string subtitle);
        ~TextAnim();
        
        void draw(float y);
        
};

#endif
