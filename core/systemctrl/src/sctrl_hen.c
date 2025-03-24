#include <pspsdk.h>
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <pspthreadman_kernel.h>
#include <pspsysevent.h>
#include <pspiofilemgr.h>
#include <pspsysmem_kernel.h>
#include <pspinit.h>
#include <systemctrl.h>
#include <systemctrl_private.h>
#include <stdio.h>
#include <string.h>
#include <module2.h>
#include <ark.h>
#include <macros.h>
#include <functions.h>
#include "rebootex.h"
#include "nidresolver.h"
#include "modulemanager.h"
#include "loadercore.h"
#include "imports.h"
#include "sysmem.h"


int sctrlHENSetMemory(u32 p2, u32 p9)
{
    return 0;
}

// Get HEN Version
int sctrlHENGetVersion()
{
    return ( (ARK_MAJOR_VERSION << 24) | (ARK_MINOR_VERSION << 16) | (ARK_MICRO_VERSION << 8) );
}

// Get HEN Minor Version
int sctrlHENGetMinorVersion()
{
    return ARK_REVISION;
}

int sctrlHENIsSE()
{
    return 1;
}

int sctrlHENIsDevhook()
{
    return 0;
}

// Find Filesystem Driver
PspIoDrv * sctrlHENFindDriver(char * drvname)
{
    // Elevate Permission Level
    unsigned int k1 = pspSdkSetK1(0);
    
    // Find Function
    int * (* findDriver)(char * drvname) = (void*)findFirstJAL(sctrlHENFindFunction("sceIOFileManager", "IoFileMgrForKernel", 0x76DA16E3));
    
    // Find Driver Wrapper
    int * wrapper = findDriver(drvname);
    
    // Search Result
    PspIoDrv * driver = NULL;
    
    // Found Driver Wrapper
    if(wrapper != NULL)
    {
        // Save Driver Pointer
        driver = (PspIoDrv *)(wrapper[1]);
    }
    
    // Restore Permission Level
    pspSdkSetK1(k1);

    if (driver == NULL){
        if(0 == stricmp(drvname, "msstor")) {
        	return sctrlHENFindDriver("eflash0a0f");
        }

        if(0 == stricmp(drvname, "msstor0p")) {
        	return sctrlHENFindDriver("eflash0a0f1p");
        }
    }
    
    // Return Driver
    return driver;
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

// Find Function Address
unsigned int sctrlHENFindFunction(char * szMod, char * szLib, unsigned int nid)
{
    // Get NID Resolver
    NidResolverLib * resolver = getNidResolverLib(szLib);
    
    // Found Resolver for Library
    if(resolver != NULL)
    {
        // Resolve NID
        nid = getNidReplacement(resolver, nid);
    }
    
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

u32 sctrlHENGetInitControl()
{
    if (kernel_init_apitype == NULL)
        return 0;
    return (u32)kernel_init_apitype - 8;
}

u32 sctrlHENFindImport(const char *szMod, const char *szLib, u32 nid)
{
    SceModule2 *mod = sceKernelFindModuleByName(szMod);
    if(!mod) return 0;

    for(int i = 0; i < mod->stub_size;)
    {
        SceLibraryStubTable *stub = (SceLibraryStubTable *)(mod->stub_top + i);

        if(stub->libname && strcmp(stub->libname, szLib) == 0)
        {
            u32 *table = stub->nidtable;

            for(int j = 0; j < stub->stubcount; j++)
            {
                if(table[j] == nid)
                {
                    return ((u32)stub->stubtable + (j * 8));
                }
            }
        }

        i += (stub->len * 4);
    }

    return 0;
}

void* sctrlHENGetArkConfig(ARKConfig* conf){
    if (conf) memcpy(conf, ark_config, sizeof(ARKConfig));
    return ark_config;
}

void sctrlHENSetArkConfig(ARKConfig* conf){
    memcpy(ark_config, conf, sizeof(ARKConfig));
}

void sctrlHENLoadModuleOnReboot(char *module_before, void *buf, int size, int flags)
{
    rebootex_config.rtm_mod.before = module_before;
    rebootex_config.rtm_mod.buffer = buf;
    rebootex_config.rtm_mod.size = size;
    rebootex_config.rtm_mod.flags = flags;
}

void sctrlHENSetSpeed(int cpuspd, int busspd)
{
    int k1 = pspSdkSetK1(0);
    SetSpeed(cpuspd, busspd);
    pspSdkSetK1(k1);
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

extern int (*ExtendDecryption)();
void* sctrlHENRegisterKDecryptHandler(int (* func)())
{
    int (* r)() = ExtendDecryption;
    ExtendDecryption = func;
    return r;
}

extern int (*MesgLedDecryptEX)();
void* sctrlHENRegisterMDecryptHandler(int (* func)())
{
    int (* r)() = (void *)MesgLedDecryptEX;
    MesgLedDecryptEX = (void *)func;
    return (void *)r;
}

extern void (*lle_handler)(void*);
void sctrlHENRegisterLLEHandler(void* handler)
{
    lle_handler = handler;
}

int sctrlHENRegisterHomebrewLoader(void* handler)
{
    // register handler and patch leda
    patchLedaPlugin(handler);
    return 0;
}

extern void* plugin_handler;
void* sctrlHENSetPluginHandler(void* handler){
    void* ret = plugin_handler;
    plugin_handler = handler;
    return ret;
}

RebootConfigARK* sctrlHENGetRebootexConfig(RebootConfigARK* config){
    if (config){
        memcpy(config, &rebootex_config, sizeof(RebootConfigARK));
    }
    return &rebootex_config;
}

void sctrlHENSetRebootexConfig(RebootConfigARK* config){
    if (config){
        memcpy(&rebootex_config, config, sizeof(RebootConfigARK));
    }
}

u32 sctrlHENFakeDevkitVersion(){
    return FW_660;
}

int sctrlHENIsToolKit()
{
    int ret = 0; // Retail
    int k1 = pspSdkSetK1(0);
    int level = sctrlKernelSetUserLevel(8);
    
    if (ark_config->exec_mode == PSP_ORIG){
        SceIoStat stat;
        if (sceIoGetstat("flash0:/kd/vshbridge_tool.prx", &stat) >= 0)
            ark_config->exec_mode = PSP_TOOL;
    }

    if (ark_config->exec_mode == PSP_TOOL){
        int baryon_ver = 0;
        int (*getBaryonVer)(void*) = sctrlHENFindFunction("sceSYSCON_Driver", "sceSyscon_driver", 0x7EC5A957);
        if (getBaryonVer) getBaryonVer(&baryon_ver);
        if (baryon_ver == 0x00020601){
            ret = 2; // DevelopmentTool
        }
        else {
            ret = 1; // TestingTool
        }
    }

    sctrlKernelSetUserLevel(level);
    pspSdkSetK1(k1);
    return ret;
}
