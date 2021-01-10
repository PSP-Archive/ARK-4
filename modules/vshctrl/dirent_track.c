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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pspsdk.h>
#include <pspthreadman_kernel.h>
#include "systemctrl.h"
#include "systemctrl_se.h"
#include "systemctrl_private.h"
#include "printk.h"
#include "utils.h"
#include "strsafe.h"
#include "dirent_track.h"
#include "main.h"

static inline void lock() {}
static inline void unlock() {}

static struct IoDirentEntry g_head = { "", -1, -1, NULL }, *g_tail = &g_head;

static char *oe_strdup(const char *str)
{
	int len;
	char *p;

	len = strlen(str) + 1;
	p = oe_malloc(len);

	if(p == NULL) {
		return p;
	}

	strcpy(p, str);

	return p;
}

int dirent_add(SceUID dfd, SceUID iso_dfd, const char *path)
{
	struct IoDirentEntry *p;
   
	p = oe_malloc(sizeof(*p));

	if(p == NULL) {
		return -1;
	}

	p->dfd = dfd;
	p->iso_dfd = iso_dfd;
	p->path = oe_strdup(path);

	if(p->path == NULL) {
		oe_free(p);

		return -2;
	}

	lock();
	g_tail->next = p;
	g_tail = p;
	g_tail->next = NULL;
	unlock();

	return 0;
}

int dirent_remove(struct IoDirentEntry *p)
{
	int ret;
	struct IoDirentEntry *fds, *prev;

	lock();

	for(prev = &g_head, fds = g_head.next; fds != NULL; prev = fds, fds = fds->next) {
		if(p == fds) {
			break;
		}
	}

	if(fds != NULL) {
		prev->next = fds->next;

		if(g_tail == fds) {
			g_tail = prev;
		}

		oe_free(fds->path);
		oe_free(fds);
		ret = 0;
	} else {
		ret = -1;
	}

	unlock();

	return ret;
}

struct IoDirentEntry *dirent_search(SceUID dfd)
{
	struct IoDirentEntry *fds;

	if (dfd < 0)
		return NULL;

	for(fds = g_head.next; fds != NULL; fds = fds->next) {
		if(fds->dfd == dfd)
			break;
	}

	return fds;
}
