#include <pspsdk.h>
#include <pspinit.h>
#include <pspiofilemgr.h>
#include <pspsysmem_kernel.h>
#include <rebootconfig.h>
#include <systemctrl.h>

#include "globals.h"
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

static const char* HOME_ID = "HOME00000";
extern RebootConfigARK rebootex_config;

int readGameIdFromDisc(char* gameid){
    // Open Disc Identifier
    int disc = sceIoOpen("disc0:/UMD_DATA.BIN", PSP_O_RDONLY, 0777);
    // Opened Disc Identifier
    if(disc >= 0)
    {
        // Read Country Code
        sceIoRead(disc, gameid, 4);
        
        // Skip Delimiter
        sceIoLseek32(disc, 1, PSP_SEEK_CUR);
        
        // Read Game ID
        sceIoRead(disc, gameid + 0x4, 5);
        
        // Close Disc Identifier
        sceIoClose(disc);
        return 1;
    }
    return 0;
}

int readGameIdFromPBP(char* gameid){
    int n = 9;
    int res = sctrlGetInitPARAM("DISC_ID", NULL, &n, rebootex_config.game_id);
    if (res < 0) return 0;
    return 1;
}

int readGameIdFromUmd(char* gameid){
    struct LbaParams param;
    memset(&param, 0, sizeof(param));

    param.cmd = 0x01E380C0;
    param.lba_top = 16;
    param.byte_size_total = 10;
    param.byte_size_start = 883;
    
    int res = sceIoDevctl("umd:", 0x01E380C0, &param, sizeof(param), rebootex_config.game_id, sizeof(rebootex_config.game_id));

    if (res < 0) return 0;

    // remove the dash in the middle: ULUS-01234 -> ULUS01234
    rebootex_config.game_id[4] = rebootex_config.game_id[5];
    rebootex_config.game_id[5] = rebootex_config.game_id[6];
    rebootex_config.game_id[6] = rebootex_config.game_id[7];
    rebootex_config.game_id[7] = rebootex_config.game_id[8];
    rebootex_config.game_id[8] = rebootex_config.game_id[9];
    rebootex_config.game_id[9] = 0;

    return 1;
}

int getGameId(char* gameid){

    int res = 1;
    char tmp[128];

    int apitype = sceKernelInitApitype();
    if (apitype == 0x141 || apitype == 0x152 || apitype >= 0x200){
        strcpy(gameid, HOME_ID);
        return res;
    }

    if (rebootex_config.game_id[0] == 0 || strncmp(rebootex_config.game_id, HOME_ID, 9) == 0){

        // Find Function
        void * (* SysMemForKernel_EF29061C)(void) = (void *)sctrlHENFindFunction("sceSystemMemoryManager", "SysMemForKernel", 0xEF29061C);
        
        // Function unavailable (how?!)
        if(SysMemForKernel_EF29061C == NULL) return 0;
        
        // Get Game Info Structure
        void * gameinfo = SysMemForKernel_EF29061C();
        
        // Structure unavailable
        if(gameinfo == NULL) return 0;
        memcpy(rebootex_config.game_id, gameinfo+0x44, 9);
        
        if (rebootex_config.game_id[0] == 0 || strncmp(rebootex_config.game_id, HOME_ID, 9) == 0){
            if (apitype == 0x144 || apitype == 0x155 || sceKernelFindModuleByName("sceNp9660_driver")!=NULL){ // PS1/PSN: read from PBP
                res = readGameIdFromPBP(rebootex_config.game_id);
            }
            else {
                res = readGameIdFromUmd(rebootex_config.game_id);
            }
        }
    }

    if (gameid) memcpy(gameid, rebootex_config.game_id, 9);

    return res;
}
