/***************************************************************************
                           regs.h  -  description
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

void SoundOn(int start,int end,unsigned short val);
void SoundOff(int start,int end,unsigned short val);
void FModOn(int start,int end,unsigned short val);
void NoiseOn(int start,int end,unsigned short val);
void SetVolumeL(unsigned char ch,short vol);
void SetVolumeR(unsigned char ch,short vol);
void SetPitch(int ch,unsigned short val);
void ReverbOn(int start,int end,unsigned short val);
void CALLBACK SPUwriteRegister(unsigned long reg, unsigned short val);

