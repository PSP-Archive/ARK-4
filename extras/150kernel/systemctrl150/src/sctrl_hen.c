#include <pspkernel.h>
#include <systemctrl.h>
#include <macros.h>

extern void (* g_module_start_handler)(SceModule2 *);

// Register Prologue Module Start Handler
STMOD_HANDLER sctrlHENSetStartModuleHandler(STMOD_HANDLER new_handler)
{
    // Get Previous Handler
    STMOD_HANDLER on_module_start = g_module_start_handler;
    
    // Register Handler
    g_module_start_handler = (void *)(KERNELIFY(new_handler));
    
    // Return Previous Handler
    return on_module_start;
}

int sctrlKernelExitVSH(struct SceKernelLoadExecVSHParam *param)
{
    u32 k1;
    int ret = -1;
    k1 = pspSdkSetK1(0);
    int (*_KernelExitVSH)(void*) = sctrlHENFindFunction("sceLoadExec", "LoadExecForKernel", 0xA3D5E142);
    ret = _KernelExitVSH(param);
    pspSdkSetK1(k1);
    return ret;
}


extern void* custom_rebootex;
void sctrlHENSetRebootexOverride(const u8 *rebootex)
{
    if (rebootex != NULL) // override rebootex
        custom_rebootex = rebootex;
}

extern int (* LoadRebootOverrideHandler)(void * arg1, unsigned int arg2, void * arg3, unsigned int arg4);
void sctrlHENSetLoadRebootOverrideHandler(int (* func)(void * arg1, unsigned int arg2, void * arg3, unsigned int arg4))
{
    LoadRebootOverrideHandler = func;
}

unsigned int sctrlHENFindFunction(char * szMod, char * szLib, unsigned int nid)
{
    // Find Target Module
    SceModule2 * pMod = (SceModule2 *)sceKernelFindModuleByName(szMod);
    
    // Module not found
    if(pMod == NULL)
    {
        // Attempt to find it by Address
        pMod = (SceModule2 *)sceKernelFindModuleByAddress((unsigned int)szMod);
        
        // Module not found
        if(pMod == NULL) return 0;
    }
    
    // Fetch Export Table Start Address
    void * entTab = pMod->ent_top;
    
    // Iterate Exports
    for (int i = 0; i < pMod->ent_size;)
    {
        // Cast Export Table Entry
        struct SceLibraryEntryTable * entry = (struct SceLibraryEntryTable *)(entTab + i);
        
        // Found Matching Library
        if(entry->libname != NULL && szLib != NULL && 0 == strcmp(entry->libname, szLib))
        {
            // Accumulate Function and Variable Exports
            unsigned int total = entry->stubcount + entry->vstubcount;
            
            // NID + Address Table
            unsigned int * vars = entry->entrytable;
            
            // Exports available
            if(total > 0)
            {
                // Iterate Exports
                for(int j = 0; j < total; j++)
                {
                    // Found Matching NID
                    if(vars[j] == nid) return vars[total + j];
                }
            }
        }
        
        // Move Pointer
        i += (entry->len * 4);
    }
    
    // Function not found
    return 0;
}

// Replace Function in Syscall Table
void sctrlHENPatchSyscall(void * addr, void * newaddr)
{
    // Syscall Table
    unsigned int * syscalls = NULL;
    
    // Get Syscall Table Pointer from Coprocessor
    __asm__ volatile("cfc0 %0, $12\n" : "=r"(syscalls));
    
    // Invalid Syscall Table
    if(syscalls == NULL) return;
    
    // Skip Table Header
    syscalls += 4; // 4 * 4 = 16
    
    // Iterate Syscalls
    for(int i = 0; i < 0xFF4; ++i)
    {
        // Found Matching Function
        if((syscalls[i] & 0x0FFFFFFF) == (((unsigned int)addr) & 0x0FFFFFFF))
        {
            // Replace Syscall Function
            syscalls[i] = (unsigned int)newaddr;
        }
    }
}
