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

// #include "stdafx.h"

#define _IN_REGISTERS

#include "externals.h"
#include "registers.h"

static void SoundOn(int start,int end,unsigned short val);
static void SoundOff(int start,int end,unsigned short val);
static void FModOn(int start,int end,unsigned short val);
static void NoiseOn(int start,int end,unsigned short val);
static void SetVolumeL(unsigned char ch,short vol);
static void SetVolumeR(unsigned char ch,short vol);
static void SetPitch(int ch,unsigned short val);
static void ReverbOn(int start,int end,unsigned short val);

////////////////////////////////////////////////////////////////////////
// WRITE REGISTERS: called by main emu
////////////////////////////////////////////////////////////////////////

void SPUwriteRegister(unsigned long reg, unsigned short val)
{
 const unsigned long r=reg&0xfff;
 // regArea[(r-0xc00)>>1] = val;

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
       // taken from regArea later
       break;
     //------------------------------------------------// level with pre-calcs
     case 8:
       {
        const unsigned long lval=val;
        //---------------------------------------------//
        s_chan[ch].ADSRX.AttackModeExp=(lval&0x8000)?1:0; 
        s_chan[ch].ADSRX.AttackRate=(lval>>8) & 0x007f;
        s_chan[ch].ADSRX.DecayRate=(lval>>4) & 0x000f;
        s_chan[ch].ADSRX.SustainLevel=lval & 0x000f;
        //---------------------------------------------//
       }
      break;
     //------------------------------------------------// adsr times with pre-calcs
     case 10:
      {
       const unsigned long lval=val;

       //----------------------------------------------//
       s_chan[ch].ADSRX.SustainModeExp = (lval&0x8000)?1:0;
       s_chan[ch].ADSRX.SustainIncrease= (lval&0x4000)?0:1;
       s_chan[ch].ADSRX.SustainRate = (lval>>6) & 0x007f;
       s_chan[ch].ADSRX.ReleaseModeExp = (lval&0x0020)?1:0;
       s_chan[ch].ADSRX.ReleaseRate = lval & 0x001f;
       //----------------------------------------------//
      }
     break;
     //------------------------------------------------// adsr volume... mmm have to investigate this
     case 12:
       break;
     //------------------------------------------------//
     case 14:                                          // loop?
       s_chan[ch].pLoop=spuMemC+((val&~1)<<3);
       break;
     //------------------------------------------------//
    }
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
      spuMem[spuAddr>>1] = val;
      spuAddr+=2;
      if(spuAddr>0x7ffff) spuAddr=0;
      break;
    //-------------------------------------------------//
    case H_SPUctrl:
      if(!(spuCtrl & CTRL_IRQ))
        spuStat&=~STAT_IRQ;
      spuCtrl=val;
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
          // sync-with-decode-buffers hack..
          if(rvb.StartAddr==0x3ff00)
            rvb.CurrAddr+=decode_pos/2;
         }
       }
      rvb.dirty = 1;
      break;
    //-------------------------------------------------//
    case H_SPUirqAddr:
      spuIrq = val;
      pSpuIrq=spuMemC+(((unsigned long) val<<3)&~0xf);
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
      iLeftXAVol=val  & 0x7fff;
      if(cddavCallback) cddavCallback(0,val);
      break;
    case H_CDRight:
      iRightXAVol=val & 0x7fff;
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
    case H_Reverb+0   : rvb.FB_SRC_A=val*4;            break;
    case H_Reverb+2   : rvb.FB_SRC_B=val*4;            break;
    case H_Reverb+4   : rvb.IIR_ALPHA=(short)val;      break;
    case H_Reverb+6   : rvb.ACC_COEF_A=(short)val;     break;
    case H_Reverb+8   : rvb.ACC_COEF_B=(short)val;     break;
    case H_Reverb+10  : rvb.ACC_COEF_C=(short)val;     break;
    case H_Reverb+12  : rvb.ACC_COEF_D=(short)val;     break;
    case H_Reverb+14  : rvb.IIR_COEF=(short)val;       break;
    case H_Reverb+16  : rvb.FB_ALPHA=(short)val;       break;
    case H_Reverb+18  : rvb.FB_X=(short)val;           break;
    case H_Reverb+20  : rvb.IIR_DEST_A0=val*4;         break;
    case H_Reverb+22  : rvb.IIR_DEST_A1=val*4;         break;
    case H_Reverb+24  : rvb.ACC_SRC_A0=val*4;          break;
    case H_Reverb+26  : rvb.ACC_SRC_A1=val*4;          break;
    case H_Reverb+28  : rvb.ACC_SRC_B0=val*4;          break;
    case H_Reverb+30  : rvb.ACC_SRC_B1=val*4;          break;
    case H_Reverb+32  : rvb.IIR_SRC_A0=val*4;          break;
    case H_Reverb+34  : rvb.IIR_SRC_A1=val*4;          break;
    case H_Reverb+36  : rvb.IIR_DEST_B0=val*4;         break;
    case H_Reverb+38  : rvb.IIR_DEST_B1=val*4;         break;
    case H_Reverb+40  : rvb.ACC_SRC_C0=val*4;          break;
    case H_Reverb+42  : rvb.ACC_SRC_C1=val*4;          break;
    case H_Reverb+44  : rvb.ACC_SRC_D0=val*4;          break;
    case H_Reverb+46  : rvb.ACC_SRC_D1=val*4;          break;
    case H_Reverb+48  : rvb.IIR_SRC_B1=val*4;          break;
    case H_Reverb+50  : rvb.IIR_SRC_B0=val*4;          break;
    case H_Reverb+52  : rvb.MIX_DEST_A0=val*4;         break;
    case H_Reverb+54  : rvb.MIX_DEST_A1=val*4;         break;
    case H_Reverb+56  : rvb.MIX_DEST_B0=val*4;         break;
    case H_Reverb+58  : rvb.MIX_DEST_B1=val*4;         break;
    case H_Reverb+60  : rvb.IN_COEF_L=(short)val;      break;
    case H_Reverb+62  : rvb.IN_COEF_R=(short)val;      break;
   }

 if ((r & ~0x3f) == H_Reverb)
  rvb.dirty = 1; // recalculate on next update
}

////////////////////////////////////////////////////////////////////////
// READ REGISTER: called by main emu
////////////////////////////////////////////////////////////////////////

int SPUreadRegister(unsigned long reg, unsigned short * result)
{
 const unsigned long r=reg&0xfff;
        
 if(r>=0x0c00 && r<0x0d80)
  {
   switch(r&0x0f)
    {
     case 12:                                          // get adsr vol
      {
       const int ch=(r>>4)-0xc0;
       if(dwNewChannel&(1<<ch)) return 1;              // we are started, but not processed? return 1
       if((dwChannelOn&(1<<ch)) &&                     // same here... we haven't decoded one sample yet, so no envelope yet. return 1 as well
          !s_chan[ch].ADSRX.EnvelopeVol)
        {
        	*result = 1;
        	return 1;
        }
       *result = (unsigned short)(s_chan[ch].ADSRX.EnvelopeVol>>16);
       return 1;
      }

     case 14:                                          // get loop address
      {
       const int ch=(r>>4)-0xc0;
       *result = (unsigned short)((s_chan[ch].pLoop-spuMemC)>>3);
       return 1;
      }
    }
  }

 switch(r)
  {
    case H_SPUctrl:
     *result = spuCtrl;
     return 1;

    case H_SPUstat:
     *result = spuStat;
     return 1;
        
    case H_SPUaddr:
     *result = (unsigned short)(spuAddr>>3);
     return 1;

    case H_SPUdata:
     {
      unsigned short s=spuMem[spuAddr>>1];
      spuAddr+=2;
      if(spuAddr>0x7ffff) spuAddr=0;
      *result = s;
      return 1;
     }

    case H_SPUirqAddr:
     *result = spuIrq;
     return 1;

    //case H_SPUIsOn1:
    // return IsSoundOn(0,16);

    //case H_SPUIsOn2:
    // return IsSoundOn(16,24);
 
  }

 return 0;
}
 
////////////////////////////////////////////////////////////////////////
// SOUND ON register write
////////////////////////////////////////////////////////////////////////

static void SoundOn(int start,int end,unsigned short val)
{
 int ch;

 for(ch=start;ch<end;ch++,val>>=1)                     // loop channels
  {
   if((val&1) && regAreaGet(ch,6))                     // mmm... start has to be set before key on !?!
    {
     // do this here, not in StartSound
     // - fixes fussy timing issues
     s_chan[ch].bStop=0;
     s_chan[ch].pCurr=spuMemC+((regAreaGet(ch,6)&~1)<<3); // must be block aligned
     s_chan[ch].pLoop=spuMemC+((regAreaGet(ch,14)&~1)<<3);
     s_chan[ch].prevflags=2;

     dwNewChannel|=(1<<ch);                            // bitfield for faster testing
     dwChannelOn|=1<<ch;
     dwChannelDead&=~(1<<ch);
    }
  }
}

////////////////////////////////////////////////////////////////////////
// SOUND OFF register write
////////////////////////////////////////////////////////////////////////

static void SoundOff(int start,int end,unsigned short val)
{
 int ch;
 for(ch=start;ch<end;ch++,val>>=1)                     // loop channels
  {
   if(val&1)                                           // && s_chan[i].bOn)  mmm...
    {
     s_chan[ch].bStop=1;

     // Jungle Book - Rhythm 'n Groove
     // - turns off buzzing sound (loop hangs)
     dwNewChannel &= ~(1<<ch);
    }                                                  
  }
}

////////////////////////////////////////////////////////////////////////
// FMOD register write
////////////////////////////////////////////////////////////////////////

static void FModOn(int start,int end,unsigned short val)
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
     if(ch>0&&s_chan[ch-1].bFMod==2)
      s_chan[ch-1].bFMod=0;
    }
  }
}

////////////////////////////////////////////////////////////////////////
// NOISE register write
////////////////////////////////////////////////////////////////////////

static void NoiseOn(int start,int end,unsigned short val)
{
 int ch;

 for(ch=start;ch<end;ch++,val>>=1)                     // loop channels
  {
   s_chan[ch].bNoise=val&1;                            // -> noise on/off
  }
}

////////////////////////////////////////////////////////////////////////
// LEFT VOLUME register write
////////////////////////////////////////////////////////////////////////

// please note: sweep and phase invert are wrong... but I've never seen
// them used

static void SetVolumeL(unsigned char ch,short vol)     // LEFT VOLUME
{
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

static void SetVolumeR(unsigned char ch,short vol)     // RIGHT VOLUME
{
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

static void SetPitch(int ch,unsigned short val)               // SET PITCH
{
 int NP;
 if(val>0x3fff) NP=0x3fff;                             // get pitch val
 else           NP=val;

 s_chan[ch].iRawPitch=NP;
 s_chan[ch].sinc=(NP<<4)|8;
 if(iUseInterpolation==1) s_chan[ch].SB[32]=1;         // -> freq change in simple interpolation mode: set flag
}

////////////////////////////////////////////////////////////////////////
// REVERB register write
////////////////////////////////////////////////////////////////////////

static void ReverbOn(int start,int end,unsigned short val)
{
 int ch;

 for(ch=start;ch<end;ch++,val>>=1)                     // loop channels
  {
   s_chan[ch].bReverb=val&1;                           // -> reverb on/off
  }
}
