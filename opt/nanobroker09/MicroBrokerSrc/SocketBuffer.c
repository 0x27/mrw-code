#include "SocketBuffer.h"
#include "LinkedList.h"
#include "Log.h"
#include "Messages.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "Heap.h"

static socket_queue* def_queue; // default queue
static List* queues; // 
static List writes;

int socketcompare(void* a, void* b)
{
	return ((socket_queue*)a)->socket == *(int*)b;
}


void SocketBuffer_newDefQ()
{
	def_queue = malloc(sizeof(socket_queue));
	def_queue->buflen = 1000;
	def_queue->buf = malloc(def_queue->buflen);
	def_queue->socket = def_queue->index = def_queue->buflen = def_queue->datalen = 0;
}


void SocketBuffer_initialize()
{
	SocketBuffer_newDefQ();
	queues = ListInitialize();
	ListZero(&writes);
}


void SocketBuffer_freeDefQ()
{
	free(def_queue->buf);
	free(def_queue);
}


void SocketBuffer_terminate()
{
	ListEmpty(&writes);
	ListFree(queues);
	SocketBuffer_freeDefQ();
}


void SocketBuffer_cleanup(int socket)
{
	ListRemoveItem(queues, &socket, socketcompare);
	if (def_queue->socket == socket)
		def_queue->socket = def_queue->index = def_queue->headerlen = def_queue->datalen = 0;
}


char* SocketBuffer_getQueuedData(int socket, int bytes, int* actual_len)
{
	socket_queue* queue = NULL;

	if (ListFindItem(queues, &socket, socketcompare))
	{  //if there is queued data for this socket, add any data read to it
		queue = (socket_queue*)(queues->current->content);
		*actual_len = queue->datalen;	
	}
	else
	{
		*actual_len = 0;
		queue = def_queue;
	}
	if (bytes > queue->buflen)
	{
		if (queue->datalen > 0)
		{
			void* newmem = malloc(bytes);
			memcpy(newmem, queue->buf, queue->datalen);
			free(queue->buf);
			queue->buf = newmem;
		}
		else
			queue->buf = realloc(queue->buf, bytes);
		queue->buflen = bytes;
	}
	return queue->buf;
}


int SocketBuffer_getQueuedChar(int socket, char* c)
{
	if (ListFindItem(queues, &socket, socketcompare))
	{  //if there is queued data for this socket, read that first
		socket_queue* queue = (socket_queue*)(queues->current->content);
		if (queue->index < queue->headerlen)
		{
			*c = queue->fixed_header[(queue->index)++];
			#if !defined(NO_DEBUG)
				Log(LOG_DEBUG, Messages_get(110), queue->index, queue->headerlen);
			#endif
			return SOCKETBUFFER_COMPLETE;
		}
		else if (queue->index > 4)
		{
			Log(LOG_ERROR, Messages_get(111));
			return SOCKET_ERROR;
		}
	}
	return SOCKETBUFFER_INTERRUPTED;  // there was no queued char
}


void SocketBuffer_interrupted(int socket, int actual_len)
{			
	socket_queue* queue;
	if (ListFindItem(queues, &socket, socketcompare))
		queue = (socket_queue*)(queues->current->content);
	else // new saved queue
	{
		queue = def_queue;
		ListAppend(queues, def_queue, sizeof(socket_queue)+def_queue->buflen);
		SocketBuffer_newDefQ();
	}
	queue->index = 0;
	queue->datalen = actual_len;
}


char* SocketBuffer_complete(int socket)
{
	if (ListFindItem(queues, &socket, socketcompare))
	{
		socket_queue* queue = (socket_queue*)(queues->current->content);
		SocketBuffer_freeDefQ();
		def_queue = queue;
		ListDetach(queues, queue);
	}
	def_queue->socket = def_queue->index = def_queue->headerlen = def_queue->datalen = 0;

	return def_queue->buf;
}


void SocketBuffer_queueChar(int socket, char c)
{
	int error = 0;
	socket_queue* curq = def_queue;

	if (ListFindItem(queues, &socket, socketcompare))
		curq = (socket_queue*)(queues->current->content);
	else if (def_queue->socket == 0)
	{
		def_queue->socket = socket;
		def_queue->index = def_queue->datalen = 0;
	}
	else if (def_queue->socket != socket)
	{
		Log(LOG_ERROR, Messages_get(112));
	  error = 1;
	}
	if (curq->index > 4)
	{
		Log(LOG_ERROR, Messages_get(113));
		error = 1;
	}
	if (!error)
	{
		curq->fixed_header[(curq->index)++] = c;
		curq->headerlen = curq->index;
	}
	#if !defined(NO_DEBUG)
		Log(LOG_DEBUG, Messages_get(114), curq->index, curq->headerlen);
	#endif
}


void SocketBuffer_pendingWrite(int socket, int count, iobuf* iovecs, int total, int bytes)
{
	int i = 0;

	// store the buffers until the whole packet is written
	pending_writes* pw = malloc(sizeof(pending_writes));
	pw->socket = socket;
	pw->bytes = bytes;
	pw->total = total;
	pw->count = count;
	for (i = 0; i < count; i++)
		pw->iovecs[i] = iovecs[i];
	ListAppend(&writes, pw, sizeof(pw) + total);
}


int pending_socketcompare(void* a, void* b)
{
	return ((pending_writes*)a)->socket == *(int*)b;
}


pending_writes* SocketBuffer_getWrite(int socket)
{
	ListElement* le = ListFindItem(&writes, &socket, pending_socketcompare);
	return (pending_writes*)(le->content);
}


int SocketBuffer_writeComplete(int socket)
{
	return ListRemoveItem(&writes, &socket, pending_socketcompare);
}
