#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <fcntl.h>

#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <png.h>

#include "psp_pg.h"
#include "psp_init.h"


 extern unsigned char psp_font[];

//constants
#define    PIXELSIZE  1        //in short
#define    LINESIZE  512        //in short
#define    FRAMESIZE  0x44000      //in byte

//480*272 = 60*38
#define CMAX_X 60
#define CMAX_Y 38

//variables
char *pg_vramtop=(char *)0x04000000;
long pg_screenmode;
long pg_showframe;
long pg_drawframe = 0;



void 
pgWaitVn(unsigned long count)
{
  for (; count>0; --count) {
    sceDisplayWaitVblankStart();
  }
}


void pgWaitV()
{
  sceDisplayWaitVblankStart();
}

char *
pgGetVramAddr(unsigned long x,unsigned long y)
{
  return pg_vramtop+(pg_drawframe?FRAMESIZE:0)+x*PIXELSIZE*2+y*LINESIZE*2+0x40000000;
}

static void 
user_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
{
	// ignore PNG warnings
}

void 
pgLoadBackgroundPng(const char* filename)
{
	/*
	u32* vram32;
	u16* vram16;
	int bufferwidth;
	int pixelformat;
	int unknown;
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned int sig_read = 0;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type, x, y;
	u32* line;
	FILE *fp;

	if ((fp = fopen(filename, "rb")) == NULL) return;
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fclose(fp);
		return;
	}
	png_set_error_fn(png_ptr, (png_voidp) NULL, (png_error_ptr) NULL, user_warning_fn);
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		fclose(fp);
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return;
	}
	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, sig_read);
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, int_p_NULL, int_p_NULL);
	png_set_strip_16(png_ptr);
	png_set_packing(png_ptr);
	if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png_ptr);
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_gray_1_2_4_to_8(png_ptr);
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);
	png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
	line = (u32*) malloc(width * 4);
	if (!line) {
		fclose(fp);
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return;
	}
	sceDisplayWaitVblankStart();  // if framebuf was set with PSP_DISPLAY_SETBUF_NEXTFRAME, wait until it is changed
	sceDisplayGetFrameBuf((void**)&vram32, &bufferwidth, &pixelformat, &unknown);

# if 1 //LDUO:
  vram32 = (u32 *)pgGetVramAddr(0,0);
# endif
	vram16 = (u16*) vram32;
	for (y = 0; y < height; y++) {
		png_read_row(png_ptr, (u8*) line, png_bytep_NULL);
		for (x = 0; x < width; x++) {
			u32 color32 = line[x];
			u16 color16;
			int r = color32 & 0xff;
			int g = (color32 >> 8) & 0xff;
			int b = (color32 >> 16) & 0xff;
			switch (pixelformat) {
				case PSP_DISPLAY_PIXEL_FORMAT_565:
					color16 = (r >> 3) | ((g >> 2) << 5) | ((b >> 3) << 11);
					vram16[x + y * bufferwidth] = color16;
					break;
				case PSP_DISPLAY_PIXEL_FORMAT_5551:
					color16 = (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10);
					vram16[x + y * bufferwidth] = color16;
					break;
				case PSP_DISPLAY_PIXEL_FORMAT_4444:
					color16 = (r >> 4) | ((g >> 4) << 4) | ((b >> 4) << 8);
					vram16[x + y * bufferwidth] = color16;
					break;
				case PSP_DISPLAY_PIXEL_FORMAT_8888:
					color32 = r | (g << 8) | (b << 16);
					vram32[x + y * bufferwidth] = color32;
					break;
			}
		}
	}
	free(line);
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
	fclose(fp);
	*/
}

void 
pgFlipShowFrameV()
{
  pgWaitV();
  pg_showframe=(pg_showframe?0:1);
  sceDisplaySetFrameBuf(pg_vramtop+(pg_showframe?FRAMESIZE:0),LINESIZE,PIXELSIZE,0);
}


void 
pgInit(void)
{
  char BackgroundFileName[128];
  strcpy(BackgroundFileName, psp_home_dir);
  strcat(BackgroundFileName,"/background.png");

  sceDisplaySetMode(0,PG_SCREEN_WIDTH,PG_SCREEN_HEIGHT);
  pgScreenFrame(2,1);
  pgLoadBackgroundPng(BackgroundFileName);
  pgFlipShowFrameV();

  /* we copy background image on the second frame */
  memcpy(pg_vramtop+FRAMESIZE,pg_vramtop,FRAMESIZE);
}

void
pgClear(void)
{
  pgWaitV();
  memcpy(pg_vramtop,pg_vramtop+FRAMESIZE,FRAMESIZE);
}

void 
pgFillvram(unsigned long color)
{
  unsigned char *vptr0;    //pointer to vram
  unsigned long i;

  vptr0=(unsigned char *)pgGetVramAddr(0,0);
  for (i=0; i<FRAMESIZE/2; i++) {
    *(unsigned short *)vptr0=color;
    vptr0+=PIXELSIZE*2;
  }
}

void 
pgBitBltTransp(unsigned long x,unsigned long y,unsigned long w,unsigned long
h,unsigned long mag,const unsigned short *d, unsigned short transparency) 
{

  unsigned char *vptr0;    //pointer to vram
  unsigned char *vptr;    //pointer to vram
  unsigned long xx,yy,mx,my;
  const unsigned short *dd;
  
  vptr0=(unsigned char *)pgGetVramAddr(x,y);

    for (yy=0; yy<h; yy++) {
      for (my=0; my<mag; my++) {
        vptr=vptr0;
        dd=d;
        for (xx=0; xx<w; xx++) {
    for (mx=0; mx<mag; mx++) {
      if (*dd != transparency) {
        *(unsigned short *)vptr=*dd;
      }
      vptr+=PIXELSIZE*2;
    }
    dd++;
        }
        vptr0+=LINESIZE*2;
      }
      d+=w;
    }

}


void 
pgBitBlt(unsigned long x,unsigned long y,unsigned long w,unsigned long h,unsigned long mag,const unsigned short *d)
{
  unsigned char *vptr0;    //pointer to vram
  unsigned char *vptr;    //pointer to vram
  unsigned long xx,yy,mx,my;
  const unsigned short *dd;
  
  vptr0=(unsigned char *)pgGetVramAddr(x,y);

  for (yy=0; yy<h; yy++) {
    for (my=0; my<mag; my++) {
      vptr=vptr0;
      dd=d;
      for (xx=0; xx<w; xx++) {
        for (mx=0; mx<mag; mx++) {
    *(unsigned short *)vptr=*dd;
    vptr+=PIXELSIZE*2;
        }
        dd++;
      }
      vptr0+=LINESIZE*2;
    }
    d+=w;
  }
}


void pgScreenFrame(long mode,long frame)
{
  pg_screenmode=mode;
  frame=(frame?1:0);
  pg_showframe=frame;
  if (mode==0) {
    //screen off
    pg_drawframe=frame;
    sceDisplaySetFrameBuf(0,0,0,1);
  } else if (mode==1) {
    //show/draw same
    pg_drawframe=frame;
    sceDisplaySetFrameBuf(pg_vramtop+(pg_showframe?FRAMESIZE:0),LINESIZE,PIXELSIZE,1);
  } else if (mode==2) {
    //show/draw different
    pg_drawframe=(frame?0:1);
    sceDisplaySetFrameBuf(pg_vramtop+(pg_showframe?FRAMESIZE:0),LINESIZE,PIXELSIZE,1);
  }
}


void pgScreenFlip()
{
  pg_showframe=(pg_showframe?0:1);
  pg_drawframe=(pg_drawframe?0:1);
  sceDisplaySetFrameBuf(pg_vramtop+(pg_showframe?FRAMESIZE:0),LINESIZE,PIXELSIZE,0);
}


void 
pgScreenFlipV()
{
  pgWaitV();
  pgScreenFlip();
}

void 
pgFlipDrawFrame()
{
  pg_drawframe=(pg_drawframe?0:1);
}

void 
pgDrawBox(int x, int y, int w, int h, int border, int color) 
{
  unsigned short *vram = (unsigned short*)pgGetVramAddr(x * 16, y * 16);
  unsigned short p;
  int xo, yo;
  for (xo = 0; xo <= w; xo++) {
    for (yo = 0; yo <= h; yo++) {
      if (xo == 0 || xo == w || yo == 0 || yo == h) {
        vram[xo + yo * LINESIZE] =  border;
      } else {
        p = vram[xo + yo * LINESIZE];
        vram[xo + yo * LINESIZE] =  color;
      }
    }
  }
}

void 
pgDrawRectangle(int x, int y, int w, int h, int border, int mode) 
{
  unsigned short *vram = (unsigned short*)pgGetVramAddr(x, y);
  int xo, yo;
  if (mode == PG_XOR) {
    for (xo = 0; xo < w; xo++) {
      vram[xo] ^=  border;
      vram[xo + h * LINESIZE] ^=  border;
    }
    for (yo = 0; yo < h; yo++) {
      vram[yo * LINESIZE] ^=  border;
      vram[w + yo * LINESIZE] ^=  border;
    }
  } else {
    for (xo = 0; xo < w; xo++) {
      vram[xo] =  border;
      vram[xo + h * LINESIZE] =  border;
    }
    for (yo = 0; yo < h; yo++) {
      vram[yo * LINESIZE] =  border;
      vram[w + yo * LINESIZE] =  border;
    }
  }
}

void 
pgFillRectangle(int x, int y, int w, int h, int color, int mode)
{
  unsigned short *vram = (unsigned short*)pgGetVramAddr(x, y);
  int xo, yo;
  if (mode == PG_XOR) {
    for (xo = 0; xo <= w; xo++) {
      for (yo = 0; yo <= h; yo++) {
        if ( ((xo == 0) && ((yo == 0) || (yo == h))) ||
             ((xo == w) && ((yo == 0) || (yo == h))) ) {
          /* Skip corner */
        } else {
          vram[xo + yo * LINESIZE] ^=  color;
        }
      }
    }
  } else {
    for (xo = 0; xo <= w; xo++) {
      for (yo = 0; yo <= h; yo++) {
        vram[xo + yo * LINESIZE] =  color;
      }
    }
  }
}

void 
pgPutPixel(int x, int y, int color) 
{
  unsigned short *vram = (unsigned short*)pgGetVramAddr(x, y);
  vram[0] = color;
}


void 
pgPutChar(int x, int y, int color, int bgcolor, char c, int drawfg, int drawbg)
{
  int cx;
  int cy;
  int b;
  int index;
  unsigned short *vram;

  vram  = (unsigned short*)pgGetVramAddr(x, y);
  index = ((unsigned int)c) * 8;

  for (cy=0; cy<8; cy++) {
    b=0x80;
    for (cx=0; cx<8; cx++) {
      if (psp_font[index] & b) {
        if (drawfg) vram[cx + cy * LINESIZE] = color;
      } else {
        if (drawbg) vram[cx + cy * LINESIZE] = bgcolor;
      }
      b = b >> 1;
    }
    index++;
  }
}

void 
pgFramePutChar(int x, int y, int color, char c)
{
  int cx;
  int cy;
  int b;
  int index;
  unsigned short *vram;
  unsigned short *vramf;

  vram  = (unsigned short*)pgGetVramAddr(x, y);
  pg_drawframe = ! pg_drawframe;
  vramf = (unsigned short*)pgGetVramAddr(x, y);
  pg_drawframe = ! pg_drawframe;

  index = ((unsigned int)c) * 8;

  for (cy=0; cy<8; cy++) {
    b=0x80;
    for (cx=0; cx<8; cx++) {
      if (psp_font[index] & b) {
        vram[cx + cy * LINESIZE] = color;
      } else {
        vram[cx + cy * LINESIZE] = vramf[cx + cy * LINESIZE];
      }
      b = b >> 1;
    }
    index++;
  }
}

void 
pgPrint(int x,int y,const char *str, int color)
{
  int index;
  int x0 = x;

  x *= 9;
  y *= 9;

  for (index = 0; str[index] != '\0'; index++) {
    pgPutChar(x, y, color, 0, str[index], 1, 0);
    x += 8;
    if (x >= (PG_SCREEN_WIDTH - 8)) {
      x = x0; y++;
    }
    if (y >= (PG_SCREEN_HEIGHT - 8)) break;
  }
}

void 
pgFramePrint(int x,int y,const char *str, int color)
{
  int index;
  int x0 = x;

  x *= 9;
  y *= 9;

  for (index = 0; str[index] != '\0'; index++) {
    pgFramePutChar(x, y, color, str[index]);
    x += 8;
    if (x >= (PG_SCREEN_WIDTH - 8)) {
      x = x0; y++;
    }
    if (y >= (PG_SCREEN_HEIGHT - 8)) break;
  }
}

void 
pgFillPrint(int x,int y,const char *str, int color, int bgcolor)
{
  int index;
  int x0 = x;

  for (index = 0; str[index] != '\0'; index++) {
    pgPutChar(x, y, color, bgcolor, str[index], 1, 1);
    x += 8;
    if (x >= (PG_SCREEN_WIDTH - 8)) {
      x = x0; y++;
    }
    if (y >= (PG_SCREEN_HEIGHT - 8)) break;
  }
}
