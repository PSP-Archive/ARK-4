/***************************************************************************
                            xa.c  -  description
                             -------------------
    begin                : Wed May 15 2002
    copyright            : (C) 2002 by Pete Bernert
    email                : BlackDove@addcom.de
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version. See also the license.txt file for *
 *   additional informations.                                              *
 *                                                                         *
 ***************************************************************************/

// #include "stdafx.h"
#define _IN_XA
#include <string.h>
#include <stdint.h>
#include <sys/time.h>
#include <pspsdk.h>
#include <psprtc.h>

// will be included from spu.c
#ifdef _IN_SPU

#define LOWORD(v) (v & 0xFFFF)
#define HIWORD(v) ((v >> 16) & 0xFFFF)

////////////////////////////////////////////////////////////////////////
// XA GLOBALS
////////////////////////////////////////////////////////////////////////

xa_decode_t   * xapGlobal=0;

uint32_t * XAFeed  = NULL;
uint32_t * XAPlay  = NULL;
uint32_t * XAStart = NULL;
uint32_t * XAEnd   = NULL;

uint32_t   XARepeat  = 0;
uint32_t   XALastVal = 0;

uint32_t * CDDAFeed  = NULL;
uint32_t * CDDAPlay  = NULL;
uint32_t * CDDAStart = NULL;
uint32_t * CDDAEnd   = NULL;

int             iLeftXAVol  = 32767;
int             iRightXAVol = 32767;

static int gauss_ptr = 0;
static int gauss_window[8] = {0, 0, 0, 0, 0, 0, 0, 0};

#define gvall0 gauss_window[gauss_ptr]
#define gvall(x) gauss_window[(gauss_ptr+x)&3]
#define gvalr0 gauss_window[4+gauss_ptr]
#define gvalr(x) gauss_window[4+((gauss_ptr+x)&3)]

////////////////////////////////////////////////////////////////////////
// MIX XA & CDDA
////////////////////////////////////////////////////////////////////////

inline void MixXA(void)
{
 int ns;
 short l, r;
 uint32_t v;
 int cursor = decode_pos;

 if(XAPlay != XAFeed || XARepeat > 0)
 {
  if(XAPlay == XAFeed)
   XARepeat--;

  v = XALastVal;
  for(ns=0;ns<NSSIZE*2;)
   {
    if(XAPlay != XAFeed) v=*XAPlay++;
    if(XAPlay == XAEnd) XAPlay=XAStart;

    l = ((int)(short)v * iLeftXAVol) >> 15;
    r = ((int)(short)(v >> 16) * iLeftXAVol) >> 15;
    SSumLR[ns++] += l;
    SSumLR[ns++] += r;
    spuMem[cursor] = l;
    spuMem[cursor + 0x400/2] = r;
    cursor = (cursor + 1) & 0x1ff;
   }
  XALastVal = v;
 }

 for(ns=0;ns<NSSIZE*2 && CDDAPlay!=CDDAFeed && (CDDAPlay!=CDDAEnd-1||CDDAFeed!=CDDAStart);)
  {
   v=*CDDAPlay++;
   if(CDDAPlay==CDDAEnd) CDDAPlay=CDDAStart;

   l = ((int)(short)v * iLeftXAVol) >> 15;
   r = ((int)(short)(v >> 16) * iLeftXAVol) >> 15;
   SSumLR[ns++] += l;
   SSumLR[ns++] += r;
   spuMem[cursor] = l;
   spuMem[cursor + 0x400/2] = r;
   cursor = (cursor + 1) & 0x1ff;
  }
}

////////////////////////////////////////////////////////////////////////
// small linux time helper... only used for watchdog
////////////////////////////////////////////////////////////////////////

static unsigned long timeGetTime_spu()
{
	unsigned long long tick = 0;
	sceRtcGetCurrentTick(&tick);
	
	// return runtime milliseconds
	return (unsigned long)(tick / (sceRtcGetTickResolution() / 1000.0f));
}

////////////////////////////////////////////////////////////////////////
// FEED XA 
////////////////////////////////////////////////////////////////////////

inline void FeedXA(xa_decode_t *xap)
{
 int sinc,spos,i,iSize,iPlace,vl,vr;

 if(!bSPUIsOpen) return;

 xapGlobal = xap;                                      // store info for save states
 XARepeat  = 100;                                      // set up repeat

#if 0//def XA_HACK
 iSize=((45500*xap->nsamples)/xap->freq);              // get size
#else
 iSize=((44100*xap->nsamples)/xap->freq);              // get size
#endif
 if(!iSize) return;                                    // none? bye

 if(XAFeed<XAPlay) iPlace=XAPlay-XAFeed;               // how much space in my buf?
 else              iPlace=(XAEnd-XAFeed) + (XAPlay-XAStart);

 if(iPlace==0) return;                                 // no place at all

 //----------------------------------------------------//
 if(iXAPitch)                                          // pitch change option?
  {
   static unsigned int dwLT=0;
   static unsigned int dwFPS=0;
   static int   iFPSCnt=0;
   static int   iLastSize=0;
   static unsigned int dwL1=0;
   unsigned int dw=timeGetTime_spu(),dw1,dw2;

   iPlace=iSize;

   dwFPS+=dw-dwLT;iFPSCnt++;

   dwLT=dw;
                                       
   if(iFPSCnt>=10)
    {
     if(!dwFPS) dwFPS=1;
     dw1=1000000/dwFPS; 
     if(dw1>=(dwL1-100) && dw1<=(dwL1+100)) dw1=dwL1;
     else dwL1=dw1;
     dw2=(xap->freq*100/xap->nsamples);
     if((!dw1)||((dw2+100)>=dw1)) iLastSize=0;
     else
      {
       iLastSize=iSize*dw2/dw1;
       if(iLastSize>iPlace) iLastSize=iPlace;
       iSize=iLastSize;
      }
     iFPSCnt=0;dwFPS=0;
    }
   else
    {
     if(iLastSize) iSize=iLastSize;
    }
  }
 //----------------------------------------------------//

 spos=0x10000L;
 sinc = (xap->nsamples << 16) / iSize;                 // calc freq by num / size

 if(xap->stereo)
{
   uint32_t * pS=(uint32_t *)xap->pcm;
   uint32_t l=0;

   if(iXAPitch)
    {
     int32_t l1,l2;short s;
     for(i=0;i<iSize;i++)
      {
       if(iUseInterpolation==2) 
        {
         while(spos>=0x10000L)
          {
           l = *pS++;
           gauss_window[gauss_ptr] = (short)LOWORD(l);
           gauss_window[4+gauss_ptr] = (short)HIWORD(l);
           gauss_ptr = (gauss_ptr+1) & 3;
           spos -= 0x10000L;
          }
         vl = (spos >> 6) & ~3;
         vr=(gauss[vl]*gvall0)&~2047;
         vr+=(gauss[vl+1]*gvall(1))&~2047;
         vr+=(gauss[vl+2]*gvall(2))&~2047;
         vr+=(gauss[vl+3]*gvall(3))&~2047;
         l= (vr >> 11) & 0xffff;
         vr=(gauss[vl]*gvalr0)&~2047;
         vr+=(gauss[vl+1]*gvalr(1))&~2047;
         vr+=(gauss[vl+2]*gvalr(2))&~2047;
         vr+=(gauss[vl+3]*gvalr(3))&~2047;
         l |= vr << 5;
        }
       else
        {
         while(spos>=0x10000L)
          {
           l = *pS++;
           spos -= 0x10000L;
          }
        }

       s=(short)LOWORD(l);
       l1=s;
       l1=(l1*iPlace)/iSize;
       ssat32_to_16(l1);
       s=(short)HIWORD(l);
       l2=s;
       l2=(l2*iPlace)/iSize;
       ssat32_to_16(l2);
       l=(l1&0xffff)|(l2<<16);

       *XAFeed++=l;

       if(XAFeed==XAEnd) XAFeed=XAStart;
       if(XAFeed==XAPlay) 
        {
         if(XAPlay!=XAStart) XAFeed=XAPlay-1;
         break;
        }

       spos += sinc;
      }
    }
   else
    {
     for(i=0;i<iSize;i++)
      {
       if(iUseInterpolation==2) 
        {
         while(spos>=0x10000L)
          {
           l = *pS++;
           gauss_window[gauss_ptr] = (short)LOWORD(l);
           gauss_window[4+gauss_ptr] = (short)HIWORD(l);
           gauss_ptr = (gauss_ptr+1) & 3;
           spos -= 0x10000L;
          }
         vl = (spos >> 6) & ~3;
         vr=(gauss[vl]*gvall0)&~2047;
         vr+=(gauss[vl+1]*gvall(1))&~2047;
         vr+=(gauss[vl+2]*gvall(2))&~2047;
         vr+=(gauss[vl+3]*gvall(3))&~2047;
         l= (vr >> 11) & 0xffff;
         vr=(gauss[vl]*gvalr0)&~2047;
         vr+=(gauss[vl+1]*gvalr(1))&~2047;
         vr+=(gauss[vl+2]*gvalr(2))&~2047;
         vr+=(gauss[vl+3]*gvalr(3))&~2047;
         l |= vr << 5;
        }
       else
        {
         while(spos>=0x10000L)
          {
           l = *pS++;
           spos -= 0x10000L;
          }
        }

       *XAFeed++=l;

       if(XAFeed==XAEnd) XAFeed=XAStart;
       if(XAFeed==XAPlay) 
        {
         if(XAPlay!=XAStart) XAFeed=XAPlay-1;
         break;
        }

       spos += sinc;
      }
    }
  }
 else
  {
   unsigned short * pS=(unsigned short *)xap->pcm;
   uint32_t l;short s=0;

   if(iXAPitch)
    {
     int32_t l1;
     for(i=0;i<iSize;i++)
      {
       if(iUseInterpolation==2) 
        {
         while(spos>=0x10000L)
          {
           gauss_window[gauss_ptr] = (short)*pS++;
           gauss_ptr = (gauss_ptr+1) & 3;
           spos -= 0x10000L;
          }
         vl = (spos >> 6) & ~3;
         vr=(gauss[vl]*gvall0)&~2047;
         vr+=(gauss[vl+1]*gvall(1))&~2047;
         vr+=(gauss[vl+2]*gvall(2))&~2047;
         vr+=(gauss[vl+3]*gvall(3))&~2047;
         l1=s= vr >> 11;
         l1 &= 0xffff;
        }
       else
        {
         while(spos>=0x10000L)
          {
           s = *pS++;
           spos -= 0x10000L;
          }
         l1=s;
        }

       l1=(l1*iPlace)/iSize;
       ssat32_to_16(l1);
       l=(l1&0xffff)|(l1<<16);
       *XAFeed++=l;

       if(XAFeed==XAEnd) XAFeed=XAStart;
       if(XAFeed==XAPlay) 
        {
         if(XAPlay!=XAStart) XAFeed=XAPlay-1;
         break;
        }

       spos += sinc;
      }
    }
   else
    {
     for(i=0;i<iSize;i++)
      {
       if(iUseInterpolation==2) 
        {
         while(spos>=0x10000L)
          {
           gauss_window[gauss_ptr] = (short)*pS++;
           gauss_ptr = (gauss_ptr+1) & 3;
           spos -= 0x10000L;
          }
         vl = (spos >> 6) & ~3;
         vr=(gauss[vl]*gvall0)&~2047;
         vr+=(gauss[vl+1]*gvall(1))&~2047;
         vr+=(gauss[vl+2]*gvall(2))&~2047;
         vr+=(gauss[vl+3]*gvall(3))&~2047;
         l=s= vr >> 11;
        }
       else
        {
         while(spos>=0x10000L)
          {
           s = *pS++;
           spos -= 0x10000L;
          }
         l=s;
        }

       l &= 0xffff;
       *XAFeed++=(l|(l<<16));

       if(XAFeed==XAEnd) XAFeed=XAStart;
       if(XAFeed==XAPlay) 
        {
         if(XAPlay!=XAStart) XAFeed=XAPlay-1;
         break;
        }

       spos += sinc;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////
// FEED CDDA
////////////////////////////////////////////////////////////////////////

inline int FeedCDDA(unsigned char *pcm, int nBytes)
{
 int space;
 space=(CDDAPlay-CDDAFeed-1)*4 & (CDDA_BUFFER_SIZE - 1);
 if(space<nBytes)
  return 0x7761; // rearmed_wait

 while(nBytes>0)
  {
   if(CDDAFeed==CDDAEnd) CDDAFeed=CDDAStart;
   space=(CDDAPlay-CDDAFeed-1)*4 & (CDDA_BUFFER_SIZE - 1);
   if(CDDAFeed+space/4>CDDAEnd)
    space=(CDDAEnd-CDDAFeed)*4;
   if(space>nBytes)
    space=nBytes;

   memcpy(CDDAFeed,pcm,space);
   CDDAFeed+=space/4;
   nBytes-=space;
   pcm+=space;
  }

 return 0x676f; // rearmed_go
}

#endif
