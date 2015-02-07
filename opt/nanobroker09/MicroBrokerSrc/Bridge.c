#if !defined(NO_BRIDGE)

#include "Bridge.h"
#include "Log.h"
#include "MQTTProtocol.h"
#include "Topics.h"
#include "Messages.h"

#include <string.h>
#include <stdio.h>

#include "Heap.h"

void Bridge_processConnection(BridgeConnections* bc);

static Bridges* bridge = NULL;
static SubscriptionEngines* subsengine = NULL;

void Bridge_initialize(Bridges* br, SubscriptionEngines* se)
{
	bridge = br;
	subsengine = se;
}


BridgeConnections* Bridge_new_connection(char* name)
{
	BridgeConnections* bc = malloc(sizeof(BridgeConnections));

	bc->name = malloc(strlen(name)+1);
	strcpy(bc->name, name);
	bc->notifications = bc->try_nano = 1;
	//bc->persistent = bc->round_robin = 0;
	bc->no_successful_connections = bc->round_robin = 0;
	bc->topics = ListInitialize();
	bc->addresses = ListInitialize();
	bc->primary = bc->backup = NULL;
	bc->notification_topic = NULL;
	return bc;
}


void Bridge_terminate(Bridges* br)
{
	
}


void Bridge_timeslice(Bridges* bridge)
{
	ListElement* curelement = NULL;

	if (bridge->connections != NULL)
	{
		while (ListNextElement(bridge->connections, &curelement))
			Bridge_processConnection( (BridgeConnections*)(curelement->content));
	}
}


void Bridge_newConnection(BridgeConnections* bc, Clients** pclient)
{
	char* clientid = NULL;

	clientid = malloc(MAX_CLIENTID_LEN+1);
	strcpy(clientid, TCPSocket_gethostname());
	if (pclient == &(bc->backup))
	{
		Log(LOG_INFO, Messages_get(123), bc->name);
		strncat(clientid, "\\", MAX_CLIENTID_LEN-strlen(clientid));
		bc->cur_address = bc->addresses->first->next;
	}
	else
	{	
		Log(LOG_INFO, Messages_get(124), bc->name);
		strncat(clientid, ".", MAX_CLIENTID_LEN-strlen(clientid));
		bc->cur_address = bc->addresses->first;
	}
	strncat(clientid, bc->name, MAX_CLIENTID_LEN-strlen(clientid));
	bc->last_connect_succeeded = 0;

	*pclient = MQTTProtocol_connect((char*)(bc->cur_address->content), clientid, (bc->addresses->count > 1), bc->try_nano);
	if (bc->primary == NULL)
	{
		free(clientid);
		Log(LOG_ERROR, Messages_get(125));
		return;
	}
	(*pclient)->bridge_context = bc;
	if (bc->notifications) 		// if notifications are on, set the will message
	{
		char* msg = "0"; // 0 means disconnected
		char* topic = "$SYS/broker/connection/%s/state"; 

		if (bc->notification_topic)
			topic = bc->notification_topic;
		(*pclient)->will = (willMessages*)malloc(sizeof(willMessages));
		(*pclient)->will->msg = malloc(strlen(msg)+1);
		strcpy((*pclient)->will->msg, msg);
		if (strstr(topic, "%s"))
		{
			(*pclient)->will->topic = malloc(strlen(topic)+strlen(clientid)+1);
			sprintf((*pclient)->will->topic, topic, clientid);
		}
		else
		{
			(*pclient)->will->topic = malloc(strlen(topic)+1);
			strcpy((*pclient)->will->topic, topic);
		}
		(*pclient)->will->retained = 0;
		(*pclient)->will->qos = 0;
	}
}


void Bridge_processPartConnection(BridgeConnections* bc, Clients** pclient)
{
	if (*pclient)
	{
		#if !defined(NO_DEBUG)
			Log(LOG_DEBUG, Messages_get(126), bc->name, 
				(*pclient)->connected, (*pclient)->connect_state);
		#endif
		if ((*pclient)->connected == 0 && (*pclient)->connect_state == 0)
		{	
			time_t now;
			time(&(now));
			if (bc->last_connect_succeeded == 0 && (*pclient)->noLocal)
			{
				(*pclient)->noLocal = bc->last_connect_succeeded = 0;
				if (bc->round_robin == 0 && pclient == &(bc->primary))
					MQTTProtocol_reconnect((char*)(bc->addresses->first->content), (*pclient));
				else
					MQTTProtocol_reconnect((char*)(bc->cur_address->content), (*pclient)); 
				(*pclient)->lastContact = now;
			}
			else if (difftime(now, (*pclient)->lastContact) > 20)
			{
				ListElement* addr = bc->cur_address;
				if (bc->try_nano && (*pclient)->noLocal == 0)
					(*pclient)->noLocal = 1;
				bc->last_connect_succeeded = 0;
				if (bc->round_robin)
				{
					if (ListNextElement(bc->addresses, &(bc->cur_address)) == NULL)
						addr = bc->cur_address = bc->addresses->first;
				}
				else if (pclient == &(bc->backup) && bc->addresses->count > 2)
				{
					if (ListNextElement(bc->addresses, &(bc->cur_address)) == NULL)
						addr = bc->cur_address = bc->addresses->first->next;
				}
				else if (pclient == &(bc->primary))
					addr = bc->addresses->first;
				Log(LOG_INFO, Messages_get(127), bc->name, (char*)(addr->content));
				MQTTProtocol_reconnect((char*)(addr->content), (*pclient)); 
				(*pclient)->lastContact = now;
			}
		}
		else if ((*pclient)->connected == 0 && (*pclient)->connect_state == 1)
		{
			time_t now;
			time(&(now));
			if (difftime(now, (*pclient)->lastContact) > 30)
			{
				Log(LOG_INFO, Messages_get(128), bc->name);
				MQTTProtocol_closeSession((*pclient), 0);
				(*pclient)->lastContact = now;
			}
		}
	}

	if (*pclient == NULL && bc->addresses->first != NULL)
	{ // create an outbound connection
		Bridge_newConnection(bc, pclient);
	}
}


void Bridge_processConnection(BridgeConnections* bc)
{
	Bridge_processPartConnection(bc, &(bc->primary));

	if (bc->primary->connected == 0 && bc->primary->connect_state == 0 && bc->primary->noLocal == 0 &&
			bc->round_robin == 0 && bc->addresses->count > 1)
		Bridge_processPartConnection(bc, &(bc->backup));
}


void Bridge_handleConnection(Clients* client)
{
	int error, 
			len = sizeof(error);
	ListElement* elem = NULL;
	BridgeConnections* bc = NULL;

	while (ListNextElement(bridge->connections, &elem))
	{
		bc = (BridgeConnections*)elem->content;
		if (client == bc->primary || client == bc->backup)
		  break;
		bc = NULL;
	}

	// check the socket state, if it's ok then we can send the connect packet
	if (getsockopt(client->socket, SOL_SOCKET, SO_ERROR, (char*)&error, &len) != 0)
	{
		Log(LOG_ERROR, Messages_get(129));
		MQTTProtocol_closeSession(client, 0);
	}
	else if (error != 0)
	{
		Log(LOG_ERROR, Messages_get(130), client->clientID, bc->cur_address->content, error);
		MQTTProtocol_closeSession(client, 0);
	}
	else
	{
		Log(LOG_PROTOCOL, Messages_get(131), client->clientID, client->socket, client->noLocal);
		MQTTPacket_send_connect(client);
		client->connect_state = 2;
		time(&(client->lastContact));
	}
}


char* Bridge_addPrefix(char* pattern, char* prefix, int* newlen)
{	
	char* newtopic;

	*newlen = strlen(pattern) + 1;
	if (prefix)
		*newlen += strlen(prefix);
	newtopic = malloc(*newlen);
	if (prefix)
	{
		strcpy(newtopic, prefix);
		strcat(newtopic, pattern);
	}
	else
		strcpy(newtopic, pattern);
	return newtopic;
}


int Bridge_handleConnacks(void* pack, int sock)
{
	Connack* connack = (Connack*)pack;
	ListElement* elem = NULL;
	BridgeConnections* bc = NULL;
	Clients* client = NULL;

	while (ListNextElement(bridge->connections, &elem))
	{
		bc = (BridgeConnections*)elem->content;
		if (bc->primary && bc->primary->socket == sock)
		{ 
			client = bc->primary;
			break;
		}
		if (bc->backup && bc->backup->socket == sock)
		{
			client = bc->backup;
			break;
		}
		bc = NULL;
	}

	Log(LOG_PROTOCOL, Messages_get(132), connack->rc, client->clientID);

	if (connack->rc != 0)
	{
		if (client)
		{
			if (connack->rc == 1 && client->noLocal == 1)
				client->noLocal = 0;
			else
				Log(LOG_WARNING, Messages_get(132), connack->rc, client->clientID);
			MQTTProtocol_closeSession(client, 0);
		}
		else
			TCPSocket_close(sock);
	} 
	else if (client) // now we can issue the subscriptions, both local and remote
	{
		ListElement* addr = bc->cur_address;

		if (client == bc->primary && bc->round_robin == 0)
			addr = bc->addresses->first;
		Log(LOG_INFO, Messages_get(133), bc->name, (char*)(addr->content));
		if (client->will)
		{
			Publish pub;
			MQTTProtocol_sys_publish(client->will->topic, "1");
			pub.payload = "1";
			pub.payloadlen = 1;
			pub.topic = client->will->topic;
			MQTTPacket_send_publish(&pub, 0, 0, 1, client->socket);
		}

		client->connect_state = 3;  //should have been 2 before
		bc->last_connect_succeeded = 1;
		(bc->no_successful_connections)++;
		client->connected = 1;
		client->good = 1;
		time(&(client->lastContact));

		if (client == bc->primary && 
			(bc->backup && (bc->backup->connected == 1 || bc->backup->connect_state != 0)))
		{
			Log(LOG_INFO, Messages_get(134), (char*)(bc->cur_address->content));
			MQTTProtocol_closeSession(bc->backup, 0);
		}

		if (bc->addresses->count > 1 || bc->no_successful_connections == 1)
		{
			List* topics = ListInitialize();
			List* qoss = ListInitialize();
			elem = NULL;
			while (ListNextElement(bc->topics, &elem))
			{
				int len;
				BridgeTopics* curtopic = (BridgeTopics*)(elem->content);

				if (curtopic->direction == 0 || curtopic->direction == 1)              // both=0, in=1, out=2
				{	// remote subscription
					int* qos;
					char* fulltopic = Bridge_addPrefix(curtopic->pattern, curtopic->remotePrefix, &len);
					ListAppend(topics, fulltopic, len);
					qos = malloc(sizeof(int));
					*qos = 2;
					ListAppend(qoss, qos, sizeof(int));
				}
				if (curtopic->direction == 0 || curtopic->direction == 2)
					SubscriptionEngines_subscribe(subsengine, client->clientID,					 // local subscription
					Bridge_addPrefix(curtopic->pattern, curtopic->localPrefix, &len), 2, 1); // this is noLocal (and keep retained flags)
			}
			if (topics->count > 0)
				MQTTProtocol_subscribe(client, topics, qoss);
			ListFree(topics);
			ListFree(qoss);
		}
	}
	free(connack);
	return TCPSOCKET_COMPLETE;
}


BridgeTopics* Bridge_stripAndReplace(BridgeConnections* bc, Publish* publish, int outbound)
{
	ListElement* elem = NULL;
	BridgeTopics* match = NULL;
	char* prefix = NULL;

	while (ListNextElement(bc->topics, &elem))
	{
		char* source;
		int len;

		BridgeTopics* bt = (BridgeTopics*)elem->content;
		if (outbound == 1 && bt->direction == 1)
			continue;
		if (outbound == 0 && bt->direction == 2)
			continue;

		prefix = outbound ? bt->localPrefix : bt->remotePrefix;
		if (prefix)
			source = Bridge_addPrefix(bt->pattern, prefix, &len);
		else
			source = bt->pattern;

		if (Topics_matches(source, publish->topic))
		{
			if (match)
			{
				if (SubscriptionEngines_mostSpecific(match->pattern, source) == source)
					match = bt;
			}
			else
				match = bt;
		}
		if (prefix)
			free(source);
	}

	if (match)
	{
		int matchSourcePrefixLen, matchDestPrefixLen;
		char* newtopic;
		char* matchSource = outbound ? match->localPrefix : match->remotePrefix;
		char* matchDest = outbound ? match->remotePrefix : match->localPrefix;

		matchSourcePrefixLen = (matchSource) ? strlen(matchSource) : 0;
		matchDestPrefixLen = (matchDest) ? strlen(matchDest) : 0;

		newtopic = malloc(strlen(publish->topic) - matchSourcePrefixLen + matchDestPrefixLen + 1);
		if (matchDest)
			strcpy(newtopic, matchDest); // add destination prefix
		else
			newtopic[0] = '\0';

		if (prefix)
			strcat(newtopic, &(publish->topic)[matchSourcePrefixLen]); // strip source prefix
		else
			strcat(newtopic, publish->topic);

		if (!outbound)
			free(publish->topic);
		publish->topic = newtopic;
	}

	return match;
}


void Bridge_handleInbound(Clients* client, Publish* publish)
{ // transform the publish packet as per bridge function
	BridgeConnections* bc = (BridgeConnections*)client->bridge_context;

	if (bc->inbound_filter)
	{
		// send this to a system topic for an application to pre-process
		
	}

	if (Bridge_stripAndReplace(bc, publish, 0) == NULL)
		Log(LOG_ERROR, Messages_get(135), publish->topic, bc->name);
}


void Bridge_handleOutbound(Clients* client, Publish* publish)
{
	BridgeConnections* bc = (BridgeConnections*)client->bridge_context;

#if 0
	if (bc->outbound_filter)
	{
		// send this to a system topic for an application to pre-process
	}
#endif

	if (Bridge_stripAndReplace(bc, publish, 1) == NULL)
		Log(LOG_ERROR, Messages_get(136), publish->topic, bc->name);
}

#endif
