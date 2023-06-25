#include <pspsdk.h>
#include <pspiofilemgr.h>
#include <pspsysmem_kernel.h>

#include "globals.h"
#include "macros.h"
#include "module2.h"

// Fixed Game Info Getter Function
void * SysMemForKernel_EF29061C_Fixed(void)
{

    // Default Game ID
    static const char * defaultid = "HOME00000";

    // Find Function
    void * (* SysMemForKernel_EF29061C)(void) = (void *)sctrlHENFindFunction("sceSystemMemoryManager", "SysMemForKernel", 0xEF29061C);
    
    // Function unavailable (how?!)
    if(SysMemForKernel_EF29061C == NULL) return defaultid;
    
    // Get Game Info Structure
    void * gameinfo = SysMemForKernel_EF29061C();
    
    // Structure unavailable
    if(gameinfo == NULL) return defaultid;
    
    if (!readGameIdFromDisc(gameinfo+0x44)){
        // Set Default Game ID
        memcpy(gameinfo + 0x44, defaultid, 9);
    }
    
    // Return Game Info Structure
    return gameinfo;
}

// Patch Game ID Getter
void patchGameInfoGetter(SceModule2 * mod)
{
    // Kernel Module
    if((mod->text_addr & 0x80000000) != 0)
    {
        // Hook Import
        hookImportByNID(mod, "SysMemForKernel", 0xEF29061C, SysMemForKernel_EF29061C_Fixed);
    }
}
