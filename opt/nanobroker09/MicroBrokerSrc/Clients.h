/* (C) Copyright IBM Corp. 2007, 2008
	 Author: Ian Craggs
*/

#if !defined(CLIENTS_H)
#define CLIENTS_H

#include <time.h>
#include "LinkedList.h"

typedef struct
{
	char *topic;
	char* payload;
	int payloadlen;
	int refcount;
} Publications;

typedef struct
{
	int qos;
	int retain;
	int msgid;
	Publications *publish;
	time_t lastTouch; /* used for retry and expiry */
	char nextMessageType; /* PUBREC, PUBREL, PUBCOMP */
} Messages;

typedef struct
{
	char *topic;
	char *msg;
	int retained;
	int qos;
} willMessages;

typedef struct
{
	char* clientID;
	int cleansession;
	int connected;
	int good; /* if we have an error on the socket we turn this off */
	int socket;
	int msgID;
	int keepAliveInterval;
	int outbound;
	int noLocal; 
	void* bridge_context; // for bridge use
	int connect_state;
	time_t lastContact;
	willMessages* will;
	List* inboundMsgs; 
	List* outboundMsgs; /* in flight */
	List* queuedMsgs;		/* queued up outbound messages - not in flight */
} Clients;

int clientIDCompare(void* a, void* b);
int clientSocketCompare(void* a, void* b);

#endif
