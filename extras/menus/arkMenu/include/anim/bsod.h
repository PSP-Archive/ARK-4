#ifndef BSOD_H
#define BSOD_H

#include "anim.h"

#define MAX_CHARS 40
#define MAX_ROWS 7

class BSoD : public Anim {

    private:

        char caRow[MAX_ROWS][MAX_CHARS+1];
        int cur_row, cur_col, r;
    
    public:
        BSoD();
        ~BSoD();
        
        void draw();
        
        bool drawBackground();
};

#endif