/* (C) Copyright IBM Corp. 2007, 2008
    Author: Ian Craggs               
*/

#if !defined(TCPSOCKET_H)
#define TCPSOCKET_H

#include <sys/types.h>

#if defined(WIN32)
#include "winsock2.h"
#else
#include <sys/socket.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#endif

#if !defined(SOCKET_ERROR)
	#define SOCKET_ERROR -1
#endif
#define TCPSOCKET_INTERRUPTED 0
#define TCPSOCKET_COMPLETE 1
#define TCPSOCKET_NOWORK -2

#if defined(WIN32)
#define MAXHOSTNAMELEN 256
#define EAGAIN WSAEWOULDBLOCK
#define EINTR WSAEINTR
#define EINPROGRESS WSAEINPROGRESS
#define EWOULDBLOCK WSAEWOULDBLOCK
#define ENOTCONN WSAENOTCONN
#define ECONNRESET WSAECONNRESET
#define ioctl ioctlsocket
#define socklen_t int
#endif

#include "LinkedList.h"

typedef struct
{
	int mySocket;

	fd_set rset, rset_saved;
	struct sockaddr_in servAddr;
	int maxfdp1;

	List* clientsds;
	ListElement* cur_clientsds;

	int SERVER_PORT;
	unsigned long SERVER_ADDRESS;

	List* connect_pending;

	List* write_pending;
	fd_set pending_wset;

} TCPSockets;


int TCPSocket_initialize(char* address, int aPort);
void TCPSocket_terminate();
int TCPSocket_getReadySocket();
int TCPSocket_getch(int socket, char* c);
char *TCPSocket_getdata(int socket, int bytes, int* actual_len);
int TCPSocket_putch(int socket, char c);
int TCPSocket_putdata(int socket, char* data, int datalen);
int TCPSocket_putdatas(int socket, char* buf0, int buf0len, int count, char** buffers, int* buflens);
void TCPSocket_close(int socket);
int TCPSocket_new(char* addr, int port, int* socket);
char* TCPSocket_gethostname();

int TCPSocket_noPendingWrites(int socket);

#endif // TCPSOCKET_H
