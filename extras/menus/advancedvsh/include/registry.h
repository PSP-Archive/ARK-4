#ifndef _REGISTRY_H
#define _REGISTRY_H

#include <psptypes.h>

#include "vsh.h"

int get_registry_value(const char *dir, const char *name, u32 *val);
int set_registry_value(const char *dir, const char *name, u32 val);

void delete_hibernation(vsh_Menu *vsh);
int activate_codecs(vsh_Menu *vsh);
int swap_buttons(vsh_Menu *vsh);

#endif