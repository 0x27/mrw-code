/* (C) Copyright IBM Corp. 2007, 2008
Author: Ian Craggs
*/

#include "TCPSocket.h"
#include "Log.h"
#include "SocketBuffer.h"
#include "Messages.h"

#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "Heap.h"

int TCPSocket_close_only(int socket);
int TCPSocket_continueWrites();

#if defined(WIN32)
#define iov_len len
#define iov_base buf
#endif

#if !defined(max)
#define max(A,B) ( (A) > (B) ? (A):(B))
#endif

static TCPSockets s;

int TCPSocket_setnonblocking(int sock)
{
	int rc;
#if defined(WIN32)
	u_long flag = 1L;
	rc = ioctl(sock, FIONBIO, &flag);
#else
	int flags;
	if ((flags = fcntl(sock, F_GETFL, 0)))
		flags = 0;
	rc = fcntl(sock, F_SETFL, flags | O_NONBLOCK);
#endif
	return rc;
}


int TCPSocket_error(char* aString, int sock)
{
#if defined(WIN32)
	int errno;
	errno = WSAGetLastError(); 
#endif
	if (errno != EINTR && errno != EAGAIN && errno != EINPROGRESS && errno != EWOULDBLOCK)
	{
		if (strcmp(aString, "shutdown") != 0 || (errno != ENOTCONN && errno != ECONNRESET))
			Log(LOG_WARNING, Messages_get(75), errno, aString, sock);
	}
	return errno;
}


int TCPSocket_initialize(char* anAddress, int aPort)
{
	int flag = 1;
#if defined(WIN32)
	WORD    winsockVer = 0x0202;
	WSADATA wsd;

	WSAStartup(winsockVer, &wsd);
#else
	signal(SIGPIPE, SIG_IGN);
#endif
	SocketBuffer_initialize();
	s.SERVER_PORT = aPort;
	if (anAddress == NULL || strcmp(anAddress, "INADDR_ANY") == 0)
		s.SERVER_ADDRESS = htonl(INADDR_ANY);
	else
		s.SERVER_ADDRESS = inet_addr(anAddress);
	s.clientsds = ListInitialize();
	s.connect_pending = ListInitialize();
	s.write_pending = ListInitialize();
	s.mySocket = socket(AF_INET, SOCK_STREAM, 0);
	s.maxfdp1 = 0;
	#if !defined(NO_DEBUG)
		Log(LOG_DEBUG, Messages_get(76), FD_SETSIZE);
	#endif
	if (s.mySocket < 0)
	{
		Log(LOG_ERROR, Messages_get(77), aPort);
		return s.mySocket;
	}

	/* bind server port */
#if !defined(WIN32)
	if (setsockopt(s.mySocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&flag, sizeof(int)) != 0)
		Log(LOG_WARNING, Messages_get(109), aPort);
#endif
	s.servAddr.sin_family = AF_INET;
	s.servAddr.sin_addr.s_addr = s.SERVER_ADDRESS;
	s.servAddr.sin_port = htons(s.SERVER_PORT);
	memset(s.servAddr.sin_zero, 0, sizeof(s.servAddr.sin_zero));
	if (bind(s.mySocket, (struct sockaddr *)&(s.servAddr), sizeof(s.servAddr)) == SOCKET_ERROR)
	{
		Log(LOG_ERROR, Messages_get(78), aPort);
		return SOCKET_ERROR;
	}
	if (listen(s.mySocket, SOMAXCONN) == SOCKET_ERROR) // second parm is max no of connections
	{
		Log(LOG_ERROR, Messages_get(79), aPort);
		return SOCKET_ERROR;
	}
	if (TCPSocket_setnonblocking(s.mySocket) == SOCKET_ERROR)
	{
		TCPSocket_error("setnonblocking", s.mySocket);
		return SOCKET_ERROR;
	}
	FD_ZERO(&(s.rset));                                                                         // Initialize the descriptor set
	FD_SET((u_int)s.mySocket, &(s.rset));         // Add the current socket descriptor
	s.maxfdp1 = s.mySocket + 1;
	memcpy((void*)&(s.rset_saved), (void*)&(s.rset), sizeof(s.rset_saved));
	s.cur_clientsds = NULL;
	return 0;
}


void TCPSocket_terminate()
{
	ListFree(s.connect_pending);
	ListFree(s.write_pending);
	ListFree(s.clientsds);
	SocketBuffer_terminate();
	TCPSocket_close_only(s.mySocket);
#if defined(WIN32)
	WSACleanup();
#endif
}


int TCPSocket_addSocket(int newSd)
{
	int rc = 0;

	if (ListFindItem(s.clientsds, &newSd, intcompare) == NULL) /* make sure we don't add the same socket twice */
	{
		int* pnewSd = malloc(sizeof(newSd));
		*pnewSd = newSd;
		ListAppend(s.clientsds, pnewSd, sizeof(newSd));
		FD_SET((u_int)newSd, &(s.rset_saved));
		s.maxfdp1 = max(s.maxfdp1, newSd + 1);
		rc = TCPSocket_setnonblocking(newSd);
	}
	#if !defined(NO_DEBUG)
		else
			Log(LOG_DEBUG, Messages_get(81), newSd);
	#endif

	return rc;
}


int isReady(int socket, fd_set* read_set, fd_set* write_set)
{
	// don't accept work from a client unless it is accepting work back, i.e. its socket is writeable
	// this seems like a reasonable form of flow control, and practically, seems to work.
  return FD_ISSET(socket, read_set) && FD_ISSET(socket, write_set);
}


/* We have two types of socket, the main broker one on which connections are accepted,
and client sockets which are generated as a result of accepting connections.  So there
are two different actions to be taken depending on the type of the connection.  We can use the
select() call to check for actions to be taken on both types of socket when especially when
we are in single-threaded mode.
*/
int TCPSocket_getReadySocket()
{
	int rc = 0;
	struct timeval timeout = {1, 0}; /* 1 second */
	static fd_set wset;
	struct timeval zero = {0, 0}; /* 0 seconds */

	while (s.cur_clientsds != NULL)
	{
		if (isReady(*((int*)(s.cur_clientsds->content)), &(s.rset), &wset))
			break;
		ListNextElement(s.clientsds, &s.cur_clientsds);
	}

	if (s.cur_clientsds == NULL)
	{
		int rc1;
		fd_set pwset;

		memcpy((void*)&(s.rset), (void*)&(s.rset_saved), sizeof(s.rset));
		memcpy((void*)&(pwset), (void*)&(s.pending_wset), sizeof(pwset));
		if ((rc = select(s.maxfdp1+1, &(s.rset), &pwset, NULL, &timeout)) == SOCKET_ERROR)
		{
			TCPSocket_error("read select", 0);
			return rc;
		}
		#if !defined(NO_DEBUG)
			Log(LOG_DEBUG, Messages_get(83), rc);
		#endif

		if (TCPSocket_continueWrites(&pwset) == SOCKET_ERROR)
			return 0;

		memcpy((void*)&wset, (void*)&(s.rset_saved), sizeof(wset));
		if ((rc1 = select(s.maxfdp1+1, NULL, &(wset), NULL, &zero)) == SOCKET_ERROR)
		{
			TCPSocket_error("write select", 0);
			return rc;
		}
		#if !defined(NO_DEBUG)
			Log(LOG_DEBUG, Messages_get(84), rc1);
		#endif
			
		if (rc == 0 && rc1 == 0)
			return rc; // no work to do
	
		if (FD_ISSET(s.mySocket, &(s.rset)))  /* if this is a new connection attempt */
		{
			int newSd;
			struct sockaddr_in cliAddr;
			int cliLen = sizeof(cliAddr);

			if ((newSd = accept(s.mySocket, (struct sockaddr *)&cliAddr, &cliLen)) == SOCKET_ERROR)
				TCPSocket_error("accept", s.mySocket);
			else
			{
				#if !defined(NO_DEBUG)
					Log(LOG_DEBUG, Messages_get(85), newSd, inet_ntoa(cliAddr.sin_addr), cliAddr.sin_port);
				#endif
				TCPSocket_addSocket(newSd);
			}
		}

		s.cur_clientsds = s.clientsds->first;
		while (s.cur_clientsds != NULL)
		{
			int cursock = *((int*)(s.cur_clientsds->content));
			if (isReady(cursock, &(s.rset), &wset))
				break;
			if  (ListFindItem(s.connect_pending, &cursock, intcompare) && FD_ISSET(cursock, &wset))
			{
				ListRemoveItem(s.connect_pending, &cursock, intcompare);
				break;
			}
			ListNextElement(s.clientsds, &s.cur_clientsds);
		}
	}

	if (s.cur_clientsds == NULL)
		rc = 0;
	else
	{
		rc = *((int*)(s.cur_clientsds->content));
		ListNextElement(s.clientsds, &s.cur_clientsds);
	}
	#if !defined(NO_DEBUG)
		Log(LOG_DEBUG, Messages_get(86), rc);
	#endif
	return rc;
} // end getReadySocket



int TCPSocket_getch(int socket, char* c)
{
	int rc = TCPSOCKET_COMPLETE;

	if ((rc = SocketBuffer_getQueuedChar(socket, c)) != 0)
		return rc;

	//The return value from recv will be 0 when the peer has performed an orderly shutdown.
	if ((rc = recv(socket, c, (size_t)1, 0)) == SOCKET_ERROR)
	{
		int err = TCPSocket_error("recv - getch", socket);
		if (err == EWOULDBLOCK || err == EAGAIN)
		{  
			rc = TCPSOCKET_INTERRUPTED;
			SocketBuffer_interrupted(socket, 0);
		}
	}
	else if (rc == 0)
		rc = SOCKET_ERROR;
	else if (rc == 1)
		SocketBuffer_queueChar(socket, *c);

	return rc;
}


char *TCPSocket_getdata(int socket, int bytes, int* actual_len)
{
	int rc;
	char* buf;

	if (bytes == 0)
		return SocketBuffer_complete(socket);

	buf = SocketBuffer_getQueuedData(socket, bytes, actual_len);

	if ((rc = recv(socket, buf + (*actual_len), (size_t)(bytes - (*actual_len)), 0)) == SOCKET_ERROR)
	{
		rc = TCPSocket_error("recv - getdata", socket);
		if (rc != EAGAIN && rc != EWOULDBLOCK)
			return NULL;
	}
	else if (rc == 0) // rc 0 means the other end closed the socket, albeit "gracefully"
		return NULL;
	else
		*actual_len += rc;

	if (*actual_len == bytes)
		SocketBuffer_complete(socket);
	else // we didn't read the whole packet
	{
		SocketBuffer_interrupted(socket, *actual_len);
		#if !defined(NO_DEBUG)
			Log(LOG_DEBUG, Messages_get(87), bytes, *actual_len);
		#endif
	}
	
	return buf;
}


int TCPSocket_noPendingWrites(int socket)
{
	int cursock = socket;
	return ListFindItem(s.write_pending, &cursock, intcompare) == NULL;
}


int TCPSocket_writev(int socket, iobuf* iovecs, int count, long* bytes)
{
	int rc;
#if defined(WIN32)
	rc = WSASend(socket, iovecs, count, bytes, 0, NULL, NULL);
	if (rc == SOCKET_ERROR)
	{
		int err = TCPSocket_error("WSASend - putdatas", socket);
		if (err == EWOULDBLOCK || err == EAGAIN)
			rc = TCPSOCKET_INTERRUPTED;
	}
#else
	*bytes = 0L;
	rc = writev(socket, iovecs, count);
	if (rc == SOCKET_ERROR)
	{
		int err = TCPSocket_error("writev - putdatas", socket);
		if (err == EWOULDBLOCK || err == EAGAIN)
			rc = TCPSOCKET_INTERRUPTED;
	}
	else
		*bytes = rc;
#endif
	return rc;
}


int TCPSocket_putdatas(int socket, char* buf0, int buf0len, int count, char** buffers, int* buflens)
{
	long bytes = 0L;
	iobuf iovecs[5];
	int rc = 0, i, total = buf0len;

	for (i = 0; i < count; i++)
		total += buflens[i];

	iovecs[0].iov_base = buf0;
	iovecs[0].iov_len = buf0len;
	for (i = 0; i < count; i++)
	{
		iovecs[i+1].iov_base = buffers[i];
		iovecs[i+1].iov_len = buflens[i];
	}

	if ((rc = TCPSocket_writev(socket, iovecs, count+1, &bytes)) != SOCKET_ERROR)
	{
		if (bytes == total)
			rc = TCPSOCKET_COMPLETE;
		else if (bytes == 0)
		{
		  Log(LOG_WARNING, Messages_get(88));
			rc = TCPSOCKET_NOWORK;
		}
		else
		{
			int* sockmem = malloc(sizeof(int));
			Log(LOG_PROTOCOL, Messages_get(89), bytes, total, socket);
			SocketBuffer_pendingWrite(socket, count+1, iovecs, total, bytes);
			*sockmem = socket;
			ListAppend(s.write_pending, sockmem, sizeof(int));
			FD_SET(socket, &(s.pending_wset));
		}
	}
  return rc;
}


int TCPSocket_close_only(int socket)
{
	int rc;
#if defined(WIN32)
	if (shutdown(socket, SD_BOTH) == SOCKET_ERROR)
		TCPSocket_error("shutdown", socket);
	return closesocket(socket);
#else
	if (shutdown(socket, SHUT_RDWR) == SOCKET_ERROR)
		TCPSocket_error("shutdown", socket);
	if ((rc = close(socket)) == SOCKET_ERROR)
		TCPSocket_error("close", socket);
#endif
	return rc;
}


void TCPSocket_close(int socket)
{
	TCPSocket_close_only(socket);
	FD_CLR((u_int)socket, &(s.rset_saved));
	if (s.cur_clientsds != NULL && *(int*)(s.cur_clientsds->content) == socket)
		s.cur_clientsds = s.cur_clientsds->next;
	ListRemoveItem(s.connect_pending, &socket, intcompare);
	ListRemoveItem(s.write_pending, &socket, intcompare);
	SocketBuffer_cleanup(socket);
	if (ListRemoveItem(s.clientsds, &socket, intcompare))
		#if !defined(NO_DEBUG)
			Log(LOG_DEBUG, Messages_get(90), socket);
		#else
			;
		#endif
	else
		Log(LOG_WARNING, Messages_get(91), socket);
	if (socket + 1 >= s.maxfdp1)
	{
		/* now we have to reset s.maxfdp1 */
		ListElement* cur_clientsds = NULL;

		s.maxfdp1 = s.mySocket;
		while (ListNextElement(s.clientsds, &cur_clientsds))
			s.maxfdp1 = max(*((int*)(cur_clientsds->content)), s.maxfdp1);
		++(s.maxfdp1);
		//Log(LOG_DEBUG, "Reset max fdp1 to %d\n", s.maxfdp1);
	}
}


int TCPSocket_new(char* addr, int port, int* sock)
{
	struct sockaddr_in address;
	int rc = SOCKET_ERROR;

	*sock = -1;
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	if ((address.sin_addr.s_addr = inet_addr(addr)) == SOCKET_ERROR)
	{
		Log(LOG_ERROR, Messages_get(92), addr);
		/*
		struct hostent* e;
		struct in_addr* s;
		if ((e = gethostbyname(addr)) == NULL)
			printf("gethostbyname error\n");
		else
		{
			s = (struct in_addr*)(e->h_addr_list[0]);
			address.sin_addr.s_addr = s->s_addr;
			found = 1;
		}
		*/
	}
	else
	{
		*sock =	socket(AF_INET, SOCK_STREAM, 0);
		#if !defined(NO_DEBUG)
			Log(LOG_DEBUG, Messages_get(93), *sock, addr, port);
		#endif
		if (TCPSocket_addSocket(*sock) == SOCKET_ERROR)
			rc = TCPSocket_error("setnonblocking", *sock);
		else
		{
			// this could complete immmediately, even though we are non-blocking 
			if ((rc = connect(*sock, (struct sockaddr*)&address, sizeof(address))) == SOCKET_ERROR)
				rc = TCPSocket_error("connect", *sock);
			if (rc == EINPROGRESS || rc == EWOULDBLOCK)
			{
				int* pnewSd = malloc(sizeof(int));
				*pnewSd = *sock;
				ListAppend(s.connect_pending, pnewSd, sizeof(int));
				#if !defined(NO_DEBUG)
					Log(LOG_DEBUG, Messages_get(94));
				#endif
			}
		}
	}

	return rc;
}


char* TCPSocket_gethostname()
{
	static char buf[MAXHOSTNAMELEN+1] = "";
	gethostname(buf, MAXHOSTNAMELEN+1);
	return buf;
}


int TCPSocket_continueWrite(int socket)
{
	int rc = 0;
	pending_writes* pw;// = SocketBuffer_getWrite(socket);
	unsigned long curbuflen = 0L, // cumulative total of buffer lengths
		bytes;
	int curbuf = -1, i;
	iobuf iovecs1[5];

	pw = SocketBuffer_getWrite(socket);

	for (i = 0; i < pw->count; ++i)
	{
		if (pw->bytes <= curbuflen)
		{ // if previously written length is less than the buffer we are currently looking at, add the whole buffer
			iovecs1[++curbuf].iov_len = pw->iovecs[i].iov_len;
			iovecs1[curbuf].iov_base = pw->iovecs[i].iov_base;
		}
		else if (pw->bytes < curbuflen + pw->iovecs[i].iov_len)
		{ // if previously written length is in the middle of the buffer we are currently looking at, add some of the buffer
			int offset = pw->bytes - curbuflen;
			iovecs1[++curbuf].iov_len = pw->iovecs[i].iov_len - offset;
			iovecs1[curbuf].iov_base = pw->iovecs[i].iov_base + offset;
			break;
		}
		curbuflen += pw->iovecs[i].iov_len;
	}

	if ((rc = TCPSocket_writev(socket, iovecs1, curbuf+1, &bytes)) != SOCKET_ERROR)
	{
		pw->bytes += bytes;
		Log(LOG_PROTOCOL, Messages_get(95), bytes, socket);
	  rc = (pw->bytes == pw->total); 
	}
	return rc;
}


int TCPSocket_continueWrites(fd_set* pwset)
{
	int rc1 = 0;
	ListElement* curpending = s.write_pending->first;

	while (curpending)
	{
		int socket = *(int*)(curpending->content);
		int rc = TCPSocket_continueWrite(socket);
		if (FD_ISSET(socket, pwset) && rc)
		{
			if (!SocketBuffer_writeComplete(socket))
				Log(LOG_ERROR, Messages_get(96));
			FD_CLR(socket, &(s.pending_wset));
			if (!ListRemove(s.write_pending, curpending->content))
			{
				Log(LOG_ERROR, Messages_get(97));
				ListNextElement(s.write_pending, &curpending);
			}
			curpending = s.write_pending->current;
		}
		else
			ListNextElement(s.write_pending, &curpending);
	}
	return rc1;
}


#if defined(TCPSOCKET_TEST)

int main(int argc, char *argv[])
{
	TCPSocket_connect("127.0.0.1", 1883);
	TCPSocket_connect("localhost", 1883);
	TCPSocket_connect("loadsadsacalhost", 1883);
}

#endif

