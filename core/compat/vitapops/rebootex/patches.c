#include "rebootex.h"

int redirect_flash = 1;

int pspemuLfatOpenExtra(BootFile* file)
{
    char* p = file->name;
    if (strcmp(p, "pspbtcnf.bin") == 0){
        p[2] = 'v'; // custom btcnf for PS Vita
        p[5] = 'x'; // psvbtxnf.bin for PS1 exploits
    }
    return -1;
}
