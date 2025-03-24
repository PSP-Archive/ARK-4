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
    
    int k1 = pspSdkSetK1(0);

    // Get Thread UIDs
    if (sceKernelGetThreadmanIdList(SCE_KERNEL_TMID_Thread, ids, NELEMS(ids), &count) >= 0)
    {
        // Iterate Results
        for(int i = 0; i < count; i++)
        {
        	// Thread Information
        	SceKernelThreadInfo info;
        	
        	// Clear Memory
        	memset(&info, 0, sizeof(info));
        	
        	// Initialize Structure Size
        	info.size = sizeof(info);
        	
        	// Fetch Thread Status
        	if (sceKernelReferThreadStatus(ids[i], &info) == 0)
        	{
        		// Matching Name
        		if(strcmp(info.name, name) == 0)
        		{
        			pspSdkSetK1(k1);
        			// Return Thread UID
        			return ids[i];
        		}
        	}
        }
    }
    pspSdkSetK1(k1);
    // Thread not found
    return -2;
}