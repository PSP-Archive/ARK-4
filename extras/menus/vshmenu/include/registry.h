#ifndef _REGISTRY_H
#define _REGISTRY_H

#include <psptypes.h>

#include "vsh.h"

void delete_hibernation(vsh_Menu *vsh);
int codecs_activated();
int activate_codecs(vsh_Menu *vsh);
int swap_buttons(vsh_Menu *vsh);

#endif