/* (C) Copyright IBM Corp. 2007, 2008
         Author: Ian Craggs
*/

#if !defined(MQTTPROTOCOL_H)
#define MQTTPROTOCOL_H

#include "NanoBroker.h"
#include "LinkedList.h"
#include "SubsEngine.h"
#include "MQTTPacket.h"
#include "Clients.h"

#define MAX_MSG_ID 65535
#define MAX_CLIENTID_LEN 24

typedef struct
{ 
	int socket;
	Publications* p;
} pending_write;


typedef struct
{
	List publications;
	unsigned int msgs_received;
	unsigned int msgs_sent;
	List pending_writes; // for qos 0 writes not complete
} MQTTProtocol;

int MQTTProtocol_initialize(BrokerStates*);
void MQTTProtocol_timeslice();
void MQTTProtocol_terminate();
void MQTTProtocol_freeClient(Clients* client);
void MQTTProtocol_emptyMessageList(List* msgList);
void MQTTProtocol_freeMessageList(List* msgList);
void MQTTProtocol_sys_publish(char* topic, char* string);

void MQTTProtocol_closeSession(Clients* client, int unclean);

int MQTTProtocol_handleConnects(void* pack, int sock);
int MQTTProtocol_handlePingreqs(void* pack, int sock);
int MQTTProtocol_handleDisconnects(void* pack, int sock);
int MQTTProtocol_handleSubscribes(void* pack, int sock);
int MQTTProtocol_handleUnsubscribes(void* pack, int sock);
int MQTTProtocol_handlePublishes(void* pack, int sock);
int MQTTProtocol_handlePubacks(void* pack, int sock);
int MQTTProtocol_handlePubrecs(void* pack, int sock);
int MQTTProtocol_handlePubrels(void* pack, int sock);
int MQTTProtocol_handlePubcomps(void* pack, int sock);

#if !defined(NO_BRIDGE)
	#include "MQTTProtocolOut.h"
#endif

#endif
