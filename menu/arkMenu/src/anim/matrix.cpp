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
	j = 7;
	k = 2;
	l = 5;
	m = 1;
}

Matrix::~Matrix(){
}

void Matrix::printColumn(int xoffset){
	int i = 0;
	for (int yoffset=10; yoffset<272; yoffset+=15){
		char text[2];
		text[0] = caRow[i];
		text[1] = 0;
		common::printText(xoffset, yoffset, text, GREEN);
		i++;
	}
}

void Matrix::drawColumn(int xoffset){

	// Output a random row of characters
	for (int i=0; i<MAX_CHARS; i++){
		if (caRow[i] != ' ') {
			caRow[i] = GetChar(j + i*i, 33, 30);
		}
	}
	caRow[MAX_CHARS] = 0;
	printColumn(xoffset);
	j = (j + 31);
	k = (k + 17);
	l = (l + 47);
	m = (m + 67);
	caRow[Modulus(j, MAX_CHARS)] = '-';
	caRow[Modulus(k, MAX_CHARS)] = ' ';
	caRow[Modulus(l, MAX_CHARS)] = '-';
	caRow[Modulus(m, MAX_CHARS)] = ' ';
}

void Matrix::draw(){
	ya2d_clear_screen(CLEAR_COLOR);
	
	for (int i=10; i<480; i+=30)
		drawColumn(i);
}

bool Matrix::drawBackground(){
	return false;
}
