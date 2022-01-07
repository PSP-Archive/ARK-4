#include "config/sdk.h"
#include "graphics/graphics.h"
#include "common/common.h"
//#include "common/rand.h"


// animations
#include "anim/snow.h"



extern unsigned long failedNID;

SceCtrlData gpaddata;

int Paused;



unsigned long colours[11];

int color, i;

void InitGlobals()
{
	colours[10] = 0x00000000L;
 	colours[1] = 0x00FFFFFFL;
 	colours[2] = 0x00FF00FFL;
 	colours[3] = 0x000000FFL;
 	colours[4] = 0x00FF0000L;
 	colours[5] = 0x0000FF00L;
 	colours[6] = 0x0000FFFFL;
 	colours[7] = 0x00FFFF00L;
 	colours[8] = 0x00FFFF88L;
 	colours[9] = 0x00888888L;
 	colours[0] = 0x00008800L;
	
 	i=0;
 	color=INDEX_GRAY;
}

void ProcessKeys(unsigned long xikeys)
{
  if (xikeys & PSP_CTRL_UP || xikeys & PSP_CTRL_RIGHT) {
             if(color>=10) { color=0; }
             else { color++; }
  }
  if (xikeys & PSP_CTRL_DOWN || xikeys & PSP_CTRL_LEFT) {
             if(color<0) { color=10; }
             else { color--; }
  }
  if (xikeys & PSP_CTRL_HOME) // if home is pressed
  {
             for(i=0;i<278;i++) {     
                                Fillvram(0x00FFFFFFL);               
                                changeBuffer();                                        // flip screen to display what i just wrote
                                Sleep(10);                                           // delay for 1.5 seconds
             }
             changeBuffer();                                        // flip the buffer back to drawing mode for fade* functions
             fadeIn();                                              // fadeIn  -----|  Nice Cute
             fadeOut();                                             // fadeOut -----|    Effect
		   sceKernelExitGame();                             // exit
  }

  if (xikeys & PSP_CTRL_TRIANGLE) createScreenshot("ms0:/screen.bmp");
}

RECT pixel;
void drawPixel(int x, int y, int sizex, int sizey, unsigned long color) {
     pixel.top = y;
     pixel.left = x;
     pixel.bottom = y+sizey;
     pixel.right = x+sizex;
     
     FillRect(&pixel, color);
}


snoflake snowflake[100];

void _start(unsigned long, unsigned long *) __attribute__ ((section (".text.start")));
void _start(unsigned long arglen, unsigned long *argp)
{
   
	sceKernelDcacheWritebackAll();

  InitGlobals();

  initScreenAndCtrl();

  fadeIn();
  fadeOut();

  Print (22,15,colours[INDEX_GREEN]," - Hello World! -");
  changeBuffer();
  Sleep(1000);
  changeBuffer();

  fadeIn();
  fadeOut();

  initSnow(snowflake);


  for(;;)
  {
     gpaddata.Buttons = 0;

     sceCtrlReadBufferPositive(&gpaddata,1);

     ProcessKeys(gpaddata.Buttons);
    
     Fillvram(colours[INDEX_BLACK]);    

     updateSnow(snowflake, colours, color);

     drawPixel(0,247,480,272,colours[INDEX_GRAY]); // ground
     Print (0,0,colours[INDEX_GRAY],"Hello World by Team OILIX");

     changeBuffer();
     //Sleep(40);
  }

}


