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
#include "common.h"
#include "vpl.h"

int umdvideolist_add(UmdVideoList *list, const char *path)
{
	char *newpath;
	UmdVideoEntry *p;

	if(path == NULL)
		return -1;

	newpath = vpl_strdup(path);

	if(newpath == NULL) {
		return -2;
	}

	p = vpl_alloc(sizeof(*p));

	if(p == NULL) {
		vpl_free(newpath);
		return -3;
	}

	list->tail->next = p;
	list->tail = p;
	list->tail->path = newpath;
	list->tail->next = NULL;
	list->count++;

	return 0;
}

static int umdvideolist_remove(UmdVideoList *list, UmdVideoEntry *pdel)
{
	UmdVideoEntry *p, *prev;

	if(list->count == 0) {
		return -1;
	}

	for(prev = &list->head, p = list->head.next; p != NULL; prev = p, p = p->next) {
		if(p == pdel) {
			break;
		}
	}

	if(p == NULL) {
		return -1;
	}

	if(list->tail == pdel) {
		list->tail = prev;
	}
	
	prev->next = NULL;
	vpl_free(pdel->path);
	vpl_free(pdel);
	list->count--;

	return 0;
}

char *umdvideolist_get(UmdVideoList *list, size_t n)
{
	UmdVideoEntry *p;

	for(p=list->head.next; p != NULL && n != 0; p=p->next, n--) {
	}

	if(p == NULL) {
		return NULL;
	}

	return p->path;
}

size_t umdvideolist_count(UmdVideoList *list)
{
	return list->count;
}

void umdvideolist_clear(UmdVideoList *list)
{
	while(list->tail != &list->head) {
		umdvideolist_remove(list, list->tail);
	}
}

int umdvideolist_find(UmdVideoList *list, const char *search)
{
	UmdVideoEntry *p;
	int i;

	for(i=0, p=list->head.next; p != NULL; p=p->next, ++i) {
		if(0 == stricmp(p->path, search)) {
			break;
		}
	}

	if(p == NULL) {
		return -1;
	}

	return i;
}

void umdvideolist_init(UmdVideoList *list)
{
	list->head.path = NULL;
	list->head.next = NULL;
	list->tail = &list->head;
	list->count = 0;
}
