#include <pspsdk.h>
#include <pspinit.h>
#include <pspiofilemgr.h>
#include <pspsysmem_kernel.h>
#include <rebootex.h>
#include <systemctrl.h>

#include <ark.h>
#include "macros.h"
#include "module2.h"

struct LbaParams {
    int unknown1; // 0
    int cmd; // 4
    int lba_top; // 8
    int lba_size; // 12
    int byte_size_total;  // 16
    int byte_size_centre; // 20
    int byte_size_start; // 24
    int byte_size_last;  // 28
};

// Default Game ID
static const struct {
    unsigned char unk[0x44];
    char id[9];
    int empty;
} defaultdata = { {0}, "HOME00000", 0 };

int readGameIdFromDisc(){
    // Open Disc Identifier
    int disc = sceIoOpen("disc0:/UMD_DATA.BIN", PSP_O_RDONLY, 0777);
    // Opened Disc Identifier
    if(disc >= 0)
    {
        // Read Country Code
        sceIoRead(disc, rebootex_config.game_id, 4);
        
        // Skip Delimiter
        sceIoLseek32(disc, 1, PSP_SEEK_CUR);
        
        // Read Game ID
        sceIoRead(disc, rebootex_config.game_id + 0x4, 5);
        
        // Close Disc Identifier
        sceIoClose(disc);
        return 1;
    }
    return 0;
}

int readGameIdFromPBP(){
    int n = 10;
    int res = sctrlGetInitPARAM("DISC_ID", NULL, &n, rebootex_config.game_id);
    if (res < 0) return 0;
    return 1;
}

int readGameIdFromISO(){

    int (*iso_read)(u32 offset, void *ptr, u32 data_len) =
        sctrlHENFindFunction("PRO_Inferno_Driver", "inferno_driver", 0xB573209C);
    if (!iso_read) return 0;

    int res = iso_read(16*2048+883, rebootex_config.game_id, sizeof(rebootex_config.game_id));
    if (res != sizeof(rebootex_config.game_id))
        return 0;

    // remove the dash in the middle: ULUS-01234 -> ULUS01234
    rebootex_config.game_id[4] = rebootex_config.game_id[5];
    rebootex_config.game_id[5] = rebootex_config.game_id[6];
    rebootex_config.game_id[6] = rebootex_config.game_id[7];
    rebootex_config.game_id[7] = rebootex_config.game_id[8];
    rebootex_config.game_id[8] = rebootex_config.game_id[9];
    rebootex_config.game_id[9] = 0;

    return 1;
}

void findGameId(){

    int apitype = sceKernelInitApitype();
    if (apitype == 0x141 || apitype == 0x152 || apitype >= 0x200 || apitype == 0x120 || apitype == 0x160){
        return;
    }

    void * (* SysMemForKernel_EF29061C)(void) = (void *)sctrlHENFindFunction("sceSystemMemoryManager", "SysMemForKernel", 0xEF29061C);
    unsigned char * gameinfo = NULL;

    if (sceKernelFindModuleByName("PRO_Inferno_Driver") != NULL){
        readGameIdFromISO();
    }
    else {
        readGameIdFromPBP();
    }

    if (rebootex_config.game_id[0] != 0){
        memcpy(defaultdata.id, rebootex_config.game_id, 9);
    }

    if (SysMemForKernel_EF29061C && (gameinfo=SysMemForKernel_EF29061C())) {
        memcpy(gameinfo+0x44, rebootex_config.game_id, 9);
    }

}

// Fixed Game Info Getter Function
void * getUMDDataFixed(void)
{

    // Find Function
    void * (* SysMemForKernel_EF29061C)(void) = (void *)sctrlHENFindFunction("sceSystemMemoryManager", "SysMemForKernel", 0xEF29061C);
    
    // Function unavailable (how?!)
    if(SysMemForKernel_EF29061C == NULL) return &defaultdata;
    
    // Get Game Info Structure
    unsigned char * gameinfo = SysMemForKernel_EF29061C();
    
    // Structure unavailable
    if(gameinfo == NULL) return &defaultdata;
    
    // Set Game ID if we know it
    if (rebootex_config.game_id[0] != 0){
        memcpy(gameinfo + 0x44, rebootex_config.game_id, 9);
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
        hookImportByNID(mod, "SysMemForKernel", 0xEF29061C, getUMDDataFixed);
    }
}

