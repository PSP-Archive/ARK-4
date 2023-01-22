#include "cstdlib"
#include "matrix.h"
#include "common.h"
#include "gfx.h"

char GetChar(int iGenerator, char cBase, int iRange) {
    return (cBase + iGenerator%iRange);
}

Matrix::Matrix(){

    r = rand();
    cur_col = r%MAX_COLS;
    cur_row = 0;

    memset(caRow, 0, sizeof(caRow));
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
        caRow[cur_col][cur_row] = GetChar(r, 33, 30);
        r += 7;
    }

    cur_row++;
    if (cur_row >= MAX_CHARS){
        r = rand();
        cur_col = r%MAX_COLS;
        cur_row = 0;
    }

    char* c = &(caRow[rand()%MAX_COLS][rand()%MAX_CHARS]);
    if (*c != 0){
        *c = GetChar(r/(u32)c, 33, 30);
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