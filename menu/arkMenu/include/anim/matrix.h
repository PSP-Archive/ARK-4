#ifndef MATRIX_H
#define MATRIX_H

#include "anim.h"

#define MAX_CHARS 30

class Matrix : public Anim {

	private:

		char caRow[MAX_CHARS+1];
		int j;
		int k;
		int l;
		int m;
		
		void printColumn(int xoffset);
		
		void drawColumn(int xoffset);
	
	public:
		Matrix();
		~Matrix();
		
		void draw();
		
		bool drawBackground();
};

#endif
