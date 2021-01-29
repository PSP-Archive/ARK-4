#include "gamesettings.h"

long strhash(char* str){
    ssize_t len = strlen(str) - 1;
    ssize_t i = len;
    unsigned char* p = str;
    
    long x = *p << 7;
    
    while (--i >= 0)
        x = (1000003*x) ^ *p++;
    
    x ^= (len);
    
    return (x == -1)? -2 : x;
}

void loadGameSettings(char* gameid, PeopsConfig* conf){
    long hash = strhash(gameid);
    memset(conf, 0, sizeof(PeopsConfig));
    
    // Reset PEOPS to default settings
    conf->enablepeopsspu = 1;
    conf->volume = 3;
    conf->reverb = 1;
    conf->interpolation = 2;
    conf->enablexaplaying = 1;
    conf->changexapitch = 1;
    conf->spuirqwait = 1;
    conf->spuupdatemode = 0;
    conf->sputhreadpriority = 0;
    
    /*
    if (ark_config->override_peops_config){
        memcpy(conf, &(ark_config->peops_config), sizeof(*conf));
        return;
    }
    */
    
    switch (hash){
        case -5548781341825711198: // Bloody Roar (SCUS-94199) - NTSC-U
            conf->reverb = REVERB_ROOM;
            conf->interpolation = 2;
            conf->enablexaplaying = 1;
            conf->changexapitch = 1;
            conf->spuirqwait = 1;
            conf->spuupdatemode = SPU_WAITVBLANK;
            conf->sputhreadpriority = SPU_PRIORITY_MEDIUM;
            break;
    }
}
