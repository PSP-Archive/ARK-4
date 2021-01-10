#ifndef TETRIS_H
#define TETRIS_H

#include <pspgu.h>
#include <stdlib.h>

#include "anim.h"
#include "ya2d.h"

#define RGBA(r,g,b,a)	((r) | ((g)<<8) | ((b)<<16) | ((a)<<24))

#define tetrisMaxSprites 8
#define tetrisFrameSkip	2
#define pieceSkip	30	//This frames till new piece is generated (frameSkip*pieceSkip)
#define rotateSkip  333	//Rotate a piece every this frames
#define fallPixels	1
#define tetrisMaxX	480	//240 120 480
#define tetrisMaxY	273	//136  68 272
#define tetrisX	0
#define tetrisY 0
#define tetrisScaleX 2
#define tetrisScaleY 2
#define tetrisBlockSz 6	//Size pg each piece's block, in pixels
class Tetris : public Anim {

	private:
		struct {
			int x;
			int y;
			int xspeed;
			int yspeed;
			//int size;
			int type;		//piece shape
			int rotate;		//rotate piece
			int moving;		//helps avoid wasting cycles on pieces that already hit the bottom/other pieces.
			u32 color;
		} tetrisSprites[tetrisMaxSprites];

		u32 pieces[16];		//Bitmap for all pieces
		u32 *rawdata1;		//Pointer to write into texture
	    struct ya2d_texture *tetrisTex;
		void drawBlock(u32 color);
		void ClrTexture();

	public:
		Tetris();
		~Tetris();
		void draw();
};

#endif
