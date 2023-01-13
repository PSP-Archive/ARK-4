#ifndef HACKER_H
#define HACKER_H

#include "anim.h"

#define MAX_CHARS 30
#define MAX_ROWS 7

class Hacker : public Anim {

    private:

        char caRow[MAX_ROWS][MAX_CHARS+1];
        int cur_row, cur_col, r;
    
    public:
        Hacker();
        ~Hacker();
        
        void draw();
        
        bool drawBackground();
};

#endif
