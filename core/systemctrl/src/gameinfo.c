#include <pspsdk.h>
#include <pspiofilemgr.h>
#include <pspsysmem_kernel.h>

#include "globals.h"
#include "macros.h"
#include "module2.h"
#include "graphics.h"

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

int getGameId(char* gameid){
    if (!readGameIdFromDisc(gameid)){
        // Find Function
        void * (* SysMemForKernel_EF29061C)(void) = (void *)sctrlHENFindFunction("sceSystemMemoryManager", "SysMemForKernel", 0xEF29061C);
        
        // Function unavailable (how?!)
        if(SysMemForKernel_EF29061C == NULL) return 0;
        
        // Get Game Info Structure
        void * gameinfo = SysMemForKernel_EF29061C();
        
        // Structure unavailable
        if(gameinfo == NULL) return 0;
        
        memcpy(gameid, gameinfo+0x44, 9);
    }
    return 1;
}
