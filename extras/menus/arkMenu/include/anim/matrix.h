#ifndef MATRIX_H
#define MATRIX_H

#include "anim.h"

#define MAX_CHARS 15
#define MAX_COLS 12

class Matrix : public Anim {

    private:

        char caRow[MAX_COLS][MAX_CHARS+1];
        int r;
        int cur_col, cur_row;
        
        void drawColumn(int xoffset, int i);
    
    public:
        Matrix();
        ~Matrix();
        
        void draw();
        
        bool drawBackground();
};

#endif