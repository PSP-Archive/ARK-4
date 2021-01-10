/*
 * Helper for use with the PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed as 'free to use and modify as long as credited appropriately'
 *
 * valloc.c - Standard C library VRAM allocation routines.
 *
 * Copyright (c) 2006 Alexander Berl <a.berl@gmx.de>
 *
 */
//#include <psptypes.h>
//#include <pspkernel.h>
#include <pspgu.h>
#include <malloc.h>
#include "valloc.h"


/* Use this to set the default valloc() alignment. */
#define DEFAULT_VALIGNMENT	16

#ifndef ALIGN
#define ALIGN(x, align) (((x)+((align)-1))&~((align)-1))
#endif


#define VRAM_SIZE 0x200000
#define VRAM_BASE ((unsigned int)sceGeEdramGetAddr())

/* _vram_mem_block_header structure. */
typedef struct _vram_mem_header {
	void *	ptr;
	size_t	size;
	struct _vram_mem_header * prev;
	struct _vram_mem_header * next;
} vram_mem_header_t;



void * __valloc_vram_base = (void*)0;
vram_mem_header_t *__valloc_vram_head = NULL;
vram_mem_header_t *__valloc_vram_tail = NULL;



size_t vgetMemorySize(unsigned int width, unsigned int height, unsigned int psm)
{
	switch (psm)
	{
		case GU_PSM_T4:
			return (width * height) >> 1;

		case GU_PSM_T8:
			return width * height;

		case GU_PSM_5650:
		case GU_PSM_5551:
		case GU_PSM_4444:
		case GU_PSM_T16:
			return 2 * width * height;

		case GU_PSM_8888:
		case GU_PSM_T32:
			return 4 * width * height;

		default:
			return 0;
	}
}

inline void* vGuPointer( void *ptr )
{
	return (void*)((u32)ptr & ~VRAM_BASE);
}

inline void* vCPUPointer( void *ptr )
{
	return (void*)((u32)ptr | VRAM_BASE);
}

/* Find the smallest block that we can allocate AFTER, returning NULL if there
   are none.  */
vram_mem_header_t * _vram_mem_fit(vram_mem_header_t *head, size_t size)
{
	vram_mem_header_t *prev_mem = head, *best_fit = NULL;
	u32 prev_top, next_bot;
	size_t best_size = 0;

	if (((u32)head->ptr+head->size+size)<VRAM_SIZE) {
		best_fit = head;
		best_size = VRAM_SIZE - ((u32)head->ptr+head->size);
	}

	while (prev_mem != NULL) {
		if (prev_mem->next != NULL) {
			prev_top = (u32)prev_mem->ptr;
			next_bot = prev_top - ((u32)prev_mem->next->ptr + prev_mem->next->size);
			if (next_bot >= size) {
				if (best_fit==NULL || next_bot<best_size) {
					best_fit = prev_mem->next;
					best_size = next_bot;
				}
			}
		}

		prev_mem = prev_mem->next;
	}

	return best_fit;
}

void * valloc(size_t size)
{
	void *ptr = NULL;
	vram_mem_header_t *new_mem, *prev_mem;
	size_t mem_sz;

	mem_sz = size;

	if ((mem_sz & (DEFAULT_VALIGNMENT - 1)) != 0)
		mem_sz = ALIGN(mem_sz, DEFAULT_VALIGNMENT);


	/* If we don't have any allocated blocks, reserve the first block
	   and initialize __valloc_vram_tail.  */
	if (__valloc_vram_head == NULL) {
		if (size>VRAM_SIZE)
			return ptr;

		__valloc_vram_head = (vram_mem_header_t *)malloc( sizeof(vram_mem_header_t) );
		if (__valloc_vram_head == NULL)
			return ptr;

		ptr = (void *)__valloc_vram_base;


		__valloc_vram_head->ptr  = ptr;
		__valloc_vram_head->size = mem_sz;
		__valloc_vram_head->prev = NULL;
		__valloc_vram_head->next = NULL;

		__valloc_vram_tail = __valloc_vram_head;
		
		return vabsptr(ptr);
	}

	/* Check to see if there's free space at the bottom of the heap.
	   NOTE: This case is now already handled in _vram_mem_fit */
	/*if (((u32)__valloc_vram_head->ptr + __valloc_vram_head->size + mem_sz) < VRAM_SIZE) {
		new_mem = (vram_mem_header_t *)malloc( sizeof(vram_mem_header_t) );
		if (new_mem == NULL)
			return ptr;
		ptr     = (void *)((u32)__valloc_vram_head->ptr + __valloc_vram_head->size);

		new_mem->ptr  = ptr;
		new_mem->size = mem_sz;
		new_mem->prev = NULL;
		new_mem->next = __valloc_vram_head;
		new_mem->next->prev = new_mem;
		__valloc_vram_head = new_mem;
		
		return ptr;
	}*/

	/* See if we can allocate the block anywhere. */
	prev_mem = _vram_mem_fit(__valloc_vram_head, mem_sz);
	if (prev_mem != NULL) {
		new_mem = (vram_mem_header_t *)malloc( sizeof(vram_mem_header_t) );
		if (new_mem == NULL)
			return ptr;
		ptr     = (void *)((u32)prev_mem->ptr + prev_mem->size);

		new_mem->ptr  = ptr;
		new_mem->size = mem_sz;
		new_mem->prev = prev_mem->prev;
		if (new_mem->prev!=NULL)
		  new_mem->prev->next = new_mem;
		new_mem->next = prev_mem;
		prev_mem->prev = new_mem;
		if (prev_mem == __valloc_vram_head)
		  __valloc_vram_head = new_mem;

		return vabsptr(ptr);
	}

	/* Now we have a problem: There's no room at the bottom and also no room in between.
	   So either we do compact the memory (time critical because memcopies needed) or we
	   just return NULL so the application has to handle this case itself.
	   For now we'll just return NULL
	*/

	return ptr;
}



void vfree(void *ptr)
{
	vram_mem_header_t *cur;

	if (!ptr)
		return;
	
	if (!__valloc_vram_head)
		return;

	ptr = vrelptr(ptr);

	/* Freeing the head pointer is a special case.  */
	if (ptr == __valloc_vram_head->ptr) {

		cur = __valloc_vram_head->next;
		free(__valloc_vram_head);

		__valloc_vram_head = cur;

		if (__valloc_vram_head != NULL) {
			__valloc_vram_head->prev = NULL;
		} else {
			__valloc_vram_tail = NULL;
		}
		
		return;
	}

	cur = __valloc_vram_head;
	while (ptr != cur->ptr)  {
		/* ptr isn't in our list */
		if (cur->next == NULL) {
			return;
		}
		cur = cur->next;
	}

	/* Deallocate the block.  */
	if (cur->next != NULL) {
		cur->next->prev = cur->prev;
	} else {
		/* If this block was the last one in the list, shrink the heap.  */
		__valloc_vram_tail = cur->prev;
	}

	cur->prev->next = cur->next;
	free( cur );
	
}


size_t vmemavail()
{
	if (__valloc_vram_head==NULL)
		return VRAM_SIZE;
	
	vram_mem_header_t *cur;
	size_t size = VRAM_SIZE - ((u32)__valloc_vram_head->ptr + __valloc_vram_head->size);

	cur = __valloc_vram_head;
	while (cur->next!=NULL)  {
		size += (u32)cur->ptr - ((u32)cur->next->ptr + cur->next->size);
		cur = cur->next;
	}

	return size;
}


size_t vlargestblock()
{
	if (__valloc_vram_head==NULL)
		return VRAM_SIZE;
	
	vram_mem_header_t *cur;
	size_t size = VRAM_SIZE - ((u32)__valloc_vram_head->ptr + __valloc_vram_head->size);
	size_t new_size;

	cur = __valloc_vram_head;
	while (cur->next!=NULL)  {
		new_size = (u32)cur->ptr - ((u32)cur->next->ptr + cur->next->size);
		if (new_size>size) size = new_size;
		cur = cur->next;
	}

	return size;
}

