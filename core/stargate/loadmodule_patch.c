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

#include <pspsdk.h>
#include <pspiofilemgr_kernel.h>
#include <psploadcore.h>
#include <pspsysmem_kernel.h>
#include <stdio.h>
#include <string.h>
#include <systemctrl.h>
#include <macros.h>
#include "loadmodule_patch.h"

// Original Function Pointer
int (* _sceKernelLoadModule)(char * fname, int flag, void * opt) = NULL;
int (* _sceUtilityLoadModule)(int id) = NULL;
int (* _sceUtilityUnloadModule)(int id) = NULL;

// Find Original Function Pointer
void getLoadModuleFuncs(void)
{
	// Find Original Function Pointer
	_sceKernelLoadModule = (void*)sctrlHENFindFunction("sceModuleManager", "ModuleMgrForUser", 0x977DE386);
	_sceUtilityLoadModule = (void*)sctrlHENFindFunction("sceUtility_Driver", "sceUtility", 0x2A2B3DE0);
	_sceUtilityUnloadModule = (void*)sctrlHENFindFunction("sceUtility_Driver", "sceUtility", 0xE49BFE92);
}

// sceKernelLoadModule Hook
int myKernelLoadModule(char * fname, int flag, void * opt)
{
	// Forward Call
	int result = _sceKernelLoadModule(fname, flag, opt);

	// Invalid PRX Type Error
	if(result == 0x80020148 || result == 0x80020130)
	{
		// Load Call was aimed at "ms0:"
		if(0 == strncmp(fname, "ms0:", sizeof("ms0:") - 1))
		{
			// Change it into invalid boot device to act like OFW
			result = 0x80020146;
		}
	}
	
	// Return Result
	return result;
}

// sceUtilityLoadModule Hook
int myUtilityLoadModule(int id)
{
	// Forward Call
	int result = _sceUtilityLoadModule(id);
	
	// Fake NPDRM Load in umdemu Runlevel
	if(result == 0x80020139 && id == 0x500) return 0;
	
	// Return Result
	return result;
}

// sceUtilityUnloadModule Hook
int myUtilityUnloadModule(int id)
{
	// Forward Call
	int result = _sceUtilityUnloadModule(id);
	
	// Fake NPDRM Unload in umdemu Runlevel
	if(result == 0x80111103 && id == 0x500) return 0;
	
	// Return Result
	return result;
}

// Hook Table
struct HookMap g_hookMap[] =
{
	{ "sceUtility", 0x2A2B3DE0, myUtilityLoadModule },
	{ "sceUtility", 0xE49BFE92, myUtilityUnloadModule },
	{ "ModuleMgrForUser", 0x977DE386, myKernelLoadModule },
};

// Hook Load Module Functions
void patchLoadModuleFuncs(SceModule2 * mod)
{
	// Iterate Hooks
	unsigned int i = 0; for(; i < NELEMS(g_hookMap); ++i)
	{
		// Hook Import
		hookImportByNID(mod, g_hookMap[i].libName, g_hookMap[i].nid, g_hookMap[i].funcAddr);
	}
}

