#ifndef UMD_H
#define UMD_H

#include <pspumd.h>
#include "entry.h"
#include "gfx.h"

#define UMD_GAME_ICON0_PATH "disc0:/PSP_GAME/ICON0.PNG"

#define UMD_GAME_ICON1_PATH "disc0:/PSP_GAME/ICON1.PMF"

#define UMD_GAME_PIC0_PATH "disc0:/PSP_GAME/PIC0.PNG"

#define UMD_GAME_PIC1_PATH "disc0:/PSP_GAME/PIC1.PNG"

#define UMD_GAME_SND0_PATH "disc0:/PSP_GAME/SND0.AT3"

#define UMD_EBOOT_BIN "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN"
#define UMD_EBOOT_OLD "disc0:/PSP_GAME/SYSDIR/EBOOT.OLD"

#define UMD_GAME_SFO_PATH "disc0:/PSP_GAME/PARAM.SFO"

#define UMD_APITYPE 0x120

using namespace std;

class UMD : public Entry{

    private:
        string umdname;

        int readGameId(char* game_id);
        bool hasUpdate(char* update_path);

    public:
    
        UMD();
        ~UMD();
    
        string getName();
        
        void loadIcon();
        void loadPics();
        void loadAVMedia();
        SfoInfo getSfoInfo();

        void doExecute();
        
        char* getType();
        char* getSubtype();
        
        static bool isUMD();
};

#endif
