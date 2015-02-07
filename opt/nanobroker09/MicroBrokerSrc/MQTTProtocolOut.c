#if !defined(NO_BRIDGE) && defined(MQTTPROTOCOL_H)

#include "MQTTProtocolOut.h"


char* MQTTProtocol_addressPort(char* ip_address, int* port)
{
	static char buf[20];
	char* pos = strchr(ip_address, ':');

	if (pos)
	{
		int len = pos - ip_address;
		*port = atoi(pos+1);
		strncpy(buf, ip_address, len);
		buf[len] = '\0';
		pos = buf;
	}
	else
	{
		*port = DEFAULT_PORT;
	  pos = ip_address;
	}
	return pos;
}


void MQTTProtocol_reconnect(char* ip_address, Clients* client)
{
	int port, rc;
	char *address = MQTTProtocol_addressPort(ip_address, &port);

	client->good = 1;
	client->connect_state = 0;
	client->connected = 0;
	if (client->cleansession)
		client->msgID = 0;

	rc = TCPSocket_new(address, port, &(client->socket));
	if (rc == EINPROGRESS || rc == EWOULDBLOCK)
		client->connect_state = 1; // TCP connect called
	else if (rc == 0)
	{
		Log(LOG_PROTOCOL, Messages_get(68), client->clientID, client->socket);
		client->connect_state = 2; // TCP connect completed, in which case send the MQTT connect packet
		MQTTPacket_send_connect(client);
		time(&(client->lastContact));
	}
}


Clients* MQTTProtocol_connect(char* ip_address, char* clientID, int cleansession, int try_nano)
{ /* outgoing connection */
	int rc, port; 
	char* addr;
	Clients* newc = malloc(sizeof(Clients));

	newc->clientID = clientID;
	newc->cleansession = cleansession;
	newc->outbound = 1;
	newc->connected = 0;
	newc->keepAliveInterval = 60; /* DCJ's requested default.  Should we make it configurable? */
	newc->msgID = 0;
	newc->outboundMsgs = ListInitialize();
	newc->inboundMsgs = ListInitialize();
	newc->queuedMsgs = ListInitialize();
	newc->will = NULL;
	newc->good = 1;
	newc->connect_state = 0;
	newc->noLocal = try_nano; /* try Nano connection first */
	time(&(newc->lastContact));

	addr = MQTTProtocol_addressPort(ip_address, &port);
	rc = TCPSocket_new(addr, port, &(newc->socket));
	if (rc == EINPROGRESS || rc == EWOULDBLOCK)
		newc->connect_state = 1; // TCP connect called
	else if (rc == 0)
	{
		Log(LOG_PROTOCOL, Messages_get(69), newc->clientID, newc->socket, newc->noLocal);
		newc->connect_state = 2; // TCP connect completed, in which case send the MQTT connect packet
		MQTTPacket_send_connect(newc);
	}
	ListAppend(bstate->clients, newc, sizeof(Clients) + strlen(newc->clientID)+1 + 3*sizeof(List));

	return newc;
}


int MQTTProtocol_handlePingresps(void* pack, int sock)
{
	Clients* client = (Clients*)(ListFindItem(bstate->clients, &sock, clientSocketCompare)->content);

	Log(LOG_PROTOCOL, Messages_get(70), client->clientID);	
	if (client->outbound == 0)
		Log(LOG_WARNING, Messages_get(71), client->clientID);
	return TCPSOCKET_COMPLETE;
}

int MQTTProtocol_subscribe(Clients* client, List* topics, List* qoss)
{
	Log(LOG_PROTOCOL, Messages_get(72), client->clientID);
	// we should stack this up for retry processing too	
	return MQTTPacket_send_subscribe(topics, qoss, MQTTProtocol_assignMsgId(client), 0, client->socket);
}


int MQTTProtocol_handleSubacks(void* pack, int sock)
{
	Suback* suback = (Suback*)pack;
	Clients* client = (Clients*)(ListFindItem(bstate->clients, &sock, clientSocketCompare)->content);
	Log(LOG_PROTOCOL, Messages_get(73), client->clientID);
	MQTTPacket_freeSuback(suback);
	return TCPSOCKET_COMPLETE;
}


int MQTTProtocol_handleUnsubacks(void* pack, int sock)
{
	Unsuback* unsuback = (Unsuback*)pack;
	Clients* client = (Clients*)(ListFindItem(bstate->clients, &sock, clientSocketCompare)->content);

	Log(LOG_PROTOCOL, Messages_get(74), client->clientID);
	free(unsuback);
	return TCPSOCKET_COMPLETE;
}

#endif
