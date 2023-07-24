/***************************************************************************
                            spu.c  -  description
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

#define _IN_SPU

#include "externals.h"
#include "regs.h"

#ifdef ENABLE_NLS
#include <libintl.h>
#include <locale.h>
#define _(x)  gettext(x)
#define N_(x) (x)
//If running under Mac OS X, use the Localizable.strings file instead.
#elif defined(_MACOSX)
#ifdef PCSXRCORE
__private_extern char* Pcsxr_locale_text(char* toloc);
#define _(String) Pcsxr_locale_text(String)
#define N_(String) String
#else
#ifndef PCSXRPLUG
#warning please define the plug being built to use Mac OS X localization!
#define _(msgid) msgid
#define N_(msgid) msgid
#else
//Kludge to get the preprocessor to accept PCSXRPLUG as a variable.
#define PLUGLOC_x(x,y) x ## y
#define PLUGLOC_y(x,y) PLUGLOC_x(x,y)
#define PLUGLOC PLUGLOC_y(PCSXRPLUG,_locale_text)
__private_extern char* PLUGLOC(char* toloc);
#define _(String) PLUGLOC(String)
#define N_(String) String
#endif
#endif
#else
#define _(x)  (x)
#define N_(x) (x)
#endif

static char * libraryInfo     = N_("P.E.Op.S. Sound Driver V1.7\nCoded by Pete Bernert and the P.E.Op.S. team\n");

// globals

// psx buffer / addresses

unsigned short* spuMem = (unsigned short *)0x49F402C0;
unsigned char * spuMemC = (unsigned char *)0x49F402C0;
unsigned char * pSpuIrq=0;
unsigned char * pSpuBuffer;
unsigned char * pMixIrq=0;

// user settings

int             iVolume=1;
int             iXAPitch=1;
int             iUseTimer=0;
int             iSPUIRQWait=1;
int             iDebugMode=0;
int             iRecordMode=0;
int             iUseReverb=0;
int             iUseInterpolation=2;
int             iDisStereo=0;
int							iFreqResponse=0;

// MAIN infos struct for each channel

SPUCHAN         s_chan[MAXCHAN+1];                     // channel + 1 infos (1 is security for fmod handling)
REVERBInfo      rvb;

unsigned long   dwNoiseVal=1;                          // global noise generator
unsigned long   dwNoiseCount;                          // global noise generator
unsigned long   dwNoiseClock;                          // global noise generator
int             iSpuAsyncWait=0;

unsigned int    decoded_ptr = 0;
unsigned int	bIrqHit = 0;

unsigned short  spuCtrl=0;                             // some vars to store psx reg infos
unsigned short  spuStat=0;
unsigned short  spuIrq=0;
unsigned long   spuAddr=0x200;                         // address into spu mem
int             bEndThread=0;                          // thread handlers
int             bThreadEnded=0;
int             bSpuInit=0;
int             bSPUIsOpen=0;

uint32_t dwNewChannel=0;                          // flags for faster testing, if new channel starts

void (CALLBACK *irqCallback)(void)=0;                  // func of main emu, called on spu irq
void (CALLBACK *cddavCallback)(unsigned short,unsigned short)=0;

// certain globals (were local before, but with the new timeproc I need em global)

static const int f[5][2] = {   {    0,  0  },
                        {   60,  0  },
                        {  115, -52 },
                        {   98, -55 },
                        {  122, -60 } };
int SSumR[NSSIZE];
int SSumL[NSSIZE];
int iFMod[NSSIZE];
int iCycle = 0;
short * pS;

int lastns=0;              // last ns pos
static int iSecureStart=0; // secure start counter

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


static INLINE void InterpolateUp(int ch)
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
   if(s_chan[ch].sinc<=0x8000)
        s_chan[ch].SB[29]=s_chan[ch].SB[30]-(s_chan[ch].SB[28]*((0x10000/s_chan[ch].sinc)-1));
   else s_chan[ch].SB[29]+=s_chan[ch].SB[28];
  }
 else                                                  // no flags? add bigger val (if possible), calc smaller step, set flag1
  s_chan[ch].SB[29]+=s_chan[ch].SB[28];
}

//
// even easier interpolation on downsampling, also no special filter, again just "Pete's common sense" tm
//

static INLINE void InterpolateDown(int ch)
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
#define gval(x) (((short*)(&s_chan[ch].SB[29]))[(gpos+x)&3])

#include "gauss_i.h"

////////////////////////////////////////////////////////////////////////

#include "xa.c"

////////////////////////////////////////////////////////////////////////
// START SOUND... called by main thread to setup a new sound on a channel
////////////////////////////////////////////////////////////////////////

static INLINE void StartSound(int ch)
{
 StartADSR(ch);
 StartREVERB(ch);

 // fussy timing issues - do in VoiceOn
 //s_chan[ch].pCurr=s_chan[ch].pStart;                   // set sample start
 //s_chan[ch].bStop=0;
 //s_chan[ch].bOn=1;

 s_chan[ch].s_1=0;                                     // init mixing vars
 s_chan[ch].s_2=0;
 s_chan[ch].iSBPos=28;

 s_chan[ch].bNew=0;                                    // init channel flags

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

static INLINE void VoiceChangeFrequency(int ch)
{
 s_chan[ch].iUsedFreq=s_chan[ch].iActFreq;             // -> take it and calc steps
 s_chan[ch].sinc=s_chan[ch].iRawPitch<<4;
 if(!s_chan[ch].sinc) s_chan[ch].sinc=1;
 if(iUseInterpolation==1) s_chan[ch].SB[32]=1;         // -> freq change in simle imterpolation mode: set flag
}

////////////////////////////////////////////////////////////////////////

static INLINE void FModChangeFrequency(int ch,int ns)
{
 int NP=s_chan[ch].iRawPitch;

 NP=((32768L+iFMod[ns])*NP)/32768L;

 if(NP>0x3fff) NP=0x3fff;
 if(NP<0x1)    NP=0x1;

 NP=(44100L*NP)/(4096L);                               // calc frequency

 s_chan[ch].iActFreq=NP;
 s_chan[ch].iUsedFreq=NP;
 s_chan[ch].sinc=(((NP/10)<<16)/4410);
 if(!s_chan[ch].sinc) s_chan[ch].sinc=1;
 if(iUseInterpolation==1)                              // freq change in simple interpolation mode
 s_chan[ch].SB[32]=1;
 iFMod[ns]=0;
}

////////////////////////////////////////////////////////////////////////

/*
Noise Algorithm
- Dr.Hell (Xebra PS1 emu)
- 100% accurate (waveform + frequency)
- http://drhell.web.fc2.com


Level change cycle
Freq = 0x8000 >> (NoiseClock >> 2);

Frequency of half cycle
Half = ((NoiseClock & 3) * 2) / (4 + (NoiseClock & 3));
- 0 = (0*2)/(4+0) = 0/4
- 1 = (1*2)/(4+1) = 2/5
- 2 = (2*2)/(4+2) = 4/6
- 3 = (3*2)/(4+3) = 6/7

-------------------------------

5*6*7 = 210
4 -  0*0 = 0
5 - 42*2 = 84
6 - 35*4 = 140
7 - 30*6 = 180
*/

// Noise Waveform - Dr. Hell (Xebra)
char NoiseWaveAdd [64] = {
	1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1
};

unsigned short NoiseFreqAdd[5] = {
	0, 84, 140, 180, 210
};

static INLINE void NoiseClock()
{
	unsigned int level;

	level = 0x8000 >> (dwNoiseClock >> 2);
	level <<= 16;

	dwNoiseCount += 0x10000;

	// Dr. Hell - fraction
	dwNoiseCount += NoiseFreqAdd[ dwNoiseClock & 3 ];
	if( (dwNoiseCount&0xffff) >= NoiseFreqAdd[4] ) {
		dwNoiseCount += 0x10000;
		dwNoiseCount -= NoiseFreqAdd[ dwNoiseClock & 3 ];
	}

	if( dwNoiseCount >= level )
	{
		while( dwNoiseCount >= level )
			dwNoiseCount -= level;

		// Dr. Hell - form
		dwNoiseVal = (dwNoiseVal<<1) | NoiseWaveAdd[ (dwNoiseVal>>10) & 63 ];
	}
}

static INLINE int iGetNoiseVal(int ch)
{
 int fa;

 fa = (short) dwNoiseVal;

 // no clip need
 //if(fa>32767L)  fa=32767L;
 //if(fa<-32767L) fa=-32767L;

 // don't upset VAG decoder
 //if(iUseInterpolation<2)                               // no gauss/cubic interpolation?
  //pChannel->SB[29] = fa;                               // -> store noise val in "current sample" slot

 // boost volume - no more!
 //return fa * 3 / 2;
 return fa;
}

////////////////////////////////////////////////////////////////////////

static INLINE void StoreInterpolationVal(int ch,int fa)
{
	/*
	// fmod channel = sound output
 if(s_chan[ch].bFMod==2)                               // fmod freq channel
  s_chan[ch].SB[29]=fa;
 else
 */
  {
   if((spuCtrl&0x4000)==0) fa=0;                       // muted?
   else                                                // else adjust
    {
     if(fa>32767L)  fa=32767L;
     if(fa<-32767L) fa=-32767L;
    }

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

static INLINE int iGetInterpolationVal(int ch)
{
 int fa;

 // fmod channel = sound output
 //if(s_chan[ch].bFMod==2) return s_chan[ch].SB[29];

 switch(iUseInterpolation)
  {
   //--------------------------------------------------//
   case 3:                                             // cubic interpolation
    {
     long xd;int gpos;
     xd = ((s_chan[ch].spos) >> 1)+1;
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
     vl = (s_chan[ch].spos >> 6) & ~3;
     gpos = s_chan[ch].SB[28];
     vr=(gauss[vl]*gval0)&~2047;
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

////////////////////////////////////////////////////////////////////////
// MAIN SPU FUNCTION
// here is the main job handler... thread, timer or direct func call
// basically the whole sound processing is done in this fat func!
////////////////////////////////////////////////////////////////////////

// 5 ms waiting phase, if buffer is full and no new sound has to get started
// .. can be made smaller (smallest val: 1 ms), but bigger waits give
// better performance

#define PAUSE_W 1
#define PAUSE_L 1000

////////////////////////////////////////////////////////////////////////

static void *MAINThread(void *arg)
{
 int s_1,s_2,fa,ns;
 int voldiv = iVolume;

 unsigned char * start;unsigned int nSample;
 int ch,predict_nr,shift_factor,flags,d,s;
 int bIRQReturn=0;
 unsigned int decoded_voice=0;

 // mute output
 if( voldiv == 5 ) voldiv = 0x7fffffff;

 while(!bEndThread)                                    // until we are shutting down
  {
   // ok, at the beginning we are looking if there is
   // enuff free place in the dsound/oss buffer to
   // fill in new data, or if there is a new channel to start.
   // if not, we wait (thread) or return (timer/spuasync)
   // until enuff free place is available/a new channel gets
   // started

   if(dwNewChannel)                                    // new channel should start immedately?
    {                                                  // (at least one bit 0 ... MAXCHANNEL is set?)
     iSecureStart++;                                   // -> set iSecure
     if(iSecureStart>1) iSecureStart=0;                //    (if it is set 5 times - that means on 5 tries a new samples has been started - in a row, we will reset it, to give the sound update a chance)
    }
   else iSecureStart=0;                                // 0: no new channel should start

   while(!iSecureStart && !bEndThread &&               // no new start? no thread end?
         (SoundGetBytesBuffered()>TESTSIZE))           // and still enuff data in sound buffer?
    {
     iSecureStart=0;                                   // reset secure

     if(iUseTimer) return 0;                           // linux no-thread mode? bye
     sceKernelDelayThread(PAUSE_L);                                  // else sleep for x ms (linux)

     if(dwNewChannel) iSecureStart=1;                  // if a new channel kicks in (or, of course, sound buffer runs low), we will leave the loop
    }

   ns=0;

   //--------------------------------------------------// continue from irq handling in timer mode?

   if(lastns>0)                                        // will be 0 if no continue is pending
    {
     ns=lastns;                                        // -> setup all kind of vars to continue
     lastns=0;
    }

   //--------------------------------------------------//
   //- main channel loop                              -//
   //--------------------------------------------------//
    {
		 decoded_voice = decoded_ptr;

		 while(ns<NSSIZE)                                // loop until 1 ms of data is reached
      {
				SSumL[ns]=0;
				SSumR[ns]=0;


				// decoded buffer values - dummy
				spuMem[ (0x000 + decoded_voice) / 2 ] = (short) 0;
				spuMem[ (0x400 + decoded_voice) / 2 ] = (short) 0;
				spuMem[ (0x800 + decoded_voice) / 2 ] = (short) 0;
				spuMem[ (0xc00 + decoded_voice) / 2 ] = (short) 0;


				NoiseClock();

				for(ch=0;ch<MAXCHAN;ch++)                         // loop em all... we will collect 1 ms of sound of each playing channel
        {
					if(s_chan[ch].bNew) {
#if 1
						StartSound(ch);																 // start new sound
						dwNewChannel&=~(1<<ch);                       // clear new channel bit
#else
						if( s_chan[ch].ADSRX.StartDelay == 0 ) {
							StartSound(ch);															// start new sound
							dwNewChannel&=~(1<<ch);                     // clear new channel bit
						} else {
							s_chan[ch].ADSRX.StartDelay--;
						}
#endif
					}
				 if(!s_chan[ch].bOn) continue;                   // channel not playing? next

				 if(s_chan[ch].iActFreq!=s_chan[ch].iUsedFreq)   // new psx frequency?
					VoiceChangeFrequency(ch);

				 if(s_chan[ch].bFMod==1 && iFMod[ns])          // fmod freq channel
          FModChangeFrequency(ch,ns);

         while(s_chan[ch].spos>=0x10000L)
          {
           if(s_chan[ch].iSBPos==28)                   // 28 reached?
            {
						 // Xenogears - Anima Relic dungeon (exp gain)
						 if( s_chan[ch].bLoopJump == 1 )
							 s_chan[ch].pCurr = s_chan[ch].pLoop;

						 s_chan[ch].bLoopJump = 0;


						 start=s_chan[ch].pCurr;                   // set up the current pos

             if (start == spuMemC)
              s_chan[ch].bOn = 0;

             if (s_chan[ch].iSilent==1 )
              {
               // silence = let channel keep running (IRQs)
							 //s_chan[ch].bOn=0;                       // -> turn everything off
							 s_chan[ch].iSilent=2;

               s_chan[ch].ADSRX.lVolume=0;
               s_chan[ch].ADSRX.EnvelopeVol=0;
              }

             s_chan[ch].iSBPos=0;

             //////////////////////////////////////////// spu irq handler here? mmm... do it later

             s_1=s_chan[ch].s_1;
             s_2=s_chan[ch].s_2;

             predict_nr=(int)*start;start++;
             shift_factor=predict_nr&0xf;
             predict_nr >>= 4;
             flags=(int)*start;start++;

						 // Silhouette Mirage - Serah fight
						 if( predict_nr > 4 ) predict_nr = 0;

             // -------------------------------------- //

             for (nSample=0;nSample<28;start++)
              {
               d=(int)*start;
               s=((d&0xf)<<12);
               if(s&0x8000) s|=0xffff0000;

               fa=(s >> shift_factor);
               fa=fa + ((s_1 * f[predict_nr][0])>>6) + ((s_2 * f[predict_nr][1])>>6);

							 // snes brr clamps
							 fa = CLAMP16(fa);

               s_2=s_1;s_1=fa;
               s=((d & 0xf0) << 8);

               s_chan[ch].SB[nSample++]=fa;


               if(s&0x8000) s|=0xffff0000;
               fa=(s>>shift_factor);
               fa=fa + ((s_1 * f[predict_nr][0])>>6) + ((s_2 * f[predict_nr][1])>>6);

							 // snes brr clamps
							 fa = CLAMP16(fa);

               s_2=s_1;s_1=fa;

               s_chan[ch].SB[nSample++]=fa;
              }

             //////////////////////////////////////////// irq check

#if 1
						// Check channel/loop IRQs (e.g. Castlevania Chronicles) and at pos-8 for unknown reason
						if( Check_IRQ( (s_chan[ch].pCurr)-spuMemC, 0 ) ||
								Check_IRQ( (start-spuMemC)-0, 0 ) ||
								Check_IRQ( (start-spuMemC)-8, 0 ) )
						{
#else
             if(irqCallback && (spuCtrl&0x40))         // some callback and irq active?
              {
               if((pSpuIrq >  start-16 &&              // irq address reached?
                   pSpuIrq <= start) ||
                  ((flags&1) &&                        // special: irq on looping addr, when stop/loop flag is set
                   (pSpuIrq >  s_chan[ch].pLoop-16 &&
                    pSpuIrq <= s_chan[ch].pLoop)))
#endif
               {
                 s_chan[ch].iIrqDone=1;                // -> debug flag
                 //irqCallback();                      // -> call main emu (checked & called on Check_IRQ)

                 if(iSPUIRQWait)                       // -> option: wait after irq for main emu
                  {
                   iSpuAsyncWait=1;
                   bIRQReturn=1;
                  }
                }
              }

             //////////////////////////////////////////// flag handler

						/*
						SPU2-X:
						$4 = set loop to current block
						$2 = keep envelope on (no mute)
						$1 = jump to loop address

						silence means no volume (ADSR keeps playing!!)
						*/

						if(flags&4)
							s_chan[ch].pLoop=start-16;


						// Jungle Book - Rhythm 'n Groove - don't reset ignore status
						// - fixes gameplay speed (IRQ hits)
						//s_chan[ch].bIgnoreLoop = 0;


						if(flags&1)
						{
							// ...?
							//s_chan[ch].bIgnoreLoop = 0;

							// Xenogears - 7 = play missing sounds
							// set jump flag
							s_chan[ch].bLoopJump = 1;


							// silence = keep playing..?
							if( (flags&2) == 0 ) {
								s_chan[ch].iSilent = 1;

								// silence = don't start release phase
								//s_chan[ch].bStop = 1;

								//start = (unsigned char *) -1;
							}
						}

#if 0
						// crash check
						if( start == 0 )
							start = (unsigned char *) -1;
						if( start >= spuMemC + 0x80000 )
							start = spuMemC - 0x80000;
#endif


						// Silhouette Mirage - ending mini-game

						// ??
						if( start - spuMemC >= 0x80000 ) {
							start -= 16;

							s_chan[ch].iSilent = 1;
							s_chan[ch].bStop = 1;
						}


             s_chan[ch].pCurr=start;                   // store values for next cycle
             s_chan[ch].s_1=s_1;
             s_chan[ch].s_2=s_2;
            }

           fa=s_chan[ch].SB[s_chan[ch].iSBPos++];      // get sample data

           StoreInterpolationVal(ch,fa);               // store val for later interpolation

           s_chan[ch].spos -= 0x10000L;
          }

         if(s_chan[ch].bNoise)
              fa=iGetNoiseVal(ch);                     // get noise val
         else fa=iGetInterpolationVal(ch);             // get sample val


				 // Voice 1/3 decoded buffer
				 if( ch == 0 ) {
					 spuMem[ (0x800 + decoded_voice) / 2 ] = (short) fa;
				 } else if( ch == 2 ) {
					 spuMem[ (0xc00 + decoded_voice) / 2 ] = (short) fa;
				 }


         s_chan[ch].sval = (MixADSR(ch) * fa) / 1023;  // mix adsr

         if(s_chan[ch].bFMod==2)                       // fmod freq channel
          iFMod[ns]=s_chan[ch].sval;                   // -> store 1T sample data, use that to do fmod on next channel

				 // mix fmod channel into output
				 // - Xenogears save icon (high pitch)
				 {
           //////////////////////////////////////////////
           // ok, left/right sound volume (psx volume goes from 0 ... 0x3fff)

           if(s_chan[ch].iMute)
            s_chan[ch].sval=0;                         // debug mute
           else
            {
             SSumL[ns]+=(s_chan[ch].sval*s_chan[ch].iLeftVolume)/0x4000L;
             SSumR[ns]+=(s_chan[ch].sval*s_chan[ch].iRightVolume)/0x4000L;
            }

           //////////////////////////////////////////////
           // now let us store sound data for reverb

           if(s_chan[ch].bRVBActive) StoreREVERB(ch,ns);
          }

				 s_chan[ch].spos += s_chan[ch].sinc;
        }

        ////////////////////////////////////////////////
        // ok, go on until 1 ms data of this channel is collected

				// decoded buffer - voice
				decoded_voice += 2;
				decoded_voice &= 0x3ff;


				// status flag
				if( decoded_voice >= 0x200 ) {
					spuStat |= STAT_DECODED;
				} else {
					spuStat &= ~STAT_DECODED;
				}


				// IRQ work
				{
					unsigned char *old_irq;
					unsigned int old_ptr;

					old_irq = pSpuIrq;
					old_ptr = decoded_voice;

#if 0
					// align to boundaries ($0, $200, $400, $600)
					pSpuIrq = ((pSpuIrq - spuMemC) & (~0x1ff)) + spuMemC;
					decoded_voice = decoded_voice & (~0x1ff);
#endif

					// check all decoded buffer IRQs - timing issue
					Check_IRQ( decoded_voice + 0x000, 0 );
					Check_IRQ( decoded_voice + 0x400, 0 );
					Check_IRQ( decoded_voice + 0x800, 0 );
					Check_IRQ( decoded_voice + 0xc00, 0 );

					pSpuIrq = old_irq;
					decoded_voice = old_ptr;
				}

         if(bIRQReturn)                            // special return for "spu irq - wait for cpu action"
          {
           bIRQReturn=0;
           if(iUseTimer!=2)
            {
             DWORD dwWatchTime=timeGetTime_spu()+2500;

             while(iSpuAsyncWait && !bEndThread &&
                   timeGetTime_spu()<dwWatchTime)
                  sceKernelDelayThread(1000);
            }
           else
            {
             lastns=ns+1;
             return 0;
           }
         }

        ns++;
      } // end ns
    }


  //---------------------------------------------------//
  //- here we have another 1 ms of sound data
  //---------------------------------------------------//
  // mix XA infos (if any)

  MixXA();


	// now safe to update decoded buffer ptr
	decoded_ptr += ns * 2;
	decoded_ptr &= 0x3ff;


  ///////////////////////////////////////////////////////
  // mix all channels (including reverb) into one buffer

  if(iDisStereo)                                       // no stereo?
   {
    int dl, dr;
    for (ns = 0; ns < NSSIZE; ns++)
     {
      SSumL[ns] += MixREVERBLeft(ns);

      dl = SSumL[ns] / voldiv; SSumL[ns] = 0;
      if (dl < -32767) dl = -32767; if (dl > 32767) dl = 32767;

      SSumR[ns] += MixREVERBRight();

      dr = SSumR[ns] / voldiv; SSumR[ns] = 0;
      if (dr < -32767) dr = -32767; if (dr > 32767) dr = 32767;
      *pS++ = (dl + dr) / 2;
     }
   }
  else                                                 // stereo:
  for (ns = 0; ns < NSSIZE; ns++)
   {
		static double _interpolation_coefficient = 3.759285613;

		if(iFreqResponse) {
			int sl,sr;
			double ldiff, rdiff, avg, tmp;

			SSumL[ns]+=MixREVERBLeft(ns);
			SSumR[ns]+=MixREVERBRight();

			sl = SSumL[ns]; SSumL[ns]=0;
			sr = SSumR[ns]; SSumR[ns]=0;


			/*
			Frequency Response
			- William Pitcock (nenolod) (UPSE PSF player)
			- accurate (!)
			- http://nenolod.net
			*/

			avg = ((sl + sr) / 2);
			ldiff = sl - avg;
			rdiff = sr - avg;

			tmp = avg + ldiff * _interpolation_coefficient;
			if (tmp < -32768)
				tmp = -32768;
			if (tmp > 32767)
				tmp = 32767;
			sl = (int)tmp;

			tmp = avg + rdiff * _interpolation_coefficient;
			if (tmp < -32768)
				tmp = -32768;
			if (tmp > 32767)
				tmp = 32767;
			sr = (int)tmp;


			*pS++=sl/voldiv;
			*pS++=sr/voldiv;
		} else {
			SSumL[ns]+=MixREVERBLeft(ns);

			d=SSumL[ns]/voldiv;SSumL[ns]=0;
			if(d<-32767) d=-32767;if(d>32767) d=32767;
			*pS++=d;

			SSumR[ns]+=MixREVERBRight();

			d=SSumR[ns]/voldiv;SSumR[ns]=0;
			if(d<-32767) d=-32767;if(d>32767) d=32767;
			*pS++=d;
		}
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

#if 0
  if(pMixIrq && irqCallback)
   {
    for(ns=0;ns<NSSIZE;ns++)
     {
      if((spuCtrl&0x40) && pSpuIrq && pSpuIrq<spuMemC+0x1000)
       {
        for(ch=0;ch<4;ch++)
         {
          if(pSpuIrq>=pMixIrq+(ch*0x400) && pSpuIrq<pMixIrq+(ch*0x400)+2)
           {irqCallback();s_chan[ch].iIrqDone=1;}
         }
       }
      pMixIrq+=2;if(pMixIrq>spuMemC+0x3ff) pMixIrq=spuMemC;
     }
   }
#endif

  InitREVERB();

  //////////////////////////////////////////////////////
  // feed the sound
  // latency = 25 ms (less pops, crackles, smoother)

	//if(iCycle++>=20)
	iCycle += APU_CYCLES_UPDATE;
	if(iCycle > 44000/1000*LATENCY + 100*LATENCY/1000)
   {
    SoundFeedStreamData((unsigned char *)pSpuBuffer,
                        ((unsigned char *)pS) - ((unsigned char *)pSpuBuffer));
    pS = (short *)pSpuBuffer;
    iCycle = 0;
   }


	if( iUseTimer == 2 )
		break;
 }

 // end of big main loop...

 bThreadEnded = 1;

 return 0;
}

// SPU ASYNC... even newer epsxe func
//  1 time every 'cycle' cycles... harhar

long cpu_cycles;
void CALLBACK SPUasync(unsigned long cycle)
{
	cpu_cycles += cycle;

 if(iSpuAsyncWait)
  {
   iSpuAsyncWait++;
   if(iSpuAsyncWait<=64) return;
   iSpuAsyncWait=0;
  }

 if(iUseTimer==2)                                      // special mode, only used in Linux by this spu (or if you enable the experimental Windows mode)
  {
   if(!bSpuInit) return;                               // -> no init, no call

	 // note: usable precision difference (not using interval_time)
	 while( cpu_cycles >= CPU_CLOCK / 44100 * NSSIZE )
	 {
		 MAINThread(0);                                      // -> linux high-compat mode

	  if (iSpuAsyncWait)
	    break;
	  cpu_cycles -= CPU_CLOCK / 44100 * NSSIZE;
	 }
  }
}

// SPU UPDATE... new epsxe func
//  1 time every 32 hsync lines
//  (312/32)x50 in pal
//  (262/32)x60 in ntsc

// since epsxe 1.5.2 (linux) uses SPUupdate, not SPUasync, I will
// leave that func in the linux port, until epsxe linux is using
// the async function as well

void CALLBACK SPUupdate(void)
{
 MAINThread(0);
}

// XA AUDIO

void CALLBACK SPUplayADPCMchannel(xa_decode_t *xap)
{
 if(!xap)       return;
 if(!xap->freq) return;                                // no xa freq ? bye

 FeedXA(xap);                                          // call main XA feeder
}

// CDDA AUDIO
void CALLBACK SPUplayCDDAchannel(short *pcm, int nbytes)
{
 if (!pcm)      return;
 if (nbytes<=0) return;

 FeedCDDA((unsigned char *)pcm, nbytes);
}

// SETUPTIMER: init of certain buffers and threads/timers
void SetupTimer(void)
{
 memset(SSumR,0,NSSIZE*sizeof(int));                   // init some mixing buffers
 memset(SSumL,0,NSSIZE*sizeof(int));
 memset(iFMod,0,NSSIZE*sizeof(int));
 pS=(short *)pSpuBuffer;                               // setup soundbuffer pointer

 bEndThread=0;                                         // init thread vars
 bThreadEnded=0;
 bSpuInit=1;                                           // flag: we are inited
}

// REMOVETIMER: kill threads/timers
void RemoveTimer(void)
{
 bEndThread=1;                                         // raise flag to end thread
 bThreadEnded=0;                                       // no more spu is running
 bSpuInit=0;
}

// SETUPSTREAMS: init most of the spu buffers
void SetupStreams(void)
{
 int i;

 pSpuBuffer=(unsigned char *)malloc(32768);            // alloc mixing buffer

 if(iUseReverb==1) i=88200*2;
 else              i=NSSIZE*2;

 sRVBStart = (int *)malloc(i*4);                       // alloc reverb buffer
 memset(sRVBStart,0,i*4);
 sRVBEnd  = sRVBStart + i;
 sRVBPlay = sRVBStart;

 XAStart =                                             // alloc xa buffer
  (uint32_t *)malloc(44100 * sizeof(uint32_t));
 XAEnd   = XAStart + 44100;
 XAPlay  = XAStart;
 XAFeed  = XAStart;

 CDDAStart =                                           // alloc cdda buffer
  (uint32_t *)malloc(44100 * sizeof(uint32_t));
 CDDAEnd   = CDDAStart + 44100;
 CDDAPlay  = CDDAStart;
 CDDAFeed  = CDDAStart;

 for(i=0;i<MAXCHAN;i++)                                // loop sound channels
  {
// we don't use mutex sync... not needed, would only
// slow us down:
//   s_chan[i].hMutex=CreateMutex(NULL,FALSE,NULL);
   s_chan[i].ADSRX.SustainLevel = 1024;                // -> init sustain
   s_chan[i].iMute=0;
   s_chan[i].iIrqDone=0;
   s_chan[i].pLoop=spuMemC;
   s_chan[i].pStart=spuMemC;
   s_chan[i].pCurr=spuMemC;
  }

  pMixIrq=spuMemC;                                     // enable decoded buffer irqs by setting the address
}

// REMOVESTREAMS: free most buffer
void RemoveStreams(void)
{
 free(pSpuBuffer);                                     // free mixing buffer
 pSpuBuffer = NULL;
 free(sRVBStart);                                      // free reverb buffer
 sRVBStart = NULL;
 free(XAStart);                                        // free XA buffer
 XAStart = NULL;
 free(CDDAStart);                                      // free CDDA buffer
 CDDAStart = NULL;
}

// INIT/EXIT STUFF

// SPUINIT: this func will be called first by the main emu
long CALLBACK SPUinit(void)
{
 iReverbOff = -1;
 spuIrq = 0;
 spuAddr = 0x200;
 bEndThread = 0;
 bThreadEnded = 0;
 pMixIrq = 0;
 pSpuIrq = 0;
 lastns = 0;
 memset((void *)s_chan,0,MAXCHAN*sizeof(SPUCHAN));
 memset((void *)&rvb,0,sizeof(REVERBInfo));
 InitADSR();
 return 0;
}

// SPUOPEN: called by main emu after init
long SPUopen(void)
{
 if(bSPUIsOpen) return 0;                              // security for some stupid main emus

 iVolume=3;
 iReverbOff=-1;   
 bEndThread=0;
 memset((void *)s_chan,0,(MAXCHAN+1)*sizeof(SPUCHAN));
 
 ReadConfig();                                         // read user stuff
 
 SetupSound();                                         // setup sound (before init!)

 SetupStreams();                                       // prepare streaming

 SetupTimer();                                         // timer for feeding data

 bSPUIsOpen=1;

 return PSE_SPU_ERR_SUCCESS;   
}

// SPUCLOSE: called before shutdown
long CALLBACK SPUclose(void)
{
 if (!bSPUIsOpen) return 0;                            // some security

 bSPUIsOpen = 0;                                       // no more open

 RemoveTimer();                                        // no more feeding
 RemoveSound();                                        // no more sound handling

 return 0;
}

// SPUSHUTDOWN: called by main emu on final exit
long CALLBACK SPUshutdown(void)
{
 SPUclose();
 RemoveStreams();                                      // no more streaming

 return 0;
}

// SETUP CALLBACKS
// this functions will be called once,
// passes a callback that should be called on SPU-IRQ/cdda volume change
void CALLBACK SPUregisterCallback(void (CALLBACK *callback)(void))
{
 irqCallback = callback;
}

void CALLBACK SPUregisterCDDAVolume(void (CALLBACK *CDDAVcallback)(unsigned short,unsigned short))
{
 cddavCallback = CDDAVcallback;
}
