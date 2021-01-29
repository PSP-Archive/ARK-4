#include "fire.h"
#include "string.h"
//#include <pspdisplay.h>

Fire :: Fire()
{
    int i,v;
    u32 *rawdata1;
	static BOOL init=0;

	if (init!=1){	//I know this is not needed but i was considering moving this to the draw loop so it doesn't run unless needed.
		  init=1;
		  memset(fireMemY,0,fireMaxX*fireMaxY);	//clear fire buffer
		  this->firetex = ya2d_create_texture (fireMaxX,fireMaxY,GU_PSM_8888,YA2D_PLACE_VRAM);//3
		  //Palette fill (OSLib / YA2D)
		  this->firePal[0]=0;//transparent where no fire
		  for (i=1;i<16;i++) this->firePal[i] = RGBA(0, 0, (i+1)*4, alpha);
		  for (i=16;i<80;i++)	this->firePal[i] = RGBA(((i-15)*4-1), 0, (79-i),alpha);
		  for (i=80;i<208;i++) this->firePal[i] = RGBA(255, ((i-79)*2)-1, 0, alpha);
		  for (i=208;i<256;i++) this->firePal[i] = RGBA(255, 255, (i-207)*5, alpha);

		  /*Palette fill (direct mem)
		  for (i=0;i<16;i++) firePal[i]=((i+1)*4)*0x10000L;
		  for (i=16;i<80;i++) firePal[i]=((79-i)*0x10000L)+((i-15)*4-1);
		  for (i=80;i<208;i++) firePal[i]=0x000000ffL+((((i-79)*2)-1)*0x100L);
		  for (i=208;i<256;i++) firePal[i]=(((i-207)*5)*0x10000L)+0x0000ffffL;
		  */
		  // bottom line draw, this makes the fire look better
		    rawdata1=(u32*)firetex->data+firetex->pow2_w*(fireMaxY-1);
		  for (v=0;v!=(fireMaxX);v++){
			  this->fireMemY[fireMaxY-1].fireMemX[v]=fireBaseColor;
			 *rawdata1=RGBA (fireBaseColor,fireBaseColor,fireBaseColor,alpha);
			 rawdata1++;
		  }

	}
};

Fire :: ~Fire()
{
	//todo here I should delete stuff
};

void Fire :: draw()
{
    int i,v,fx,fy;
    u32 *rawdata1;		//Pointer to write into texture
	static int fireFrameCount=0;
	//     for (v=0;v!=256;v++)drawPixel (v,10,1,10,firePal[v]); //Draw palette, debug only (direct mem)
	//     for (v=0;v!=256;v++)ya2d_draw_rect (v,10,1,10,firePal[v],1); //Draw palette, debug only (YA2D)
		 if (fireFrameCount==0) {
		     for (v=0;v!=fireMaxX;v++) this->fireMemY[fireMaxY-2].fireMemX[v]=(rand()%2*fireBaseColor);//Baseline new fire generator
			 for (fy=1;fy!=(fireMaxY-1);fy++){
				 rawdata1=(u32*)this->firetex->data + this->firetex->pow2_w*fy+1;
				 for (fx=1;fx!=(fireMaxX-1);fx++){
					 v=(this->fireMemY[fy].fireMemX[fx] + this->fireMemY[fy].fireMemX[fx-1] + this->fireMemY[fy].fireMemX[fx+1]
						+ this->fireMemY[fy+1].fireMemX[fx])/4;
					 v=v-fireDecay;
					 if (v<0) v=0;
					 this->fireMemY[fy-1].fireMemX[fx]=v;
					 *rawdata1=this->firePal[v];
					 rawdata1++;
	    		 }
	    	 }
	     }
		 if (fireScale==1)
			 ya2d_draw_texture(this->firetex, firePosX, firePosY);
		 else
			 ya2d_draw_texture_scale(this->firetex, firePosX, firePosY,fireScale,fireScale);

         //tinyfont_draw_stringf(10, 10,  GU_RGBA(0,0,255,255), "x:%d px:%d y:%d py:%d", fireMaxX, firetex->pow2_w,fireMaxY,firetex->pow2_h);

	     fireFrameCount++;
	     if (fireFrameCount==fireFrameSkip)fireFrameCount=0;


};
