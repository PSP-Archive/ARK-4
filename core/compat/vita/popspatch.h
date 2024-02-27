/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

#ifndef _PROSPU_H_
#define _PROSPU_H_

/*
 * These functions have been extracted from 660 pops_05g.prx !
 * They build the core callbacks for our SPU emulator to bring sound onto Vita.
 */

// SPU Background Thread Starter
void _sceMeAudio_DE630CD2(void * loopCore, void * stack);

// Shutdown SPU
void spuShutdown(void);

void patchPspPops(SceModule2 * mod);
void patchPspPopsman(SceModule2* mod);

#endif

