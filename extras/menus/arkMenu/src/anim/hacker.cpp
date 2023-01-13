#include "cstdlib"
#include "hacker.h"
#include "common.h"
#include "gfx.h"

extern char GetChar(int iGenerator, char cBase, int iRange);

Hacker::Hacker(){
    r = rand();
    cur_col = r%MAX_CHARS;
    cur_row = 0;

    // Output a random row of characters
    for (int i=0; i<MAX_ROWS; i++){
        for (int j=0; j<MAX_CHARS; j++){
            caRow[i][j] = GetChar(r, 33, 30);
            r += 7;
        }
        caRow[i][MAX_CHARS] = 0;
    }
}

Hacker::~Hacker(){
}

void Hacker::draw(){

    if (caRow[cur_col][cur_row] != ' '){
        caRow[cur_col][cur_row] = ' ';
    }
    else{
        cur_row++;
        r += 7;
        caRow[cur_row-1][cur_col] = GetChar(r, 33, 30);
        if (cur_row >= MAX_ROWS){
            r = rand();
            cur_col = r%MAX_CHARS;
            cur_row = 0;
        }
    }
    
    int yoffset = 45;
    for (int i=0; i<MAX_ROWS; i++){
        common::printText(10, yoffset, &(caRow[i][0]), GREEN, SIZE_HUGE);
        yoffset += 37;
    }
}

bool Hacker::drawBackground(){
    return false;
}
