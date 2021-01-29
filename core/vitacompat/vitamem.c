#include "vitamem.h"

extern ARKConfig* ark_config;

void (* SetMemoryPartitionTable)(void *sysmem_config, SceSysmemPartTable *table);

void SetMemoryPartitionTablePatched(void *sysmem_config, SceSysmemPartTable *table)
{
    SetMemoryPartitionTable(sysmem_config, table);
    // Add partition 11
    table->extVshell.addr = 0x0B000000;
    table->extVshell.size = 12 * 1024 * 1024;
}

int PatchSysMem(void *a0, void *sysmem_config)
{
    int (* module_bootstart)(SceSize args, void *sysmem_config) = (void *)_lw((u32)a0 + 0x28);

    u32 text_addr = 0x88000000;
    int i;

    for (i = 0; i < 0x12000; i += 4) {
        if ((_lw(text_addr + i) == 0x8C860010) && (_lw(text_addr + i + 8) == 0xAE060008)) {
            // Patch to add new partition
            _sw(JAL(SetMemoryPartitionTablePatched), text_addr + i + 4);
        } else if ((_lw(text_addr + i) == 0x2405000C) && (_lw(text_addr + i + 8) == 0x00608821)) {
            // Change attribute to 0xF (user accessable)
            _sh(0xF, text_addr + i);
        } else if (_lw(text_addr + i) == 0x8C830014) {
            // Patch to add new partition
            SetMemoryPartitionTable = (void *)(text_addr + i);
            break;
        }
    }

    // Patch to add new partition
    //SetMemoryPartitionTable = (void *)text_addr + 0x11BD8;
    //_sw(JAL(SetMemoryPartitionTablePatched), text_addr + 0x10F70);

    // Change attribute to 0xF (user accessable)
    //_sh(0xF, text_addr + 0x115F8);

    flushCache();

    return module_bootstart(4, sysmem_config);
}

void patchVitaMem(){
    //12 MB extra ram through p11 on Vita
    if (ark_config->exec_mode != PSV_POPS){
        u32 i;
        for (i = 0; i < 0x4000; i += 4) {
            u32 addr = 0x88600000 + i;
            // Patch call to SysMem module_bootstart
            if (_lw(addr) == 0x24040004) {
                _sw(0x02402021, addr); //move $a0, $s2
                _sw(JAL(PatchSysMem), addr + 0x64);
                continue;
            }
        }
    }
}

void handleVitaMemory(){
    // unprotect extra RAM available on PS Vita
    u32 *prot = (u32 *)0xBC000040;
    for (int i = 0; i < 0x10; i++)
        prot[i] = 0xFFFFFFFF;
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
    
    // Open Disc Identifier
    int disc = sceIoOpen("disc0:/UMD_DATA.BIN", PSP_O_RDONLY, 0777);
    
    // Opened Disc Identifier
    if(disc >= 0)
    {
        // Read Country Code
        sceIoRead(disc, gameinfo + 0x44, 4);
        
        // Skip Delimiter
        sceIoLseek32(disc, 1, PSP_SEEK_CUR);
        
        // Read Game ID
        sceIoRead(disc, gameinfo + 0x48, 5);
        
        // Close Disc Identifier
        sceIoClose(disc);
    }
    else{
        // Set Default Game ID
        memcpy(gameinfo + 0x44, defaultid, strlen(defaultid));
    }
    
    // Return Game Info Structure
    return gameinfo;
}
