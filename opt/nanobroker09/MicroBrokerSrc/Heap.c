
#include "LinkedList.h"
#include "Log.h"
#include "Messages.h"

#include <memory.h>
#include <stdlib.h>
#include <string.h>

#include "Heap.h"

#undef malloc
#undef realloc
#undef free

static heap_info state = {0, 0};

int ListRemoveCurrentItem(List* aList);

typedef struct
{
	char* file;
	int line;
	void* ptr;
	size_t size;
} storageElement;

static List heap = {NULL, NULL, NULL};


void* mymalloc(char* file, int line, size_t size)
{
	storageElement* s = malloc(sizeof(storageElement));

	s->file = malloc(strlen(file)+1);
	strcpy(s->file, file);
	s->line = line;
	s->ptr = malloc(size);
	s->size = size;
	//Log(LOG_DEBUG, "Allocating %d bytes in heap at file %s line %d ptr %p\n", size, file, line, s->ptr);
	ListAppendNoMalloc(&heap, s, malloc(sizeof(ListElement)), sizeof(ListElement));
	state.current_size += size;
	if (state.current_size > state.max_size)
		state.max_size = state.current_size;
	return s->ptr;
}


int ptrCompare(void* a, void* b)
{
	storageElement* s = (storageElement*)a;
	return s->ptr == b;
}


void myfree(char* file, int line, void* p)
{
	ListElement* e = ListFindItem(&heap, p, ptrCompare);
	if (e == NULL)
		Log(LOG_ERROR, Messages_get(99), file, line);
	else
	{
		storageElement* s = (storageElement*)(heap.current->content);
		//Log(LOG_DEBUG, "Freeing %d bytes in heap at file %s line %d, heap use now %d bytes\n", s->size, file, line, state.current_size);
		free(s->file);
		state.current_size -= s->size;
		ListRemoveCurrentItem(&heap);
	}
	free(p);
}


void *myrealloc(char* file, int line, void* p, size_t size)
{
	void* rc = NULL;
	ListElement* e = ListFindItem(&heap, p, ptrCompare);
	if (e == NULL)
		Log(LOG_ERROR, Messages_get(100), file, line);
	else
	{
		storageElement* s = (storageElement*)(heap.current->content);
		state.current_size += size - s->size;
		if (state.current_size > state.max_size)
			state.max_size = state.current_size;
		rc = s->ptr = realloc(s->ptr, size);
		s->size = size;
		s->file = realloc(s->file, strlen(file)+1);
		strcpy(s->file, file);
		s->line = line;
	}
	return rc;
}


int ListRemoveCurrentItem(List* aList)
{
	ListElement* next = NULL;

	if (aList->current->prev == NULL) 
		/* so this is the first element, and we have to update the "first" pointer */
		aList->first = aList->current->next;
	else
		aList->current->prev->next = aList->current->next;

	if (aList->current->next == NULL)
		aList->last = aList->current->prev;
	else
		aList->current->next->prev = aList->current->prev;

	next = aList->current->next;
	free(aList->current->content);
	free(aList->current);
	aList->current = next;
	return 1; /* successfully removed item */
}


void HeapScan()
{
	ListElement* current = NULL;
	Log(LOG_INFO, Messages_get(101), state.current_size);
	while (ListNextElement(&heap, &current))
	{
		storageElement* s = (storageElement*)(current->content);
		Log(LOG_INFO, Messages_get(102), s->size, s->line, s->file, s->ptr);
		Log(LOG_INFO, Messages_get(103), (10 > s->size) ? s->size : 10, s->ptr);
	}
	Log(LOG_INFO, Messages_get(104));
}


int Heap_initialize()
{
	return 0;
}


void Heap_terminate()
{
	Log(LOG_INFO, "Maximum heap use was %d bytes\n", state.max_size);
	if (state.current_size > 0)
	{
		Log(LOG_ERROR, Messages_get(106));
		HeapScan();
	}
}


heap_info* Heap_get_info()
{
	return &state;
}
