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

#include "stdafx.h"

#define _IN_XA
#include <stdint.h>

// will be included from spu.c
#ifdef _IN_SPU

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

int             iLeftXAVol  = 0x8000;
int             iRightXAVol = 0x8000;

static int gauss_ptr = 0;
static int gauss_window[8] = {0, 0, 0, 0, 0, 0, 0, 0};

#define gvall0 gauss_window[gauss_ptr]
#define gvall(x) gauss_window[(gauss_ptr+x)&3]
#define gvalr0 gauss_window[4+gauss_ptr]
#define gvalr(x) gauss_window[4+((gauss_ptr+x)&3)]

long cdxa_dbuf_ptr;

////////////////////////////////////////////////////////////////////////
// MIX XA & CDDA
////////////////////////////////////////////////////////////////////////

static int lastxa_lc, lastxa_rc;
static int lastcd_lc, lastcd_rc;

static INLINE void MixXA(void)
{
 int ns;
 int lc,rc;
 unsigned long cdda_l;
 int decoded_xa;
 int decoded_cdda;

 decoded_xa = decoded_ptr;

 lc = 0;
 rc = 0;

 for(ns=0;ns<NSSIZE && XAPlay!=XAFeed;ns++)
  {
	 XALastVal=*XAPlay++;
   if(XAPlay==XAEnd) XAPlay=XAStart;

	 lc = (short)(XALastVal&0xffff);
	 rc = (short)((XALastVal>>16) & 0xffff);


	 // improve crackle - buffer under
	 // - not update fast enough
	 lastxa_lc = lc;
	 lastxa_rc = rc;


	 // Tales of Phantasia - voice meter
	 spuMem[ (decoded_xa + 0x000)/2 ] = (short) lc;
	 spuMem[ (decoded_xa + 0x400)/2 ] = (short) rc;

	 decoded_xa += 2;
	 if( decoded_xa >= 0x400 )
		 decoded_xa = 0;

 
	 lc = CLAMP16( (lc * iLeftXAVol) / 0x8000 );
	 rc = CLAMP16( (rc * iRightXAVol) / 0x8000 );


	 // reverb write flag
	 if( spuCtrl & CTRL_CD_REVERB ) {
		StoreREVERB_CD( lc, rc, ns );
	 }


	 // play flag
	 if( spuCtrl & CTRL_CD_PLAY ) {
		 SSumL[ns]+=lc;
		 SSumR[ns]+=rc;
	 }
 }

 if(XAPlay==XAFeed && XARepeat)
  {
   //XARepeat--;
   for(;ns<NSSIZE;ns++)
    {
		 // improve crackle - buffer under
		 // - not update fast enough
		 lc = lastxa_lc;
		 rc = lastxa_rc;


		 // Tales of Phantasia - voice meter
		 spuMem[ (decoded_xa + 0x000)/2 ] = (short) lc;
		 spuMem[ (decoded_xa + 0x400)/2 ] = (short) rc;

		 decoded_xa += 2;
		 if( decoded_xa >= 0x400 )
			 decoded_xa = 0;


		 lc = CLAMP16( (lc * iLeftXAVol) / 0x8000 );
		 rc = CLAMP16( (rc * iRightXAVol) / 0x8000 );


		 // reverb write flags
		 if( spuCtrl & CTRL_CD_REVERB ) {
			StoreREVERB_CD( lc, rc, ns );
		 }


		 // play flag
		 if( spuCtrl & CTRL_CD_PLAY ) {
			 SSumL[ns]+=lc;
			 SSumR[ns]+=rc;
		 }
    }
  }



 decoded_cdda = decoded_ptr;

 for(ns=0;ns<NSSIZE && CDDAPlay!=CDDAFeed && (CDDAPlay!=CDDAEnd-1||CDDAFeed!=CDDAStart);ns++)
  {
   cdda_l=*CDDAPlay++;
   if(CDDAPlay==CDDAEnd) CDDAPlay=CDDAStart;

	 lc = (short)(cdda_l&0xffff);
	 rc = (short)((cdda_l>>16) & 0xffff);


	 // improve crackle - buffer under
	 // - not update fast enough
	 lastcd_lc = lc;
	 lastcd_rc = rc;


	 // Vib Ribbon - playback
	 spuMem[ (decoded_cdda + 0x000)/2 ] = (short) lc;
	 spuMem[ (decoded_cdda + 0x400)/2 ] = (short) rc;

	 decoded_cdda += 2;
	 if( decoded_cdda >= 0x400 )
		 decoded_cdda = 0;


	 // Rayman - stage end fadeout
	 lc = CLAMP16( (lc * iLeftXAVol) / 0x8000 );
	 rc = CLAMP16( (rc * iRightXAVol) / 0x8000 );


	 // reverb write flag
	 if( spuCtrl & CTRL_CD_REVERB ) {
		StoreREVERB_CD( lc, rc, ns );
	 }


	 // play flag
	 if( spuCtrl & CTRL_CD_PLAY ) {
		 SSumL[ns]+=lc;
		 SSumR[ns]+=rc;
	 }
	}


 if(CDDAPlay==CDDAFeed && XARepeat)
  {
   //XARepeat--;
   for(;ns<NSSIZE;ns++)
    {
		 // improve crackle - buffer under
		 // - not update fast enough
		 lc = lastcd_lc;
		 rc = lastcd_rc;


		 // Vib Ribbon - playback
		 spuMem[ (decoded_cdda + 0x000)/2 ] = (short) lc;
		 spuMem[ (decoded_cdda + 0x400)/2 ] = (short) rc;

		 decoded_cdda += 2;
		 if( decoded_cdda >= 0x400 )
			 decoded_cdda = 0;


		 // Rayman - stage end fadeout
		 lc = CLAMP16( (lc * iLeftXAVol) / 0x8000 );
		 rc = CLAMP16( (rc * iRightXAVol) / 0x8000 );


		 // reverb write flag
		 if( spuCtrl & CTRL_CD_REVERB ) {
			StoreREVERB_CD( lc, rc, ns );
		 }


		 // play flag
		 if( spuCtrl & CTRL_CD_PLAY ) {
			 SSumL[ns]+=lc;
			 SSumR[ns]+=rc;
		 }
	 }
  }
}

////////////////////////////////////////////////////////////////////////
// small linux time helper... only used for watchdog
////////////////////////////////////////////////////////////////////////

#ifndef _WINDOWS

unsigned long timeGetTime_spu()
{
 struct timeval tv;
 gettimeofday(&tv, 0);                                 // well, maybe there are better ways
 return tv.tv_sec * 1000 + tv.tv_usec/1000;            // to do that, but at least it works
}

#endif

////////////////////////////////////////////////////////////////////////
// FEED XA 
////////////////////////////////////////////////////////////////////////

static INLINE void FeedXA(xa_decode_t *xap)
{
 int sinc,spos,i,iSize,iPlace;

 if(!bSPUIsOpen) return;

 xapGlobal = xap;                                      // store info for save states
 XARepeat  = 100;                                      // set up repeat

#ifdef XA_HACK
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
   static DWORD dwLT=0;
   static DWORD dwFPS=0;
   static int   iFPSCnt=0;
   static int   iLastSize=0;
   static DWORD dwL1=0;
   DWORD dw=timeGetTime_spu(),dw1,dw2;

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
       while(spos>=0x10000L)
        {
         l = *pS++;
         spos -= 0x10000L;
        }

       s=(short)LOWORD(l);
       l1=s;
       l1=(l1*iPlace)/iSize;
       if(l1<-32767) l1=-32767;
       if(l1> 32767) l1=32767;
       s=(short)HIWORD(l);
       l2=s;
       l2=(l2*iPlace)/iSize;
       if(l2<-32767) l2=-32767;
       if(l2> 32767) l2=32767;
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
       while(spos>=0x10000L)
        {
         l = *pS++;
         spos -= 0x10000L;
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
       while(spos>=0x10000L)
        {
         s = *pS++;
         spos -= 0x10000L;
        }
       l1=s;

       l1=(l1*iPlace)/iSize;
       if(l1<-32767) l1=-32767;
       if(l1> 32767) l1=32767;
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
       while(spos>=0x10000L)
        {
         s = *pS++;
         spos -= 0x10000L;
        }
       l=s;

       *XAFeed++=((l&0xffff)|(l<<16));

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

unsigned int cdda_ptr;

static INLINE void FeedCDDA(unsigned char *pcm, int nBytes)
{
 while(nBytes>0)
  {
   if(CDDAFeed==CDDAEnd) CDDAFeed=CDDAStart;
   while(CDDAFeed==CDDAPlay-1||
         (CDDAFeed==CDDAEnd-1&&CDDAPlay==CDDAStart))
   {
    if (!iUseTimer) sceKernelDelayThread(1000);
    else return;
   }
   *CDDAFeed++=(*pcm | (*(pcm+1)<<8) | (*(pcm+2)<<16) | (*(pcm+3)<<24));
   nBytes-=4;
   pcm+=4;
 }
}

#endif
