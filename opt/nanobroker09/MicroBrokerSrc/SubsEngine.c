/* (C) Copyright IBM Corp. 2007, 2008
         Author: Ian Craggs
*/

#include "SubsEngine.h"
#include "Topics.h"
#include "Log.h"
#include "Persistence.h"
#include "Messages.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Heap.h"

Subscriptions* Subscriptions_initialize(char* clientid, char* topic, int qos, int noLocal)
{
  Subscriptions* news = malloc(sizeof(Subscriptions));
	news->clientName = clientid;
	news->topicName = topic;
	news->qos = qos;
	news->noLocal = noLocal;
	return news;
}


SubscriptionEngines* SubscriptionEngines_initialize()
{
	RetainedPublications* r;

	SubscriptionEngines* newse = malloc(sizeof(SubscriptionEngines));
	newse->subs = ListInitialize();
	newse->retaineds = ListInitialize();
	newse->system.subs = ListInitialize();
	newse->system.retaineds = ListInitialize();

	if (Persistence_open_retained('r'))
	{
		while (r = Persistence_read_retained())
		{
			if (strncmp(r->topicName, sysprefix, strlen(sysprefix)) == 0)
				ListAppend(newse->system.retaineds, r, sizeof(SubscriptionEngines)+4*sizeof(List));
			else
				ListAppend(newse->retaineds, r, sizeof(SubscriptionEngines)+4*sizeof(List));
		}
		Persistence_close_retained();
	}

	return newse;
}


void saveRetaineds(List* retaineds, int free)
{
	ListElement *current = NULL;

	while (ListNextElement(retaineds, &current))
	{
		/* also, write retained messages to persistence */
		RetainedPublications* r = current->content;
		Persistence_write_retained(r->payload, r->payloadlen, r->qos, r->topicName);
		if (free)
		{
			free(r->topicName);
			free(r->payload);
		}
  }
	if (free)
		ListFree(retaineds);
}


void SubscriptionEngines_saveRetained(SubscriptionEngines* se)
{
	Persistence_open_retained('w');
	saveRetaineds(se->retaineds, 0);
	saveRetaineds(se->system.retaineds, 0);
	Persistence_close_retained();
}


void SubscriptionEngines_terminate(SubscriptionEngines* se)
{
	Persistence_open_retained('w');
	saveRetaineds(se->retaineds, 1);
	ListFree(se->subs);
	saveRetaineds(se->system.retaineds, 1);
	Persistence_close_retained();

	ListFree(se->system.subs);
	free(se);
}



int SubscriptionEngines_subscribe1(List* sl, char* aClientid, char* aTopic, int qos, int noLocal)
{
	ListElement *current = NULL;
	while (ListNextElement(sl, &current))
	{
		Subscriptions* s = current->content;
		if (strcmp(s->clientName, aClientid) == 0 && strcmp(s->topicName, aTopic) == 0)
		{
			#if !defined(NO_DEBUG)
				Log(LOG_DEBUG, Messages_get(115), aClientid, aTopic, qos);
			#endif
			free(s->topicName); // make sure we free the old topic name, even though it is the same
			s->topicName = aTopic; // point to the new (same value) one
			s->qos = qos;
			s->noLocal = noLocal;
			break;
		}
	}
	if (current == NULL)
	{
		#if !defined(NO_DEBUG)
			Log(LOG_DEBUG, Messages_get(116), aClientid, aTopic, qos);
		#endif
		ListAppend(sl, Subscriptions_initialize(aClientid, aTopic, qos, noLocal), sizeof(Subscriptions));
	}
	return qos;
}


int SubscriptionEngines_subscribe(SubscriptionEngines* se, char* aClientid, char* aTopic, int qos, int noLocal)
{
	if (strncmp(aTopic, sysprefix, strlen(sysprefix)) == 0)
		return SubscriptionEngines_subscribe1(se->system.subs, aClientid, aTopic, qos, noLocal);
	else
		return SubscriptionEngines_subscribe1(se->subs, aClientid, aTopic, qos, noLocal);
}


void SubscriptionEngines_unsubscribe1(List* sl, char* aClientid, char* aTopic, char* wildcard)
{
	ListElement* current = NULL;

	ListNextElement(sl, &current);
	while (current)
	{
		Subscriptions* s = current->content;
		ListNextElement(sl, &current);
		if (strcmp(s->clientName, aClientid) == 0 &&
			(strcmp(s->topicName, aTopic) == 0 || strcmp(aTopic, wildcard) == 0))
		{
			/* clientid is the same string held in the main client list, so it will be freed separately */
			#if !defined(NO_DEBUG)
				Log(LOG_DEBUG,  Messages_get(117), s->clientName, s->topicName, s->qos);
			#endif
			free(s->topicName);
			if (ListRemove(sl, s) == 0)
				Log(LOG_ERROR,  Messages_get(118));
			if (strcmp(aTopic, MULTI_LEVEL_WILDCARD) != 0) /* wildcard removes all subscriptions */
				break;	
		}
	}
}


void SubscriptionEngines_unsubscribe(SubscriptionEngines* se, char* aClientid, char* aTopic)
{
	if (strncmp(aTopic, sysprefix, strlen(sysprefix)) == 0)
		SubscriptionEngines_unsubscribe1(se->system.subs, aClientid, aTopic, "$SYS/#");
	else
		SubscriptionEngines_unsubscribe1(se->subs, aClientid, aTopic, (char*)MULTI_LEVEL_WILDCARD);
}


int subsClientIDCompare(void* a, void* b)
{
	Subscriptions* s = (Subscriptions*)a;
	return strcmp(s->clientName, (char*)b) == 0;
}

char* SubscriptionEngines_mostSpecific(char* topicA, char* topicB)
{
	char* rc = topicB;
	int t1 = strcspn(topicA, "#+"); 
	int t2 = strcspn(topicB, "#+");
	if ((t1 == t2 && topicB[t1] == '+') || t2 > t1)
		rc = topicA;
	return rc;
}

	
List* SubscriptionEngines_getSubscribers1(List* sl, char* aTopic, char* clientID)
{
	List* rc = ListInitialize(); /* list of subscription structures */
	ListElement* current = NULL;

	while (ListNextElement(sl, &current))
	{
		Subscriptions* s = current->content;
		#if !defined(NO_DEBUG)
			Log(LOG_DEBUG, Messages_get(119), s->clientName, s->qos, s->topicName);
		#endif
		if (Topics_matches(s->topicName, aTopic) &&
			((s->noLocal == 0) || (strcmp(s->clientName, clientID) != 0)))
		{
			rc->current = NULL;
			if (ListFindItem(rc, s->clientName, subsClientIDCompare))
			{
				//int t1, t2;
				Subscriptions* rcs = rc->current->content;
				/* determine which QoS to use */
				if (SubscriptionEngines_mostSpecific(rcs->topicName, s->topicName) == rcs->topicName)
				//t1 = strcspn(rcs->topicName, "#+"); 
				//t2 = strcspn(s->topicName, "#+");
				//if ((t1 == t2 && s->topicName[t1] == '+') || t2 > t1)
					rcs->qos = s->qos;
			}
			else
			{
				Subscriptions* rcs = malloc(sizeof(Subscriptions));
				#if !defined(NO_DEBUG)
					Log(LOG_DEBUG, Messages_get(120), s->clientName);
				#endif
				rcs->clientName = s->clientName;
				rcs->qos = s->qos;
				rcs->topicName = s->topicName;
				ListAppend(rc, rcs, sizeof(Subscriptions));
			}
		}
	}
	return rc;
}


List* SubscriptionEngines_getSubscribers(SubscriptionEngines* se, char* aTopic, char* clientID)
{
	if (strncmp(aTopic, sysprefix, strlen(sysprefix)) == 0)
		return SubscriptionEngines_getSubscribers1(se->system.subs, aTopic, clientID);
	else
		return SubscriptionEngines_getSubscribers1(se->subs, aTopic, clientID);
}


void SubscriptionEngines_setRetained1(List* rl, char* topicName, int qos, char* payload, unsigned int payloadlen)
{
	RetainedPublications* found = NULL;
	if (rl->current != NULL)
	{
		RetainedPublications* r = rl->current->content;
		if (strcmp(r->topicName, topicName) == 0)
			found = r;
	}
	if (found == NULL)
	{
		ListElement* current = NULL;
		while (ListNextElement(rl, &current))
		{
			RetainedPublications* r = current->content;
			if (strcmp(r->topicName, topicName) == 0)
			{
				found = r;
				break;
			}
		}
	}
	if (payloadlen == 0)
	{
		if (found != NULL)
		{
			/* remove current retained publication */
			free(found->topicName);
			free(found->payload);
			ListRemove(rl, found);
		}
		return;
	}
	if (found == NULL)
	{
		found = malloc(sizeof(RetainedPublications));
		ListAppend(rl, found, sizeof(RetainedPublications));
		found->topicName = NULL;
		found->payload = NULL;
	}
	if (found->topicName != NULL)
	{
		if (strlen(topicName) != strlen(found->topicName))
			found->topicName = realloc(found->topicName, strlen(topicName)+1);
	}
	else
		found->topicName = malloc(strlen(topicName)+1);
	strcpy(found->topicName, topicName);
	found->qos = qos;
	if (found->payload != NULL)
	{
		if (payloadlen != found->payloadlen)
			found->payload = realloc(found->payload, payloadlen);
	}
	else
		found->payload = malloc(payloadlen);
	found->payloadlen = payloadlen;
	memcpy(found->payload, payload, payloadlen);
}


void SubscriptionEngines_setRetained(SubscriptionEngines* se, char* topicName, int qos, char* payload, unsigned int payloadlen)
{
	if (strncmp(topicName, sysprefix, strlen(sysprefix)) == 0)
		SubscriptionEngines_setRetained1(se->system.retaineds, topicName, qos, payload, payloadlen);
	else
		SubscriptionEngines_setRetained1(se->retaineds, topicName, qos, payload, payloadlen);
}


List* SubscriptionEngines_getRetained1(List* retaineds, char* topicName)
{
	List* rc = ListInitialize(); /* list of RetainedPublication structures */
	ListElement* current = NULL;

	while (ListNextElement(retaineds, &current))
	{
		RetainedPublications* r = current->content;
		#if !defined(NO_DEBUG)
			Log(LOG_DEBUG, Messages_get(121), r->topicName, topicName);
		#endif
		if (Topics_matches(topicName, r->topicName))
		{
			#if !defined(NO_DEBUG)
				Log(LOG_DEBUG, Messages_get(122), r->topicName, topicName);
			#endif
			ListAppend(rc, r, sizeof(RetainedPublications));
		}
	}
	return rc;
}


List* SubscriptionEngines_getRetained(SubscriptionEngines* se, char* topicName)
{
	if (strncmp(topicName, sysprefix, strlen(sysprefix)) == 0)
		return SubscriptionEngines_getRetained1(se->system.retaineds, topicName);
	else
		return SubscriptionEngines_getRetained1(se->retaineds, topicName);
}


#if defined(SUBSENGINE_TEST)

void SubscriptionEngines_List(SubscriptionEngines* se)
{
	printf("Subscriptions:\n");
	se->subs->current = NULL;
	while (ListNextElement(se->subs))
	{
		Subscriptions* s = (Subscriptions*)(se->subs->current->content);
		printf("Subscription: %s %s %d\n", s->clientName, s->topicName, s->qos);
	}
	printf("End subscriptions\n\n");
}

char *newTopic(char* str)
{
	char* n = malloc(strlen(str)+1);
	strcpy(n, str);
	return n;
}

int main(int argc, char *argv[])
{
	char* aClientid = "test_client";
	int i, *ip, *todelete;

	Log_setlevel(LOG_DEBUG);

	SubscriptionEngines* se = SubscriptionEngines_initialize();
	
	SubscriptionEngines_subscribe(se, aClientid, newTopic("aaa"), 0);
	SubscriptionEngines_subscribe(se, aClientid, newTopic("#"), 1);
	SubscriptionEngines_List(se);

	SubscriptionEngines_subscribe(se, aClientid, newTopic("aaa"), 2);
	SubscriptionEngines_List(se);

	printf("Unsubscribing all\n");
	SubscriptionEngines_unsubscribe(se, aClientid, "#");
	SubscriptionEngines_List(se);

	SubscriptionEngines_free(se);

	HeapScan();
}

#endif


