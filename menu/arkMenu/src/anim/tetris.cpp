#include "tetris.h"
#include "string.h"
//#include <tinyfont.h>
//#include <ctime>
//#include "common.h"
#define clrcol 0x00000000	//This color will be used as clear/empty space in the texture. trying different values b/c that weird cache problem
Tetris::Tetris(){
	//pieces[0]=0x0F82664C;  //Bitmap for all pieces
	//pieces[1]=0x00EE6CE6;	//Bitmap for all pieces
	//pieces[0]=0x5;  //Bitmap for all pieces
	//pieces[1]=0xAAAAAAAA;
	pieces[0]=0x00000000;	//Bitmap for all pieces
	pieces[1]=0x00000000;	//Bitmap for all pieces
	pieces[2]=0x0082664C;	//Bitmap for all pieces
	pieces[3]=0x0FEE6CE6;	//Bitmap for all pieces
	pieces[4]=0x04000000;	//Bitmap for all pieces (rotated cw)
	pieces[5]=0x04C40442;	//Bitmap for all pieces (rotated cw)
	pieces[6]=0x04846666;	//Bitmap for all pieces (rotated cw)
	pieces[7]=0x04866244;	//Bitmap for all pieces (rotated cw)
	pieces[8]=0x00000000;	//Bitmap for all pieces (inverted)
	pieces[9]=0x00000000;	//Bitmap for all pieces (inverted)
	pieces[10]=0x00EE66EC;	//Bitmap for all pieces (inverted)
	pieces[11]=0x0F286C46;	//Bitmap for all pieces (inverted)
	pieces[12]=0x04000000;	//Bitmap for all pieces (rotated ccw)
	pieces[13]=0x04260422;	//Bitmap for all pieces (rotated ccw)
	pieces[14]=0x04226666;	//Bitmap for all pieces (rotated ccw)
	pieces[15]=0x04626224;	//Bitmap for all pieces (rotated ccw)

	this->tetrisTex = ya2d_create_texture (tetrisMaxX, tetrisMaxY, GU_PSM_8888, YA2D_PLACE_VRAM);//3
	//srand(time(NULL));
	for (int i=0; i<tetrisMaxSprites; i++){
		//tetrisSprites[i].x = rand() % (tetrisMaxX-4*tetrisBlockSz+1);
		tetrisSprites[i].x = (rand() % ((tetrisMaxX-tetrisBlockSz*4)/tetrisBlockSz))*tetrisBlockSz;
		tetrisSprites[i].y = 0;
		tetrisSprites[i].xspeed = 0;
		tetrisSprites[i].yspeed = fallPixels;
		tetrisSprites[i].type = rand() % 7;
		tetrisSprites[i].rotate = 0;
		tetrisSprites[i].moving = 0;
		//tetrisSprites[i].size = (float)(rand() % 5 + 5)/10.f;
		tetrisSprites[i].color= RGBA ( rand()%256, rand()%256,rand()%256, 255);
		//tetrisSprites[i].color= RGBA (255, 255, 255, 255);
		Tetris::ClrTexture();
	}

}
void Tetris::ClrTexture(){
	//clear tetrisTex
	for (int y = 0; y < tetrisTex->height; y++)
	    for (int x = 0; x < tetrisTex->width; x++)
	        ((unsigned int*)tetrisTex->data)[x+ y * tetrisTex->pow2_w] = clrcol; //clear texture with clear color
	rawdata1=(u32*)tetrisTex->data+tetrisTex->pow2_w*(tetrisMaxY-1);
	for (int x=0;x<tetrisMaxX;x++) *rawdata1++=0xFFFFFFFF;

}

Tetris::~Tetris(){
}
void Tetris::drawBlock(u32 color){
	for (int y = 0; y < tetrisBlockSz; y++){
	    for (int x = 0; x < tetrisBlockSz; x++)
	        *rawdata1++ = color;
		rawdata1+= tetrisTex->pow2_w - tetrisBlockSz;
	}
	rawdata1-= tetrisTex->pow2_w * tetrisBlockSz - tetrisBlockSz;
}

void Tetris::draw(){
	static int frameCount=tetrisFrameSkip;
	static int genPiece=pieceSkip;
	static int rotateNow=0;
	int i,v,l;
	int hitmap1=0;
	//u32 p;
	if (frameCount==tetrisFrameSkip){
		frameCount=0;
		if (genPiece==pieceSkip){
			genPiece=0;
			for (int i=0;i<tetrisMaxSprites;i++){
				if (tetrisSprites[i].moving==0){
					tetrisSprites[i].moving=1;
					tetrisSprites[i].y=0;
					tetrisSprites[i].x = (rand() % ((tetrisMaxX-tetrisBlockSz*4)/tetrisBlockSz))*tetrisBlockSz;
					tetrisSprites[i].color= RGBA ( rand()%256, rand()%256,rand()%256, 255);
					break;
				}
			}
		}
		genPiece++;
		
		for (i = 0; i < tetrisMaxSprites; i++){
			if (tetrisSprites[i].moving==1){
				v=tetrisTex->pow2_w*(tetrisSprites[i].y);
				rawdata1=(u32*)tetrisTex->data + tetrisSprites[i].x + v;
				v=this->tetrisSprites[i].type*4 ;	//Loop to erase sprite previous location
				for (l = 0; l < 4; l++){
					if ((1 & (pieces[l + tetrisSprites[i].rotate]>>(3+v)))) Tetris::drawBlock (clrcol); else rawdata1+=tetrisBlockSz;
					if ((1 & (pieces[l + tetrisSprites[i].rotate]>>(2+v)))) Tetris::drawBlock (clrcol); else rawdata1+=tetrisBlockSz;
					if ((1 & (pieces[l + tetrisSprites[i].rotate]>>(1+v)))) Tetris::drawBlock (clrcol); else rawdata1+=tetrisBlockSz;
					if ((1 & (pieces[l + tetrisSprites[i].rotate]>>(0+v)))) Tetris::drawBlock (clrcol); else rawdata1+=tetrisBlockSz;
					rawdata1+= tetrisTex->pow2_w * tetrisBlockSz - 4 * tetrisBlockSz; //go down one line and 4 blocks left
				}
				//Rotate needs to be done after clearing previous piece, so can't be done on random piece
				if (rotateNow==rotateSkip){
					rotateNow=0;
					//tetrisSprites[i].rotate = 4 - tetrisSprites[i].rotate;
					tetrisSprites[i].rotate += 4;
					if (tetrisSprites[i].rotate==16) tetrisSprites[i].rotate=0;
				}
				rotateNow++;

				tetrisSprites[i].y+=tetrisSprites[i].yspeed;
				v=tetrisTex->pow2_w*(tetrisSprites[i].y);
				rawdata1=(u32*)tetrisTex->data + tetrisSprites[i].x + v;

				v=this->tetrisSprites[i].type*4 ;
				hitmap1=0;
				for (l = 0; l < 4; l++){
					if ((1 & (pieces[l + tetrisSprites[i].rotate]>>(3+v)))) {
						if (*(rawdata1+tetrisTex->pow2_w*tetrisBlockSz)!=clrcol) hitmap1=1;
						Tetris::drawBlock (tetrisSprites[i].color);
					}
					else rawdata1+=tetrisBlockSz;
					if ((1 & (pieces[l + tetrisSprites[i].rotate]>>(2+v)))) {
						if (*(rawdata1+tetrisTex->pow2_w*tetrisBlockSz)!=clrcol) hitmap1=1;
						Tetris::drawBlock (tetrisSprites[i].color);
					}
					else rawdata1+=tetrisBlockSz;
					if ((1 & (pieces[l + tetrisSprites[i].rotate]>>(1+v)))) {
						if (*(rawdata1+tetrisTex->pow2_w*tetrisBlockSz)!=clrcol) hitmap1=1;
						Tetris::drawBlock (tetrisSprites[i].color);
					}
					else rawdata1+=tetrisBlockSz;
					if ((1 & (pieces[l + tetrisSprites[i].rotate]>>(0+v)))) {
						if (*(rawdata1+tetrisTex->pow2_w*tetrisBlockSz)!=clrcol) hitmap1=1;
						Tetris::drawBlock (tetrisSprites[i].color);
					}
					else rawdata1+=tetrisBlockSz;
					rawdata1+= tetrisTex->pow2_w * tetrisBlockSz - 4 * tetrisBlockSz; //go down one line and 4 blocks left
				}

				if (hitmap1==1) {
					tetrisSprites[i].moving=0;
					if (tetrisSprites[i].y==tetrisSprites[i].yspeed) Tetris::ClrTexture();	//Screen is full, clear everything
				}
				//if (tetrisSprites[i].y >= (tetrisMaxY-1)) {
				//	tetrisSprites[i].y=0;
				//	tetrisSprites[i].x = rand() % (tetrisMaxX-3);
				//}
				//if (tetrisSprites[i].y >= (tetrisMaxY-5*tetrisBlockSz)) tetrisSprites[i].y=0;
			}
		}
	}
	ya2d_draw_texture(tetrisTex, tetrisX, tetrisY);
	//ya2d_draw_texture_scale(tetrisTex, tetrisX, tetrisY, tetrisScaleX, tetrisScaleY);
    //ya2d_draw_rect(tetrisX,tetrisY,this->tetrisTex->pow2_w*tetrisScale, this->tetrisTex->pow2_h*tetrisScale, 0xFF00FF00, 0);
    //ya2d_draw_rect(tetrisX,tetrisY,this->tetrisTex->width*tetrisScale, this->tetrisTex->height*tetrisScale, 0xFF00FF00, 0);
	//tinyfont_draw_stringf(300, 60,  GU_RGBA(255,0,0,255), "x:%d y:%d yp:%d", this->tetrisSprites[0].x, this->tetrisSprites[0].y, this->tetrisTex->pow2_w*(this->tetrisSprites[0].y) );
	//tinyfont_draw_stringf(300, 70,  GU_RGBA(255,0,0,255), "y:%d py:%d", this->tetrisTex->height,this->tetrisTex->pow2_h);
	frameCount++;
	//drawCommon(false);
}
