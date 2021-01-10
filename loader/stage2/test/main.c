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

// ARK.BIN mockup (to test H.BIN)

#include <psptypes.h>
#include <graphics.h>

// Entry Point
int _start(ARKConfig* arg0, FunctionTable* arg1) __attribute__((section(".text.startup")));
int _start(ARKConfig* arg0, FunctionTable* arg1)
{

	clearBSS();

    cls();
    PRTSTR1("ARK-4 test successfuly running in: %s", arg0->arkpath);

	while (1){
	}
}

void memset(u8* start, u8 data, u32 size){
	u32 i = 0;
	for (; i<size; i++){
		start[i] = data;
	}
}

// Clear BSS Segment of Payload
void clearBSS(void)
{
	// BSS Start and End Address from Linkfile
	extern char __bss_start, __bss_end;
	
	// Clear Memory
	memset(&__bss_start, 0, &__bss_end - &__bss_start);
}
