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
    r=7;
    for (int i=0; i<MAX_COLS; i++){
        for (int j=0; j<MAX_CHARS; j++){
            if (caRow[i][j] != ' ') {
                caRow[i][j] = GetChar(r + j*j, 33, 30);
            }
        }
        caRow[i][MAX_CHARS] = 0;
        r += 31;
    }
}

Matrix::~Matrix(){
}

void Matrix::printColumn(int xoffset, int i){
    int j = 0;
    for (int yoffset=10; yoffset<272; yoffset+=15){
        char text[2];
        text[0] = caRow[i][j];
        text[1] = 0;
        if (i==cur_col && j==cur_row)
            common::printText(xoffset, yoffset, ".", GREEN, SIZE_BIG, 1);
        else
            common::printText(xoffset, yoffset, text, GREEN);
        j++;
    }
}

void Matrix::drawColumn(int xoffset, int i){

    printColumn(xoffset, i);
}

void Matrix::draw(){

    cur_row++;
    r += 78;
    caRow[cur_col][cur_row-1] = GetChar(r + cur_row*cur_row, 33, 30);
    if (cur_row >= MAX_CHARS){
        cur_col = rand()%MAX_COLS;
        cur_row = 0;
    }

    ya2d_clear_screen(CLEAR_COLOR);
    
    int i=0;
    for (int x=10; x<480; x+=40){
        drawColumn(x, i);
        i++;
    }
}

bool Matrix::drawBackground(){
    return false;
}
