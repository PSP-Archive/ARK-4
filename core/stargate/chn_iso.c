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

#include <pspsdk.h>
#include <pspkernel.h>
#include <pspsysmem_kernel.h>
#include <pspthreadman_kernel.h>
#include <pspdebug.h>
#include <pspinit.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "systemctrl.h"
#include "systemctrl_private.h"
#include "kubridge.h"
#include "strsafe.h"
#include "globals.h"
#include "macros.h"

typedef struct _pspMsPrivateDirent {
	SceSize size;
	char s_name[16];
	char l_name[1024];
} pspMsPrivateDirent;

static int get_ISO_longname(char *l_name, const char *s_name, u32 size)
{
	const char *p;
	SceUID fd;
	char *prefix = NULL;
	pspMsPrivateDirent *pri_dirent = NULL;
	SceIoDirent *dirent = NULL;
	int result = -7; /* Not found */

	if (s_name == NULL || l_name == NULL) {
		result = -2;
		goto exit;
	}

	p = strrchr(s_name, '/');

	if (p == NULL) {
		result = -2;
		goto exit;
	}

	prefix = oe_malloc(512);
	pri_dirent = oe_malloc(sizeof(*pri_dirent));
	dirent = oe_malloc(sizeof(*dirent));

	if(prefix == NULL) {
		result = -3;
		goto exit;
	}

	if(pri_dirent == NULL) {
		result = -4;
		goto exit;
	}

	if(dirent == NULL) {
		return -5;
		goto exit;
	}

	strncpy(prefix, s_name, MIN(p + 1 - s_name, 512));
	prefix[MIN(p + 1 - s_name, 512-1)] = '\0';
	printk("%s: prefix %s\n", __func__, prefix);

	fd = sceIoDopen(prefix);

	if (fd >= 0) {
		int ret;

		do {
			memset(dirent, 0, sizeof(*dirent));
			memset(pri_dirent, 0, sizeof(*pri_dirent));
			pri_dirent->size = sizeof(*pri_dirent);
			dirent->d_private = (void*)pri_dirent;
			ret = sceIoDread(fd, dirent);

			if (ret >= 0) {
				if (!strcmp(pri_dirent->s_name, p+1)) {
					strncpy(l_name, s_name, MIN(p + 1 - s_name, size));
					l_name[MIN(p + 1 - s_name, size-1)] = '\0';
					strncat(l_name, size, dirent->d_name);
					printk("%s: final %s\n", __func__, l_name);
					result = 0;

					break;
				}
			}
		} while (ret > 0);

		sceIoDclose(fd);
	} else {
		printk("%s: dopen %s -> 0x%08X\n", __func__, prefix, fd);
		result = -6;
		goto exit;
	}

exit:
	oe_free(prefix);
	oe_free(pri_dirent);
	oe_free(dirent);

	return result;
}

int myIoOpen_kernel_chn(char *file, int flag, int mode)
{
	int ret = sceIoOpen(file, flag, mode);

	// convert the iso name back to longname
	if (ret < 0 && strlen(file) > sizeof("ms0:") && 0 == strncasecmp(file + sizeof("ms0:") - 1, "/ISO/", sizeof("/ISO/")-1)) {
		char filename[256];

		ret = get_ISO_longname(filename, file, sizeof(filename));

		if(ret == 0) {
			ret = sceIoOpen(filename, flag, mode);
			printk("%s: %s -> 0x%08X\n", __func__, filename, ret);
		}
	}

	return ret;
}

void patch_IsoDrivers(void)
{
	SceModule2 *mod = (SceModule2*) sceKernelFindModuleByName("PRO_Inferno_Driver");
	if(mod != NULL) {
		hookImportByNID((SceModule*)mod, "IoFileMgrForKernel", 0x109F50BC, &myIoOpen_kernel_chn);
	}
}
