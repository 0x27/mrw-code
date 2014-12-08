/* (C) Copyright IBM Corp. 2007, 2008
	 Author: Ian Craggs
*/

#include "LinkedList.h"

#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include "Heap.h"


void ListZero(List* newl)
{
	newl->first = NULL;
	newl->last = NULL;
	newl->current = NULL;
	newl->count = newl->size = 0;
}


List* ListInitialize()
{
	List* newl = malloc(sizeof(List));
	ListZero(newl);
	return newl;
}


void ListAppendNoMalloc(List* aList, void* content, ListElement* newel, int size)
{ /* for heap use */
	newel->content = content;
	newel->next = NULL;
	newel->prev = aList->last;
	if (aList->first == NULL)
		aList->first = newel;
	else
		aList->last->next = newel;
	aList->last = newel;
	++(aList->count);
}


void ListAppend(List* aList, void* content, int size)
{
	ListElement* newel = malloc(sizeof(ListElement));
	ListAppendNoMalloc(aList, content, newel, size);
}


ListElement* ListFind(List* aList, void* content)
{
	return ListFindItem(aList, content, NULL);
}


ListElement* ListFindItem(List* aList, void* content, int(*callback)(void*, void*))
{
	ListElement* rc = NULL;

	if (aList->current != NULL && ((callback == NULL && aList->current->content == content) ||
		   (callback != NULL && callback(aList->current->content, content))))
		rc = aList->current;
	else
	{
		ListElement* current = NULL;

		/* find the content */
		while (ListNextElement(aList, &current) != NULL)
		{
			if (callback == NULL)
			{
				if (current->content == content)
				{
					rc = current;
					break;
				}
			}
			else
			{
				if (callback(current->content, content))
				{
					rc = current;
					break;
				}
			}
		}
		if (rc != NULL)
			aList->current = rc;
	}
	return rc;
}


int ListUnlink(List* aList, void* content, int(*callback)(void*, void*), int freeContent)
{ 
	ListElement* next = NULL;
	ListElement* saved = aList->current;
	int saveddeleted = 0;

	if (!ListFindItem(aList, content, callback))
		return 0; /* false, did not remove item */

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
	if (freeContent)
		free(aList->current->content);
	if (saved == aList->current)
		saveddeleted = 1;
	free(aList->current);
	if (saveddeleted)
		aList->current = next;
	else
		aList->current = saved;
	--(aList->count);
	return 1; /* successfully removed item */
}


int ListDetach(List* aList, void* content)
{
	return ListUnlink(aList, content, NULL, 0);
}


int ListRemove(List* aList, void* content)
{
	return ListUnlink(aList, content, NULL, 1);
}


int ListDetachItem(List* aList, void* content, int(*callback)(void*, void*))
{ /* do not free the content */
	return ListUnlink(aList, content, callback, 0);
}


int ListRemoveItem(List* aList, void* content, int(*callback)(void*, void*))
{ /* remove from list and free the content */
	return ListUnlink(aList, content, callback, 1);
}

void ListEmpty(List* aList)
{
	while (aList->first != NULL)
	{
		ListElement* first = aList->first;
		if (first->content != NULL)
			free(first->content);
		aList->first = first->next;
		free(first);
	}
	aList->count = aList->size = 0;
	aList->current = aList->first = aList->last = NULL;
}


void ListFree(List* aList)
{
	ListEmpty(aList);
	free(aList);
}


void ListFreeNoContent(List* aList)
{
	while (aList->first != NULL)
	{
		ListElement* first = aList->first;
		aList->first = first->next;
		free(first);
	}
	free(aList);
}


ListElement* ListNextElement(List* aList, ListElement** pos)
{
	return *pos = (*pos == NULL) ? aList->first : (*pos)->next;
}


ListElement* ListPrevElement(List* aList, ListElement** pos)
{
	return *pos = (*pos == NULL) ? aList->last : (*pos)->prev;
}


int intcompare(void* a, void* b)
{
	return *((int*)a) == *((int*)b);
}

int stringcompare(void* a, void* b)
{
	return strcmp((char*)a, (char*)b) == 0;
}


#if defined(LIST_TEST)


int main(int argc, char *argv[])
{
	int i, *ip, *todelete;
	ListElement* current = NULL;
	List* l = ListInitialize();
	printf("List initialized\n");

	for (i = 0; i < 10; i++)
	{
		ip = malloc(sizeof(int));
		*ip = i;
		ListAppend(l, (void*)ip);
		if (i==5)
			todelete = ip;
		printf("List element appended %d\n",  *((int*)(l->last->content)));
	}

	printf("List contents:\n");
	current = NULL;
	while (ListNextElement(l, &current) != NULL)
		printf("List element: %d\n", *((int*)(current->content)));

	printf("List contents in reverse order:\n");
	current = NULL;
	while (ListPrevElement(l, &current) != NULL)
		printf("List element: %d\n", *((int*)(current->content)));

	//if ListFindItem(l, *ip, intcompare)->content

	printf("List contents having deleted element %d:\n", *todelete);
	ListRemove(l, todelete);
	current = NULL;
	while (ListNextElement(l, &current) != NULL)
		printf("List element: %d\n", *((int*)(current->content)));

	i = 9;
	ListRemoveItem(l, &i, intcompare);
	printf("List contents having deleted another element, %d, size now %d:\n", i, l->size);
	current = NULL;
	while (ListNextElement(l, &current) != NULL)
		printf("List element: %d\n", *((int*)(current->content)));

	ListFree(l);
	printf("List freed\n");
}

#endif





