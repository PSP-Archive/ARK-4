#ifndef TEXT_H
#define TEXT_H

#include <string>
#include <time.h>
#include "common.h"
#include "graphics.h"

// Colors
enum colors {
    RED =    0xFF0000FF,
    GREEN =    0xFF00FF00,
    BLUE =    0xFFFF0000,
    WHITE =    0xFFFFFFFF,
    LITEGRAY = 0xFFBFBFBF,
    GRAY =  0xFF7F7F7F,
    DARKGRAY = 0xFF3F3F3F,        
    BLACK = 0xFF000000,
};

using namespace std;

class TextAnim{

    private:

        int scroll;
        string title;
        string subtitle;
        
    public:
    
        TextAnim(string title, string subtitle);
        ~TextAnim();
        
        void draw(float y);
        
};

#endif
