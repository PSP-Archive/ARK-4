#include "rebootex.h"

int redirect_flash = 1;

int pspemuLfatOpenExtra(BootFile* file)
{
    char* p = file->name;
    if (strcmp(p, "pspbtcnf.bin") == 0){
        p[2] = 'x'; // custom btcnf for PS1
    }
    return -1;
}
