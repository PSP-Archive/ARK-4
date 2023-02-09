#ifndef REGION_FREE_H
#define REGION_FREE_H

#define REGION_JAPAN 3
#define REGION_AMERICA 4
#define REGION_EUROPE 5

extern int region_change;

void patch_umd_idslookup(SceModule2* mod);
int patch_umd_thread(SceSize args, void *argp);
void patch_vsh_main_region(SceModule2* mod);

#endif