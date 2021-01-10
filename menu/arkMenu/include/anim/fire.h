#ifndef FIRE_H
#define FIRE_H

//#include <stdio.h>
#include <pspgu.h>
//#include <pspgum.h>
//#include <math.h>
#include <stdlib.h>

#include "anim.h"
#include "ya2d.h"

#define RGBA(r,g,b,a)	((r) | ((g)<<8) | ((b)<<16) | ((a)<<24))
typedef unsigned char BOOL;
#define firePosX	-15
#define firePosY	81	//178
#define fireScale	2		//!=1 will scale texture using ya2d_draw_texture_scale
#define fireMaxX	255		//Minus (fireBorderCut*2)
#define fireMaxY	98
#define fireFrameSkip	2	//To slow down fire animation, it was intended for 30fps and psp is 60
#define fireDecay	3		//1=tall fire, ++ will produce shorter fires (adjust fireMaxY for more room)
#define fireBaseColor	255	//affects base color, higher will produce more yellowish fires
#define fireLowCut	2		//will not print lower lines where the effect doensn't look as good
#define fireBorderCut	1	//cut both H borders to eliminate 1st black column, and more if we need square fire
							//CUTS NOT WORKING in texture mode, try to position x into negative and draw wider than screen.
#define alpha 255			//Alpha channel for fire palette

typedef struct s_FireMemX {
	unsigned char fireMemX[fireMaxX];
}t_FireMemX;


class Fire : public Anim
{
	public:
	
		Fire();
		~Fire();
	
		void set_colors(unsigned top, unsigned bottom);
		void draw();
	
	private:
	
		void update();
	
		t_FireMemX fireMemY[fireMaxY];
		u32 firePal[256];
	    struct ya2d_texture *firetex;
	
};

#endif
