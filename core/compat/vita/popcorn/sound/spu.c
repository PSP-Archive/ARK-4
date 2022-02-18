/***************************************************************************
                            spu.c  -  description
                             -------------------
    begin                : Wed May 15 2002
    copyright            : (C) 2002 by Pete Bernert
    email                : BlackDove@addcom.de

 Portions (C) Gražvydas "notaz" Ignotas, 2010-2012

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

#define _IN_SPU

#include "externals.h"
#include "registers.h"
#include "out.h"

#ifdef ENABLE_NLS
#include <libintl.h>
#include <locale.h>
#define _(x)  gettext(x)
#define N_(x) (x)
#else
#define _(x)  (x)
#define N_(x) (x)
#endif

#ifdef __ARM_ARCH_7A__
 #define ssat32_to_16(v) \
  asm("ssat %0,#16,%1" : "=r" (v) : "r" (v))
#else
 #define ssat32_to_16(v) do { \
  if (v < -32768) v = -32768; \
  else if (v > 32767) v = 32767; \
 } while (0)
#endif

#define PSXCLK	33868800	/* 33.8688 MHz */

/*
#if defined (USEMACOSX)
static char * libraryName     = N_("Mac OS X Sound");
#elif defined (USEALSA)
static char * libraryName     = N_("ALSA Sound");
#elif defined (USEOSS)
static char * libraryName     = N_("OSS Sound");
#elif defined (USESDL)
static char * libraryName     = N_("SDL Sound");
#elif defined (USEPULSEAUDIO)
static char * libraryName     = N_("PulseAudio Sound");
#else
static char * libraryName     = N_("NULL Sound");
#endif

static char * libraryInfo     = N_("P.E.Op.S. Sound Driver V1.7\nCoded by Pete Bernert and the P.E.Op.S. team\n");
*/

// globals

// psx buffer / addresses

unsigned short * regArea = (unsigned short *)PSP_SPU_REGISTER;
unsigned short  spuMem[256*1024];
unsigned char * spuMemC;
unsigned char * pSpuIrq=0;
unsigned char * pSpuBuffer;

// user settings

int             iVolume=768; // 1024 is 1.0
int             iXAPitch=1;
int             iUseReverb=2;
int             iUseInterpolation=2;

// MAIN infos struct for each channel

SPUCHAN         s_chan[MAXCHAN+1];                     // channel + 1 infos (1 is security for fmod handling)
REVERBInfo      rvb;

unsigned int    dwNoiseVal;                            // global noise generator
unsigned int    dwNoiseCount;

unsigned short  spuCtrl=0;                             // some vars to store psx reg infos
unsigned short  spuStat=0;
unsigned short  spuIrq=0;
unsigned long   spuAddr=0xffffffff;                    // address into spu mem
int             bSpuInit=0;
int             bSPUIsOpen=0;

unsigned int dwNewChannel=0;                           // flags for faster testing, if new channel starts
unsigned int dwChannelOn=0;                            // not silent channels
unsigned int dwPendingChanOff=0;
unsigned int dwChannelDead=0;                          // silent+not useful channels

void (*irqCallback)(void)=0;                  // func of main emu, called on spu irq
void (*cddavCallback)(unsigned short,unsigned short)=0;

// certain globals (were local before, but with the new timeproc I need em global)

static const int f[8][2] = {   {    0,  0  },
                        {   60,  0  },
                        {  115, -52 },
                        {   98, -55 },
                        {  122, -60 } };
int ChanBuf[NSSIZE+3];
int SSumLR[(NSSIZE+3)*2];
int iFMod[NSSIZE];
int iCycle = 0;
short * pS;

static int decode_dirty_ch;
int decode_pos;
int had_dma;
int lastch=-1;             // last channel processed on spu irq in timer mode
static int lastns=0;       // last ns pos
static int cycles_since_update;

#define CDDA_BUFFER_SIZE (16384 * sizeof(uint32_t)) // must be power of 2

////////////////////////////////////////////////////////////////////////
// CODE AREA
////////////////////////////////////////////////////////////////////////

// dirty inline func includes

#include "reverb.c"
#include "adsr.c"

////////////////////////////////////////////////////////////////////////
// helpers for simple interpolation

//
// easy interpolation on upsampling, no special filter, just "Pete's common sense" tm
//
// instead of having n equal sample values in a row like:
//       ____
//           |____
//
// we compare the current delta change with the next delta change.
//
// if curr_delta is positive,
//
//  - and next delta is smaller (or changing direction):
//         \.
//          -__
//
//  - and next delta significant (at least twice) bigger:
//         --_
//            \.
//
//  - and next delta is nearly same:
//          \.
//           \.
//
//
// if curr_delta is negative,
//
//  - and next delta is smaller (or changing direction):
//          _--
//         /
//
//  - and next delta significant (at least twice) bigger:
//            /
//         __- 
//
//  - and next delta is nearly same:
//           /
//          /
//


inline void InterpolateUp(int ch)
{
 if(s_chan[ch].SB[32]==1)                              // flag == 1? calc step and set flag... and don't change the value in this pass
  {
   const int id1=s_chan[ch].SB[30]-s_chan[ch].SB[29];  // curr delta to next val
   const int id2=s_chan[ch].SB[31]-s_chan[ch].SB[30];  // and next delta to next-next val :)

   s_chan[ch].SB[32]=0;

   if(id1>0)                                           // curr delta positive
    {
     if(id2<id1)
      {s_chan[ch].SB[28]=id1;s_chan[ch].SB[32]=2;}
     else
     if(id2<(id1<<1))
      s_chan[ch].SB[28]=(id1*s_chan[ch].sinc)/0x10000L;
     else
      s_chan[ch].SB[28]=(id1*s_chan[ch].sinc)/0x20000L; 
    }
   else                                                // curr delta negative
    {
     if(id2>id1)
      {s_chan[ch].SB[28]=id1;s_chan[ch].SB[32]=2;}
     else
     if(id2>(id1<<1))
      s_chan[ch].SB[28]=(id1*s_chan[ch].sinc)/0x10000L;
     else
      s_chan[ch].SB[28]=(id1*s_chan[ch].sinc)/0x20000L; 
    }
  }
 else
 if(s_chan[ch].SB[32]==2)                              // flag 1: calc step and set flag... and don't change the value in this pass
  {
   s_chan[ch].SB[32]=0;

   s_chan[ch].SB[28]=(s_chan[ch].SB[28]*s_chan[ch].sinc)/0x20000L;
   //if(s_chan[ch].sinc<=0x8000)
   //     s_chan[ch].SB[29]=s_chan[ch].SB[30]-(s_chan[ch].SB[28]*((0x10000/s_chan[ch].sinc)-1));
   //else
   s_chan[ch].SB[29]+=s_chan[ch].SB[28];
  }
 else                                                  // no flags? add bigger val (if possible), calc smaller step, set flag1
  s_chan[ch].SB[29]+=s_chan[ch].SB[28];
}

//
// even easier interpolation on downsampling, also no special filter, again just "Pete's common sense" tm
//

inline void InterpolateDown(int ch)
{
 if(s_chan[ch].sinc>=0x20000L)                                 // we would skip at least one val?
  {
   s_chan[ch].SB[29]+=(s_chan[ch].SB[30]-s_chan[ch].SB[29])/2; // add easy weight
   if(s_chan[ch].sinc>=0x30000L)                               // we would skip even more vals?
    s_chan[ch].SB[29]+=(s_chan[ch].SB[31]-s_chan[ch].SB[30])/2;// add additional next weight
  }
}

////////////////////////////////////////////////////////////////////////
// helpers for gauss interpolation

#define gval0 (((short*)(&s_chan[ch].SB[29]))[gpos])
#define gval(x) ((int)((short*)(&s_chan[ch].SB[29]))[(gpos+x)&3])

#include "gauss_i.h"

////////////////////////////////////////////////////////////////////////

#include "xa.c"

static void do_irq(void)
{
 //if(!(spuStat & STAT_IRQ))
 {
  spuStat |= STAT_IRQ;                                 // asserted status?
  if(irqCallback) irqCallback();
 }
}

static int check_irq(int ch, unsigned char *pos)
{
 if((spuCtrl & CTRL_IRQ) && pos == pSpuIrq)
 {
  //printf("ch%d irq %04x\n", ch, pos - spuMemC);
  do_irq();
  return 1;
 }
 return 0;
}

////////////////////////////////////////////////////////////////////////
// START SOUND... called by main thread to setup a new sound on a channel
////////////////////////////////////////////////////////////////////////

inline void StartSound(int ch)
{
 StartADSR(ch);
 StartREVERB(ch);

 // fussy timing issues - do in VoiceOn
 //s_chan[ch].pCurr=s_chan[ch].pStart;                   // set sample start
 //s_chan[ch].bStop=0;
 //s_chan[ch].bOn=1;

 s_chan[ch].SB[26]=0;                                  // init mixing vars
 s_chan[ch].SB[27]=0;
 s_chan[ch].iSBPos=28;

 s_chan[ch].SB[29]=0;                                  // init our interpolation helpers
 s_chan[ch].SB[30]=0;

 if(iUseInterpolation>=2)                              // gauss interpolation?
      {s_chan[ch].spos=0x30000L;s_chan[ch].SB[28]=0;}  // -> start with more decoding
 else {s_chan[ch].spos=0x10000L;s_chan[ch].SB[31]=0;}  // -> no/simple interpolation starts with one 44100 decoding

 dwNewChannel&=~(1<<ch);                               // clear new channel bit
}

////////////////////////////////////////////////////////////////////////
// ALL KIND OF HELPERS
////////////////////////////////////////////////////////////////////////

inline int FModChangeFrequency(int ch,int ns)
{
 unsigned int NP=s_chan[ch].iRawPitch;
 int sinc;

 NP=((32768L+iFMod[ns])*NP)/32768L;

 if(NP>0x3fff) NP=0x3fff;
 if(NP<0x1)    NP=0x1;

 sinc=NP<<4;                                           // calc frequency
 if(iUseInterpolation==1)                              // freq change in simple interpolation mode
  s_chan[ch].SB[32]=1;
 iFMod[ns]=0;

 return sinc;
}                    

////////////////////////////////////////////////////////////////////////

inline void StoreInterpolationVal(int ch,int fa)
{
 if(s_chan[ch].bFMod==2)                               // fmod freq channel
  s_chan[ch].SB[29]=fa;
 else
  {
   ssat32_to_16(fa);

   if(iUseInterpolation>=2)                            // gauss/cubic interpolation
    {     
     int gpos = s_chan[ch].SB[28];
     gval0 = fa;          
     gpos = (gpos+1) & 3;
     s_chan[ch].SB[28] = gpos;
    }
   else
   if(iUseInterpolation==1)                            // simple interpolation
    {
     s_chan[ch].SB[28] = 0;                    
     s_chan[ch].SB[29] = s_chan[ch].SB[30];            // -> helpers for simple linear interpolation: delay real val for two slots, and calc the two deltas, for a 'look at the future behaviour'
     s_chan[ch].SB[30] = s_chan[ch].SB[31];
     s_chan[ch].SB[31] = fa;
     s_chan[ch].SB[32] = 1;                            // -> flag: calc new interolation
    }
   else s_chan[ch].SB[29]=fa;                          // no interpolation
  }
}

////////////////////////////////////////////////////////////////////////

inline int iGetInterpolationVal(int ch, int spos)
{
 int fa;

 if(s_chan[ch].bFMod==2) return s_chan[ch].SB[29];

 switch(iUseInterpolation)
  {   
   //--------------------------------------------------//
   case 3:                                             // cubic interpolation
    {
     long xd;int gpos;
     xd = (spos >> 1)+1;
     gpos = s_chan[ch].SB[28];

     fa  = gval(3) - 3*gval(2) + 3*gval(1) - gval0;
     fa *= (xd - (2<<15)) / 6;
     fa >>= 15;
     fa += gval(2) - gval(1) - gval(1) + gval0;
     fa *= (xd - (1<<15)) >> 1;
     fa >>= 15;
     fa += gval(1) - gval0;
     fa *= xd;
     fa >>= 15;
     fa = fa + gval0;

    } break;
   //--------------------------------------------------//
   case 2:                                             // gauss interpolation
    {
     int vl, vr;int gpos;
     vl = (spos >> 6) & ~3;
     gpos = s_chan[ch].SB[28];
     vr=(gauss[vl]*(int)gval0)&~2047;
     vr+=(gauss[vl+1]*gval(1))&~2047;
     vr+=(gauss[vl+2]*gval(2))&~2047;
     vr+=(gauss[vl+3]*gval(3))&~2047;
     fa = vr>>11;
    } break;
   //--------------------------------------------------//
   case 1:                                             // simple interpolation
    {
     if(s_chan[ch].sinc<0x10000L)                      // -> upsampling?
          InterpolateUp(ch);                           // --> interpolate up
     else InterpolateDown(ch);                         // --> else down
     fa=s_chan[ch].SB[29];
    } break;
   //--------------------------------------------------//
   default:                                            // no interpolation
    {
     fa=s_chan[ch].SB[29];                  
    } break;
   //--------------------------------------------------//
  }

 return fa;
}

static void decode_block_data(int *dest, const unsigned char *src, int predict_nr, int shift_factor)
{
 int nSample;
 int fa, s_1, s_2, d, s;

 s_1 = dest[27];
 s_2 = dest[26];

 for (nSample = 0; nSample < 28; src++)
 {
  d = (int)*src;
  s = (int)(signed short)((d & 0x0f) << 12);

  fa = s >> shift_factor;
  fa += ((s_1 * f[predict_nr][0])>>6) + ((s_2 * f[predict_nr][1])>>6);
  s_2=s_1;s_1=fa;

  dest[nSample++] = fa;

  s = (int)(signed short)((d & 0xf0) << 8);
  fa = s >> shift_factor;
  fa += ((s_1 * f[predict_nr][0])>>6) + ((s_2 * f[predict_nr][1])>>6);
  s_2=s_1;s_1=fa;

  dest[nSample++] = fa;
 }
}

static int decode_block(int ch)
{
 unsigned char *start;
 int predict_nr,shift_factor,flags;
 int ret = 0;

 start=s_chan[ch].pCurr;                   // set up the current pos

 if(s_chan[ch].prevflags&1)                // 1: stop/loop
 {
  if(!(s_chan[ch].prevflags&2))
  {
   dwChannelOn&=~(1<<ch);                  // -> turn everything off
   s_chan[ch].bStop=1;
   s_chan[ch].ADSRX.EnvelopeVol=0;
  }

  start = s_chan[ch].pLoop;
 }
 else
  ret = check_irq(ch, start);              // hack, see check_irq below..

 predict_nr=(int)start[0];
 shift_factor=predict_nr&0xf;
 predict_nr >>= 4;

 decode_block_data(s_chan[ch].SB, start + 2, predict_nr, shift_factor);

 flags=(int)start[1];
 if(flags&4)
  s_chan[ch].pLoop=start;                  // loop adress

 start+=16;

 if(flags&1) {                             // 1: stop/loop
  start = s_chan[ch].pLoop;
  ret |= check_irq(ch, start);             // hack.. :(
 }

 if (start - spuMemC >= 0x80000)
  start = spuMemC;

 s_chan[ch].pCurr = start;                 // store values for next cycle
 s_chan[ch].prevflags = flags;

 return ret;
}

// do block, but ignore sample data
static int skip_block(int ch)
{
 unsigned char *start = s_chan[ch].pCurr;
 int flags = start[1];
 int ret = check_irq(ch, start);

 if(s_chan[ch].prevflags & 1)
  start = s_chan[ch].pLoop;

 if(flags & 4)
  s_chan[ch].pLoop = start;

 start += 16;

 if(flags & 1)
  start = s_chan[ch].pLoop;

 s_chan[ch].pCurr = start;
 s_chan[ch].prevflags = flags;

 return ret;
}

#define make_do_samples(name, fmod_code, interp_start, interp1_code, interp2_code, interp_end) \
static int do_samples_##name(int ch, int ns, int ns_to) \
{                                            \
 int sinc = s_chan[ch].sinc;                 \
 int spos = s_chan[ch].spos;                 \
 int sbpos = s_chan[ch].iSBPos;              \
 int *SB = s_chan[ch].SB;                    \
 int ret = -1;                               \
 int d, fa;                                  \
 interp_start;                               \
                                             \
 for (; ns < ns_to; ns++)                    \
 {                                           \
  fmod_code;                                 \
                                             \
  while (spos >= 0x10000)                    \
  {                                          \
   if(sbpos == 28)                           \
   {                                         \
    sbpos = 0;                               \
    d = decode_block(ch);                    \
    if(d)                                    \
     ret = ns_to = ns + 1;                   \
   }                                         \
                                             \
   fa = SB[sbpos++];                         \
   interp1_code;                             \
   spos -= 0x10000;                          \
  }                                          \
                                             \
  interp2_code;                              \
  spos += sinc;                              \
 }                                           \
                                             \
 s_chan[ch].sinc = sinc;                     \
 s_chan[ch].spos = spos;                     \
 s_chan[ch].iSBPos = sbpos;                  \
 interp_end;                                 \
                                             \
 return ret;                                 \
}

#define fmod_recv_check \
  if(s_chan[ch].bFMod==1 && iFMod[ns]) \
    sinc = FModChangeFrequency(ch,ns)

make_do_samples(default, fmod_recv_check, ,
  StoreInterpolationVal(ch, fa),
  ChanBuf[ns] = iGetInterpolationVal(ch, spos), )
make_do_samples(noint, , fa = s_chan[ch].SB[29], , ChanBuf[ns] = fa, s_chan[ch].SB[29] = fa)

#define simple_interp_store \
  s_chan[ch].SB[28] = 0; \
  s_chan[ch].SB[29] = s_chan[ch].SB[30]; \
  s_chan[ch].SB[30] = s_chan[ch].SB[31]; \
  s_chan[ch].SB[31] = fa; \
  s_chan[ch].SB[32] = 1

#define simple_interp_get \
  if(sinc<0x10000)          /* -> upsampling? */ \
       InterpolateUp(ch);   /* --> interpolate up */ \
  else InterpolateDown(ch); /* --> else down */ \
  ChanBuf[ns] = s_chan[ch].SB[29]

make_do_samples(simple, , ,
  simple_interp_store, simple_interp_get, )

static int do_samples_noise(int ch, int ns, int ns_to)
{
 int level, shift, bit;
 int ret = -1, d;

 s_chan[ch].spos += s_chan[ch].sinc * (ns_to - ns);
 while (s_chan[ch].spos >= 28*0x10000)
 {
  d = skip_block(ch);
  if (d)
   ret = ns_to;
  s_chan[ch].spos -= 28*0x10000;
 }

 // modified from DrHell/shalma, no fraction
 level = (spuCtrl >> 10) & 0x0f;
 level = 0x8000 >> level;

 for (; ns < ns_to; ns++)
 {
  dwNoiseCount += 2;
  if (dwNoiseCount >= level)
  {
   dwNoiseCount -= level;
   shift = (dwNoiseVal >> 10) & 0x1f;
   bit = (0x69696969 >> shift) & 1;
   if (dwNoiseVal & 0x8000)
    bit ^= 1;
   dwNoiseVal = (dwNoiseVal << 1) | bit;
  }

  ChanBuf[ns] = (signed short)dwNoiseVal;
 }

 return ret;
}

#ifdef __arm__
// asm code; lv and rv must be 0-3fff
extern void mix_chan(int start, int count, int lv, int rv);
extern void mix_chan_rvb(int start, int count, int lv, int rv);
#else
static void mix_chan(int start, int count, int lv, int rv)
{
 int *dst = SSumLR + start * 2;
 const int *src = ChanBuf + start;
 int l, r;

 while (count--)
  {
   int sval = *src++;

   l = (sval * lv) >> 14;
   r = (sval * rv) >> 14;
   *dst++ += l;
   *dst++ += r;
  }
}

static void mix_chan_rvb(int start, int count, int lv, int rv)
{
 int *dst = SSumLR + start * 2;
 int *drvb = sRVBStart + start * 2;
 const int *src = ChanBuf + start;
 int l, r;

 while (count--)
  {
   int sval = *src++;

   l = (sval * lv) >> 14;
   r = (sval * rv) >> 14;
   *dst++ += l;
   *dst++ += r;
   *drvb++ += l;
   *drvb++ += r;
  }
}
#endif

// 0x0800-0x0bff  Voice 1
// 0x0c00-0x0fff  Voice 3
static void noinline do_decode_bufs(int which, int start, int count)
{
 const int *src = ChanBuf + start;
 unsigned short *dst = &spuMem[0x800/2 + which*0x400/2];
 int cursor = decode_pos;

 while (count-- > 0)
  {
   dst[cursor] = *src++;
   cursor = (cursor + 1) & 0x1ff;
  }

 // decode_pos is updated and irqs are checked later, after voice loop
}

////////////////////////////////////////////////////////////////////////
// MAIN SPU FUNCTION
// here is the main job handler...
// basically the whole sound processing is done in this fat func!
////////////////////////////////////////////////////////////////////////

static int do_samples(int forced_updates)
{
 int volmult = iVolume;
 int ns,ns_from,ns_to;
 int ch,d,silentch;
 int bIRQReturn=0;

 while(1)
  {
   // ok, at the beginning we are looking if there is
   // enuff free place in the dsound/oss buffer to
   // fill in new data, or if there is a new channel to start.
   // if not, we wait (thread) or return (timer/spuasync)
   // until enuff free place is available/a new channel gets
   // started

   if(!forced_updates && out_current->busy())          // still enuff data in sound buffer?
    {
     return 0;
    }

   cycles_since_update = 0;
   if(forced_updates > 0)
    forced_updates--;

   //--------------------------------------------------// continue from irq handling in timer mode? 

   ns_from=0;
   ns_to=NSSIZE;
   ch=0;
   if(lastch>=0)                                       // will be -1 if no continue is pending
    {
     ch=lastch; ns_from=lastns; lastch=-1;             // -> setup all kind of vars to continue
    }

   silentch=~(dwChannelOn|dwNewChannel);

   //--------------------------------------------------//
   //- main channel loop                              -// 
   //--------------------------------------------------//
    {
     for(;ch<MAXCHAN;ch++)                             // loop em all... we will collect 1 ms of sound of each playing channel
      {
       if(dwNewChannel&(1<<ch)) StartSound(ch);        // start new sound
       if(!(dwChannelOn&(1<<ch))) continue;            // channel not playing? next

       if(s_chan[ch].bNoise)
        d=do_samples_noise(ch, ns_from, ns_to);
       else if(s_chan[ch].bFMod==2 || (s_chan[ch].bFMod==0 && iUseInterpolation==0))
        d=do_samples_noint(ch, ns_from, ns_to);
       else if(s_chan[ch].bFMod==0 && iUseInterpolation==1)
        d=do_samples_simple(ch, ns_from, ns_to);
       else
        d=do_samples_default(ch, ns_from, ns_to);
       if(d>=0)
        {
         bIRQReturn=1;
         lastch=ch; 
         lastns=ns_to=d;
        }

       MixADSR(ch, ns_from, ns_to);

       if(ch==1 || ch==3)
        {
         do_decode_bufs(ch/2, ns_from, ns_to-ns_from);
         decode_dirty_ch |= 1<<ch;
        }

       if(s_chan[ch].bFMod==2)                         // fmod freq channel
        memcpy(iFMod, ChanBuf, sizeof(iFMod));
       else if(s_chan[ch].bRVBActive)
        mix_chan_rvb(ns_from,ns_to-ns_from,s_chan[ch].iLeftVolume,s_chan[ch].iRightVolume);
       else
        mix_chan(ns_from,ns_to-ns_from,s_chan[ch].iLeftVolume,s_chan[ch].iRightVolume);
      }
    }

    // advance "stopped" channels that can cause irqs
    // (all chans are always playing on the real thing..)
    if(spuCtrl&CTRL_IRQ)
     for(ch=0;ch<MAXCHAN;ch++)
      {
       if(!(silentch&(1<<ch))) continue;               // already handled
       if(dwChannelDead&(1<<ch)) continue;
       if(s_chan[ch].pCurr > pSpuIrq && s_chan[ch].pLoop > pSpuIrq)
        continue;

       s_chan[ch].spos += s_chan[ch].sinc * (ns_to - ns_from);
       while(s_chan[ch].spos >= 28 * 0x10000)
        {
         unsigned char *start = s_chan[ch].pCurr;

         // no need for bIRQReturn since the channel is silent
         skip_block(ch);
         if(start == s_chan[ch].pCurr)
          {
           // looping on self
           dwChannelDead |= 1<<ch;
           s_chan[ch].spos = 0;
           break;
          }

         s_chan[ch].spos -= 28 * 0x10000;
        }
      }

    if(bIRQReturn)                                     // special return for "spu irq - wait for cpu action"
      return 0;

  if(unlikely(silentch & decode_dirty_ch & (1<<1)))    // must clear silent channel decode buffers
   {
    memset(&spuMem[0x800/2], 0, 0x400);
    decode_dirty_ch &= ~(1<<1);
   }
  if(unlikely(silentch & decode_dirty_ch & (1<<3)))
   {
    memset(&spuMem[0xc00/2], 0, 0x400);
    decode_dirty_ch &= ~(1<<3);
   }

  //---------------------------------------------------//
  //- here we have another 1 ms of sound data
  //---------------------------------------------------//
  // mix XA infos (if any)

  MixXA();
  
  ///////////////////////////////////////////////////////
  // mix all channels (including reverb) into one buffer

  if(iUseReverb)
   REVERBDo();

  if((spuCtrl&0x4000)==0) // muted? (rare, don't optimize for this)
   {
    memset(pS, 0, NSSIZE * 2 * sizeof(pS[0]));
    pS += NSSIZE*2;
   }
  else
  for (ns = 0; ns < NSSIZE*2; )
   {
    d = SSumLR[ns]; SSumLR[ns] = 0;
    d = d * volmult >> 10;
    ssat32_to_16(d);
    *pS++ = d;
    ns++;

    d = SSumLR[ns]; SSumLR[ns] = 0;
    d = d * volmult >> 10;
    ssat32_to_16(d);
    *pS++ = d;
    ns++;
   }

  //////////////////////////////////////////////////////                   
  // special irq handling in the decode buffers (0x0000-0x1000)
  // we know: 
  // the decode buffers are located in spu memory in the following way:
  // 0x0000-0x03ff  CD audio left
  // 0x0400-0x07ff  CD audio right
  // 0x0800-0x0bff  Voice 1
  // 0x0c00-0x0fff  Voice 3
  // and decoded data is 16 bit for one sample
  // we assume: 
  // even if voices 1/3 are off or no cd audio is playing, the internal
  // play positions will move on and wrap after 0x400 bytes.
  // Therefore: we just need a pointer from spumem+0 to spumem+3ff, and 
  // increase this pointer on each sample by 2 bytes. If this pointer
  // (or 0x400 offsets of this pointer) hits the spuirq address, we generate
  // an IRQ. Only problem: the "wait for cpu" option is kinda hard to do here
  // in some of Peops timer modes. So: we ignore this option here (for now).

  if(unlikely((spuCtrl&CTRL_IRQ) && pSpuIrq && pSpuIrq<spuMemC+0x1000))
   {
    int irq_pos=(pSpuIrq-spuMemC)/2 & 0x1ff;
    if((decode_pos <= irq_pos && irq_pos < decode_pos+NSSIZE)
       || (decode_pos+NSSIZE > 0x200 && irq_pos < ((decode_pos+NSSIZE) & 0x1ff)))
     {
      //printf("decoder irq %x\n", decode_pos);
      do_irq();
     }
   }
  decode_pos = (decode_pos + NSSIZE) & 0x1ff;

  InitREVERB();

  // feed the sound
  // wanna have around 1/60 sec (16.666 ms) updates
  if (iCycle++ > 16/FRAG_MSECS)
   {
    out_current->feed(pSpuBuffer,
                        ((unsigned char *)pS) - ((unsigned char *)pSpuBuffer));
    //memset(pSpuBuffer, 0, 32 * 1024);
    pS = (short *)pSpuBuffer;
    iCycle = 0;
   }
 }

 return 0;
}

// SPU ASYNC... even newer epsxe func
//  1 time every 'cycle' cycles... harhar

// rearmed: called every 2ms now

void SPUasync(unsigned long cycle)
{
 static int old_ctrl;
 int forced_updates = 0;
 int do_update = 0;

 if(!bSpuInit) return;                               // -> no init, no call

 cycles_since_update += cycle;

 if(dwNewChannel || had_dma)
  {
   forced_updates = 1;
   do_update = 1;
   had_dma = 0;
  }

 if((spuCtrl&CTRL_IRQ) && (((spuCtrl^old_ctrl)&CTRL_IRQ) // irq was enabled
    || cycles_since_update > PSXCLK/60 / 4)) {
  do_update = 1;
  forced_updates = cycles_since_update / (PSXCLK/44100) / NSSIZE;
 }
 // with no irqs, once per frame should be fine (using a bit more because of BIAS)
 else if(cycles_since_update > PSXCLK/60 * 5/4)
  do_update = 1;

 old_ctrl = spuCtrl;

 if(do_update)
  do_samples(forced_updates);
}

// SPU UPDATE... new epsxe func
//  1 time every 32 hsync lines
//  (312/32)x50 in pal
//  (262/32)x60 in ntsc

// since epsxe 1.5.2 (linux) uses SPUupdate, not SPUasync, I will
// leave that func in the linux port, until epsxe linux is using
// the async function as well

void SPUupdate(void)
{
 SPUasync(0);
}

// XA AUDIO

void SPUplayADPCMchannel(xa_decode_t *xap)
{
 if(!xap)       return;
 if(!xap->freq) return;                                // no xa freq ? bye

 FeedXA(xap);                                          // call main XA feeder
}

// CDDA AUDIO
int SPUplayCDDAchannel(short *pcm, int nbytes)
{
 if (!pcm)      return -1;
 if (nbytes<=0) return -1;

 return FeedCDDA((unsigned char *)pcm, nbytes);
}

// to be called after state load
void ClearWorkingState(void)
{
 memset(SSumLR,0,sizeof(SSumLR));                      // init some mixing buffers
 memset(iFMod,0,sizeof(iFMod));     
 pS=(short *)pSpuBuffer;                               // setup soundbuffer pointer
}

// SETUPSTREAMS: init most of the spu buffers
void SetupStreams(void)
{ 
 int i;

 // pSpuBuffer=(unsigned char *)malloc(32768);            // alloc mixing buffer
 pSpuBuffer = (unsigned char *)0x09FF8000; // might as well use the 32kb stack from original media engine thread

 if(iUseReverb==1) i=88200*2;
 else              i=NSSIZE*2;

 // sRVBStart = (int *)malloc(i*4);                       // alloc reverb buffer
 sRVBStart = (int *)0x8B000000; // stealing memory from flash0 memory range?
 memset(sRVBStart,0,i*4);
 sRVBEnd  = sRVBStart + i;
 sRVBPlay = sRVBStart;

 XAStart =                                             // alloc xa buffer
 (uint32_t *)(0x8B000000 + i * 4);
 // (uint32_t *)malloc(44100 * sizeof(uint32_t));
 
 XAEnd   = XAStart + 44100;
 XAPlay  = XAStart;
 XAFeed  = XAStart;

 CDDAStart =                                           // alloc cdda buffer
 // (uint32_t *)malloc(CDDA_BUFFER_SIZE);
 (uint32_t *)(0x8B000000 + i * 4 + 44100 * sizeof(uint32_t));
 CDDAEnd   = CDDAStart + 16384;
 CDDAPlay  = CDDAStart;
 CDDAFeed  = CDDAStart;

 for(i=0;i<MAXCHAN;i++)                                // loop sound channels
  {
// we don't use mutex sync... not needed, would only 
// slow us down:
//   s_chan[i].hMutex=CreateMutex(NULL,FALSE,NULL);
   s_chan[i].ADSRX.SustainLevel = 0xf;                 // -> init sustain
   s_chan[i].pLoop=spuMemC;
   s_chan[i].pCurr=spuMemC;
  }

 ClearWorkingState();

 bSpuInit=1;                                           // flag: we are inited
}

// REMOVESTREAMS: free most buffer
void RemoveStreams(void)
{ 
 //free(pSpuBuffer);                                     // free mixing buffer
 pSpuBuffer = NULL;
 //free(sRVBStart);                                      // free reverb buffer
 sRVBStart = NULL;
 //free(XAStart);                                        // free XA buffer
 XAStart = NULL;
 //free(CDDAStart);                                      // free CDDA buffer
 CDDAStart = NULL;
}

// INIT/EXIT STUFF

// SPUINIT: this func will be called first by the main emu
long SPUinit(void)
{
 spuMemC = (unsigned char *)spuMem;                    // just small setup
 memset((void *)&rvb, 0, sizeof(REVERBInfo));
 InitADSR();

 spuIrq = 0;
 spuAddr = 0xffffffff;
 spuMemC = (unsigned char *)spuMem;
 decode_pos = 0;
 memset((void *)s_chan, 0, (MAXCHAN + 1) * sizeof(SPUCHAN));
 pSpuIrq = 0;
 lastch = -1;

 SetupStreams();                                       // prepare streaming

 return 0;
}

// SPUOPEN: called by main emu after init
long SPUopen(void)
{
 if (bSPUIsOpen) return 0;                             // security for some stupid main emus

 SetupSound();                                         // setup sound (before init!)

 bSPUIsOpen = 1;

 return PSE_SPU_ERR_SUCCESS;
}

// SPUCLOSE: called before shutdown
long SPUclose(void)
{
 if (!bSPUIsOpen) return 0;                            // some security

 bSPUIsOpen = 0;                                       // no more open

 out_current->finish();                                // no more sound handling

 return 0;
}

// SPUSHUTDOWN: called by main emu on final exit
long SPUshutdown(void)
{
 SPUclose();
 RemoveStreams();                                      // no more streaming
 bSpuInit=0;

 return 0;
}

// SPUTEST: we don't test, we are always fine ;)
long SPUtest(void)
{
 return 0;
}

// SPUCONFIGURE: call config dialog
long SPUconfigure(void)
{
#ifdef _MACOSX
 DoConfiguration();
#else
// StartCfgTool("CFG");
#endif
 return 0;
}

// SPUABOUT: show about window
void SPUabout(void)
{
#ifdef _MACOSX
 DoAbout();
#else
// StartCfgTool("ABOUT");
#endif
}

// COMMON PLUGIN INFO FUNCS
/*
char * PSEgetLibName(void)
{
 return _(libraryName);
}

unsigned long PSEgetLibType(void)
{
 return  PSE_LT_SPU;
}

unsigned long PSEgetLibVersion(void)
{
 return (1 << 16) | (6 << 8);
}

char * SPUgetLibInfos(void)
{
 return _(libraryInfo);
}
*/

// debug
void spu_get_debug_info(int *chans_out, int *run_chans, int *fmod_chans_out, int *noise_chans_out)
{
 int ch = 0, fmod_chans = 0, noise_chans = 0, irq_chans = 0;

 for(;ch<MAXCHAN;ch++)
 {
  if (!(dwChannelOn & (1<<ch)))
   continue;
  if (s_chan[ch].bFMod == 2)
   fmod_chans |= 1 << ch;
  if (s_chan[ch].bNoise)
   noise_chans |= 1 << ch;
  if((spuCtrl&CTRL_IRQ) && s_chan[ch].pCurr <= pSpuIrq && s_chan[ch].pLoop <= pSpuIrq)
   irq_chans |= 1 << ch;
 }

 *chans_out = dwChannelOn;
 *run_chans = ~dwChannelOn & ~dwChannelDead & irq_chans;
 *fmod_chans_out = fmod_chans;
 *noise_chans_out = noise_chans;
}

// vim:shiftwidth=1:expandtab
