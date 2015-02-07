/* (C) Copyright IBM Corp. 2007, 2008
         Author: Ian Craggs
*/

#if !defined(SUBSENGINE_H)
#define SUBSENGINE_H

#include "LinkedList.h"

static const char* sysprefix = "$SYS/";

typedef struct
{
	char* clientName;
	char* topicName;
	int qos;
	int noLocal;
	char* localPrefix;
	char* remotePrefix;
} Subscriptions;

typedef struct
{
	char* topicName;
	int qos;
	char* payload;
	unsigned int payloadlen;
} RetainedPublications;

Subscriptions* Subscriptions_initialize(char*, char*, int, int);

typedef struct
{
	List* subs;
	List* retaineds;
	struct
	{
		List* subs; /* system topics */
		List* retaineds;
	} system;
} SubscriptionEngines;

SubscriptionEngines* SubscriptionEngines_initialize();
void SubscriptionEngines_saveRetained(SubscriptionEngines* se);
void SubscriptionEngines_terminate(SubscriptionEngines* se);

int SubscriptionEngines_subscribe(SubscriptionEngines*, char*, char*, int, int);
void SubscriptionEngines_unsubscribe(SubscriptionEngines*, char*, char*);
char* SubscriptionEngines_mostSpecific(char* topicA, char* topicB);
List* SubscriptionEngines_getSubscribers(SubscriptionEngines*, char* topic, char* clientID);

void SubscriptionEngines_setRetained(SubscriptionEngines* se, char* topicName, int qos, char* payload, unsigned int payloadlen);
List* SubscriptionEngines_getRetained(SubscriptionEngines* se, char* topicName);

#endif
