/* (C) Copyright IBM Corp. 2007, 2008
  Author: Ian Craggs
*/

#ifndef BRIDGE_H
#define BRIDGE_H

#include "Clients.h"
#include "SubsEngine.h"
#include "MQTTPacket.h"


typedef struct
{
	char *pattern, 
			*localPrefix, 
			*remotePrefix;
	int direction;  /* out, in or both */
} BridgeTopics;

typedef struct
{
	char* name; /* and cientid? */
	List* addresses; /* of strings */
	ListElement* cur_address;
	int round_robin;
	int try_nano;
	int last_connect_succeeded;
	int no_successful_connections;
	int notifications;
	char* notification_topic;
	//int persistent;  /* cleansession or not */
	int inbound_filter;
	List* topics; /* of BridgeTopics */
	Clients* primary;
	Clients* backup; /* not needed in round robin mode */
} BridgeConnections;

typedef struct
{
	List* connections;
} Bridges;

void Bridge_initialize(Bridges* br, SubscriptionEngines* se);
BridgeConnections* Bridge_new_connection(char* name);
void Bridge_terminate(Bridges* br);
void Bridge_timeslice(Bridges* bridge);
void Bridge_handleConnection(Clients* client);
int Bridge_handleConnacks(void* pack, int sock);
void Bridge_handleInbound(Clients* client, Publish* publish);
void Bridge_handleOutbound(Clients* client, Publish* publish);

#endif // BRIDGE_H
