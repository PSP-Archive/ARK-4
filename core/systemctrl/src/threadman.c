#include <sdk.h>
#include <psploadexec.h>
#include <psploadexec_kernel.h>
#include <psputility_modules.h>
#include <module2.h>
#include <macros.h>
#include <systemctrl_se.h>
#include <string.h>

int sctrlGetThreadUIDByName(const char * name)
{
	// Invalid Arguments
	if(name == NULL) return -1;
	
	// Thread UID List
	int ids[100];
	
	// Clear Memory
	memset(ids, 0, sizeof(ids));
	
	// Thread Counter
	int count = 0;

    int (*KernelGetThreadmanIdList)() = sctrlHENFindFunction("sceThreadManager", "ThreadManForUser", 0x94416130);
    int (*KernelReferThreadStatus)() = sctrlHENFindFunction("sceThreadManager", "ThreadManForUser", 0x17C1684E);
	
	// Get Thread UIDs
	if(KernelGetThreadmanIdList(SCE_KERNEL_TMID_Thread, ids, NELEMS(ids), &count) >= 0)
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
			if(KernelReferThreadStatus(ids[i], &info) == 0)
			{
				// Matching Name
				if(strcmp(info.name, name) == 0)
				{
					
					// Return Thread UID
					return ids[i];
				}
			}
		}
	}
	
	// Thread not found
	return -2;
}