#ifndef _REGISTRY_H
#define _REGISTRY_H

#include <psptypes.h>

int get_registry_value(const char *dir, const char *name, u32 *val);
int set_registry_value(const char *dir, const char *name, u32 val);

#endif