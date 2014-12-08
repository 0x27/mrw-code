/* (C) Copyright IBM Corp. 2008
  Author: Ian Craggs
*/

#if !defined(HEAP_H)
#define HEAP_H

#include <memory.h>

#define malloc(x) mymalloc(__FILE__, __LINE__, x)
#define realloc(a, b) myrealloc(__FILE__, __LINE__, a, b)
#define free(x) myfree(__FILE__, __LINE__, x)

void* mymalloc(char*, int, size_t size);
void* myrealloc(char*, int, void* p, size_t size);
void myfree(char*, int, void* p);

typedef struct
{
	int current_size;
	int max_size;
} heap_info;

void HeapScan();
int Heap_initialize();
void Heap_terminate();
heap_info* Heap_get_info();


#endif
