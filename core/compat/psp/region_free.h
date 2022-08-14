#ifndef REGION_FREE_H
#define REGION_FREE_H

#define REGION_DEBUG_CODE 1

#define REGION_JAPAN 3
#define REGION_AMERICA 4
#define REGION_EUROPE 5

extern int region_change;

void patch_region(void);
void patch_umd_idslookup(SceModule2* mod);
int replace_umd_keys();
int patch_umd_thread(SceSize args, void *argp);

#endif