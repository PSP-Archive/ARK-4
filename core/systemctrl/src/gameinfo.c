#include <pspsdk.h>
#include <pspinit.h>
#include <pspiofilemgr.h>
#include <pspsysmem_kernel.h>

#include "globals.h"
#include "macros.h"
#include "module2.h"
#include "graphics.h"

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

int readGameIdFromDisc(char* gameid){
    static char game_id[10] = {0};

    if (game_id[0] == 0){
        int apitype = sceKernelInitApitype();
        if (apitype == 0x144 || apitype == 0x155){ // PS1: read from PBP
            int n = 9;
            int res = sctrlGetInitPARAM("DISC_ID", NULL, &n, game_id);
            if (res < 0) return 0;
        }
        else { // PSP: read from disc
            struct LbaParams param;
            memset(&param, 0, sizeof(param));

            param.cmd = 0x01E380C0;
            param.lba_top = 16;
            param.byte_size_total = 10;
            param.byte_size_start = 883;
            
            int res = sceIoDevctl("umd:", 0x01E380C0, &param, sizeof(param), game_id, sizeof(game_id));

            if (res < 0) return 0;

            // remove the dash in the middle: ULUS-01234 -> ULUS01234
            game_id[4] = game_id[5];
            game_id[5] = game_id[6];
            game_id[6] = game_id[7];
            game_id[7] = game_id[8];
            game_id[8] = game_id[9];
            game_id[9] = 0;
        }
    }

    if (gameid) memcpy(gameid, game_id, 9);
    return 1;
}

int getGameId(char* gameid){
    // Find Function
    void * (* SysMemForKernel_EF29061C)(void) = (void *)sctrlHENFindFunction("sceSystemMemoryManager", "SysMemForKernel", 0xEF29061C);
    
    // Function unavailable (how?!)
    if(SysMemForKernel_EF29061C == NULL) return 0;
    
    // Get Game Info Structure
    void * gameinfo = SysMemForKernel_EF29061C();
    
    // Structure unavailable
    if(gameinfo == NULL) return 0;
    memcpy(gameid, gameinfo+0x44, 9);

    if (gameid[0] == 0 || strncmp(gameid, "HOME00000", 9) == 0){
        return readGameIdFromDisc(gameid);
    }

    return 1;
}
