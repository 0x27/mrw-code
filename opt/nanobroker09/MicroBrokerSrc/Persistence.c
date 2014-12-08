
#include "Persistence.h"
#include "Log.h"
#include "NanoBroker.h"
#include "TCPSocket.h"
#include "SubsEngine.h"
#include "Bridge.h"
#include "MQTTProtocol.h"
#include "Messages.h"

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <memory.h>
#include <stdlib.h>

#include "Heap.h"

typedef struct
{
	char* name;
	int type; // 0 = int, 1 = string, 2 = boolean, 3 = list of strings
	int location;
} property;

#if !defined(NO_BRIDGE)
#define broker_props_count 11
#else
#define broker_props_count 10
#endif

property brokerProps[broker_props_count] =
{
	{ "trace_level", 0, offsetof(BrokerStates, trace_level) },
	{ "connection_messages", 2, offsetof(BrokerStates, connection_messages) },
	{ "port", 0, offsetof(BrokerStates, port) },
	{ "bind_address", 1, offsetof(BrokerStates, bind_address) },
	{ "max_inflight_messages", 0, offsetof(BrokerStates, max_inflight_messages) },
	{ "max_queued_messages", 0, offsetof(BrokerStates, max_queued_messages) },
	{ "retry_interval", 0, offsetof(BrokerStates, retry_interval) } ,
	{ "persistence_location", 1, offsetof(BrokerStates, persistence_location) },
	{ "retained_persistence", 2, offsetof(BrokerStates, retained_persistence) },
	{ "clientid_prefixes", 3, offsetof(BrokerStates, clientid_prefixes) },
#if !defined(NO_BRIDGE)
	{ "connection", 1, offsetof(BridgeConnections, name) },
#endif
};

#if !defined(NO_BRIDGE)

#define bridge_props_count 7

property bridgeProps[bridge_props_count] =
{
	{ "notifications", 2, offsetof(BridgeConnections, notifications) },
	{ "round_robin", 2, offsetof(BridgeConnections, round_robin) },
	{ "try_nano", 2, offsetof(BridgeConnections, try_nano) },
	//{ "persistent", 2, offsetof(BridgeConnections, persistent) },
	{ "addresses", 3, offsetof(BridgeConnections, addresses) },
	{ "topic", 1, offsetof(BridgeConnections, topics) },
	{ "connection", 1, offsetof(BridgeConnections, name) },
	{ "notification_topic", 1, offsetof(BridgeConnections, notification_topic) },
};

#endif


#if defined(WIN32)
#define strtok_r strtok_s
#else
#define _unlink unlink
#endif

char* delims = " \t\r\n";

static BrokerStates* bstate;


int Persistence_read_config(char* filename, BrokerStates* bs, int config_set)
{
	int success = 0;
	FILE* cfile = NULL;
	property* propsTable = brokerProps;
	int props_count = broker_props_count;
	void* s = bs;

	bs->clientid_prefixes = ListInitialize();
	bstate = bs;
	if ((cfile = fopen(filename, "r")) == NULL)
	{
		if (config_set)
			Log(LOG_ERROR, Messages_get(0), filename);
	}
	else
	{
		char curline[120], *curpos;
		int line_no = 0;
		while (fgets(curline, sizeof(curline), cfile))
		{
			char *pword = strtok_r(curline, delims, &curpos);

			line_no++;
			if (pword && pword[0] != '\0' && pword[0] != '#')
			{
				int i = 0;
				for (i = 0; i < props_count; ++i)
					if (strcmp(pword, propsTable[i].name) == 0)
					{
						char* val = strtok_r(NULL, delims, &curpos);
						if (!val)
							Log(LOG_ERROR, Messages_get(1), pword, line_no);
						else
						{
#if !defined(NO_BRIDGE)
							if (strcmp(pword, "connection") == 0)
							{
								propsTable = bridgeProps;
								props_count = bridge_props_count;
								s = Bridge_new_connection(val);
								if (bs->bridge.connections == NULL)
									bs->bridge.connections = ListInitialize();
								ListAppend(bs->bridge.connections, s, 0);
								//printf("Adding connection %s %d\n", val, bs->bridge.connections->count);
							}
							else if (strcmp(pword, "topic") == 0)
							{
								/* List of topic structures: pattern, direction, localPrefix, remotePrefix */
								int ok = 0;
								BridgeTopics* t = malloc(sizeof(BridgeTopics));
								t->localPrefix = t->remotePrefix = NULL;
								if (strcmp(val, "\"\"") == 0)
								{
									t->pattern = malloc(1);
									t->pattern[0] = '\0';
								}
								else
								{
									t->pattern = malloc(strlen(val)+1);
									strcpy(t->pattern, val);
								}
								ok = 1;
								t->direction = 2; // default - out
								if (val = strtok_r(NULL, delims, &curpos))
								{
									if (strcmp(val, "both") == 0)
										t->direction = 0;
									else if (strcmp(val, "in") == 0)
										t->direction = 1;
									else if (strcmp(val, "out") == 0)
										t->direction = 2;
									else
										Log(LOG_WARNING, Messages_get(2), val);
									if (val = strtok_r(NULL, delims, &curpos))
									{
										if (strcmp(val, "\"\"") != 0)
										{
											t->localPrefix = malloc(strlen(val)+1);
											strcpy(t->localPrefix, val);
										}
										if (val = strtok_r(NULL, delims, &curpos))
										{
											t->remotePrefix = malloc(strlen(val)+1);
											strcpy(t->remotePrefix, val);
										}
									}
								}
								if (ok)
									ListAppend(((BridgeConnections*)s)->topics, t, sizeof(BridgeTopics));
							}
							else
#endif
								if (propsTable[i].type == 1)
								{
									char** loc = (char**)&((char*)s)[propsTable[i].location];
									*loc = malloc(strlen(val)+1);
									strcpy(*loc, val);
									Log(LOG_INFO, Messages_get(4), pword, val);
								}
								else if (propsTable[i].type == 2)
								{ /* boolean */
									int ival = -1;
									if (strcmp(val, "true") == 0)
										ival = 1;
									else if (strcmp(val, "false") == 0)
										ival = 0;
									else
										Log(LOG_WARNING, Messages_get(4), val, line_no);
									if (ival != -1)
									{
										Log(LOG_INFO, Messages_get(5), pword, val);
										memcpy(&(((char*)s)[propsTable[i].location]), &ival, sizeof(int));
									}
								}
								else if (propsTable[i].type == 3)
								{
									List** list = (List**)&((char*)s)[propsTable[i].location];
									/* list of strings */
									while (val)
									{
										int len = strlen(val)+1;
										char *loc = malloc(len);
										strcpy(loc, val);
										ListAppend(*list, loc, len);
										Log(LOG_INFO, Messages_get(6), val, pword);
										val = strtok_r(NULL, delims, &curpos);
									}
								}
								else
								{
									int ival = atoi(val);
									Log(LOG_INFO, Messages_get(7), pword, ival);
									memcpy(&(((char*)s)[propsTable[i].location]), &ival, sizeof(int));
								}
						}
						break;
					}
					if (i == props_count)
						Log(LOG_WARNING, Messages_get(8), pword, line_no);
			}
		}
		fclose(cfile);
		success = 1;
	}
	return success;
}


void Persistence_free_config(BrokerStates* bs)
{
	ListElement* current = NULL;

	if (bs->persistence_location)
		free(bs->persistence_location);
	if (bs->bind_address)
		free(bs->bind_address);
	ListFree(bs->clientid_prefixes);

#if !defined(NO_BRIDGE)
	if (bs->bridge.connections == NULL)
		return;

	while (ListNextElement(bs->bridge.connections, &current))
	{
		ListElement* curtopic = NULL;

		BridgeConnections* cur_connection = (BridgeConnections*)(current->content);
		free(cur_connection->name);
		ListFree(cur_connection->addresses);
		while (ListNextElement(cur_connection->topics, &curtopic))
		{
			BridgeTopics* topic = (BridgeTopics*)(curtopic->content);
			free(topic->pattern);
			if (topic->localPrefix)
				free(topic->localPrefix);
			if (topic->remotePrefix)
				free(topic->remotePrefix);
		}
		ListFree(cur_connection->topics);
		if (cur_connection->notification_topic)
			free(cur_connection->notification_topic);
	}
	ListFree(bs->bridge.connections);
#endif
}


char* retained_fn = "nanobroker.rms";
char* retained_backup_fn = "nanobroker.1ms";
char* retained_backup_fn1 = "nanobroker.2ms";
static FILE* rfile = NULL;


char* add_prefix(char* fn)
{
	char* rc = fn;

	if (bstate->persistence_location)
	{
		rc = malloc(strlen(fn) + strlen(bstate->persistence_location)+1);
		strcpy(rc, bstate->persistence_location);
		strcat(rc, fn);
	}
	return rc;
}


void free_prefix(char* rc, char* fn)
{
	if (rc != fn)
		free(rc);
}


FILE* Persistence_open_retained(char mode)
{
	if (!bstate->retained_persistence)
		rfile = NULL;
	else if (mode == 'w')
	{
		char* loc = add_prefix(retained_fn);
		char* bak = add_prefix(retained_backup_fn);
		char* bak1 = add_prefix(retained_backup_fn1);

		_unlink(bak1);
		rename(bak, bak1);
		_unlink(bak);

		/* copy existing file to a backup copy */
		rename(loc, bak);
		_unlink(bak1);

		/* open new file */
		if ((rfile = fopen(loc, "wb")) == NULL)
			Log(LOG_ERROR, Messages_get(9), loc);
		free_prefix(loc, retained_fn);
		free_prefix(bak, retained_backup_fn);
		free_prefix(bak1, retained_backup_fn1);
	}
	else if (mode == 'r')
	{
		char* loc = add_prefix(retained_fn);
		if ((rfile = fopen(loc, "rb")) == NULL)
			Log(LOG_WARNING, Messages_get(10));
		else
			Log(LOG_INFO, Messages_get(11), loc);
		free_prefix(loc, retained_fn);
	}
	return rfile;
}


void Persistence_write_retained(char* payload, int payloadlen, int qos, char* topicName)
{
	if (rfile)
	{
		int topiclen = strlen(topicName);
		fwrite(&payloadlen, sizeof(int), 1, rfile);
		fwrite(payload, payloadlen, 1, rfile);
		fwrite(&qos, sizeof(int), 1, rfile);
		fwrite(&topiclen, sizeof(int), 1, rfile);
		fwrite(topicName, topiclen, 1, rfile);
	}
}


RetainedPublications* Persistence_read_retained()
{
	if (rfile == NULL)
		return NULL;
	else
	{
		int topiclen;
		int success = 0;
		RetainedPublications* r = malloc(sizeof(RetainedPublications));

		if (fread(&(r->payloadlen), sizeof(int), 1, rfile) == 1)
		{
			r->payload = malloc(r->payloadlen);
			if (fread(r->payload, r->payloadlen, 1, rfile) == 1 &&
				fread(&(r->qos), sizeof(int), 1, rfile) == 1 &&
				fread(&topiclen, sizeof(int), 1, rfile) == 1)
			{
				r->topicName = malloc(topiclen + 1);
				if (fread(r->topicName, topiclen, 1, rfile) == 1)
				{
					r->topicName[topiclen] = '\0';
					success = 1;
				}
				else
				{
					free(r->topicName);
					free(r->payload);
				}
			}
			else
				free(r->payload);
		}
		if (!success)
		{
			free(r);
			r = NULL;
		}
		return r;
	}
}


void Persistence_close_retained()
{
	if (rfile)
		fclose(rfile);
}


