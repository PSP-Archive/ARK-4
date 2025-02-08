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

//*************************************************************************//
// History of changes:
//
// 2004/09/18 - LDChen
// - pre-calculated ADSRX values
//
// 2003/02/09 - kode54
// - removed &0x3fff from reverb volume registers, fixes a few games,
//   hopefully won't be breaking anything
//
// 2003/01/19 - Pete
// - added Neill's reverb
//
// 2003/01/06 - Pete
// - added Neill's ADSR timings
//
// 2002/05/15 - Pete
// - generic cleanup for the Peops release
//
//*************************************************************************//

#include "stdafx.h"

#define _IN_REGISTERS

#include "externals.h"
#include "registers.h"
#include "regs.h"
#include "reverb.h"

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
       s_chan[ch].pStart=spuMemC+((unsigned long) val<<3);
       break;
     //------------------------------------------------// level with pre-calcs
     case 8:
     {
        //---------------------------------------------//
        s_chan[ch].ADSRX.AttackModeExp=(val&0x8000)?1:0; 
        s_chan[ch].ADSRX.AttackRate = ((val>>8) & 0x007f)^0x7f;
        s_chan[ch].ADSRX.DecayRate = 4*(((val>>4) & 0x000f)^0x1f);
        s_chan[ch].ADSRX.SustainLevel = (val & 0x000f) << 27;
        //---------------------------------------------//
     }
      break;
     //------------------------------------------------// adsr times with pre-calcs
     case 10:
     {
       //----------------------------------------------//
       s_chan[ch].ADSRX.SustainModeExp = (val&0x8000)?1:0;
       s_chan[ch].ADSRX.SustainIncrease= (val&0x4000)?0:1;
       s_chan[ch].ADSRX.SustainRate = ((val>>6) & 0x007f)^0x7f;
       s_chan[ch].ADSRX.ReleaseModeExp = (val&0x0020)?1:0;
       s_chan[ch].ADSRX.ReleaseRate = 4*((val & 0x001f)^0x1f);
       //----------------------------------------------//
       }
     break;
     //------------------------------------------------//
     case 14:                                          // loop?
       s_chan[ch].pLoop=spuMemC+((unsigned long) val<<3);
       s_chan[ch].bIgnoreLoop=1;
       break;
     //------------------------------------------------//
    }

   return;
  }

 switch(r)
   {
    //-------------------------------------------------//
    case H_SPUctrl:
      spuCtrl=val;
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
    case H_SPUrvolL:
      rvb.VolLeft=val;
      break;
    //-------------------------------------------------//
    case H_SPUrvolR:
      rvb.VolRight=val;
      break;
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
      break;
    case H_CDRight:
      iRightXAVol=val & 0x7fff;
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
     s_chan[ch].bIgnoreLoop=0;
     s_chan[ch].bNew=1;
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
