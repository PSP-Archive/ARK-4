#include "../config/sdk.h"
#include "graphics.h"
#include "font.h"


// get vram address for character position
unsigned char *GetVramAddr(unsigned long x,unsigned long y)
{
	return vramtop+(drawframe?FRAMESIZE:0)+x*4+y*LINESIZE*4+0x40000000;
}

// print a single character
void PutChar(unsigned long x,unsigned long y,unsigned long color,unsigned long bgcolor,unsigned char ch,char drawfg,char drawbg,char mag)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned char *vptr;		//pointer to vram
	const unsigned char *cfont;		//pointer to font
	unsigned long cx,cy;
	unsigned long b;
	char mx,my;

	cfont=font+ch*8;

	vptr0=GetVramAddr(x,y);

	for (cy=0; cy<8; cy++)
	{
		for (my=0; my<mag; my++)
		{
			vptr=vptr0;
			b=0x80;
			for (cx=0; cx<8; cx++)
			{
				for (mx=0; mx<mag; mx++)
				{
					if ((*cfont&b)!=0)
					{
						if (drawfg)
            {
              *(unsigned long *)vptr=color;
            }
					}
          else
          {
						if (drawbg)
            {
              *(unsigned long *)vptr=bgcolor;
            }
					}
					vptr+=4;
				}
				b=b>>1;
			}
			vptr0+=LINESIZE*4; // 2
		}
		cfont++;
	}
}


// print a string
void Print(unsigned long x,unsigned long y,unsigned long color,const char *str)
{
	while (*str!=0 && x<CMAX_X && y<CMAX_Y) {
		PutChar(x*8,y*8,color,0,*str,1,1,1);
		str++;
		x++;
		if (x>=CMAX_X) {
			x=0;
			y++;
		}
	}
}

// x is in pixels rather than character cells
void PrintFine(unsigned long x,unsigned long y,unsigned long color,const char *str)
{
	while (*str!=0 && x<(CMAX_X*8) && y<CMAX_Y) {
		PutChar(x,y*8,color,0,*str,1,1,1);
		str++;
		x+=8;
		if (x>=(CMAX_X*8)) {
			x=0;
			y++;
		}
	}
}

// clear video ram
void Fillvram(unsigned long color)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned long i;

	vptr0=GetVramAddr(0,0);

	for (i=0; i<(FRAMESIZE/4); i++)
	{
		*(unsigned long *)vptr0=color;
		vptr0+=4;
	}
}

void WriteNibble(int x, int y, unsigned char val)
{
  if (val > 9)
  {
    val = 'A' + val - 10;
  }
  else
  {
    val = '0' + val;
  }
	PutChar(x*8,y*8,TEXTCOLOUR,0,val,1,1,1);
}

void WriteByte(int x, int y, unsigned char val)
{
  char lchr;

  lchr = (val & 0xF0) >> 4;
  WriteNibble(x,y,lchr);
  lchr = (val & 0xF);
  WriteNibble(x+1,y,lchr);
}

void WriteDword(int x, int y, unsigned long val)
{
  WriteByte(x,y,(val & 0xFF000000L) >> 24);
  WriteByte(x+2,y,(val & 0xFF0000L) >> 16);
  WriteByte(x+4,y,(val & 0xFF00L) >> 8);
  WriteByte(x+6,y,(val & 0xFFL));
}

void WriteNibbleDummy(int x, int y, unsigned char val)
{
  if (val > 9)
  {
    val = 'A' + val - 10;
  }
  else
  {
    val = '0' + val;
  }
	PutChar(x*8,y*8,0,0,val,1,1,1);
}

void WriteByteDummy(int x, int y, unsigned char val)
{
  char lchr;

  lchr = (val & 0xF0) >> 4;
  WriteNibbleDummy(x,y,lchr);
  lchr = (val & 0xF);
  WriteNibbleDummy(x+1,y,lchr);
}

void WriteDwordDummy(int x, int y, unsigned long val)
{
  WriteByteDummy(x,y,(val & 0xFF000000L) >> 24);
  WriteByteDummy(x+2,y,(val & 0xFF0000L) >> 16);
  WriteByteDummy(x+4,y,(val & 0xFF00L) >> 8);
  WriteByteDummy(x+6,y,(val & 0xFFL));
}

void FillRect(RECT *xirect, unsigned long xicolour)
{
  unsigned long *vptrl;
  unsigned char *vptrc;
  int      y;

  for (y = xirect->top; y < xirect->bottom; y++)
  {
    int x = xirect->left;
    vptrc = GetVramAddr(x,y);
    vptrl = (unsigned long*)vptrc;

    while (x < xirect->right)
    {
      *vptrl++ = xicolour;
      x++;
    }
  }
}

void MoveToEx(short x, short y)
{
  gcursor.x = x;
  gcursor.y = y;
}

void LineTo(short x, short y)
{
  // XXX to do
}

void changeBuffer()
{
  sceDisplayWaitVblankStart();

	sceDisplaySetFrameBuf(
      (void*)GetVramAddr(0,0),
      512,
    	PSP_DISPLAY_PIXEL_FORMAT_8888
      ,0);

	drawframe=(drawframe?0:1);
}

void DebugValue(int y, char * label, unsigned long val)
{
  Print(0,y,TEXTCOLOUR,label);
  WriteDword(10,y,val);
}

void WriteDecimal(int x, int y, unsigned long val)
{
  unsigned long divisor = 10000000L;
  unsigned char nibble;

  val = val % 100000000L;

  while (divisor > 0)
  {
    nibble = val / divisor;

    WriteNibble(x, y, nibble);

    val = val - (nibble * divisor);
    x++;
    divisor /= 10;
  }
}

void initScreenAndCtrl()
{
  drawframe = 0;
	sceDisplaySetMode(0,SCREEN_WIDTH,SCREEN_HEIGHT);

	sceDisplaySetFrameBuf(
      (void*)vramtop,
      512,
    	PSP_DISPLAY_PIXEL_FORMAT_8888
      ,1);
	Fillvram(0x00000000);
  drawframe = 1;
	Fillvram(0x00000000);
  sceCtrlSetSamplingMode(1);
}


/*****************************************************************************/
/* Screenshot code borrowed from PSPSokoban: Niklas Nordebo                  */
/*****************************************************************************/
#define PIXELSIZE 4
void createScreenshot(char* savePath) {

  unsigned char  bmpHeader24[] = { 0x42, 0x4d, 0x38, 0xfa, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0xe0, 0x01, 0x00, 0x00, 0x10, 0x01, 0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x0b, 0x00, 0x00, 0x12, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  unsigned char  buffer[SCREEN_WIDTH*3];
  unsigned char  r,g,b;
  int bufferIndex = 0;
  unsigned long p;
  int file;
  unsigned char *vptr;
  unsigned char *vptr0;
  int i=0;
  int e=0;

  file = sceIoOpen(savePath,PSP_O_CREAT | PSP_O_TRUNC | PSP_O_RDWR, 0777);

  // write bmp header
  sceIoWrite(file,bmpHeader24,54);

  // write bmp data
  vptr0 = GetVramAddr(0,271);

  for(i=0; i<SCREEN_HEIGHT; i++)
    {
      vptr=vptr0;
      for(e=0; e<480; e++)
	{
	  p = *(unsigned long *)vptr;
	  r = (unsigned char)(p & 0x000000FFL);
	  g = (unsigned char)((p & 0x0000FF00L) >> 8);
	  b = (unsigned char)((p & 0x00FF0000L) >> 16);

	  buffer[bufferIndex] = b;
	  bufferIndex++;
	  buffer[bufferIndex] = g;
	  bufferIndex++;
	  buffer[bufferIndex] = r;
	  bufferIndex++;

	  vptr+=PIXELSIZE;
	}
      // write chunk
      sceIoWrite(file,buffer,480*3);
      bufferIndex=0;
      vptr0-=LINESIZE*4;
    }

  // bmp end
  unsigned char end[] = { 0x00, 0x00 };
  sceIoWrite(file,end,2);

  sceIoClose(file);

}
