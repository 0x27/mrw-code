/* (C) Copyright IBM Corp. 2007, 2008
    Author: Ian Craggs               
*/

#if !defined(SOCKETBUFFER_H)
#define SOCKETBUFFER_H

#if defined(WIN32)
#include "winsock2.h"
#else
#include <sys/socket.h>
#endif

#if defined(WIN32)
	typedef WSABUF iobuf;
#else 
	typedef struct iovec iobuf;
#endif

typedef struct
{
	int socket;
	int index, headerlen;
	char fixed_header[5];  // header plus up to 4 length bytes
	int buflen, // total length of the buffer
		datalen; // current length of data in buf
	char* buf;
} socket_queue;

typedef struct
{
	int socket, total, count;
	unsigned long bytes;
	iobuf iovecs[5];
} pending_writes;

#if !defined(SOCKET_ERROR)
	#define SOCKET_ERROR -1
#endif
#define SOCKETBUFFER_INTERRUPTED 0
#define SOCKETBUFFER_COMPLETE 1

void SocketBuffer_initialize();
void SocketBuffer_terminate();
void SocketBuffer_cleanup(int socket);
char* SocketBuffer_getQueuedData(int socket, int bytes, int* actual_len);
int SocketBuffer_getQueuedChar(int socket, char* c);
void SocketBuffer_interrupted(int socket, int actual_len);
char* SocketBuffer_complete(int socket);
void SocketBuffer_queueChar(int socket, char c);

void SocketBuffer_pendingWrite(int socket, int count, iobuf* iovecs, int total, int bytes);
pending_writes* SocketBuffer_getWrite(int socket);
int SocketBuffer_writeComplete(int socket);

#endif
