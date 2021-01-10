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

#include <pspiofilemgr.h>
#include <stdio.h>
#include <string.h>
#include <systemctrl.h>
#include "imports.h"
#include "exception.h"

// Exception Handler
PspDebugErrorHandler curr_handler = NULL;

// Register Snapshot
PspDebugRegBlock * exception_regs = NULL;

// ASM Exception Handler Payload
void _pspDebugExceptionHandler(void);

// Bluescreen Exception Handler
void bluescreenHandler(PspDebugRegBlock * regs);

// Bluescreen Register Snapshot
static PspDebugRegBlock bluescreenRegs;

/*
// Exception Code String Literals
static const char * codeTxt[32] =
{
    "Interrupt", "TLB modification", "TLB load/inst fetch", "TLB store",
    "Address load/inst fetch", "Address store", "Bus error (instr)",
    "Bus error (data)", "Syscall", "Breakpoint", "Reserved instruction",
    "Coprocessor unusable", "Arithmetic overflow", "Unknown 14",
    "Unknown 15", "Unknown 16", "Unknown 17", "Unknown 18", "Unknown 19",
    "Unknown 20", "Unknown 21", "Unknown 22", "Unknown 23", "Unknown 24",
    "Unknown 25", "Unknown 26", "Unknown 27", "Unknown 28", "Unknown 29",
    "Unknown 31"
};

// Register String Literals
static const unsigned char regName[32][5] =
{
    "zr", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"
};
*/

// Register Exception Handler
void registerExceptionHandler(PspDebugErrorHandler handler, PspDebugRegBlock * regs)
{
	// Valid Arguments
	if(handler != NULL && regs != NULL)
	{
		// Save Arguments
		curr_handler = handler;
		exception_regs = regs;
	}
	
	// Register Bluescreen Handler
	else
	{
		// Save Arguments
		curr_handler = bluescreenHandler;
		exception_regs = &bluescreenRegs;
	}
	
	// Register Exception Handler
	sceKernelRegisterDefaultExceptionHandler((void *)_pspDebugExceptionHandler);
}

// Bluescreen Exception Handler
void bluescreenHandler(PspDebugRegBlock * regs)
{
	// TODO Code Exception Handler
}

