#ifndef GOL_ANIM_H
#define GOL_ANIM_H

#include "anim.h"

#define GOL_ROW 30
#define GOL_COL 80

#define GOL_W 480/GOL_COL
#define GOL_H 272/GOL_ROW

#define MAX_GENERATIONS 500

// Game of Life background animation

class GoLAnim : public Anim {

    private:
    
        int generation;
        unsigned char a[GOL_ROW][GOL_COL]; // current generation
        unsigned char b[GOL_ROW][GOL_COL]; // next generation
        
        void generateRandom();
        int count_live_neighbour_cell(int r, int c);

    public:
    
        GoLAnim();
        ~GoLAnim();
        
        void draw();
};

#endif
