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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pspsdk.h>
#include <pspdebug.h>
#include <pspkernel.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <psputility.h>
#include <pspreg.h>

#include "systemctrl.h"
#include "systemctrl_se.h"
#include "vpl.h"

int get_registry_value(const char *dir, const char *name, u32 *val)
{
    int ret = 0;
    struct RegParam reg;
    REGHANDLE h;

    memset(&reg, 0, sizeof(reg));
    reg.regtype = 1;
    reg.namelen = strlen("/system");
    reg.unk2 = 1;
    reg.unk3 = 1;
    strcpy(reg.name, "/system");
    if(sceRegOpenRegistry(&reg, 2, &h) == 0)
    {
        REGHANDLE hd;
        if(!sceRegOpenCategory(h, dir, 2, &hd))
        {
            REGHANDLE hk;
            unsigned int type, size;

            if(!sceRegGetKeyInfo(hd, name, &hk, &type, &size))
            {
                if(!sceRegGetKeyValue(hd, hk, val, 4))
                {
                    ret = 1;
                    sceRegFlushCategory(hd);
                }
            }
            sceRegCloseCategory(hd);
        }
        sceRegFlushRegistry(h);
        sceRegCloseRegistry(h);
    }
    return ret;
}

int set_registry_value(const char *dir, const char *name, u32 val)
{
    int ret = 0;
    struct RegParam reg;
    REGHANDLE h;

    memset(&reg, 0, sizeof(reg));
    reg.regtype = 1;
    reg.namelen = strlen("/system");
    reg.unk2 = 1;
    reg.unk3 = 1;
    strcpy(reg.name, "/system");
    if(sceRegOpenRegistry(&reg, 2, &h) == 0)
    {
        REGHANDLE hd;
        if(!sceRegOpenCategory(h, dir, 2, &hd))
        {
            if(!sceRegSetKeyValue(hd, name, &val, 4))
            {
                ret = 1;
                sceRegFlushCategory(hd);
            }
			else
			{
				sceRegCreateKey(hd, name, REG_TYPE_INT, 4);
				sceRegSetKeyValue(hd, name, &val, 4);
				ret = 1;
                sceRegFlushCategory(hd);
			}
            sceRegCloseCategory(hd);
        }
        sceRegFlushRegistry(h);
        sceRegCloseRegistry(h);
    }

	return ret;
}
