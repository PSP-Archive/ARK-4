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

#include <systemctrl.h>
#include <macros.h>
#include <pspthreadman.h>
#include <string.h>

// Find UID of the specified Thread
int sctrlGetThreadUIDByName(const char * name)
{
	// Invalid Arguments
	if(name == NULL) return -1;
	
	// Elevate Permission Level
	unsigned int k1 = pspSdkSetK1(0);
	
	// Thread UID List
	int ids[100];
	
	// Clear Memory
	memset(ids, 0, sizeof(ids));
	
	// Thread Counter
	int count = 0;
	
	// Get Thread UIDs
	if(sceKernelGetThreadmanIdList(SCE_KERNEL_TMID_Thread, ids, NELEMS(ids), &count) >= 0)
	{
		// Iterate Results
		int i = 0; for(; i < count; i++)
		{
			// Thread Information
			SceKernelThreadInfo info;
			
			// Clear Memory
			memset(&info, 0, sizeof(info));
			
			// Initialize Structure Size
			info.size = sizeof(info);
			
			// Fetch Thread Status
			if(sceKernelReferThreadStatus(ids[i], &info) == 0)
			{
				// Matching Name
				if(strcmp(info.name, name) == 0)
				{
					// Restore Permission Level
					pspSdkSetK1(k1);
					
					// Return Thread UID
					return ids[i];
				}
			}
		}
	}
	
	// Restore Permission Level
	pspSdkSetK1(k1);
	
	// Thread not found
	return -2;
}

// Return Thread Context of specified Thread (Search by UID)
int sctrlGetThreadContextByUID(int uid, SceKernelThreadKInfo * ctx)
{
	// Invalid Arguments
	if(uid < 0 || ctx == NULL) return -1;
	
	// Elevate Permission Level
	unsigned int k1 = pspSdkSetK1(0);
	
	// Thread Kernel Info Structure
	SceKernelThreadKInfo info;
	
	// Clear Memory
	memset(&info, 0, sizeof(info));
	
	// Set Structure Size
	info.size = sizeof(info);
	
	// Found Thread Info
	int found = 0;
	
	// Disable Interrupts
	unsigned int interrupts = pspSdkDisableInterrupts();
	
	// Found Thread Information
	if(ThreadManForKernel_2D69D086(uid, &info) == 0)
	{
		// Output Context Buffer
		void * outputContext = NULL;
		
		// Output Context Buffer available
		if(ctx->thContext != NULL)
		{
			// Backup Buffer Address
			outputContext = ctx->thContext;
			
			// Context available
			if(info.thContext != NULL)
			{
				// Copy Context
				memcpy(outputContext, info.thContext, sizeof(SceThreadContext));
			}
		}
		
		// Copy Kernel Thread Information
		memcpy(ctx, &info, sizeof(info));
		
		// Restore Context Output Buffer
		ctx->thContext = outputContext;
		
		// Found Thread Info
		found = 1;
	}
	
	// Enable Interrupts
	pspSdkEnableInterrupts(interrupts);
	
	// Restore Permission Level
	pspSdkSetK1(k1);
	
	// Found Thread Info
	if(found) return 0;
	
	// Thread Context not found
	return -2;
}

// Return Thread Context of specified Thread (Search by Name)
int sctrlGetThreadContextByName(const char * name, SceKernelThreadKInfo * ctx)
{
	// Forward Call
	return sctrlGetThreadContextByUID(sctrlGetThreadUIDByName(name), ctx);
}

