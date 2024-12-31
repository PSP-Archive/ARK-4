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
 
////////////////////////////////////////////////////////////////////////
// globals
////////////////////////////////////////////////////////////////////////

// psx buffer / addresses

unsigned char * spuMemC = (unsigned char *)0x49F402C0;;
unsigned char * pSpuBuffer;


// user settings

int             iVolume=1;
int             iXAPitch=0;
int             iUseReverb=0;
int             iUseInterpolation=2;

// MAIN infos struct for each channel

SPUCHAN         s_chan[MAXCHAN+1];                     // channel + 1 infos (1 is security for fmod handling)
REVERBInfo      rvb;

unsigned long   dwNoiseVal=1;                          // global noise generator

unsigned short  spuCtrl=0;                             // some vars to store psx reg infos
int             bEndThread=0;                          // thread handlers
int             bSpuInit=0;
int             bSPUIsOpen=0;

unsigned long dwNewChannel=0;                          // flags for faster testing, if new channel starts

// certain globals (were local before, but with the new timeproc I need em global)

const int f[5][2] = {   {    0,  0  },
                        {   60,  0  },
                        {  115, -52 },
                        {   98, -55 },
                        {  122, -60 } };
int SSumR[NSSIZE];
int SSumL[NSSIZE];
int iFMod[NSSIZE];
int iCycle=0;
short * pS;

static int lastch=-1;      // last channel processed on spu irq in timer mode
static int lastns=0;       // last ns pos
static int iSecureStart=0; // secure start counter

////////////////////////////////////////////////////////////////////////
// CODE AREA
////////////////////////////////////////////////////////////////////////

// dirty inline func includes

#include "reverb.c"        
#include "adsr.c"

INLINE void InterpolateUp(SPUCHAN * pChannel)
{
 if(pChannel->SB[32]==1)                               // flag == 1? calc step and set flag... and don't change the value in this pass
  {
   const int id1=pChannel->SB[30]-pChannel->SB[29];    // curr delta to next val
   const int id2=pChannel->SB[31]-pChannel->SB[30];    // and next delta to next-next val :)

   pChannel->SB[32]=0;

   if(id1>0)                                           // curr delta positive
    {
     if(id2<id1)
      {pChannel->SB[28]=id1;pChannel->SB[32]=2;}
     else
     if(id2<(id1<<1))
      pChannel->SB[28]=(id1*pChannel->sinc)/0x10000L;
     else
      pChannel->SB[28]=(id1*pChannel->sinc)/0x20000L; 
    }
   else                                                // curr delta negative
    {
     if(id2>id1)
      {pChannel->SB[28]=id1;pChannel->SB[32]=2;}
     else
     if(id2>(id1<<1))
      pChannel->SB[28]=(id1*pChannel->sinc)/0x10000L;
     else
      pChannel->SB[28]=(id1*pChannel->sinc)/0x20000L; 
    }
  }
 else
 if(pChannel->SB[32]==2)                               // flag 1: calc step and set flag... and don't change the value in this pass
  {
   pChannel->SB[32]=0;

   pChannel->SB[28]=(pChannel->SB[28]*pChannel->sinc)/0x20000L;
   if(pChannel->sinc<=0x8000)
        pChannel->SB[29]=pChannel->SB[30]-(pChannel->SB[28]*((0x10000/pChannel->sinc)-1));
   else pChannel->SB[29]+=pChannel->SB[28];
  }
 else                                                  // no flags? add bigger val (if possible), calc smaller step, set flag1
  pChannel->SB[29]+=pChannel->SB[28];
}

//
// even easier interpolation on downsampling, also no special filter, again just "Pete's common sense" tm
//

INLINE void InterpolateDown(SPUCHAN * pChannel)
{
 if(pChannel->sinc>=0x20000L)                                // we would skip at least one val?
  {
   pChannel->SB[29]+=(pChannel->SB[30]-pChannel->SB[29])/2;  // add easy weight
   if(pChannel->sinc>=0x30000L)                              // we would skip even more vals?
    pChannel->SB[29]+=(pChannel->SB[31]-pChannel->SB[30])/2; // add additional next weight
  }
}

////////////////////////////////////////////////////////////////////////
// helpers for gauss interpolation

#define gval0 (((short*)(&pChannel->SB[29]))[gpos])
#define gval(x) (((short*)(&pChannel->SB[29]))[(gpos+x)&3])

#include "gauss_i.h"

////////////////////////////////////////////////////////////////////////

#include "xa.c"

////////////////////////////////////////////////////////////////////////
// START SOUND... called by main thread to setup a new sound on a channel
////////////////////////////////////////////////////////////////////////

INLINE void StartSound(SPUCHAN * pChannel)
{
 StartADSR(pChannel);
 StartREVERB(pChannel);      
                          
 pChannel->pCurr=pChannel->pStart;                     // set sample start
                         
 pChannel->s_1=0;                                      // init mixing vars
 pChannel->s_2=0;
 pChannel->iSBPos=28;

 pChannel->bNew=0;                                     // init channel flags
 pChannel->bStop=0;                                   
 pChannel->bOn=1;

 pChannel->SB[29]=0;                                   // init our interpolation helpers
 pChannel->SB[30]=0;

 if(iUseInterpolation>=2)                              // gauss interpolation?
      {pChannel->spos=0x30000L;pChannel->SB[28]=0;}    // -> start with more decoding
 else {pChannel->spos=0x10000L;pChannel->SB[31]=0;}    // -> no/simple interpolation starts with one 44100 decoding
}

////////////////////////////////////////////////////////////////////////
// ALL KIND OF HELPERS
////////////////////////////////////////////////////////////////////////

INLINE void VoiceChangeFrequency(SPUCHAN * pChannel)
{
 pChannel->iUsedFreq=pChannel->iActFreq;               // -> take it and calc steps
 pChannel->sinc=pChannel->iRawPitch<<4;
 if(!pChannel->sinc) pChannel->sinc=1;
 if(iUseInterpolation==1) pChannel->SB[32]=1;          // -> freq change in simle imterpolation mode: set flag
}

////////////////////////////////////////////////////////////////////////

INLINE void FModChangeFrequency(SPUCHAN * pChannel,int ns)
{
 int NP=pChannel->iRawPitch;

 NP=((32768L+iFMod[ns])*NP)/32768L;

 if(NP>0x3fff) NP=0x3fff;
 if(NP<0x1)    NP=0x1;

 NP=(44100L*NP)/(4096L);                               // calc frequency

 pChannel->iActFreq=NP;
 pChannel->iUsedFreq=NP;
 pChannel->sinc=(((NP/10)<<16)/4410);
 if(!pChannel->sinc) pChannel->sinc=1;
 if(iUseInterpolation==1) pChannel->SB[32]=1;          // freq change in simple interpolation mode
  
 iFMod[ns]=0;
}                    

////////////////////////////////////////////////////////////////////////

// noise handler... just produces some noise data
// surely wrong... and no noise frequency (spuCtrl&0x3f00) will be used...
// and sometimes the noise will be used as fmod modulation... pfff

INLINE int iGetNoiseVal(SPUCHAN * pChannel)
{
 int fa;

 if((dwNoiseVal<<=1)&0x80000000L)
  {
   dwNoiseVal^=0x0040001L;
   fa=((dwNoiseVal>>2)&0x7fff);
   fa=-fa;
  }
 else fa=(dwNoiseVal>>2)&0x7fff;

 // mmm... depending on the noise freq we allow bigger/smaller changes to the previous val
 fa=pChannel->iOldNoise+((fa-pChannel->iOldNoise)/((0x001f-((spuCtrl&0x3f00)>>9))+1));
 if(fa>32767L)  fa=32767L;
 if(fa<-32767L) fa=-32767L;              
 pChannel->iOldNoise=fa;

 if(iUseInterpolation<2)                               // no gauss/cubic interpolation?
  pChannel->SB[29] = fa;                               // -> store noise val in "current sample" slot
 return fa;
}                                 

////////////////////////////////////////////////////////////////////////

INLINE void StoreInterpolationVal(SPUCHAN * pChannel,int fa)
{
 if(pChannel->bFMod==2)                                // fmod freq channel
  pChannel->SB[29]=fa;
 else
  {
   if((spuCtrl&0x4000)==0) fa=0;                       // muted?
   else                                                // else adjust
    {
     if(fa>32767L)  fa=32767L;
     if(fa<-32767L) fa=-32767L;              
    }

   if(iUseInterpolation>=2)                            // gauss/cubic interpolation
    {     
     int gpos = pChannel->SB[28];
     gval0 = fa;
     gpos = (gpos+1) & 3;
     pChannel->SB[28] = gpos;
    }
   else
   if(iUseInterpolation==1)                            // simple interpolation
    {
     pChannel->SB[28] = 0;                    
     pChannel->SB[29] = pChannel->SB[30];              // -> helpers for simple linear interpolation: delay real val for two slots, and calc the two deltas, for a 'look at the future behaviour'
     pChannel->SB[30] = pChannel->SB[31];
     pChannel->SB[31] = fa;
     pChannel->SB[32] = 1;                             // -> flag: calc new interolation
    }
   else pChannel->SB[29]=fa;                           // no interpolation
  }
}

////////////////////////////////////////////////////////////////////////

INLINE int iGetInterpolationVal(SPUCHAN * pChannel)
{
 int fa;

 if(pChannel->bFMod==2) return pChannel->SB[29];

 switch(iUseInterpolation)
  {   
   //--------------------------------------------------//
   case 3:                                             // cubic interpolation
    {
     long xd;int gpos;
     xd = ((pChannel->spos) >> 1)+1;
     gpos = pChannel->SB[28];

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
     vl = (pChannel->spos >> 6) & ~3;
     gpos = pChannel->SB[28];
     vr=(gauss[vl]*gval0)&~2047;
     vr+=(gauss[vl+1]*gval(1))&~2047;
     vr+=(gauss[vl+2]*gval(2))&~2047;
     vr+=(gauss[vl+3]*gval(3))&~2047;
     fa = vr>>11;
    } break;
   //--------------------------------------------------//
   case 1:                                             // simple interpolation
    {
     if(pChannel->sinc<0x10000L)                       // -> upsampling?
          InterpolateUp(pChannel);                     // --> interpolate up
     else InterpolateDown(pChannel);                   // --> else down
     fa=pChannel->SB[29];
    } break;
   //--------------------------------------------------//
   default:                                            // no interpolation
    {
     fa=pChannel->SB[29];                  
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

static void *MAINThread(void *arg)
{
 int s_1,s_2,fa,ns,voldiv=iVolume;
 unsigned char * start;unsigned int nSample;
 int ch,predict_nr,shift_factor,flags,d,s;
 SPUCHAN * pChannel;
                            

 while(!bEndThread)                                    // until we are shutting down
  {
   //--------------------------------------------------//
   // ok, at the beginning we are looking if there is
   // enuff free place in the dsound/oss buffer to
   // fill in new data, or if there is a new channel to start.
   // if not, we wait (thread) or return (timer/spuasync)
   // until enuff free place is available/a new channel gets
   // started

   if(dwNewChannel)                                    // new channel should start immedately?
    {                                                  // (at least one bit 0 ... MAXCHANNEL is set?)
     iSecureStart++;                                   // -> set iSecure
     if(iSecureStart>5) iSecureStart=0;                //    (if it is set 5 times - that means on 5 tries a new samples has been started - in a row, we will reset it, to give the sound update a chance)
    }
   else iSecureStart=0;                                // 0: no new channel should start

   while(!iSecureStart && !bEndThread &&               // no new start? no thread end?
         (SoundGetBytesBuffered()>TESTSIZE))           // and still enuff data in sound buffer?
    {
     iSecureStart=0;                                   // reset secure

     return 0;                           // linux no-thread mode? bye
    }

   //--------------------------------------------------// continue from irq handling in timer mode? 

   if(lastch>=0)                                       // will be -1 if no continue is pending
    {
     ch=lastch; ns=lastns; lastch=-1;                  // -> setup all kind of vars to continue
     pChannel=&s_chan[ch];
     goto GOON;                                        // -> directly jump to the continue point
    }

   //--------------------------------------------------//
   //- main channel loop                              -// 
   //--------------------------------------------------//
    {
     pChannel=s_chan;
     for(ch=0;ch<MAXCHAN;ch++,pChannel++)              // loop em all... we will collect 1 ms of sound of each playing channel
      {
       if(pChannel->bNew) 
        {
         StartSound(pChannel);                         // start new sound
         dwNewChannel&=~(1<<ch);                       // clear new channel bit
        }

       if(!pChannel->bOn) continue;                    // channel not playing? next

       if(pChannel->iActFreq!=pChannel->iUsedFreq)     // new psx frequency?
        VoiceChangeFrequency(pChannel);

       ns=0;
       while(ns<NSSIZE)                                // loop until 1 ms of data is reached
        {
         if(pChannel->bFMod==1 && iFMod[ns])           // fmod freq channel
          FModChangeFrequency(pChannel,ns);

         while(pChannel->spos>=0x10000L)
          {
           if(pChannel->iSBPos==28)                    // 28 reached?
            {
             start=pChannel->pCurr;                    // set up the current pos

             if (start == (unsigned char*)-1)          // special "stop" sign
              {
               pChannel->bOn=0;                        // -> turn everything off
               pChannel->ADSRX.lVolume=0;
               pChannel->ADSRX.EnvelopeVol=0;
               goto ENDX;                              // -> and done for this channel
              }

             pChannel->iSBPos=0;

             //////////////////////////////////////////// spu irq handler here? mmm... do it later

             s_1=pChannel->s_1;
             s_2=pChannel->s_2;

             predict_nr=(int)*start;start++;
             shift_factor=predict_nr&0xf;
             predict_nr >>= 4;
             flags=(int)*start;start++;

             // -------------------------------------- // 

             for (nSample=0;nSample<28;start++)      
              {
               d=(int)*start;
               s=((d&0xf)<<12);
               if(s&0x8000) s|=0xffff0000;

               fa=(s >> shift_factor);
               fa=fa + ((s_1 * f[predict_nr][0])>>6) + ((s_2 * f[predict_nr][1])>>6);
               s_2=s_1;s_1=fa;
               s=((d & 0xf0) << 8);

               pChannel->SB[nSample++]=fa;

               if(s&0x8000) s|=0xffff0000;
               fa=(s>>shift_factor);              
               fa=fa + ((s_1 * f[predict_nr][0])>>6) + ((s_2 * f[predict_nr][1])>>6);
               s_2=s_1;s_1=fa;

               pChannel->SB[nSample++]=fa;
              }     

             //////////////////////////////////////////// flag handler

             if((flags&4) && (!pChannel->bIgnoreLoop))
              pChannel->pLoop=start-16;                // loop adress

             if(flags&1)                               // 1: stop/loop
              {
               // We play this block out first...
               //if(!(flags&2))                          // 1+2: do loop... otherwise: stop
               if(flags!=3 || pChannel->pLoop==NULL)   // PETE: if we don't check exactly for 3, loop hang ups will happen (DQ4, for example)
                {                                      // and checking if pLoop is set avoids crashes, yeah
                 start = (unsigned char*)-1;
                }
               else
                {
                 start = pChannel->pLoop;
                }
              }

             pChannel->pCurr=start;                    // store values for next cycle
             pChannel->s_1=s_1;
             pChannel->s_2=s_2;      

GOON: ;

            }

           fa=pChannel->SB[pChannel->iSBPos++];        // get sample data

           StoreInterpolationVal(pChannel,fa);         // store val for later interpolation

           pChannel->spos -= 0x10000L;
          }

         ////////////////////////////////////////////////
                                
         if(pChannel->bNoise)
              fa=iGetNoiseVal(pChannel);               // get noise val
         else fa=iGetInterpolationVal(pChannel);       // get sample val

         pChannel->sval=(MixADSR(pChannel)*fa)/1023;   // mix adsr

         if(pChannel->bFMod==2)                        // fmod freq channel
          iFMod[ns]=pChannel->sval;                    // -> store 1T sample data, use that to do fmod on next channel
         else                                          // no fmod freq channel
          {                                          
           //////////////////////////////////////////////
           // ok, left/right sound volume (psx volume goes from 0 ... 0x3fff)

             SSumL[ns]+=(pChannel->sval*pChannel->iLeftVolume)/0x4000L;
             SSumR[ns]+=(pChannel->sval*pChannel->iRightVolume)/0x4000L;
        
           //////////////////////////////////////////////
           // now let us store sound data for reverb    
                                                          
           if(pChannel->bRVBActive) StoreREVERB(pChannel,ns);
          }

         ////////////////////////////////////////////////
         // ok, go on until 1 ms data of this channel is collected
                                                       
         ns++;                                          
         pChannel->spos += pChannel->sinc;             
                                                              
        }        
ENDX:   ;                                                      
      }
    }                                                         
   
  //---------------------------------------------------//
  //- here we have another 1 ms of sound data
  //---------------------------------------------------//
  // mix XA infos (if any)

  MixXA();

  ///////////////////////////////////////////////////////
  // mix all channels (including reverb) into one buffer

  for(ns=0;ns<NSSIZE;ns++)
   {            
    SSumL[ns]+=MixREVERBLeft(ns);
                                              
    d=SSumL[ns]/voldiv;SSumL[ns]=0;
    if(d<-32767) d=-32767;if(d>32767) d=32767;
    *pS++=d;
        
    SSumR[ns]+=MixREVERBRight();

    d=SSumR[ns]/voldiv;SSumR[ns]=0;
    if(d<-32767) d=-32767;if(d>32767) d=32767;
    *pS++=d;
   }

  InitREVERB();

  //////////////////////////////////////////////////////                   
  // feed the sound
  // wanna have around 1/60 sec (16.666 ms) updates

  if(iCycle++>16)
   {
    SoundFeedStreamData((unsigned char*)pSpuBuffer,
                        ((unsigned char *)pS)-
                        ((unsigned char *)pSpuBuffer));
    pS=(short *)pSpuBuffer;
    iCycle=0;
   }
 }

 // end of big main loop...

 return 0;
}

////////////////////////////////////////////////////////////////////////
// SPU UPDATE... new epsxe func
//  1 time every 32 hsync lines
//  (312/32)x50 in pal
//  (262/32)x60 in ntsc
////////////////////////////////////////////////////////////////////////

// since epsxe 1.5.2 (linux) uses SPUupdate, not SPUasync, I will
// leave that func in the linux port, until epsxe linux is using
// the async function as well

void CALLBACK SPUupdate(void)
{
    MAINThread(0);                                      // -> linux high-compat mode
}

////////////////////////////////////////////////////////////////////////
// XA AUDIO
////////////////////////////////////////////////////////////////////////

void CALLBACK SPUplayADPCMchannel(xa_decode_t *xap)
{
 if(!xap)       return;
 if(!xap->freq) return;                                // no xa freq ? bye

 FeedXA(xap);                                          // call main XA feeder
}

void CALLBACK SPUplayCDDAchannel(unsigned char *pcm, int nbytes)
{
 if (!pcm)      return;
 if (nbytes<=0) return;

 FeedCDDA(pcm, nbytes);
}

////////////////////////////////////////////////////////////////////////
// INIT/EXIT STUFF
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// SPUINIT: this func will be called first by the main emu
////////////////////////////////////////////////////////////////////////
              
long CALLBACK SPUinit(void)
{
 memset((void *)s_chan,0,MAXCHAN*sizeof(SPUCHAN));
 memset((void *)&rvb,0,sizeof(REVERBInfo));
 InitADSR();
 return 0;
}

////////////////////////////////////////////////////////////////////////
// SETUPTIMER: init of certain buffers and threads/timers
////////////////////////////////////////////////////////////////////////

void SetupTimer(void)
{
 memset(SSumR,0,NSSIZE*sizeof(int));                   // init some mixing buffers
 memset(SSumL,0,NSSIZE*sizeof(int));
 memset(iFMod,0,NSSIZE*sizeof(int));

 pS=(short *)pSpuBuffer;                               // setup soundbuffer pointer

 bEndThread=0;                                         // init thread vars
 bSpuInit=1;                                           // flag: we are inited
}

////////////////////////////////////////////////////////////////////////
// REMOVETIMER: kill threads/timers
////////////////////////////////////////////////////////////////////////

void RemoveTimer(void)
{
 bEndThread=1;                                         // raise flag to end thread
 bSpuInit=0;
}

////////////////////////////////////////////////////////////////////////
// SETUPSTREAMS: init most of the spu buffers
////////////////////////////////////////////////////////////////////////

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
  (uint32_t *)malloc(16384 * sizeof(uint32_t));
 CDDAEnd   = CDDAStart + 16384;
 CDDAPlay  = CDDAStart;
 CDDAFeed  = CDDAStart;

 for(i=0;i<MAXCHAN;i++)                                // loop sound channels
  {
   s_chan[i].ADSRX.SustainLevel = 0xf<<27;             // -> init sustain
   s_chan[i].pLoop=spuMemC;
   s_chan[i].pStart=spuMemC;
   s_chan[i].pCurr=spuMemC;
  }
}

////////////////////////////////////////////////////////////////////////
// REMOVESTREAMS: free most buffer
////////////////////////////////////////////////////////////////////////

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


////////////////////////////////////////////////////////////////////////
// SPUOPEN: called by main emu after init
////////////////////////////////////////////////////////////////////////
long SPUopen(void)
{
 if(bSPUIsOpen) return 0;                              // security for some stupid main emus

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

////////////////////////////////////////////////////////////////////////
// SPUCLOSE: called before shutdown
////////////////////////////////////////////////////////////////////////

long CALLBACK SPUclose(void)
{
 if(!bSPUIsOpen) return 0;                             // some security

 bSPUIsOpen=0;                                         // no more open

 RemoveTimer();                                        // no more feeding

 RemoveSound();                                        // no more sound handling

 RemoveStreams();                                      // no more streaming

 return 0;
}

////////////////////////////////////////////////////////////////////////
// SPUSHUTDOWN: called by main emu on final exit
////////////////////////////////////////////////////////////////////////

long CALLBACK SPUshutdown(void)
{
 return 0;
}