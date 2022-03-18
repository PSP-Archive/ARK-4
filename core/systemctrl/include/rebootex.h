/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

#ifndef _REBOOTEX_H_
#define _REBOOTEX_H_

#include <rebootconfig.h>

// Reboot Buffer Configuration Functions
typedef struct{
    void (*SetBootConfFileIndex)(int index);
    unsigned int (*GetBootConfFileIndex)(void);
    void (*SetDiscType)(int type);
    int (*GetDiscType)(void);
    void (*SetRebootModule)(char *module_before, void *buf, int size, int flags);
}RebootConfigFunctions;
extern RebootConfigFunctions* reboot_funcs;

// Reboot ISO Path
extern char* reboot_config_isopath;

// Backup Reboot Buffer
void backupRebootBuffer(void);

// Restore Reboot Buffer
void restoreRebootBuffer(void);

// Patch loadexec_01g.prx
void patchLoadExec(SceModule2*);

#endif

