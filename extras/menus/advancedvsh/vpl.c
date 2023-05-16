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

#include "systemctrl.h"
#include "systemctrl_se.h"
#include "vpl.h"

static SceUID g_vpl_uid = -1;

void vpl_init(void)
{
	g_vpl_uid = sceKernelCreateVpl("SateliteVPL", 2, 0, VPL_POOL_SIZE, NULL);
}

void vpl_finish(void)
{
	sceKernelDeleteVpl(g_vpl_uid);
}

void *vpl_alloc(int size)
{
	void *p;
	int ret;

	ret = sceKernelAllocateVpl(g_vpl_uid, size, &p, NULL);

	if(ret == 0)
		return p;

	return NULL;
}

char *vpl_strdup(const char *str)
{
	int len;
	char *p;

	len = strlen(str) + 1;
	p = vpl_alloc(len);

	if(p == NULL) {
		return p;
	}

	strcpy(p, str);

	return p;
}

void vpl_free(void *p)
{
	int ret;

	ret = sceKernelFreeVpl(g_vpl_uid, p);

#ifdef DEBUG
	if(ret != 0) {
		__asm("break 0x8492");
	}
#endif
}

void *vpl_realloc(void *ptr, size_t size)
{
	void *p;

	if(size == 0 && ptr != NULL) {
		vpl_free(ptr);

		return NULL;
	}

	p = vpl_alloc(size);

	if(p == NULL) {
		return p;
	}

	if(ptr == NULL) {
		memset(p, 0, size);
	} else {
		memcpy(p, ptr, size);
		vpl_free(ptr);
	}

	return p;
}
