#ifndef REGION_FREE_H
#define REGION_FREE_H

void patch_umd_idslookup(SceModule2* mod);
int patch_umd_thread(SceSize args, void *argp);
void patch_vsh_region_check(SceModule2* mod);

#endif