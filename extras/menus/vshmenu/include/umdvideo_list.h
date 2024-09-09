#ifndef _UMDVIDEO_LIST_H
#define _UMDVIDEO_LIST_H


#include <stdio.h>


typedef struct _UmdVideoEntry {
	char *path;
	struct _UmdVideoEntry *next;
} UmdVideoEntry;

typedef struct _UmdVideoList {
	UmdVideoEntry head, *tail;
	size_t count;
} UmdVideoList;


int umdvideolist_add(UmdVideoList *list, const char *path);
char *umdvideolist_get(UmdVideoList *list, size_t n);
size_t umdvideolist_count(UmdVideoList *list);
void umdvideolist_clear(UmdVideoList *list);
int umdvideolist_find(UmdVideoList *list, const char *search);
void umdvideolist_init(UmdVideoList *list);
int get_umdvideo(UmdVideoList *list, char *path);


#endif