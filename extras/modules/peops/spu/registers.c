/***************************************************************************
                         registers.c  -  description
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

#define _IN_REGISTERS

#include "externals.h"
#include "registers.h"
#include "regs.h"
#include "reverb.h"

/*
// adsr time values (in ms) by James Higgs ... see the end of
// the adsr.c source for details

#define ATTACK_MS     514L
#define DECAYHALF_MS  292L
#define DECAY_MS      584L
#define SUSTAIN_MS    450L
#define RELEASE_MS    446L
*/

// we have a timebase of 1.020408f ms, not 1 ms... so adjust adsr defines
#define ATTACK_MS      494L
#define DECAYHALF_MS   286L
#define DECAY_MS       572L
#define SUSTAIN_MS     441L
#define RELEASE_MS     437L






int Check_IRQ( int addr, int force ) {
	if(spuCtrl & CTRL_IRQ)         // some callback and irq active?
	{
		if( ( bIrqHit == 0 ) &&
				( force == 1 || pSpuIrq == spuMemC+addr ) )
		{
			if(irqCallback)
				irqCallback();                        // -> call main emu

			// one-time
			bIrqHit = 1;
			spuStat |= STAT_IRQ;

#if 0
			MessageBox( NULL, "IRQ", "SPU", MB_OK );
#endif

			return 1;
		}
	}


	return 0;
}



////////////////////////////////////////////////////////////////////////
// WRITE REGISTERS: called by main emu
////////////////////////////////////////////////////////////////////////

void CALLBACK SPUwriteRegister(unsigned long reg, unsigned short val)
{
 const unsigned long r=reg&0xfff;

 if(r>=0x0c00 && r<0x0d80)                             // some channel info?
  {
   int ch=(r>>4)-0xc0;                                 // calc channel
   switch(r&0x0f)
    {
     //------------------------------------------------// r volume
     case 0:                                           
       SetVolumeL((unsigned char)ch,val);
       break;
     //------------------------------------------------// l volume
     case 2:                                           
       SetVolumeR((unsigned char)ch,val);
       break;
     //------------------------------------------------// pitch
     case 4:                                           
       SetPitch(ch,val);
       break;
     //------------------------------------------------// start
     case 6:      
			 // Brain Dead 13 - align to 16 boundary
       s_chan[ch].pStart= spuMemC+(unsigned long)((val<<3)&~0xf);
       break;
     //------------------------------------------------// level with pre-calcs
     case 8:
       {
        const unsigned long lval=val;unsigned long lx;
        //---------------------------------------------//
        s_chan[ch].ADSRX.AttackModeExp=(lval&0x8000)?1:0; 
        s_chan[ch].ADSRX.AttackRate=(lval>>8) & 0x007f;
        s_chan[ch].ADSRX.DecayRate=(lval>>4) & 0x000f;
        s_chan[ch].ADSRX.SustainLevel=lval & 0x000f;
        //---------------------------------------------//
        if(!iDebugMode) break;
        //---------------------------------------------// stuff below is only for debug mode

        s_chan[ch].ADSR.AttackModeExp=(lval&0x8000)?1:0;        //0x007f

        lx=(((lval>>8) & 0x007f)>>2);                  // attack time to run from 0 to 100% volume
        lx=min(31,lx);                                 // no overflow on shift!
        if(lx) 
         { 
          lx = (1<<lx);
          if(lx<2147483) lx=(lx*ATTACK_MS)/10000L;     // another overflow check
          else           lx=(lx/10000L)*ATTACK_MS;
          if(!lx) lx=1;
         }
        s_chan[ch].ADSR.AttackTime=lx;                

        s_chan[ch].ADSR.SustainLevel=                 // our adsr vol runs from 0 to 1024, so scale the sustain level
         (1024*((lval) & 0x000f))/15;

        lx=(lval>>4) & 0x000f;                         // decay:
        if(lx)                                         // our const decay value is time it takes from 100% to 0% of volume
         {
          lx = ((1<<(lx))*DECAY_MS)/10000L;
          if(!lx) lx=1;
         }
        s_chan[ch].ADSR.DecayTime =                   // so calc how long does it take to run from 100% to the wanted sus level
         (lx*(1024-s_chan[ch].ADSR.SustainLevel))/1024;
       }
      break;
     //------------------------------------------------// adsr times with pre-calcs
     case 10:
      {
       const unsigned long lval=val;unsigned long lx;

       //----------------------------------------------//
       s_chan[ch].ADSRX.SustainModeExp = (lval&0x8000)?1:0;
       s_chan[ch].ADSRX.SustainIncrease= (lval&0x4000)?0:1;
       s_chan[ch].ADSRX.SustainRate = (lval>>6) & 0x007f;
       s_chan[ch].ADSRX.ReleaseModeExp = (lval&0x0020)?1:0;
       s_chan[ch].ADSRX.ReleaseRate = lval & 0x001f;
       //----------------------------------------------//
       if(!iDebugMode) break;
       //----------------------------------------------// stuff below is only for debug mode

       s_chan[ch].ADSR.SustainModeExp = (lval&0x8000)?1:0;
       s_chan[ch].ADSR.ReleaseModeExp = (lval&0x0020)?1:0;
                   
       lx=((((lval>>6) & 0x007f)>>2));                 // sustain time... often very high
       lx=min(31,lx);                                  // values are used to hold the volume
       if(lx)                                          // until a sound stop occurs
        {                                              // the highest value we reach (due to 
         lx = (1<<lx);                                 // overflow checking) is: 
         if(lx<2147483) lx=(lx*SUSTAIN_MS)/10000L;     // 94704 seconds = 1578 minutes = 26 hours... 
         else           lx=(lx/10000L)*SUSTAIN_MS;     // should be enuff... if the stop doesn't 
         if(!lx) lx=1;                                 // come in this time span, I don't care :)
        }
       s_chan[ch].ADSR.SustainTime = lx;

       lx=(lval & 0x001f);
       s_chan[ch].ADSR.ReleaseVal     =lx;
       if(lx)                                          // release time from 100% to 0%
        {                                              // note: the release time will be
         lx = (1<<lx);                                 // adjusted when a stop is coming,
         if(lx<2147483) lx=(lx*RELEASE_MS)/10000L;     // so at this time the adsr vol will 
         else           lx=(lx/10000L)*RELEASE_MS;     // run from (current volume) to 0%
         if(!lx) lx=1;
        }
       s_chan[ch].ADSR.ReleaseTime=lx;

       if(lval & 0x4000)                               // add/dec flag
            s_chan[ch].ADSR.SustainModeDec=-1;
       else s_chan[ch].ADSR.SustainModeDec=1;
      }
     break;
     //------------------------------------------------// adsr volume... mmm have to investigate this
     case 12:
       break;
     //------------------------------------------------//
     case 14:                                          // loop?
       //WaitForSingleObject(s_chan[ch].hMutex,2000);        // -> no multithread fuckups
       
			 s_chan[ch].pLoop=spuMemC+((unsigned long)((val<<3)&~0xf));
       
			 //s_chan[ch].bIgnoreLoop=1;
       //ReleaseMutex(s_chan[ch].hMutex);                    // -> oki, on with the thread
       break;
     //------------------------------------------------//
    }
   iSpuAsyncWait=0;
   return;
  }

 switch(r)
   {
    //-------------------------------------------------//
    case H_SPUaddr:
      spuAddr = (unsigned long) val<<3;
      break;
    //-------------------------------------------------//
    case H_SPUdata:
      // BIOS - allow dma 00
      Check_IRQ( spuAddr, 0 );

      spuMem[spuAddr>>1] = val;
      spuAddr+=2;
      if(spuAddr>0x7ffff) spuAddr=0;
      break;
    //-------------------------------------------------//
    case H_SPUctrl:
      spuCtrl=val;


			// flags
			if( spuCtrl & CTRL_CD_PLAY )
				spuStat |= CTRL_CD_PLAY;
			else
				spuStat &= ~CTRL_CD_PLAY;

			if( spuCtrl & CTRL_CD_REVERB )
				spuStat |= STAT_CD_REVERB;
			else
				spuStat &= ~STAT_CD_REVERB;


			if( spuCtrl & CTRL_EXT_PLAY )
				spuStat |= STAT_EXT_PLAY;
			else
				spuStat &= ~STAT_EXT_PLAY;

			if( spuCtrl & CTRL_EXT_REVERB )
				spuStat |= STAT_EXT_REVERB;
			else
				spuStat &= ~STAT_EXT_REVERB;


			
			spuStat &= ~(STAT_DMA_NON | STAT_DMA_R | STAT_DMA_W);

			if( spuCtrl & CTRL_DMA_F )
				spuStat |= STAT_DMA_F;

			if( (spuCtrl & CTRL_DMA_F) == CTRL_DMA_R )
				spuStat |= STAT_DMA_R;



			// reset IRQ flag
			if( (spuCtrl & CTRL_IRQ) == 0 ) {
				bIrqHit = 0;
				spuStat &= ~STAT_IRQ;
			}


			dwNoiseClock = (spuCtrl & CTRL_NOISE)>>8;
      break;
    //-------------------------------------------------//
    case H_SPUstat:
      spuStat=val & 0xf800;
      break;
    //-------------------------------------------------//
    case H_SPUReverbAddr:
      if(val==0xFFFF || val<=0x200)
       {rvb.StartAddr=rvb.CurrAddr=0;}
      else
       {
        const long iv=(unsigned long)val<<2;
        if(rvb.StartAddr!=iv)
         {
          rvb.StartAddr=(unsigned long)val<<2;
          rvb.CurrAddr=rvb.StartAddr;
         }
       }
      break;
    //-------------------------------------------------//
    case H_SPUirqAddr:
      spuIrq = val;
      pSpuIrq=spuMemC+((unsigned long) val<<3);
      break;
    //-------------------------------------------------//
    case H_SPUrvolL:
      rvb.VolLeft=val;
      break;
    //-------------------------------------------------//
    case H_SPUrvolR:
      rvb.VolRight=val;
      break;
    //-------------------------------------------------//

/*
    case H_ExtLeft:
     //auxprintf("EL %d\n",val);
      break;
    //-------------------------------------------------//
    case H_ExtRight:
     //auxprintf("ER %d\n",val);
      break;
    //-------------------------------------------------//
    case H_SPUmvolL:
     //auxprintf("ML %d\n",val);
      break;
    //-------------------------------------------------//
    case H_SPUmvolR:
     //auxprintf("MR %d\n",val);
      break;
    //-------------------------------------------------//
    case H_SPUMute1:
     //auxprintf("M0 %04x\n",val);
      break;
    //-------------------------------------------------//
    case H_SPUMute2:
     //auxprintf("M1 %04x\n",val);
      break;
*/
    //-------------------------------------------------//
    case H_SPUon1:
      SoundOn(0,16,val);
      break;
    //-------------------------------------------------//
     case H_SPUon2:
      SoundOn(16,24,val);
      break;
    //-------------------------------------------------//
    case H_SPUoff1:
      SoundOff(0,16,val);
      break;
    //-------------------------------------------------//
    case H_SPUoff2:
      SoundOff(16,24,val);
      break;
    //-------------------------------------------------//
    case H_CDLeft:
			iLeftXAVol = val;
			if(cddavCallback) cddavCallback(0,val);
			break;
    case H_CDRight:
			iRightXAVol = val;
			if(cddavCallback) cddavCallback(1,val);
			break;
    //-------------------------------------------------//
    case H_FMod1:
      FModOn(0,16,val);
      break;
    //-------------------------------------------------//
    case H_FMod2:
      FModOn(16,24,val);
      break;
    //-------------------------------------------------//
    case H_Noise1:
      NoiseOn(0,16,val);
      break;
    //-------------------------------------------------//
    case H_Noise2:
      NoiseOn(16,24,val);
      break;
    //-------------------------------------------------//
    case H_RVBon1:
      ReverbOn(0,16,val);
      break;
    //-------------------------------------------------//
    case H_RVBon2:
      ReverbOn(16,24,val);
      break;
    //-------------------------------------------------//
    case H_Reverb+0:

      rvb.FB_SRC_A=val;

      // OK, here's the fake REVERB stuff...
      // depending on effect we do more or less delay and repeats... bah
      // still... better than nothing :)

      SetREVERB(val);
      break;


    case H_Reverb+2   : rvb.FB_SRC_B=(short)val;       break;
    case H_Reverb+4   : rvb.IIR_ALPHA=(short)val;      break;
    case H_Reverb+6   : rvb.ACC_COEF_A=(short)val;     break;
    case H_Reverb+8   : rvb.ACC_COEF_B=(short)val;     break;
    case H_Reverb+10  : rvb.ACC_COEF_C=(short)val;     break;
    case H_Reverb+12  : rvb.ACC_COEF_D=(short)val;     break;
    case H_Reverb+14  : rvb.IIR_COEF=(short)val;       break;
    case H_Reverb+16  : rvb.FB_ALPHA=(short)val;       break;
    case H_Reverb+18  : rvb.FB_X=(short)val;           break;
    case H_Reverb+20  : rvb.IIR_DEST_A0=(short)val;    break;
    case H_Reverb+22  : rvb.IIR_DEST_A1=(short)val;    break;
    case H_Reverb+24  : rvb.ACC_SRC_A0=(short)val;     break;
    case H_Reverb+26  : rvb.ACC_SRC_A1=(short)val;     break;
    case H_Reverb+28  : rvb.ACC_SRC_B0=(short)val;     break;
    case H_Reverb+30  : rvb.ACC_SRC_B1=(short)val;     break;
    case H_Reverb+32  : rvb.IIR_SRC_A0=(short)val;     break;
    case H_Reverb+34  : rvb.IIR_SRC_A1=(short)val;     break;
    case H_Reverb+36  : rvb.IIR_DEST_B0=(short)val;    break;
    case H_Reverb+38  : rvb.IIR_DEST_B1=(short)val;    break;
    case H_Reverb+40  : rvb.ACC_SRC_C0=(short)val;     break;
    case H_Reverb+42  : rvb.ACC_SRC_C1=(short)val;     break;
    case H_Reverb+44  : rvb.ACC_SRC_D0=(short)val;     break;
    case H_Reverb+46  : rvb.ACC_SRC_D1=(short)val;     break;
    case H_Reverb+48  : rvb.IIR_SRC_B1=(short)val;     break;
    case H_Reverb+50  : rvb.IIR_SRC_B0=(short)val;     break;
    case H_Reverb+52  : rvb.MIX_DEST_A0=(short)val;    break;
    case H_Reverb+54  : rvb.MIX_DEST_A1=(short)val;    break;
    case H_Reverb+56  : rvb.MIX_DEST_B0=(short)val;    break;
    case H_Reverb+58  : rvb.MIX_DEST_B1=(short)val;    break;
    case H_Reverb+60  : rvb.IN_COEF_L=(short)val;      break;
    case H_Reverb+62  : rvb.IN_COEF_R=(short)val;      break;
   }

 iSpuAsyncWait=0;
}

////////////////////////////////////////////////////////////////////////
// SOUND ON register write
////////////////////////////////////////////////////////////////////////

void SoundOn(int start,int end,unsigned short val)     // SOUND ON PSX COMAND
{
 int ch;

 for(ch=start;ch<end;ch++,val>>=1)                     // loop channels
  {
   if((val&1) && s_chan[ch].pStart)                    // mmm... start has to be set before key on !?!
    {
		 s_chan[ch].bLoopJump = 0;
     s_chan[ch].bNew=1;

		 // do this here, not in StartSound
		 // - fixes fussy timing issues
		 s_chan[ch].iSilent=0;
		 s_chan[ch].bStop=0;
		 s_chan[ch].bOn=1;
		 s_chan[ch].pCurr=s_chan[ch].pStart;

#if 0
		 // ADSR init time (guess to # apu cycles)
		 s_chan[ch].ADSRX.StartDelay = 0;
#endif

		 // Final Fantasy 7 - don't do any of these
		 // - sets loop address before VoiceOn
		 //s_chan[ch].pLoop = s_chan[ch].pStart;

     dwNewChannel|=(1<<ch);                            // bitfield for faster testing
    }
  }
}

////////////////////////////////////////////////////////////////////////
// SOUND OFF register write
////////////////////////////////////////////////////////////////////////

void SoundOff(int start,int end,unsigned short val)    // SOUND OFF PSX COMMAND
{
 int ch;
 for(ch=start;ch<end;ch++,val>>=1)                     // loop channels
  {
   if(val&1)                                           // && s_chan[i].bOn)  mmm...
    {
     s_chan[ch].bStop=1;

		 // Jungle Book - Rhythm 'n Groove
		 // - turns off buzzing sound (loop hangs)
		 s_chan[ch].bNew=0;
		 dwNewChannel &= ~(1<<ch);
		}                                                  
  }
}

////////////////////////////////////////////////////////////////////////
// FMOD register write
////////////////////////////////////////////////////////////////////////

void FModOn(int start,int end,unsigned short val)      // FMOD ON PSX COMMAND
{
 int ch;

 for(ch=start;ch<end;ch++,val>>=1)                     // loop channels
  {
   if(val&1)                                           // -> fmod on/off
    {
     if(ch>0) 
      {
       s_chan[ch].bFMod=1;                             // --> sound channel
       s_chan[ch-1].bFMod=2;                           // --> freq channel
      }
    }
   else
    {
     s_chan[ch].bFMod=0;                               // --> turn off fmod
    }
  }
}

////////////////////////////////////////////////////////////////////////
// NOISE register write
////////////////////////////////////////////////////////////////////////

void NoiseOn(int start,int end,unsigned short val)     // NOISE ON PSX COMMAND
{
 int ch;

 for(ch=start;ch<end;ch++,val>>=1)                     // loop channels
  {
   if(val&1)                                           // -> noise on/off
    {
     s_chan[ch].bNoise=1;
    }
   else 
    {
     s_chan[ch].bNoise=0;
    }
  }
}

////////////////////////////////////////////////////////////////////////
// LEFT VOLUME register write
////////////////////////////////////////////////////////////////////////

// please note: sweep and phase invert are wrong... but I've never seen
// them used

void SetVolumeL(unsigned char ch,short vol)            // LEFT VOLUME
{
 s_chan[ch].iLeftVolRaw=vol;

 if(vol&0x8000)                                        // sweep?
  {
   short sInc=1;                                       // -> sweep up?
   if(vol&0x2000) sInc=-1;                             // -> or down?
   if(vol&0x1000) vol^=0xffff;                         // -> mmm... phase inverted? have to investigate this
   vol=((vol&0x7f)+1)/2;                               // -> sweep: 0..127 -> 0..64
   vol+=vol/(2*sInc);                                  // -> HACK: we don't sweep right now, so we just raise/lower the volume by the half!
   vol*=128;
  }
 else                                                  // no sweep:
  {
   if(vol&0x4000)                                      // -> mmm... phase inverted? have to investigate this
    //vol^=0xffff;
    vol=0x3fff-(vol&0x3fff);
  }

 vol&=0x3fff;
 s_chan[ch].iLeftVolume=vol;                           // store volume
}

////////////////////////////////////////////////////////////////////////
// RIGHT VOLUME register write
////////////////////////////////////////////////////////////////////////

void SetVolumeR(unsigned char ch,short vol)            // RIGHT VOLUME
{
 s_chan[ch].iRightVolRaw=vol;

 if(vol&0x8000)                                        // comments... see above :)
  {
   short sInc=1;
   if(vol&0x2000) sInc=-1;
   if(vol&0x1000) vol^=0xffff;
   vol=((vol&0x7f)+1)/2;        
   vol+=vol/(2*sInc);
   vol*=128;
  }
 else            
  {
   if(vol&0x4000) //vol=vol^=0xffff;
    vol=0x3fff-(vol&0x3fff);
  }

 vol&=0x3fff;

 s_chan[ch].iRightVolume=vol;
}

////////////////////////////////////////////////////////////////////////
// PITCH register write
////////////////////////////////////////////////////////////////////////

void SetPitch(int ch,unsigned short val)               // SET PITCH
{
 int NP;
 if(val>0x3fff) NP=0x3fff;                             // get pitch val
 else           NP=val;

 s_chan[ch].iRawPitch=NP;

 NP=(44100L*NP)/4096L;                                 // calc frequency
 if(NP<1) NP=1;                                        // some security
 s_chan[ch].iActFreq=NP;                               // store frequency
}

////////////////////////////////////////////////////////////////////////
// REVERB register write
////////////////////////////////////////////////////////////////////////

void ReverbOn(int start,int end,unsigned short val)    // REVERB ON PSX COMMAND
{
 int ch;

 for(ch=start;ch<end;ch++,val>>=1)                     // loop channels
  {
   if(val&1)                                           // -> reverb on/off
    {
     s_chan[ch].bReverb=1;
    }
   else 
    {
     s_chan[ch].bReverb=0;
    }
  }
}
