/*
    Custom Emulator Firmware
    Copyright (C) 2012-2014, ColdBird/Total_Noob/Acid_Snake

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __MAIN_H__
#define __MAIN_H__

#include "spu/stdafx.h"

void CALLBACK SPUupdate(void);
void CALLBACK SPUplayADPCMchannel(xa_decode_t *xap);
void CALLBACK SPUplayCDDAchannel(unsigned char *pcm, int nbytes);
void CALLBACK SPUwriteRegister(unsigned long reg, unsigned short val);
long CALLBACK SPUinit(void);
long SPUopen(void);
long CALLBACK SPUshutdown(void);
long CALLBACK SPUclose(void);

#endif