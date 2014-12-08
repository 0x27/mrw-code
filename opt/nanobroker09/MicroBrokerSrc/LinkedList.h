/* (C) Copyright IBM Corp. 2007, 2008
	 Author: Ian Craggs
*/

#if !defined(LINKEDLIST_H)
#define LINKEDLIST_H

typedef struct ListElementStruct
{
	struct ListElementStruct *prev, *next; 
	void* content;
} ListElement;

typedef struct
{
	ListElement *first, *last, *current;
	int count,  /* no of items */
	    size;  /* heap storage used */
} List;

void ListZero(List*);
List* ListInitialize();

void ListAppend(List* aList, void* content, int size);
void ListAppendNoMalloc(List* aList, void* content, ListElement* newel, int size);

int ListRemove(List* aList, void* content);
int ListRemoveItem(List* aList, void* content, int(*callback)(void*, void*));

int ListDetach(List* aList, void* content);
int ListDetachItem(List* aList, void* content, int(*callback)(void*, void*));

void ListFree(List* aList);
void ListEmpty(List* aList);
void ListFreeNoContent(List* aList);

ListElement* ListNextElement(List* aList, ListElement** pos);
ListElement* ListPrevElement(List* aList, ListElement** pos);

ListElement* ListFind(List* aList, void* content);
ListElement* ListFindItem(List* aList, void* content, int(*callback)(void*, void*));

int intcompare(void* a, void* b);
int stringcompare(void* a, void* b);

#endif
