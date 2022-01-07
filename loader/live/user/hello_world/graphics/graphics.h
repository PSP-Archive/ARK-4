#ifndef GRAPHICS_H
#define GRAPHICS_H


#define SCREEN_HEIGHT 272
#define SCREEN_WIDTH  512
#define CMAX_X 60
#define CMAX_Y 38
#define LINESIZE 	512				//in long
#define FRAMESIZE  0xAA000			//in byte

#define TEXTCOLOUR 0x00FFFFFFL
#define BLACK      0x00000000L
#define BLUE       0x00FF0000L
#define RED        0x000000FFL
#define GREEN      0x0000FF00L


#define UINT unsigned int

typedef struct {
  UINT left;
  UINT right;
  UINT top;
  UINT bottom;
} RECT;

typedef struct {
  UINT cx;
  UINT cy;
} SIZE;

typedef struct {
  UINT x;
  UINT y;
} POINT;


unsigned long drawframe;
POINT gcursor;


static unsigned char *vramtop=(unsigned char *)0x04000000;

// get vram address for character position
unsigned char *GetVramAddr(unsigned long x,unsigned long y);

// print a single character
void PutChar(unsigned long x,unsigned long y,unsigned long color,unsigned long bgcolor,unsigned char ch,char drawfg,char drawbg,char mag);

// print a string
void Print(unsigned long x,unsigned long y,unsigned long color,const char *str);

// x is in pixels rather than character cells
void PrintFine(unsigned long x,unsigned long y,unsigned long color,const char *str);

// clear video ram
void Fillvram(unsigned long color);

void WriteNibble(int x, int y, unsigned char val);

void WriteByte(int x, int y, unsigned char val);

void WriteDword(int x, int y, unsigned long val);

void WriteNibbleDummy(int x, int y, unsigned char val);

void WriteByteDummy(int x, int y, unsigned char val);

void WriteDwordDummy(int x, int y, unsigned long val);

void FillRect(RECT *xirect, unsigned long xicolour);

void MoveToEx(short x, short y);

void LineTo(short x, short y);

void changeBuffer();

void DebugValue(int y, char * label, unsigned long val);

void WriteDecimal(int x, int y, unsigned long val);

void initScreenAndCtrl();

void createScreenshot(char* savePath);

#endif
