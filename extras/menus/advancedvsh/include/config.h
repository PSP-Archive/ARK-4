#ifndef _CONFIG_H
#define _CONFIG_H


#include "vsh.h"


void config_check(vsh_Menu *vsh);
void config_load(vsh_Menu *vsh);
void config_save(vsh_Menu *vsh, int saveumdregion, int savevshregion);

void config_recreate_region_setting(vsh_Menu *vsh, char* oldtext, char* newtext);
void config_recreate_umd_keys(void);

#endif