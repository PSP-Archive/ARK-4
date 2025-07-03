#include "rebootex.h"

int redirect_flash = 1;

int pspemuLfatOpenExtra(BootFile* file)
{
    char* p = file->name;
    if (strcmp(p, "pspbtcnf.bin") == 0){
        p[2] = 'v'; // custom btcnf for PS Vita
        if (reboot_conf->iso_mode == MODE_INFERNO || reboot_conf->iso_mode == MODE_MARCH33){
            reboot_conf->iso_mode = MODE_INFERNO;
            p[5] = 'i'; // use inferno ISO mode (psvbtinf.bin)
        }
        // else use psvbtcnf.bin for np9660
    }
    return -1;
}
