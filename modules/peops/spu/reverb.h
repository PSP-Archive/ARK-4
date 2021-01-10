/***************************************************************************
                          reverb.h  -  description
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
// 2002/05/15 - Pete
// - generic cleanup for the Peops release
//
//*************************************************************************//


extern void SetREVERB(unsigned short val);
extern INLINE void InitREVERB(void);
extern INLINE void StartREVERB(SPUCHAN * pChannel);
extern INLINE void StoreREVERB(SPUCHAN * pChannel,int ns);
extern INLINE int MixREVERBLeft(int ns);
extern INLINE int MixREVERBRight(void);

