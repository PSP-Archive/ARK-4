#ifndef MATRIX_H
#define MATRIX_H

#include "anim.h"

#define MAX_CHARS 30
#define MAX_ROWS 7

class Matrix : public Anim {

    private:

        char caRow[MAX_ROWS][MAX_CHARS+1];
        int cur_row, cur_col, r;
    
    public:
        Matrix();
        ~Matrix();
        
        void draw();
        
        bool drawBackground();
};

#endif
