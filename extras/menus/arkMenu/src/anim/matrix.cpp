#include "cstdlib"
#include "matrix.h"
#include "common.h"
#include "gfx.h"

int Modulus(int iN, int iMod) {
    int iQ = (iN/iMod);
    return iN - (iQ*iMod);
}

char GetChar(int iGenerator, char cBase, int iRange) {
    return (cBase + Modulus(iGenerator, iRange));
}

Matrix::Matrix(){

    cur_col = rand()%MAX_COLS;
    cur_row = 0;

    // Output a random row of characters
    for (int i=0; i<MAX_COLS; i++){
        for (int j=0; j<MAX_CHARS; j++){
            if (caRow[i][j] != ' ') {
                caRow[i][j] = 0;
            }
        }
        caRow[i][MAX_CHARS] = 0;
        r += 31;
    }
}

Matrix::~Matrix(){
}

void Matrix::drawColumn(int xoffset, int i){
    int yoffset=35;
    for (int j=0; j<MAX_CHARS; j++){
        char text[2];
        text[0] = caRow[i][j];
        text[1] = 0;
        if (text[0] != 0)
            common::printText(xoffset, yoffset, text, GREEN, SIZE_MEDIUM);
        yoffset += 15;
    }
}

void Matrix::draw(){

    if (caRow[cur_col][cur_row] != 0){
        caRow[cur_col][cur_row] = 0;
    }
    else{
        caRow[cur_col][cur_row] = GetChar(r + cur_row*cur_row, 33, 30);
        r += 7;
    }

    cur_row++;
    if (cur_row >= MAX_CHARS){
        cur_col = rand()%MAX_COLS;
        cur_row = 0;
    }
    
    int xoffset = 10;
    for (int i=0; i<MAX_COLS; i++){
        drawColumn(xoffset, i);
        xoffset += 40;
    }
}

bool Matrix::drawBackground(){
    return false;
}