/* (C) Copyright IBM Corp. 2007, 2008
	 Author: Ian Craggs
*/

#include "MQTTProtocol.h"
#include "Log.h"
#include "Topics.h"
#include "Clients.h"
#include "Bridge.h"
#include "Messages.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "Heap.h"

void MQTTProtocol_clean_clients();
void MQTTProtocol_keepalive(time_t);
void MQTTProtocol_retry(time_t, int);
Publications* MQTTProtocol_storePublication(Publish* publish, int* len);
void MQTTProtocol_removePublication(Publications* p);
int MQTTProtocol_assignMsgId(Clients* client);
void MQTTProtocol_processPublication(Publish* publish, char* originator);
Messages* MQTTProtocol_createMessage(Publish* publish, Publications** p, int qos, int retained, int* len);
void MQTTProtocol_removeAllSubscriptions(char* clientID);
int MQTTProtocol_startOrQueuePublish(Clients* pubclient, Publish* publish, int qos, int retained, Publications** p);

typedef int (*pf)(void*, int);

#if defined(NO_BRIDGE)
static pf handle_packets[] =
{ 
	NULL, // RESERVED
	MQTTProtocol_handleConnects,
	NULL, // connack
	MQTTProtocol_handlePublishes,
	MQTTProtocol_handlePubacks,
	MQTTProtocol_handlePubrecs,
	MQTTProtocol_handlePubrels,
	MQTTProtocol_handlePubcomps, 
	MQTTProtocol_handleSubscribes,
	NULL, // suback
	MQTTProtocol_handleUnsubscribes,
	NULL, // unsuback
	MQTTProtocol_handlePingreqs,
	NULL, // pingresp
	MQTTProtocol_handleDisconnects
};
#else
static pf handle_packets[] =
{ 
	NULL, // RESERVED
	MQTTProtocol_handleConnects,
	Bridge_handleConnacks,
	MQTTProtocol_handlePublishes,
	MQTTProtocol_handlePubacks,
	MQTTProtocol_handlePubrecs,
	MQTTProtocol_handlePubrels,
	MQTTProtocol_handlePubcomps, 
	MQTTProtocol_handleSubscribes,
	MQTTProtocol_handleSubacks,
	MQTTProtocol_handleUnsubscribes,
	MQTTProtocol_handleUnsubacks, 
	MQTTProtocol_handlePingreqs,
	MQTTProtocol_handlePingresps, 
	MQTTProtocol_handleDisconnects
};
#endif

static MQTTProtocol state;
static BrokerStates* bstate;
static time_t last;
static time_t start_time;
static int restarts = -1;

#if !defined(NO_BRIDGE)
#include "MQTTProtocolOut.c"
#endif


int MQTTProtocol_reinitialize()
{
	int rc = 0;

	time(&(last));
	time(&(start_time));
	++restarts;
	Log(LOG_INFO, Messages_get(14), bstate->port);
	if ((rc = TCPSocket_initialize(bstate->bind_address, bstate->port)) != 0)
		Log(LOG_ERROR, Messages_get(15), bstate->port);
	return rc;
}


int MQTTProtocol_initialize(BrokerStates* aBrokerState)
{
	bstate = aBrokerState;
	memset(&state, '\0', sizeof(state));
  return MQTTProtocol_reinitialize();
}


void MQTTProtocol_shutdown(int terminate)
{
	ListElement* current = NULL;

	Log(LOG_INFO, Messages_get(16));
	ListNextElement(bstate->clients, &current);
	while (current)
	{
		Clients* client = (Clients*)(current->content);
		ListNextElement(bstate->clients, &current);
		Log(LOG_INFO, Messages_get(17), client->clientID);
		if (terminate)
			client->cleansession = 1; /* no persistence, so everything is clean */
		MQTTProtocol_closeSession(client, 0);
	}
	TCPSocket_terminate();
}


void MQTTProtocol_terminate()
{
	MQTTProtocol_shutdown(1);
}


void MQTTProtocol_sys_publish(char* topic, char* string)
{
	Publish publish;
	publish.header.byte = 0;
	publish.header.retain = 1;
	publish.payload = string;
	publish.payloadlen = strlen(string);
	publish.topic = topic;
	MQTTProtocol_handlePublishes(&publish, 0);
}


void MQTTProtocol_update(time_t now)
{
	ListElement* current = NULL;
	static char buf[30];
	int connected_clients = 0;

	sprintf(buf, "%d", state.msgs_sent);
	MQTTProtocol_sys_publish("$SYS/broker/messages/sent", buf);
	sprintf(buf, "%d", state.msgs_received);
	MQTTProtocol_sys_publish("$SYS/broker/messages/received", buf);
	sprintf(buf, "%d bytes", Heap_get_info()->current_size);
	MQTTProtocol_sys_publish("$SYS/broker/heap/current size", buf);
	sprintf(buf, "%d bytes", Heap_get_info()->max_size);
	MQTTProtocol_sys_publish("$SYS/broker/heap/maximum size", buf);
	sprintf(buf, "%d seconds", (int)difftime(now, start_time));
	MQTTProtocol_sys_publish("$SYS/broker/uptime", buf);
	sprintf(buf, "%d", restarts);
	MQTTProtocol_sys_publish("$SYS/broker/restart count", buf);

	ListNextElement(bstate->clients, &current);
	while (current)
	{
		Clients* client =	(Clients*)(current->content);
		ListNextElement(bstate->clients, &current);
		if (client->outbound == 0 && client->good && client->connected)
			++connected_clients;
	}
	sprintf(buf, "%d", connected_clients);
	MQTTProtocol_sys_publish("$SYS/broker/client count/connected", buf);
}


void MQTTPacket_checkPendingWrites()
{
	if (state.pending_writes.count > 0)
	{
		ListElement* le = state.pending_writes.first;
		while (le)
		{
			if (TCPSocket_noPendingWrites(((pending_write*)(le->content))->socket))
			{
				MQTTProtocol_removePublication(((pending_write*)(le->content))->p);
				ListRemove(&(state.pending_writes), le->content); // does NextElement itself
				le = state.pending_writes.current;
			}	
			else
				ListNextElement(&(state.pending_writes), &le);
		}
	}
}

/* This flag indicates when we are reading, or trying to read, a packet from its socket.  During this call we mustn't cleanup the client 
structure in MQTTProtocol_closesession even if we get a socket error as we still rely on that structure in MQTTProtocol_timeslice.  
MQTTProtocol_timeslice will cleanup the client structure instead. */
static int in_MQTTPacket_Factory = 0;  

void MQTTProtocol_timeslice()
{
  int sock;
	time_t now = 0;
	MQTTPacket* pack;
	int bridge_connection = 0;
	//static int last_socket_error = 0;

	if ((sock = TCPSocket_getReadySocket()) == SOCKET_ERROR)
	{
		#if defined(WIN32)
		int errno;
		errno = WSAGetLastError(); 
		#endif
		if (errno != EINTR && errno != EAGAIN && errno != EINPROGRESS && errno != EWOULDBLOCK)
		{
			//if (last_socket_error)
			//{
				Log(LOG_WARNING, Messages_get(98));
				MQTTProtocol_shutdown(0);
				SubscriptionEngines_saveRetained(bstate->se);
				MQTTProtocol_reinitialize();
			//	last_socket_error = 0;
			//}
			//else
			//{
			//	Log(LOG_WARNING, Messages_get(82));
			//	MQTTProtocol_clean_clients();
			//	last_socket_error = 1;
			//}
			return;
		}
	}

	MQTTPacket_checkPendingWrites();
	if (sock > 0)
	{
		Clients* client = NULL;
		if (ListFindItem(bstate->clients, &sock, clientSocketCompare) != NULL)
		{
			client = (Clients*)(bstate->clients->current->content);
			time(&(now));
			client->lastContact = now;
			#if !defined(NO_BRIDGE)
				if (client->outbound && client->connect_state == 1)
				{
					Bridge_handleConnection(client);
					bridge_connection = 1;
				}
			#endif
		}
		if (bridge_connection == 0)
		{
			int error;
			in_MQTTPacket_Factory = sock;
			pack = MQTTPacket_Factory(sock, &error);
			in_MQTTPacket_Factory = 0;
			if (pack == NULL)
			{ /* there was an error on the socket, so clean it up */
				if (error == SOCKET_ERROR || error == BAD_MQTT_PACKET)
				{
					//client = NULL;
					//if (ListFindItem(bstate->clients, &sock, clientSocketCompare) != NULL)
					//	client = (Clients*)(bstate->clients->current->content);
					if (client != NULL)
					{
						client->good = 0; // make sure we don't try and send messages to ourselves
						if (error == SOCKET_ERROR)
							Log(LOG_WARNING,  Messages_get(18), client->clientID);
						else
							Log(LOG_WARNING, Messages_get(19), client->clientID);
						MQTTProtocol_closeSession(client, 1);
					}
					else
					{
						if (error == SOCKET_ERROR)
							Log(LOG_WARNING, Messages_get(20), sock);
						else
							Log(LOG_WARNING, Messages_get(21), sock);
						TCPSocket_close(sock);
					}
				}
			}
			else if (handle_packets[pack->header.type] == NULL)
				Log(LOG_WARNING, Messages_get(22), pack->header.type, handle_packets[pack->header.type]);
			else
			{
				if (client == NULL && pack->header.type != CONNECT)
				{
					Log(LOG_WARNING, Messages_get(23), sock, MQTTPacket_name(pack->header.type));
					MQTTPacket_free_packet(pack);
					TCPSocket_close(sock);
				}
				else
				{
					ListElement* elem = NULL;
					if ((*handle_packets[pack->header.type])(pack, sock) == SOCKET_ERROR)
					{
						if (client != NULL)
						{
							client->good = 0; // make sure we don't try and send messages to ourselves
							Log(LOG_WARNING,  Messages_get(18), client->clientID);
							MQTTProtocol_closeSession(client, 1);
						}
						else
						{
							Log(LOG_WARNING, Messages_get(20), sock);
							TCPSocket_close(sock);
						}
					}
					else
					{
						if ((elem = ListFindItem(bstate->clients, &sock, clientSocketCompare)) != NULL)
							time( &(((Clients*)(elem->content))->lastContact) );
					}
				}
			}
		}
	}

	time(&(now));
	if (difftime(now, last) > 5)
	{
		time(&(last));
		MQTTProtocol_keepalive(now);
		MQTTProtocol_retry(now, 1);
		MQTTProtocol_update(now);
	}
	else
		MQTTProtocol_retry(now, 0);
}


void MQTTProtocol_clean_clients()
{
	int error, 
		len = sizeof(error);
	ListElement* current = NULL;

	ListNextElement(bstate->clients, &current);
	while (current)
	{
		Clients* client =	(Clients*)(current->content);
		ListNextElement(bstate->clients, &current);

		if (getsockopt(client->socket, SOL_SOCKET, SO_ERROR, (char*)&error, &len) == SOCKET_ERROR)
		{
			client->good = 0; // make sure we don't try and send messages to ourselves
			Log(LOG_WARNING,  Messages_get(18), client->clientID);
			MQTTProtocol_closeSession(client, 1);
		}
	}
}


void MQTTProtocol_keepalive(time_t now)
{
	ListElement* current = NULL;

	ListNextElement(bstate->clients, &current);
	while (current)
	{
		Clients* client =	(Clients*)(current->content);
		ListNextElement(bstate->clients, &current);
#if !defined(NO_BRIDGE)		
		if (client->outbound)
		{
			if (client->connected && client->keepAliveInterval > 0 
					&& (difftime(now, client->lastContact) >= client->keepAliveInterval))
			{
				MQTTPacket_send_pingreq(client->socket);
				client->lastContact = now;
			}
		}
		else
#endif
		if (client->connected && client->keepAliveInterval > 0 
					&& (difftime(now, client->lastContact) > 2*(client->keepAliveInterval)))
		{ /* zero keepalive interval means never disconnect */
			Log(LOG_INFO, Messages_get(24), client->keepAliveInterval, client->clientID);
			MQTTProtocol_closeSession(client, 1);
		}
	}
}


void MQTTProtocol_processQueued(Clients* client)
{
	#if !defined(NO_DEBUG)
		Log(LOG_DEBUG, Messages_get(25), client->clientID);
	#endif
	while (client->outboundMsgs->count < bstate->max_inflight_messages && client->queuedMsgs->count > 0)
	{
		Publish p;
		Publications* pp = NULL;
		Messages* m = (Messages*)(client->queuedMsgs->first->content);

		#if !defined(NO_DEBUG)
			Log(LOG_DEBUG, Messages_get(26), client->clientID);
		#endif
		p.header.byte = 0;
		p.header.qos = m->qos;
		p.header.retain = m->retain;
		p.header.type = PUBLISH;
		p.msgId = m->msgid;
		p.payload = m->publish->payload;
		p.payloadlen = m->publish->payloadlen;
		p.topic = m->publish->topic;

		MQTTProtocol_startOrQueuePublish(client, &p, m->qos, m->retain, &pp);

		MQTTProtocol_removePublication(m->publish);
		if (!ListRemove(client->queuedMsgs, m))
			Log(LOG_WARNING, Messages_get(27));
	}
}


void MQTTProtocol_retry(time_t now, int doRetry)
{
	ListElement* current = NULL;

	ListNextElement(bstate->clients, &current);
	/* look through the outbound message list of each client, checking to see if a retry is necessary */
	while (current)
	{
		Clients* client =	(Clients*)(current->content);
		ListElement* outcurrent = NULL;

		ListNextElement(bstate->clients, &current);
		if (client->connected == 0 || client->good == 0)
			continue;
		if (doRetry)
		while (ListNextElement(client->outboundMsgs, &outcurrent))
		{
			Messages* m = (Messages*)(outcurrent->content);
			if (difftime(now, m->lastTouch) > bstate->retry_interval)
			{
				if (m->qos == 1 || (m->qos == 2 && m->nextMessageType == PUBREC))
				{
					Publish publish;
					Log(LOG_WARNING, Messages_get(28), client->clientID, client->socket, m->msgid);
					publish.msgId = m->msgid;
					publish.topic = m->publish->topic;
					publish.payload = m->publish->payload;
					publish.payloadlen = m->publish->payloadlen;
					if (MQTTPacket_send_publish(&publish, 1, m->qos, m->retain, client->socket) < 1)
					{
						client->good = 0;
						Log(LOG_WARNING, Messages_get(29), client->clientID, client->socket);
						MQTTProtocol_closeSession(client, 1);
						client = NULL;
					}
					else
						time(&(m->lastTouch));
				}
				else if (m->qos && m->nextMessageType == PUBCOMP)
				{
					Log(LOG_WARNING, Messages_get(30), client->clientID, m->msgid);				
					if (MQTTPacket_send_pubrel(m->msgid, 1, client->socket) < 1)
					{
						client->good = 0;
						Log(LOG_WARNING, Messages_get(18), client->clientID);
						MQTTProtocol_closeSession(client, 1);
						client = NULL;
					}
					else
						time(&(m->lastTouch));
				}
				break;
			}
		}
		if (client)
			MQTTProtocol_processQueued(client);
	}
}


void MQTTProtocol_setWill(Connect* connect, Clients* client)
{
	if (connect->flags.will)
	{
		if (client->will != NULL)
		{
			free(client->will->msg);
			free(client->will->topic);
		}
		else
			client->will = (willMessages*)malloc(sizeof(willMessages));
		client->will->msg = connect->willMsg;
		connect->willMsg = NULL; /* don't free this memory */
		client->will->topic = connect->willTopic;
		connect->willTopic = NULL; /* don't free this memory */
		client->will->retained = connect->flags.willRetain;
		client->will->qos = connect->flags.willQoS;
	}
	else
	{
		if (client->will != NULL)
		{
			free(client->will->msg);
			free(client->will->topic);
			free(client->will);
			client->will = NULL;
		}
	}
}


void MQTTProtocol_removeAllSubscriptions(char* clientID)
{
	SubscriptionEngines_unsubscribe(bstate->se, clientID, (char*)MULTI_LEVEL_WILDCARD);
	SubscriptionEngines_unsubscribe(bstate->se, clientID, "$SYS/#");
}


int clientPrefixCompare(void* prefix, void* clientid)
{
	//printf("comparing prefix %s with clientid %s\n", (char*)prefix, (char*)clientid);
	return strncmp((char*)prefix, (char*)clientid, strlen(prefix)) == 0;
}

	
int MQTTProtocol_handleConnects(void* pack, int sock)
{
	Connect* connect = (Connect*)pack;
	ListElement* elem = NULL;
	int terminate = 0;
	int rc = TCPSOCKET_COMPLETE;
	struct sockaddr sa;
	socklen_t sal = sizeof(sa);
#define ADDR_LEN 64 //INET6_ADDRSTRLEN 
	int addrlen = ADDR_LEN;
	char addr_string[ADDR_LEN];

	if (bstate->clientid_prefixes->count > 0 && 
		!ListFindItem(bstate->clientid_prefixes, connect->clientID, clientPrefixCompare))
	{
		Log(LOG_WARNING, Messages_get(31), connect->clientID);
		terminate = 1;
	}
	else if (!MQTTPacket_checkVersion(pack))
	{
		Log(LOG_WARNING, Messages_get(32), connect->Protocol, connect->version);
		MQTTPacket_send_connack(1, sock); /* send response */
		terminate = 1;
	}

	if (terminate)
	{
		MQTTPacket_freeConnect(connect);
		TCPSocket_close(sock);
		return TCPSOCKET_COMPLETE;
	}

	getpeername(sock, &sa, &sal);
#if defined(WIN32)
	if (WSAAddressToString(&sa, sizeof(sa), NULL, (LPWSTR)addr_string, &addrlen) == SOCKET_ERROR)
		printf("Last error %d\n", WSAGetLastError());
#else
	inet_ntop(sa.sa_family, sa.sa_data, addr_string, addrlen);
#endif

	Log((bstate->connection_messages) ? LOG_INFO : LOG_PROTOCOL, 
		Messages_get(33), connect->clientID, addr_string);
	
	elem = ListFindItem(bstate->clients, connect->clientID, clientIDCompare);
	if (elem == NULL)
	{
		Clients* newc = malloc(sizeof(Clients));
		newc->clientID = connect->clientID;
		newc->cleansession = connect->flags.cleanstart;
		newc->outbound = 0;
		newc->socket = sock;
		newc->connected = 1;
		newc->keepAliveInterval = connect->keepAliveTimer;
		newc->msgID = 0;
		newc->outboundMsgs = ListInitialize();
		newc->inboundMsgs = ListInitialize();
		newc->queuedMsgs = ListInitialize();
		newc->will = NULL;
		newc->good = 1;
		newc->noLocal = (connect->version == NANO_PROTOCOL_VERSION) ? 1 : 0;
		MQTTProtocol_setWill(connect, newc);
		ListAppend(bstate->clients, newc, sizeof(Clients) + strlen(newc->clientID)+1 + 3*sizeof(List));
		connect->clientID = NULL; /* don't want to free this space as it is being used in the clients list above */
		rc = MQTTPacket_send_connack(0, sock); /* send response */
	}
	else
	{
		Clients* client = (Clients*)(elem->content);
		if (client->connected)
		{
			Log(LOG_INFO, Messages_get(34), connect->clientID);
			if (client->socket != sock)
				TCPSocket_close(client->socket);
		}
		client->socket = sock;
		client->connected = 1;
		client->good = 1;
		client->cleansession = connect->flags.cleanstart;
		if (client->cleansession)
		{
			MQTTProtocol_removeAllSubscriptions(client->clientID);
			/* empty pending message lists */
			MQTTProtocol_emptyMessageList(client->outboundMsgs);
			MQTTProtocol_emptyMessageList(client->inboundMsgs);
			MQTTProtocol_emptyMessageList(client->queuedMsgs);
		}
		client->keepAliveInterval = connect->keepAliveTimer;
		MQTTProtocol_setWill(connect, client);
		rc = MQTTPacket_send_connack(0, sock); /* send response */
		MQTTProtocol_processQueued(client);
	}
	MQTTPacket_freeConnect(connect);
	return rc;
}


int MQTTProtocol_handlePingreqs(void* pack, int sock)
{
	Clients* client = (Clients*)(ListFindItem(bstate->clients, &sock, clientSocketCompare)->content);
	int rc = TCPSOCKET_COMPLETE;

	Log(LOG_PROTOCOL, Messages_get(35), client->clientID);
	rc = MQTTPacket_send_pingresp(sock);
	//if (MQTTPacket_send_pingresp(sock) == SOCKET_ERROR) /* send response */
	//{
	//	Log(LOG_WARNING, Messages_get(18), client->clientID);
	//	MQTTProtocol_closeSession(client, 1);
	//}
	return rc;
}


void MQTTProtocol_emptyMessageList(List* msgList)
{
	ListElement* current = NULL;
	while (ListNextElement(msgList, &current))
	{
		Messages* m = (Messages*)(current->content);
		MQTTProtocol_removePublication(m->publish);
	}
	ListEmpty(msgList);
}


void MQTTProtocol_freeMessageList(List* msgList)
{
	MQTTProtocol_emptyMessageList(msgList);
	ListFree(msgList);
}


void MQTTProtocol_freeClient(Clients* client)
{
	MQTTProtocol_removeAllSubscriptions(client->clientID);
	/* free up pending message lists here, and any other allocated data */
	MQTTProtocol_freeMessageList(client->outboundMsgs);
	MQTTProtocol_freeMessageList(client->inboundMsgs);
	MQTTProtocol_freeMessageList(client->queuedMsgs);
	free(client->clientID);
	if (client->will != NULL)
	{
		willMessages* w = client->will;
		free(w->msg);
		free(w->topic);
		free(client->will);
	}
	//free(client);
}


void MQTTProtocol_closeSession(Clients* client, int unclean)
{
	if (in_MQTTPacket_Factory == client->socket)
		return;
	client->good = 0;
	if (client->socket > 0)
	{
		if (client->outbound && client->connected)
		{
			if (client->will)
			{
				Publish pub;
				pub.payload = "0";
				pub.payloadlen = 1;
				pub.topic = client->will->topic;
				MQTTPacket_send_publish(&pub, 0, 0, 1, client->socket);
				MQTTProtocol_sys_publish(client->will->topic, "0");
			}
			MQTTPacket_send_disconnect(client->socket);
		}
		TCPSocket_close(client->socket);
		client->socket = 0;
	}
	if (client->connected || client->connect_state)
	{
		client->connected = 0;
		client->connect_state = 0;
	}
	if (client->outbound == 0 && client->will != NULL && unclean)
	{
		Publish publish;
		publish.payload = client->will->msg;
		publish.payloadlen = strlen(client->will->msg);
		publish.topic = client->will->topic;
		publish.header.qos = client->will->qos;
		publish.header.retain = client->will->retained;
		MQTTProtocol_processPublication(&publish, client->clientID);
	}
	if (client->cleansession)
	{
		if (client->outbound && !(bstate->toStop))
		{ // bridge outbound client structures are reused on reconnection
			MQTTProtocol_emptyMessageList(client->inboundMsgs);
			MQTTProtocol_emptyMessageList(client->outboundMsgs);
			MQTTProtocol_emptyMessageList(client->queuedMsgs);
			client->msgID = 0;
		}
		else
		{
			int saved_socket = client->socket;
			char* saved_clientid = malloc(strlen(client->clientID)+1);
			strcpy(saved_clientid, client->clientID);
			MQTTProtocol_freeClient(client);
			if (!ListRemove(bstate->clients, client))
				Log(LOG_ERROR, Messages_get(36));
			#if !defined(NO_DEBUG)
				else
					Log(LOG_DEBUG, Messages_get(37), saved_clientid, saved_socket);
			#endif
			free(saved_clientid);
		}
	}
	else
		client->socket = 0;
} 


int MQTTProtocol_handleDisconnects(void* pack, int sock)
{
	Clients* client = (Clients*)(ListFindItem(bstate->clients, &sock, clientSocketCompare)->content);

	Log((bstate->connection_messages) ? LOG_INFO : LOG_PROTOCOL, Messages_get(38), client->clientID);
	MQTTProtocol_closeSession(client, 0);
	return TCPSOCKET_COMPLETE;
}


int MQTTProtocol_startOrQueuePublish(Clients* pubclient, Publish* publish, int qos, int retained, Publications** p)
{
	int rc = TCPSOCKET_COMPLETE;

	if (pubclient->connected && pubclient->good && TCPSocket_noPendingWrites(pubclient->socket) &&
		(qos == 0 || pubclient->outboundMsgs->count < bstate->max_inflight_messages))
	{
		if (qos > 0)
		{
			Messages* m;
			int len;
			publish->msgId = MQTTProtocol_assignMsgId(pubclient);
			m = MQTTProtocol_createMessage(publish, p, qos, retained, &len);
			ListAppend(pubclient->outboundMsgs, m, len);
		}
		else if (strlen(publish->topic) < 5 || strncmp(publish->topic, "$SYS/", 5) != 0)
			(++state.msgs_sent);
		//Log(LOG_PROTOCOL, Messages_get(42), clientName, qos);
		if (strlen(publish->topic) < 15 || strncmp(publish->topic, "$SYS/broker/log", 15) != 0)
			Log(LOG_PROTOCOL, Messages_get(42), pubclient->clientID, qos);
		rc = MQTTPacket_send_publish(publish, 0, qos, retained, pubclient->socket);
		if (rc == TCPSOCKET_INTERRUPTED && qos == 0)
		{
			int len;
			// store the publication until the write is finished
			pending_write* pw = malloc(sizeof(pending_write));
			Log(LOG_WARNING, Messages_get(43));
			pw->p = MQTTProtocol_storePublication(publish, &len);
			pw->socket = pubclient->socket;
			ListAppend(&(state.pending_writes), pw, sizeof(pending_write)+len);
		}
	}
	else if (qos > 0)
	{	// queue the message up;
#if !defined(NO_DEBUG)
		//Log(LOG_DEBUG, Messages_get(44), clientName, qos);
		Log(LOG_DEBUG, Messages_get(44), pubclient->clientID, qos);
#endif
		if (pubclient->queuedMsgs->count < bstate->max_queued_messages)
		{
			int len;
			Messages *m = MQTTProtocol_createMessage(publish, p, qos, retained, &len);
			ListAppend(pubclient->queuedMsgs, m, len);
		}
		else
			Log(LOG_WARNING, Messages_get(45), pubclient->clientID);
	}
	return rc;
}


void MQTTProtocol_processRetaineds(Clients* client, char* topic, int qos)
{
	List* rpl = NULL;
	ListElement* currp = NULL;

	rpl = SubscriptionEngines_getRetained(bstate->se, topic);
	while (ListNextElement(rpl, &currp))
	{
		int curqos;
		Publish publish;
		Publications* p = NULL;
		RetainedPublications* rp = (RetainedPublications*)(currp->content);

		publish.payload = rp->payload;
		publish.payloadlen = rp->payloadlen;
		publish.topic = rp->topicName;
		curqos = (rp->qos < qos) ? rp->qos : qos;
		/*if (curqos > 0)
		{
			Messages* m;
			int len;
			//Publications* p = NULL;
			publish.msgId = MQTTProtocol_assignMsgId(client);
			m = MQTTProtocol_createMessage(&publish, &p, curqos, 1, &len);
			ListAppend(client->outboundMsgs, m, len);
		}
		else if (strlen(publish.topic) < 5 || strncmp(publish.topic, "$SYS/", 5) != 0)
			(++state.msgs_sent);
		MQTTPacket_send_publish(&publish, 0, curqos, 1, client->socket);*/
		if (MQTTProtocol_startOrQueuePublish(client, &publish, curqos, 1, &p) == SOCKET_ERROR)
			break;
	}
	ListFreeNoContent(rpl);
}


int MQTTProtocol_handleSubscribes(void* pack, int sock)
{
	int i, *aq;
	Subscribe* subscribe = (Subscribe*)pack;
	ListElement *curtopic = NULL, *curqos = NULL;
	Clients* client = (Clients*)(ListFindItem(bstate->clients, &sock, clientSocketCompare)->content);
	int rc = TCPSOCKET_COMPLETE;

	Log(LOG_PROTOCOL, Messages_get(39), subscribe->msgId, client->clientID, sock);
	aq = malloc(sizeof(int)*(subscribe->noTopics));
	for (i = 0; i < subscribe->noTopics; ++i)
	{
		int j;
		ListElement *duptopic = NULL;
		ListNextElement(subscribe->topics, &curtopic);
		for (j = 0; j < i; ++j)
		{
			char* prevtopic = (char*)(ListNextElement(subscribe->topics, &duptopic)->content);
			if (strcmp(prevtopic, (char*)(curtopic->content)) == 0)
				duptopic->content = curtopic->content;
		}
		aq[i] = SubscriptionEngines_subscribe(bstate->se, client->clientID, 
			(char*)(curtopic->content),
			//(char*)(ListNextElement(subscribe->topics, &curtopic)->content), 
			*(int*)(ListNextElement(subscribe->qoss, &curqos)->content), client->noLocal);
	}
	/* send suback before sending the retained publications because a lot of retained publications could fill up the socket buffer */
	Log(LOG_PROTOCOL, Messages_get(40), subscribe->msgId, client->clientID, sock);
	if ((rc = MQTTPacket_send_suback(subscribe->msgId, subscribe->noTopics, aq, sock)) != SOCKET_ERROR)
	{
		curtopic = curqos = NULL;
		for (i = 0; i < subscribe->noTopics; ++i)
		{
			/* careful if you get >1 subscriptions using the same topic name in the same packet! */
			MQTTProtocol_processRetaineds(client, (char*)(ListNextElement(subscribe->topics, &curtopic)->content),
				*(int*)(ListNextElement(subscribe->qoss, &curqos)->content));
		}
	}
	free(aq);
	MQTTPacket_freeSubscribe(subscribe, 0);
	return rc;
}


int MQTTProtocol_handleUnsubscribes(void* pack, int sock)
{
	int i;
	ListElement* current = NULL;
	Unsubscribe* unsubscribe = (Unsubscribe*)pack;
	char* clientID = ((Clients*)(ListFindItem(bstate->clients, &sock, clientSocketCompare)->content))->clientID;
	int rc = TCPSOCKET_COMPLETE;

	Log(LOG_PROTOCOL, Messages_get(41), unsubscribe->msgId, clientID);
	current = NULL;
	for (i = 0; i < unsubscribe->noTopics; ++i)
		SubscriptionEngines_unsubscribe(bstate->se, clientID, (char*)(ListNextElement(unsubscribe->topics, &current)->content));
	rc = MQTTPacket_send_unsuback(unsubscribe->msgId, sock); /* send response */
	MQTTPacket_freeUnsubscribe(unsubscribe);
	return rc;
}


int messageIDCompare(void* a, void* b)
{
	Messages* msg = (Messages*)a;
	return msg->msgid == *(int*)b;
}


int MQTTProtocol_assignMsgId(Clients* client)
{
	++(client->msgID);
	while (ListFindItem(client->outboundMsgs, &(client->msgID), messageIDCompare) != NULL)
		++(client->msgID);
	if (client->msgID == MAX_MSG_ID)
		client->msgID = 1;
	return client->msgID;
}


Messages* MQTTProtocol_createMessage(Publish* publish, Publications** p, int qos, int retained, int* len)
{
	Messages* m = malloc(sizeof(Messages));
	*len = sizeof(Messages);
	if (*p == NULL)
	{
		int len1;
		*p = MQTTProtocol_storePublication(publish, &len1);
		*len += len1;
	}
	else
		++((*p)->refcount);
	m->msgid = publish->msgId;
	m->publish = *p;
	m->qos = qos;
	m->retain = retained;
	time(&(m->lastTouch));
	if (qos == 2)
		m->nextMessageType = PUBREC;
	return m;
}


void MQTTProtocol_processPublication(Publish* publish, char* originator)
{
	Publications* p = NULL;
	List* clients;
	ListElement* current = NULL;
	int savedMsgId = publish->msgId;

	if (publish->header.retain)
		SubscriptionEngines_setRetained(bstate->se, publish->topic, publish->header.qos, publish->payload, publish->payloadlen);

	clients = SubscriptionEngines_getSubscribers(bstate->se, publish->topic, originator);
	current = NULL;
	while (ListNextElement(clients, &current))
	{
		Clients* pubclient = NULL;
		unsigned int qos = ((Subscriptions*)(current->content))->qos;
		char* clientName = ((Subscriptions*)(current->content))->clientName;
		if (publish->header.qos < qos) /* reduce qos if > subscribed qos */
			qos = publish->header.qos;
		pubclient = (Clients*)(ListFindItem(bstate->clients, clientName, clientIDCompare)->content);
		if (pubclient != NULL)
		{
			int retained = 0;
			char* original_topic = publish->topic;
#if !defined(NO_BRIDGE)
			if (pubclient->outbound || pubclient->noLocal)
			{
				retained = publish->header.retain; // outbound and noLocal mean outward/inward bridge client, so keep retained flag
				if (pubclient->outbound)
					Bridge_handleOutbound(pubclient, publish);
			}
#endif
			if (MQTTProtocol_startOrQueuePublish(pubclient, publish, qos, retained, &p) == SOCKET_ERROR)
			{
				pubclient->good = 0; // make sure we don't try and send messages to ourselves
				Log(LOG_WARNING,  Messages_get(18), pubclient->clientID);
				MQTTProtocol_closeSession(pubclient, 1);
			}
			if (publish->topic != original_topic)
			{
				free(publish->topic);
				publish->topic = original_topic;
			}
		}
	}
	publish->msgId = savedMsgId;
	ListFree(clients);
}



int MQTTProtocol_handlePublishes(void* pack, int sock)
{
	Publish* publish = (Publish*)pack;
	Clients* client = NULL;
	char* clientid = NULL;
	int rc = TCPSOCKET_COMPLETE;

	if (sock == 0)
	  clientid = "$SYS"; // this is an internal client
	else
	{
		client = (Clients*)(ListFindItem(bstate->clients, &sock, clientSocketCompare)->content);
		clientid = client->clientID;
		Log(LOG_PROTOCOL, Messages_get(46), clientid, publish->header.qos);
	}

#if !defined(NO_BRIDGE)
	if (client && client->outbound)
		Bridge_handleInbound(client, publish);
#endif

	if (publish->header.qos == 0)
	{
		MQTTProtocol_processPublication(publish, clientid);
		if (strlen(publish->topic) < 5 || strncmp(publish->topic, "$SYS/", 5) != 0)
			++(state.msgs_received);
	}
	else if (publish->header.qos == 1)
	{	
		Log(LOG_PROTOCOL, Messages_get(47), publish->msgId);
		/* send puback before processing the publications because a lot of return publications could fill up the socket buffer */
		rc = MQTTPacket_send_puback(publish->msgId, sock);  
		MQTTProtocol_processPublication(publish, clientid);
		++(state.msgs_received);
	}
	else if (publish->header.qos == 2)
	{
		/* store publication in inbound list */
		int len;
		Messages* m = malloc(sizeof(Messages));
		Publications* p = MQTTProtocol_storePublication(publish, &len);
		m->publish = p;
		m->msgid = publish->msgId;
		m->qos = publish->header.qos;
		m->retain = publish->header.retain;
		m->nextMessageType = PUBREL;
		ListAppend(client->inboundMsgs, m, sizeof(Messages) + len);
		Log(LOG_PROTOCOL, Messages_get(48), publish->msgId);
		rc = MQTTPacket_send_pubrec(publish->msgId, sock);
	}
	if (sock > 0)
		MQTTPacket_freePublish(publish);
	return rc;
}


Publications* MQTTProtocol_storePublication(Publish* publish, int* len)
{ /* store publication for possible retry */
	Publications* p = malloc(sizeof(Publications));
	p->refcount = 1;

	*len = strlen(publish->topic)+1;
	p->topic = malloc(*len);
	strcpy(p->topic, publish->topic);
	*len += sizeof(Publications);

	p->payloadlen = publish->payloadlen;
	p->payload = malloc(publish->payloadlen);
	memcpy(p->payload, publish->payload, p->payloadlen);
	*len += publish->payloadlen;

	ListAppend(&(state.publications), p, *len);
	return p;
}


void MQTTProtocol_removePublication(Publications* p)
{
	if (--(p->refcount) == 0)
	{
		free(p->payload);
		free(p->topic);
		ListRemove(&(state.publications), p);
	}
}


int MQTTProtocol_handlePubacks(void* pack, int sock)
{
	Puback* puback = (Puback*)pack;
	Clients* client = (Clients*)(ListFindItem(bstate->clients, &sock, clientSocketCompare)->content);
	int rc = TCPSOCKET_COMPLETE;

	Log(LOG_PROTOCOL, Messages_get(49), puback->msgId, client->clientID);

	/* look for the message by message id in the records of outbound messages for this client */
	if (ListFindItem(client->outboundMsgs, &(puback->msgId), messageIDCompare) == NULL)
		Log(LOG_ERROR, Messages_get(50), client->clientID, puback->msgId);
	else
	{
		Messages* m = (Messages*)(client->outboundMsgs->current->content);
		if (m->qos != 1)
			Log(LOG_ERROR, Messages_get(51), client->clientID, puback->msgId, m->qos);
		else
		{
			#if !defined(NO_DEBUG)
				Log(LOG_DEBUG, Messages_get(52), client->clientID, puback->msgId);
			#endif
			MQTTProtocol_removePublication(m->publish);
			ListRemove(client->outboundMsgs, m);
			(++state.msgs_sent);
		}
	}
	free(pack);
	return rc;
}


int MQTTProtocol_handlePubrecs(void* pack, int sock)
{
	Pubrec* pubrec = (Pubrec*)pack;
	Clients* client = (Clients*)(ListFindItem(bstate->clients, &sock, clientSocketCompare)->content);
	int rc = TCPSOCKET_COMPLETE;

	Log(LOG_PROTOCOL, Messages_get(53), pubrec->msgId, client->clientID);

	/* look for the message by message id in the records of outbound messages for this client */
	client->outboundMsgs->current = NULL;
	if (ListFindItem(client->outboundMsgs, &(pubrec->msgId), messageIDCompare) == NULL)
	{
		if (pubrec->header.dup == 0)
			Log(LOG_ERROR, Messages_get(54), client->clientID, pubrec->msgId);
	}
	else
	{
		Messages* m = (Messages*)(client->outboundMsgs->current->content);
		if (m->qos != 2)
		{
			if (pubrec->header.dup == 0)
				Log(LOG_ERROR, Messages_get(55), client->clientID, pubrec->msgId, m->qos);
		}
		else if (m->nextMessageType != PUBREC)
		{
			if (pubrec->header.dup == 0)
				Log(LOG_ERROR, Messages_get(56), client->clientID, pubrec->msgId);
		}
		else
		{
			Log(LOG_PROTOCOL, Messages_get(57), client->clientID, pubrec->msgId);
			rc = MQTTPacket_send_pubrel(pubrec->msgId, 0, sock);
			m->nextMessageType = PUBCOMP;
			time(&(m->lastTouch));
		}
	}
	free(pack);
	return rc;
}


int MQTTProtocol_handlePubrels(void* pack, int sock)
{ 
	Pubrel* pubrel = (Pubrel*)pack;
	Clients* client = (Clients*)(ListFindItem(bstate->clients, &sock, clientSocketCompare)->content);
	int rc = TCPSOCKET_COMPLETE;

	Log(LOG_PROTOCOL, Messages_get(58), pubrel->msgId, client->clientID);

	/* look for the message by message id in the records of inbound messages for this client */
	if (ListFindItem(client->inboundMsgs, &(pubrel->msgId), messageIDCompare) == NULL)
	{
		if (pubrel->header.dup == 0)
			Log(LOG_ERROR, Messages_get(59), client->clientID, pubrel->msgId);
		else
		{
			// Apparently this is "normal" behaviour, so we don't need to issue a warning
			//Log(LOG_WARNING, "Duplicate PUBREL received from client %s for msg id %d, but no record of that msgid found.  Sending PUBCOMP anyway to clean up.\n", 
			//						client->clientID, pubrel->msgId);
			rc = MQTTPacket_send_pubcomp(pubrel->msgId, sock);
		}
	}
	else
	{
		Messages* m = (Messages*)(client->inboundMsgs->current->content);
		if (m->qos != 2)
			Log(LOG_ERROR, Messages_get(60), client->clientID, pubrel->msgId, m->qos);
		else if (m->nextMessageType != PUBREL)
			Log(LOG_ERROR, Messages_get(61), client->clientID, pubrel->msgId);
		else
		{ 
			Publish publish;

			/* send pubcomp before processing the publications because a lot of return publications could fill up the socket buffer */
			Log(LOG_PROTOCOL, Messages_get(62), client->clientID, pubrel->msgId);
			rc = MQTTPacket_send_pubcomp(pubrel->msgId, sock);
			publish.header.qos = m->qos;
			publish.header.retain = m->retain;
			publish.msgId = m->msgid;
			publish.topic = m->publish->topic;
			publish.payload = m->publish->payload;
			publish.payloadlen = m->publish->payloadlen;
			MQTTProtocol_processPublication(&publish, client->clientID);
			MQTTProtocol_removePublication(m->publish);
			ListRemove(client->inboundMsgs, m);
			++(state.msgs_received);
		}
	}
	free(pack);
	return rc;
}


int MQTTProtocol_handlePubcomps(void* pack, int sock)
{
	Pubcomp* pubcomp = (Pubcomp*)pack;
	Clients* client = (Clients*)(ListFindItem(bstate->clients, &sock, clientSocketCompare)->content);
	int rc = TCPSOCKET_COMPLETE;

	Log(LOG_PROTOCOL, Messages_get(63), pubcomp->msgId, client->clientID);

	/* look for the message by message id in the records of outbound messages for this client */
	if (ListFindItem(client->outboundMsgs, &(pubcomp->msgId), messageIDCompare) == NULL)
	{
		if (pubcomp->header.dup == 0)
			Log(LOG_ERROR, Messages_get(64), client->clientID, pubcomp->msgId);
	}
	else
	{
		Messages* m = (Messages*)(client->outboundMsgs->current->content);
		if (m->qos != 2)
			Log(LOG_ERROR, Messages_get(65), client->clientID, pubcomp->msgId, m->qos);
		else
		{
			if (m->nextMessageType != PUBCOMP)
				Log(LOG_ERROR, Messages_get(66), client->clientID, pubcomp->msgId);
			else
			{
				#if !defined(NO_DEBUG)
					Log(LOG_DEBUG, Messages_get(67), client->clientID, pubcomp->msgId);
				#endif
				MQTTProtocol_removePublication(m->publish);
				ListRemove(client->outboundMsgs, m);
				(++state.msgs_sent);
			}
		}
	}
	free(pack);
	return rc;
}

